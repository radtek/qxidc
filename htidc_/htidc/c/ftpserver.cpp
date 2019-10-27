// 过几天运行以下SQL，看看哪些FTP没有采集到文件，没采集到文件的FTP要检查一下。
//select * from t_fileftptask where ftptype=1 and rsts=1 and taskid not in (select taskid from t_fileftplist where ddatetime>sysdate-2);

#include "idcapp.h"
#include "_ftp.h"

// 当ftp超时的时候，调用该函数
void FtpLoginTimeOut(int sigid);
void CallQuit(int sig);

char strAPPConnStr[201]; 
char strLogPath[201]; 
char strTmpPath[201]; 

ftplib         ftp;
FILE          *listfp;
connection     conn;
CLogFile       logfile;
CFILEFTPTASK   FTASK;
CFILEFTPLIST   FLIST;
CProgramActive ProgramActive;
CDir           Dir;
char           strRemoteIP[21];

// 执行文件传输任务
BOOL _ExecFtpTask();

// 建立与对方服务器的连接，并登录
BOOL _FtpLogin();

// 获取对方服务器的文件清单
BOOL GetRemoteFileList();

// 删除对方目录中不匹配文件
BOOL DeleteRemoteFile();

// 从对方服务器get文件
BOOL FtpGetFile(char *in_FileName);

// 把文件发送到对方服务器目录
BOOL FtpPutFile();

// 判断文件的大小是否合法
BOOL JudgeFileSizeStr(UINT uFileSize,char *strFileSizeStr);

