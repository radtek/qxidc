#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CDir           Dir;
CFILELIST      FILELIST;

// 返回值：0-成功；1-入库参数错；2-待入库的文件的时间不正确；3-操作数据库表失败
int _wfiletodb();

int main(int argc,char *argv[])
{
  if (argc != 6)
  {
    printf("\n");
    printf("Using:./wfiletodb logfilename connstr stdfilepath bakfilepath errfilepath\n");

    printf("Example:/htidc/htidc/bin/procctl 5 /htidc/htidc/bin/wfiletodb /log/szqx/wfiletodb_stdsz.log szidc/pwdidc@SZQX_10.153.98.13 /qxdata/szqx/wfile/stdsz /qxdata/szqx/wfile/stdbak /qxdata/szqx/wfile/stderr\n\n");
 
    printf("二进制文件入库的主程序，负责把wfilestdpath及其子目录下的XML文件入库。\n");
    printf("wfilestdpath目录下的文件入库后，被转移到wfilestdpathath目录，由deletefiles定时清理。\n");
    printf("如果XML文件入库时发生了错误，发生错误的文件就会转移到wfilestdpathath目录下。\n");
    printf("入库发生错误时，会产生告警日志，系统管理员应该定期查看wfilestdpathath目录下有没有错误文件。\n\n\n");
 
    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("wfiletodb");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"wfiletodb",300);

  FILELIST.BindConnLog(&conn,&logfile);

  int  iToDBRet=0;

  char strLocalTime[20]; // 当前时间点5秒之前的时间

  Dir.SetDateFMT("yyyymmddhh24miss");

  // 打开标准格式文件目录
  if (Dir.OpenDir(argv[3],TRUE) == FALSE)
  {
    logfile.Write("Dir.OpenDir %s failed.\n",argv[3]); CallQuit(-1);
  }

  // 逐行获取每个文件并入库
  while (Dir.ReadDir() != 0)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 如果文件的时间在当前时间的前5秒之内，就暂时不入库，这么做的目的是为了保证数据文件的完整性。
    LocalTime(strLocalTime,"yyyymmddhh24miss",0-5);
    if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

    if (conn.state == conn.not_connected)
    {
      // 把与连接数据库的代码放在这里的目的是为了确保当有文件需要入库时才连接数据库
      if (conn.connecttodb(argv[2]) != 0)
      {
        logfile.Write("conn.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
      }

      // 载入文件种类参数
      if (FILELIST.LoadFileCFG() != 0) CallQuit(-1);
    }

    char strSTDBAKFileName[301],strSTDERRFileName[301];
    memset(strSTDBAKFileName,0,sizeof(strSTDBAKFileName));
    memset(strSTDERRFileName,0,sizeof(strSTDERRFileName));
    snprintf(strSTDBAKFileName,300,"%s/%s",argv[4],Dir.m_FileName);
    snprintf(strSTDERRFileName,300,"%s/%s",argv[5],Dir.m_FileName);

    // 开始处理每个文件
    logfile.Write("Process file %s...",Dir.m_FileName);

    // 返回值：0-成功；1-入库参数错；2-待入库的文件的时间不正确；3-操作数据库表失败
    iToDBRet=_wfiletodb();
  
    if (iToDBRet == 0)
    {
      RENAME(Dir.m_FullFileName,strSTDBAKFileName); logfile.WriteEx("ok.\n"); conn.commitwork(); continue;
    }

    conn.rollbackwork();

    // 返回值：0-成功；1-入库参数错；2-待入库的文件的时间不正确；3-操作数据库表失败
    if (iToDBRet == 1)
    {
      RENAME(Dir.m_FullFileName,strSTDERRFileName); logfile.WriteEx("failed,invalid parameter.\n"); continue;
    }

    // 返回值：0-成功；1-入库参数错；2-待入库的文件的时间不正确；3-操作数据库表失败
    if (iToDBRet == 2)
    {
      RENAME(Dir.m_FullFileName,strSTDERRFileName); logfile.WriteEx("failed.\n");continue;
    }

    // 其它情况，文件不删除，程序也不退出。
    /*
    // 返回值：0-成功；1-入库参数错；2-待入库的文件的时间不正确；3（其它）-操作数据库表失败
    RENAME(Dir.m_FullFileName,strSTDERRFileName); 

    logfile.WriteEx("failed!,database error.\n"); 

    CallQuit(-1);
    */
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("wfiletodb exit.\n");

  exit(0);
}

// 返回值：0-成功；1-入库参数错；2-待入库的文件的时间不正确；3-操作数据库表失败。
int _wfiletodb()
{
  // 文件名
  strcpy(FILELIST.m_filename,Dir.m_FileName);
  strcpy(FILELIST.m_fullfilename,Dir.m_FullFileName);

  // 根据文件名和文件类型获取气象文件类型参数
  if (FILELIST.GETFILECFG() == FALSE) { logfile.WriteEx("FILELIST.GETFILECFG "); return 1; }

  if (strlen(FILELIST.m_addatetime)!=0)
  {
    DeleteRChar(FILELIST.m_addatetime,';'); // 删除SQL中的分号

    // 有解析数据时间的方法
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare(FILELIST.m_addatetime);
    stmt.bindin(1,FILELIST.m_filename,100);
    stmt.bindout(1,FILELIST.m_ddatetime,14);
    stmt.execute();
    stmt.next();
  }
  else
  {
    // 文件名中包括了完整的时间的情况
    char *posdtime=0;

    // 这里是为了兼容N年前的文件入库
    if ( (posdtime=strstr(FILELIST.m_filename,"_20")) != 0 ) 
    {
      strncpy(FILELIST.m_ddatetime,posdtime+1,14);
    }
    else
    {
      strcpy(FILELIST.m_ddatetime,Dir.m_ModifyTime);
    }
  }

  // 判断数据时间是否合法，即是否在dmintime日到当前时间之后的24个小时
  if (CheckDDateTime(FILELIST.m_ddatetime,FILELIST.m_dmintime) == FALSE) { logfile.WriteEx("invalid date "); return 2; }

  // 根据数据类别和文件名，查询表中是否已存在该文件
  if (FILELIST.FindFExist() != 0) { logfile.WriteEx("FILELIST.CheckIfExist "); return 3; }

  FILELIST.m_IsInsertFile=FALSE;

  if (FILELIST.FindFExistNext() != 0)
  {
    // 新文件，入库
    logfile.WriteEx("insert %s ",FILELIST.m_tname); 

    return FILELIST.InsertFileToDBEx();
  }
  else
  {
    // 文件已存在，判断是否需要更新和更新的时限
    if ( (FILELIST.m_upttype != 1) || ((FILELIST.m_timeexist > FILELIST.m_upttlimit) && (FILELIST.m_upttlimit != 0)) ) 
    {
      logfile.WriteEx("no changed "); return 0;
    }

    // 更新已存在的文件
    logfile.WriteEx("update %s ",FILELIST.m_tname); 

    return FILELIST.UpdateFileToDBEx();
  }

  return 0;
}


