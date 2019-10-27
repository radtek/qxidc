#include "idcapp.h"

void CallQuit(int sig);
BOOL DealWithFile();

CLogFile logfile;
CDir Dir;
CProgramActive ProgramActive;
connection conn;
sqlstatement stmtins,stmtdel;
FILE *fp;
char fieldval[100][2001];
char deleteflag[11];
UINT ufilecount=0;

int main( int argc, char *argv[] )
{
  if ( argc != 5 )
  {
    printf("\nUsage:./instabdata username/password@tnsname filepath sleeptime deleteflag\n\n");

    printf("Example:/htidc/htidc/bin/procctl 20 /htidc/htidc/bin/instabdata esa/esaserver /home/jwtout/send 2 false\n");
    printf("        /htidc/htidc/bin/procctl 20 /htidc/htidc/bin/instabdata esa/esaserver /home/jwtin/recv  2 false\n\n");

    printf("该程序用于将filepath目录下XML文件导入到数据库表中，表名根据XML文件名来确定。\n" );
    printf("程序运行的日志文件名为/tmp/htidc/log/instabdata.log。\n");
    printf("username/password@tnsname 为数据库的连接参数。\n");
    printf("filepath 为待入库的xml文件存放的目录。\n");
    printf("sleeptime 表示每次执行数据导出的时间间隔，如果时间间隔为0，表示该程序执行一次导出后立即退出。\n");
    printf("deleteflag 表示插入数据前是否先删除目的表数据。\n\n\n" );

    return 0;
  }

  char connstr[101];
  char filepath[301];
  int  sleeptime = 0;

  memset(connstr,0,sizeof(connstr));
  memset(filepath,0,sizeof(filepath));
  memset(deleteflag,0,sizeof(deleteflag));

  strncpy(connstr,argv[1],50);
  strncpy(filepath,argv[2],300);
  sleeptime=atoi(argv[3]);
  strncpy(deleteflag,argv[4],10);
  ToUpper(deleteflag);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止此进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal();
  signal(SIGTERM,CallQuit);   // 按ctl+c
  signal(SIGINT,CallQuit);    // kill 或 killall

  if (logfile.Open("/tmp/htidc/log/instabdata.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/instabdata.log) failed.\n"); CallQuit(-1);
  }

  //打开告警
  logfile.SetAlarmOpt("instabdata");

  // 注意，程序超时是80秒
  ProgramActive.SetProgramInfo(&logfile,"instabdata",80);

  if (conn.connecttodb(connstr)!=0)
  {
    logfile.Write("connect to db failed(%s)\n",connstr); CallQuit(-1);
  } 
  
  while ( TRUE )
  {
    if (Dir.OpenDir(filepath) == FALSE )
    {
      logfile.Write("Dir.Open(%s) failed.\n",filepath); CallQuit(-1);
    }
  
    // 注意，程序超时是80秒
    ProgramActive.SetProgramInfo(&logfile,"instabdata",80);
  
    while ( Dir.ReadDir() == TRUE )
    {
      // 写入进程活动信息
      ProgramActive.WriteToFile();
  
      // 文件全名超长
      if (strlen(Dir.m_FullFileName) >= 300) continue;
  
      // 如果文件名不是以.XML结束，就不处理该文件
      if (MatchFileName(Dir.m_FileName,"*_20*.xml") == FALSE) continue;
  
      logfile.Write("process %s...",Dir.m_FileName );
  
      if (DealWithFile() == FALSE ) continue;

      if (REMOVE(Dir.m_FullFileName) == FALSE)
      {
        logfile.WriteEx("failed.\n,REMOVE %s failed.\n",Dir.m_FullFileName);
        conn.rollbackwork();
        continue;
      }

      logfile.WriteEx("ok(%lu).\n",ufilecount);

      conn.commitwork();
    }

    if (sleeptime == 0) break;

    sleep(sleeptime);
  }

  return 0;
}
      