int main(int argc,char *argv[])
{
  if (argc != 5)
  {
    printf("\n");
    printf("Using:./ftpserver username/password@tnsname tmppath logpath remoteip\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/ftpserver sqxt/pwdidc@SZQX_10.148.124.85 /htidc/sqxt/tmp /htidc/sqxt/log 172.22.1.15\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/ftpserver sqxt/pwdidc@SZQX_10.148.124.85 /htidc/sqxt/tmp /htidc/sqxt/log 172.22.1.17\n\n");
 
    printf("本程序是数据中心的公共功能模块，文件采集、分发和目录维护的服务程序，目前只支持FTP协议。\n");
    printf("本程序读取username/password@tnsname数据库中T_FILEFTPTASK表的任务参数并执行它。\n");
    printf("本程序的临时文件存放在tmppath目录中，日志文件存放在logpath中，remoteip是远程服务器的IP。\n");
    printf("每一个类型的任务由一个单独的ftpserver程序执行，remoteip参数指定了FTP远程服务器的IP。\n");
    printf("为了提高效率，把T_FILEFTPTASK表中全部的任务按远程IP分为多个任务类型，互不干扰。\n");
    printf("如果需要执行多个remoteip的FTP任务，就必须启动多个ftpserver程序。\n");
    printf("remoteip可以是远程服务器的IP，也可以是英文域名。\n");
 
    return -1;
  }

  listfp=0;

  memset(strAPPConnStr,0,sizeof(strAPPConnStr));
  memset(strTmpPath,0,sizeof(strTmpPath));
  memset(strLogPath,0,sizeof(strLogPath));

  strcpy(strAPPConnStr,argv[1]);
  strcpy(strTmpPath,argv[2]);
  strcpy(strLogPath,argv[3]);
  strcpy(strRemoteIP,argv[4]);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,200,"%s/ftpserver%s.log",strLogPath,strRemoteIP);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }
  
  //打开告警
  logfile.SetAlarmOpt("ftpserver");

  char strprogname[101];
  memset(strprogname,0,sizeof(strprogname));
  sprintf(strprogname,"ftpserver_%s",strRemoteIP);
  ProgramActive.SetProgramInfo(&logfile,strprogname,500);

  // 连接数据库，此数据库连接用于正常的数据处理，在此程序中，它是自动提交模式
  if (conn.connecttodb(strAPPConnStr,TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strAPPConnStr); CallQuit(-1);
  }

  // 注意，程序超时是300秒，我认为300就足够了，如果不够，列出理由。
  // 注意，程序超时之前是500秒，现在改为300秒，因为上面连数据库失败时可能要很长时间，如果把时间设为300秒，
  // 可能导致告警无法捕获。
  // 当采集DVBS系统数据的时候，对文文件太多，list需要较长的时间，所以暂时用300秒。
  ProgramActive.SetProgramInfo(&logfile,strprogname,300);

  logfile.Write("ftpserver beging.\n");

  FTASK.BindConnLog(&conn,&logfile);
  FLIST.BindConnLog(&conn,&logfile);

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    // 获取全部需要立即执行的文件同步任务
    if (FTASK.LoadFtpTask(strRemoteIP) != 0)
    {
      logfile.Write("FTASK.SelFtpTask failed.\n"); CallQuit(-1);
    }

    // 逐行处理每个文件采集任务
    while (FTASK.LoadFtpTaskNext() == 0)
    {

      // 处理对方目录和待获取文件名中的时间宏变量
      if (FTASK.dstpathmatchbz==1) 
      {
        ProcMatchDTime(FTASK.matchstr); ProcMatchDTime(FTASK.dstpath); 
      }

      /*
      logfile.Write(\
        "task=%s,url=%s:%ld,login=%s,%s,dst=%s,local=%s,filename=%s,OK.\n",\
         FTASK.taskname,FTASK.remoteip,FTASK.port,\
         FTASK.username,FTASK.password,FTASK.dstpath,FTASK.localpath,FTASK.matchstr);
      */

      if (FTASK.ftpcmode==1) ftp.mp_ftphandle->cmode=ftp.ftplib::pasv;

      if (FTASK.ftpcmode==2) ftp.mp_ftphandle->cmode=ftp.ftplib::port;

      FTASK.ftpsts=1;

      // 执行文件传输任务
      _ExecFtpTask();

      // 每次执行完FTP任务之后，关掉句柄
      ftp.CloseHandle();
      
      // ftpsts，1-正常;2-连接失败；3-登录失败。
      FTASK.UptFtpSTS();
    }

    // 获取最近需要执行的任务记录的时间和当前时间间隔的秒数
    FTASK.SelLeastTimeTvl(strRemoteIP); 

    ProgramActive.WriteToFile();

    if (FTASK.m_sleeptvl <  10) FTASK.m_sleeptvl=10;

    if (FTASK.m_sleeptvl > 120) FTASK.m_sleeptvl=120;

    sleep(FTASK.m_sleeptvl);

    ProgramActive.WriteToFile();
  }

  logfile.Write("ftpserver exit.\n");

  return 0;
}

// 当ftp超时的时候，调用该函数，关闭ftp的socket句柄，ftp的应用函数就会出错返回
void FtpLoginTimeOut(int sigid)
{
  FTASK.ftpsts = 2;

  FTASK.UptFtpSTS();

  logfile.Write("连接服务器%s（%s/%s）超时.\n",FTASK.remoteip,FTASK.username,FTASK.password);

  ftp.m_timeout=1;

  ftp.CloseHandle();

  CallQuit(-1);
}

// 执行文件传输任务
BOOL _ExecFtpTask()
{
  // 连接服务器
  if (_FtpLogin() == FALSE)
  {
    logfile.Write("服务程序：ftpserver，登录服务器%s（%s/%s）失败!",FTASK.remoteip,FTASK.username,FTASK.password);
    return FALSE;
  }

  ProgramActive.WriteToFile();

  int iFileCount=0;
  //logfile.Write("Login OK.\n");

  UINT ii;

  // 采集
  if (FTASK.ftptype == 1)
  {
    FLIST.m_taskid=FTASK.taskid;

    // 从对方服务器获取文件
    // 获取对方服务器的文件清单
    if (GetRemoteFileList() == FALSE)
    {
      logfile.Write("GetRemoteFileList Failed,%s.\n",ftp.mp_ftphandle->response); return FALSE;
    }

    // 逐行获取每一条记录，从对方服务器获取文件
    for (ii=0;ii<FLIST.m_vRemoteFile.size();ii++)
    {
      memcpy(&FLIST.m_stRemoteFile,&FLIST.m_vRemoteFile[ii],sizeof(FLIST.m_stRemoteFile));

      // 从对方服务器Ftp文件，如果获取成功，就把文件信息插入T_FILEFTPLIST表中
      if (FtpGetFile(FLIST.m_stRemoteFile.filename) == TRUE)
      {
        if (FLIST.InsertList() != 0)
        {
          logfile.Write("FLIST.InsertList failed.\n"); return FALSE;
        }
      }

      if (iFileCount++>10) { iFileCount=0; ProgramActive.WriteToFile(); }
    }
  }

  // 分发

  if (FTASK.ftptype == 2)
  {
    // 向对方服务器发布文件
    if (Dir.OpenDir(FTASK.localpath) == FALSE) 
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",FTASK.localpath); return FALSE;
    }

    // 进入发送目录
    chdir(FTASK.localpath);

    // 进入目标目录
    if (ftp.Chdir(FTASK.dstpath) == FALSE)
    {
      // 如果进入目标目录失败，有可能是目标目录尚未创建，这种情况不一定是错误，所以不必返回失败。
      // logfile.Write("ftp.Chdir(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
      return TRUE;
    }

    while (Dir.ReadDir() == TRUE)
    {
      // 判断文件名是否和MatchStr匹配，如果不匹配，不分发该文件
      if (MatchFileName(Dir.m_FileName,FTASK.matchstr) == FALSE) continue;

      // 如果待分发的文件名匹配tmp或匹配了*topwalk*结束，就跳过这个文件
      if (MatchFileName(Dir.m_FileName,"*TMP,*TOPWALK*") == TRUE) continue;

      // 如果文件的时间在当前时间的前10秒之内，就暂时不发送
      char strLocalTime[21]; 
      LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-10); // 当前时间点10秒之前的时间
      if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

      // 向对方服务器Ftp文件
      FtpPutFile();
    }
  }

  // 维护
  if (FTASK.ftptype == 3)
  {
    // 删除对方目录中匹配文件
    if (DeleteRemoteFile() == FALSE)
    {
      logfile.Write("DeleteRemoteFile Failed,%s.\n",ftp.mp_ftphandle->response); return FALSE;
    }
  }

  return TRUE;
}

// 建立与对方服务器的连接，并登录
BOOL _FtpLogin()
{
  char strURL[51];
  memset(strURL,0,sizeof(strURL));

  snprintf(strURL,50,"%s:%ld",FTASK.remoteip,FTASK.port);

  signal(SIGALRM,FtpLoginTimeOut); alarm(CONNECTTIMEOUT);

  BOOL bConnect=ftp.Connect(strURL);

  signal(SIGALRM,SIG_IGN); 

  if (bConnect == FALSE)
  {
    FTASK.ftpsts=2; 
    logfile.Write("ftp.Connect(%s) Failed,%s.\n",strURL,ftp.mp_ftphandle->response); 
    return FALSE;
  }

  if (ftp.Login(FTASK.username,FTASK.password) == FALSE)
  {
    logfile.Write("ftp.Login(%s,%s/%s) Failed,%s.\n",strURL,FTASK.username,FTASK.password,ftp.mp_ftphandle->response); 
    FTASK.ftpsts=3; 
    return FALSE;
  } 

  return TRUE;
}