// 返回值：0-成功；1-应用数据定义错；2-打开文件失败或文件状态不正确；3-操作数据库表错误
BOOL DealWithFile()
{
  ufilecount=0;
  CTABFIELD TABFIELD;
  fp=0;
  char tablename[51];  //用于保存表名
  char strBuffer[8001]; //用于保存一行文件数据
  char strtmp[51];  
  char strbind[501];
  char *pos;  //用于指定表名最后字符位置

  memset(tablename,0,sizeof(tablename) );
  memset(strbind,0,sizeof(strbind) );
  memset(strtmp,0,sizeof(strtmp) );

  // 根据文件名得出表名，第一个“_20”前面的就是表名
  pos=strstr(Dir.m_FileName,"_20");

  strncpy(tablename,Dir.m_FileName,pos-Dir.m_FileName);
 
  // 得到表的字段信息
  TABFIELD.GetALLField(&conn, tablename);

  // 字段数为0说明表不存在
  if (TABFIELD.m_vALLFIELD.size() == 0)
  {
    logfile.WriteEx("failed.\ntable %s is not exist.\n",tablename); return FALSE;
  }

  // 打开XML文件
  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0 ) 
  { 
    logfile.WriteEx("failed.\nFOPEN %s failed.\n",Dir.m_FullFileName); return FALSE;
  }

  // 判断文件是否以"</data>"结束
  if (CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    fclose(fp); fp=0; return FALSE; 
  }
      
  // 根据表字段信息，拼出values内部绑定语句
  memset(strbind,0,sizeof(strbind));

  for (UINT ii=0; ii<TABFIELD.m_vALLFIELD.size(); ii++ )
  {
    if (strcmp(TABFIELD.m_vALLFIELD[ii].datatype,"date") == 0 ) 
    {
      if (ii == 0) sprintf(strtmp," to_date(:%lu,'yyyy-mm-dd hh24:mi:ss')",ii+1);
      if (ii >  0) sprintf(strtmp,",to_date(:%lu,'yyyy-mm-dd hh24:mi:ss')",ii+1);
    }
    else 
    {
      if (ii == 0) sprintf(strtmp," :%lu",ii+1);
      if (ii >  0) sprintf(strtmp,",:%lu",ii+1);
    }

    strcat(strbind,strtmp);
  }
       
  //无需自己拼接，TABFIELD有m_allfieldstr成员变量保存全部字段名信息
  stmtins.connect( &conn );
  stmtins.prepare( "insert into %s(%s) values(%s)", tablename,TABFIELD.m_allfieldstr,strbind);

  for (UINT ii=0;ii<TABFIELD.m_vALLFIELD.size();ii++)
  {
    stmtins.bindin(ii+1,fieldval[ii],2000);
  }
  
  if (strcmp(deleteflag,"TRUE") == 0 )
  {
    stmtdel.connect(&conn );
    stmtdel.prepare("delete from %s",tablename);
    if (stmtdel.execute() != 0)
    {
      logfile.WriteEx("failed.\nexecute sql failed.\n%s\n%s\n",stmtdel.m_sql,stmtdel.cda.message); 
      fclose(fp); fp=0; return FALSE; 
    }
  }

  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    memset(strBuffer,0,sizeof(strBuffer));
    memset(fieldval,0,sizeof(fieldval));

    // 读取一行
    if (FGETS(strBuffer,8000,fp,"<endl/>") == FALSE) break;  

    // 行的内容不可能小于5，如果小于5，一般是空行，丢弃它。
    if (strlen(strBuffer) < 5) continue;

    //处理一行数据 
    for (UINT ii=0;ii<TABFIELD.m_vALLFIELD.size();ii++ )
    {
      GetXMLBuffer(strBuffer,TABFIELD.m_vALLFIELD[ii].fieldname,fieldval[ii],2000);
    }

    if (stmtins.execute() != 0)
    {
      // 如果错误信息为1，不打印
      // 即使有其它错误，也要继续处理其它的数据，不能返回，也不能退出。
      if ( stmtins.cda.rc != 1 ) 
      {
        logfile.WriteEx("failed.\nexecute sql failed.\n%s\n%s\n",stmtins.m_sql,stmtins.cda.message); 
      }
    }

    ufilecount++;
  }

  fclose(fp); 

  fp=0; 

  return TRUE;
}

void CallQuit(int sig)
{
  signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  if (fp != 0) { fclose(fp); fp=0; }

  logfile.Write("instabdata exit.\n");

  exit(0);
}