// 获取对方服务器的文件清单
BOOL GetRemoteFileList()
{
  char strListFileName[201];

  memset(strListFileName,0,sizeof(strListFileName));
  snprintf(strListFileName,200,"%s/ftp_%s.list",strTmpPath,strRemoteIP);

  // 清空对方服务器待取文件列表
  FLIST.m_vRemoteFile.clear();

  // 进入目标目录
  if (ftp.Chdir(FTASK.dstpath) == FALSE)
  {
    // 如果进入目标目录失败，有可能是目标目录尚未创建，这种情况不一定是错误，所以不必返回失败。
    // logfile.Write("ftp.Chdir(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
    return TRUE;
  }

  // 获取对方服务器的文件清单
  if (ftp.Nlst(strListFileName,"*") == FALSE)
  {
    logfile.Write("ftp.Nlst(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
  }

  ProgramActive.WriteToFile();

  //logfile.Write("Nlst OK.\n");

  // 判断清单文件是否为空
  if (FileSize(strListFileName) == 0) return TRUE;

  // 打开对方服务器的文件清单文件
  listfp=0;
  if ( (listfp=FOPEN(strListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN(%s) failed.\n",strListFileName); return FALSE;
  }

  // 从对方服务器的文件清单文件中取出每行，判断是否要重获取该文件，如果要，就放到待取文件列表中
  char strLine[201];
  vector<string> vFileName;

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
   
    // 从文件中获取一行
    if (FGETS(strLine,100,listfp) == FALSE) break;

    // 如果对方的文件名匹配tmp或匹配了*topwalk*结束，就跳过这个文件
    if (MatchFileName(strLine,"*TMP,*TOPWALK*") == TRUE) continue;

    // 有些FTP会在文件名的前页加“./”，所以要去掉它。
    UpdateStr(strLine,"./","");

    vFileName.push_back(strLine);
  }

  fclose(listfp); listfp=0;

  // 把文件名排序
  sort(vFileName.begin(),vFileName.end());

  for (UINT uPOS=0;uPOS<vFileName.size();uPOS++)
  {
    // 判断文件名是否和MatchStr匹配，如果不匹配，不获取该文件
    if (MatchFileName(vFileName[uPOS].c_str(),FTASK.matchstr) == FALSE) continue;

    ProgramActive.WriteToFile();

    memset(&FLIST.m_stRemoteFile,0,sizeof(FLIST.m_stRemoteFile));

    strcpy(FLIST.m_stRemoteFile.filename,vFileName[uPOS].c_str());

    // 从已获取的文件清单中获取该文件的大小和修改日期
    if (FLIST.FindFList() != 0)
    {
      logfile.Write("FLIST.FindFList failed.\n"); return FALSE;
    }

    // 如果文件不在已获取的文件清单中获取文件中，表示是一个新文件。
    if (FLIST.FindFListNext() != 0)
    {
      FLIST.m_vRemoteFile.push_back(FLIST.m_stRemoteFile); continue;
    }

    // 如果不是新文件，判断是否需要检查文件的时间，如果文件的时间和以前的不同，就把该文件放入列表中
    if (FTASK.cmodtime == 1)
    {
      char strModTime[21]; memset(strModTime,0,sizeof(strModTime));

      if (ftp.ModDate(vFileName[uPOS].c_str(),strModTime,14) == FALSE) continue;

      if (strncmp(strModTime,FLIST.m_stRemoteFile.modtime,14) != 0)
      {
        // 一定要把filesize清空，因为这个filesize是从表中取出来的，并不是该文件当前在
        // 对方服务器上的真实filesize
        FLIST.m_stRemoteFile.filesize=0;
        strncpy(FLIST.m_stRemoteFile.modtime,strModTime,14);
        FLIST.m_vRemoteFile.push_back(FLIST.m_stRemoteFile); 
        continue;
      }
    }

    // 如果不是新文件，判断是否需要检查文件的大小，如果文件的大小和以前的不同，就把该文件放入列表中
    if (FTASK.cfilesize == 1)
    {
      int iFileSize=0;

      if (ftp.Size(vFileName[uPOS].c_str(),&iFileSize) == FALSE) continue;

      if (iFileSize != FLIST.m_stRemoteFile.filesize)
      {
        // 一定要把modtime清空，因为这个modtime是从表中取出来的，并不是该文件当前在
        // 对方服务器上的真实modtime
        memset(FLIST.m_stRemoteFile.modtime,0,sizeof(FLIST.m_stRemoteFile.modtime));
        FLIST.m_stRemoteFile.filesize = iFileSize;
        FLIST.m_vRemoteFile.push_back(FLIST.m_stRemoteFile); 
        continue;
      }
    }
  }

  ProgramActive.WriteToFile();

  //logfile.Write("Check OK.\n");

  return TRUE;
}

// 从对方服务器get文件
BOOL FtpGetFile(char *in_FileName)
{
  ProgramActive.WriteToFile();

  // 获取文件的时间
  if (strlen(FLIST.m_stRemoteFile.modtime) == 0)
  {
    if (ftp.ModDate(in_FileName,FLIST.m_stRemoteFile.modtime,14)==FALSE) return FALSE;
  }

  // 获取文件的大小
  if (FLIST.m_stRemoteFile.filesize == 0)
  {
    if (ftp.Size(in_FileName,&FLIST.m_stRemoteFile.filesize)==FALSE) return FALSE;
  }

  // 如果修改时间小于有效时间，直接返回成功，不获取文件
  if (strncmp(FLIST.m_stRemoteFile.modtime,FTASK.m_validdays,14) < 0) return TRUE;

  // 如果文件的大小是0或不在期待的大小范围中
  if ( (FLIST.m_stRemoteFile.filesize==0) || (JudgeFileSizeStr(FLIST.m_stRemoteFile.filesize,FTASK.filesizestr) == FALSE) ) return FALSE;

  logfile.Write("transfer(get) %s ...",in_FileName);

  char strDstFileName[201],strDstFileName1[201],strTmpFileName[201],strLocalFileName[201];

  memset(strDstFileName,0,sizeof(strDstFileName));
  memset(strDstFileName1,0,sizeof(strDstFileName1));
  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strLocalFileName,0,sizeof(strLocalFileName));

  // 生成远程文件名，本地临时文件名和本地文件名
  snprintf(strDstFileName,200,"%s",in_FileName);
  snprintf(strDstFileName1,200,"%s/%s",FTASK.renamepath,in_FileName);
  snprintf(strTmpFileName,200,"%s/file_%s.tmp",strTmpPath,strRemoteIP);
  snprintf(strLocalFileName,200,"%s/%s",FTASK.localpath,FLIST.m_stRemoteFile.filename);

  // 获取文件
  if (ftp.Get(strTmpFileName,strDstFileName,ftplib::image) == FALSE)
  {
    logfile.WriteEx("Failed,ftp.Get,%s\n",ftp.mp_ftphandle->response); return FALSE;
  }

  // 获取文件之后再判断文件是否改变
  char strModTime[21];  // 采集之后文件的时间
  int  iFileSize=0;     // 采集之后文件的大小

  memset(strModTime,0,sizeof(strModTime));

  // 再次获取文件的时间
  if (ftp.ModDate(in_FileName,strModTime,14)==FALSE)
  {
    logfile.WriteEx("Failed,ftp.ModDate,%s.\n",ftp.mp_ftphandle->response); return FALSE;
  }

  // 再次获取文件的大小
  if (ftp.Size(in_FileName,&iFileSize)==FALSE)
  {
    logfile.WriteEx("Failed,ftp.Size,%s\n",ftp.mp_ftphandle->response); return FALSE;
  }

  // 比较获取文件前后的文件大小和修改时间有没有改变
  if ( (strncmp(FLIST.m_stRemoteFile.modtime,strModTime,14) != 0) || 
       (FLIST.m_stRemoteFile.filesize != iFileSize) )
  {
    logfile.WriteEx("Failed,file changed during transfer(%s,%d,%s,%d).\n",FLIST.m_stRemoteFile.modtime,FLIST.m_stRemoteFile.filesize,strModTime,iFileSize); return FALSE;
  }

  // 如果需要删除对方目录上的文件。
  if (FTASK.deletebz == 1) ftp.Delete(strDstFileName);

  // 如果需要把对方目录上的文件移到其它的目录备份
  if (FTASK.renamebz == 1) ftp.Rename(strDstFileName,strDstFileName1);

  if (RENAME(strTmpFileName,strLocalFileName) == FALSE) 
  { 
    logfile.WriteEx("RENAME failed.\n"); return FALSE; 
  }

  logfile.WriteEx("ok,size=%d.\n",iFileSize); 

  return TRUE; 
}

// 把文件发送到对方服务器目录
BOOL FtpPutFile()
{
  ProgramActive.WriteToFile();

  char strFileName[201];
  char strFileNameTmp[201];
  char strFileNameRename[201];

  memset(strFileName,0,sizeof(strFileName));
  memset(strFileNameTmp,0,sizeof(strFileNameTmp));
  memset(strFileNameRename,0,sizeof(strFileNameRename));

  snprintf(strFileName,200,"%s",Dir.m_FileName);
  snprintf(strFileNameTmp,200,"%s.tmp",Dir.m_FileName);
  snprintf(strFileNameRename,200,"%s/%s",FTASK.renamepath,Dir.m_FileName);

  logfile.Write("transfer(put) %s ...",strFileName);

  // 检查本地目录该文件的大小
  int iLocalFileSize=FileSize(strFileName);

  // 如果本地目录该文件的大小是0，就返回，不发送该文件
  if (iLocalFileSize <= 0) { logfile.WriteEx("failed,filesize=0\n"); REMOVE(strFileName); return FALSE; }

  // 发送文件
  if (ftp.Put(strFileName,strFileNameTmp,ftplib::image) == FALSE)
  {
    logfile.WriteEx("Failed,%s\n",ftp.mp_ftphandle->response); return FALSE;
  }

  if (FTASK.renamebz == 1) ftp.Rename(strFileNameTmp,strFileNameRename);
  if (FTASK.renamebz == 2) ftp.Rename(strFileNameTmp,strFileName);

  int iRemoteFileSize=0;

  if (FTASK.renamebz == 1) ftp.Size(strFileNameRename,&iRemoteFileSize);
  if (FTASK.renamebz == 2) ftp.Size(strFileName ,&iRemoteFileSize);

  // 比较发送文件前后的文件大小和修改时间有没有改变
  if (iLocalFileSize != iRemoteFileSize)
  {
    logfile.WriteEx("Failed,file changed during transfer(%d,%d).\n",iLocalFileSize,iRemoteFileSize); return FALSE;
  }

  strcpy(FTASK.m_stLocalFile.filename,Dir.m_FileName);
  FTASK.m_stLocalFile.taskid=FTASK.taskid;
  strcpy(FTASK.m_stLocalFile.modtime ,Dir.m_ModifyTime);
  FTASK.m_stLocalFile.filesize=Dir.m_FileSize;

  // 把已发送的文件的信息插入文件分发日志表
  if (FTASK.InsertPutList() != 0)
  {
    logfile.WriteEx("failed,call FTASK.InsertPutList failed.\n"); return FALSE;
  }

  // 删除本地目录的该文件
  if (REMOVE(strFileName) == FALSE) { logfile.WriteEx("failed,REMOVE failed.\n"); return FALSE; }

  logfile.WriteEx("ok.\n"); 

  return TRUE; 
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.CloseHandle();

  if (listfp != 0) { fclose(listfp); listfp=0; }

  logfile.Write("ftpserver exit.\n");

  exit(0);
}

// 判断文件的大小是否合法
BOOL JudgeFileSizeStr(UINT uFileSize,char *strFileSizeStr)
{
  // 如果匹配表达式是*，就直接返回TRUE
  if (strcmp(strFileSizeStr,"*") == 0) return TRUE;

  CCmdStr CmdStr;

  if (strchr(strFileSizeStr,'-') > 0)
  {
    CmdStr.SplitToCmd(strFileSizeStr,"-");
    char strMinSize[21],strMaxSize[21];
    memset(strMinSize,0,sizeof(strMinSize));
    memset(strMaxSize,0,sizeof(strMaxSize));
    UINT uMinSize,uMaxSize;
    uMinSize=uMaxSize=0;
    CmdStr.GetValue(0,&uMinSize);
    CmdStr.GetValue(1,&uMaxSize);
    CmdStr.GetValue(0,strMinSize,15);
    CmdStr.GetValue(1,strMaxSize,15);
    if ( (uFileSize>=uMinSize) && (uFileSize<=uMaxSize) ) return TRUE;
    if ( (uFileSize>=uMinSize) && (strstr(strMaxSize,"*")!=0) ) return TRUE;
    if ( (strstr(strMinSize,"*")!=0) && (uFileSize<=uMaxSize) ) return TRUE;
  }
  else
  {
    CmdStr.SplitToCmd(strFileSizeStr,",");
    UINT uSize;
    for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
    {
      uSize=0;
      CmdStr.GetValue(ii,&uSize);
      if (uFileSize == uSize) return TRUE;
    }
  }
  
  return FALSE;
}

// 删除对方目录中不匹配文件
BOOL DeleteRemoteFile()
{
  char strListFileName[201];

  memset(strListFileName,0,sizeof(strListFileName));
  snprintf(strListFileName,200,"%s/ftp_%s.list",strTmpPath,strRemoteIP);

  // 进入目标目录
  if (ftp.Chdir(FTASK.dstpath) == FALSE)
  {
    // 如果进入目标目录失败，有可能是目标目录尚未创建，这种情况不一定是错误，所以不必返回失败。
    // logfile.Write("ftp.Chdir(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
    return TRUE;
  }

  // 获取对方服务器的文件清单
  if (ftp.Nlst(strListFileName,"*") == FALSE)
  {
    logfile.Write("ftp.Nlst(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
  }

  ProgramActive.WriteToFile();

  // 打开对方服务器的文件清单文件
  listfp=0;
  if ( (listfp=FOPEN(strListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN(%s) failed.\n",strListFileName); return FALSE;
  }

  // 从对方服务器的文件清单文件中取出每行，判断是否要重获取该文件，如果要，就放到待取文件列表中
  char strFileName[201];
  char strModTime[21]; 
  char strFullFileName[301];

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    memset(strFileName,0,sizeof(strFileName));
    memset(strModTime,0,sizeof(strModTime));
    memset(strFullFileName,0,sizeof(strFullFileName));

    // 从文件中获取一行
    if (FGETS(strFileName,100,listfp) == FALSE) break;

    snprintf(strFullFileName,200,"%s/%s",FTASK.dstpath,strFileName);

    if (ftp.ModDate(strFullFileName,strModTime,14) == FALSE) continue;
    
    // 比较对方文件是否在有效时间之内，如果对文件的时间小于有效时间，就删除它。
    if (strncmp(strModTime,FTASK.m_validdays,14) < 0)
    {
      // 判断文件名是否和MatchStr匹配，如果不匹配，不获取该文件
      if (MatchFileName(strFileName,FTASK.matchstr) == FALSE) continue;

      if (ftp.Delete(strFullFileName) == TRUE)
      {
        logfile.Write("delete %s(%s,%s) ok.\n",strFileName,FTASK.m_validdays,strModTime);
      }
      else
      {
        logfile.Write("delete %s(%s,%s) Failed,%s.\n",strFileName,FTASK.m_validdays,strModTime,ftp.mp_ftphandle->response);
      }
    }
  }

  fclose(listfp); listfp=0;

  ProgramActive.WriteToFile();

  return TRUE;
}
