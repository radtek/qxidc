#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;

CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 7)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/backupdbuser hostname username/password@tnsname backuppath timeout starttime rows\n");  
 
    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/backupdbuser 59.33.252.244 scott/tiger /tmp/htidc/bak 10 Y\n\n");

    printf("此程序采用exp备份指定数据库的用户的全部对象，包括了表、视图、触发器和序列等。\n");
    printf("hostname指需要备份的服务器的名称，一般用系统名或IP，备份结果的文件为hostname_yyyymmdd.*.gz。\n");
    printf("username/password@tnsname连接需要备份的数据库的用户名和密码。\n");
    printf("backuppath备份后的dmp文件和log文件存放的目录。\n");
    printf("timeout指执行备份指令的超时时间，单位：分钟。\n");
    printf("starttime指程序每天启动的时间，只支持一个时间，取值从00,01,02,03,04......22,23。\n");
    printf("rows是否备份出数据，取值Y或N。\n");
    printf("注意，本程序一定要3600秒启动一次，调用操作系统的exp命令，程序的日志文件为/tmp/htidc/log/backupdbuser_username.log。\n\n\n");
 
    return -1;
  }

  char strPUserName[201];   // 用户名和密码
  char strUserName[51];     // 临时存放用户名
  char strBackupPath[301];  // 备份后的dmp文件和log文件存放的目录
  char strLocalTime[21];    // 系统时间
  char strFileName[201];    // 存放备份文件的文件名
  char strLogName[201];     // 存放备份日志的日志文件
  char strFullLogName[201]; // 存放备份日志的日志全路径
  char strExp[501];         // exp语句
  char strGzip[501];        // gzip压缩语句
  int  iTimeOut=0;          // 超时时间秒
  char *p=0;

  memset(strPUserName,0,sizeof(strPUserName));
  memset(strFileName,0,sizeof(strFileName));
  memset(strFullLogName,0,sizeof(strFullLogName));
  memset(strGzip,0,sizeof(strGzip));

  strcpy(strPUserName,argv[2]);
  strcpy(strBackupPath,argv[3]);

  // 把超时的时间单位转为秒
  iTimeOut=atoi(argv[4])*60;

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);  

  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  if (strncmp(strLocalTime+8,argv[5],2) != 0) return 0;

  // 获取系统时间
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd");

  // 获取存放的文件名和日志文件名
  memset(strUserName,0,sizeof(strUserName));
  p=strstr(strPUserName,"/");
  strncpy(strUserName,strPUserName,strlen(strPUserName)-strlen(p));
  sprintf(strFileName,"%s_%s_%s.dmp",argv[1],strUserName,strLocalTime);
  sprintf(strLogName,"%s_%s_%s.log",argv[1],strUserName,strLocalTime);
  
  // 获取日志文件全路径
  sprintf(strFullLogName,"%s/%s",strBackupPath,strLogName);

  // 拼凑exp执行语句
  sprintf(strExp,"exp %s file=%s/%s log=%s rows=%s 1>/dev/null 2>/dev/null",strPUserName,strBackupPath,strFileName,strFullLogName,argv[6]);

  // 打开日志文件
  char strLogfile[201];   
  memset(strLogfile,0,sizeof(strLogfile)); 
  sprintf(strLogfile,"/tmp/htidc/log/backupdbuser_%s.log",strUserName);
  if (logfile.Open(strLogfile,"a+") == FALSE) 
  {
    printf("logfile.Open backupdbuser_%s.log filed",strUserName); CallQuit (-1);
  }

  logfile.SetAlarmOpt("backupdbuser");

  // 注意，程序超时是iTimeOut秒
  ProgramActive.SetProgramInfo(&logfile,"backupdbuser",iTimeOut);

  // 开始备份
  logfile.Write("backup %s/%s ...",strBackupPath,strFileName);

  // 执行备份exp语句
  system(strExp);

  // 打开备份的日志文件
  if(CheckFileSTS(strFullLogName,"没有出现警告。") == FALSE)
  {
    if(CheckFileSTS(strFullLogName,"successfully without warnings.") == FALSE)
    {
      logfile.WriteEx("failed.\n"); logfile.Write("%s\n",strExp); CallQuit (-1); 
    }
  }

  char strDmpFileName[301];
  memset(strDmpFileName,0,sizeof(strDmpFileName));
  snprintf(strDmpFileName,300,"%s/%s",strBackupPath,strFileName);

  // 执行压缩命令
  sprintf(strGzip,"/usr/bin/gzip -c %s > %s.tmp 2>/dev/null",strDmpFileName,strDmpFileName);

  system(strGzip);

  REMOVE(strDmpFileName);
  
  strncat(strDmpFileName,".tmp",4);

  char strDmpFileNameGZ[301];
  memset(strDmpFileNameGZ,0,sizeof(strDmpFileNameGZ));
  snprintf(strDmpFileNameGZ,300,"%s/%s.gz",strBackupPath,strFileName);

  // 把文件改名
  if (RENAME(strDmpFileName,strDmpFileNameGZ) == FALSE)
  {
    logfile.Write("RENAME %s to %s failed.\n",strDmpFileName,strDmpFileNameGZ); return -1;
  }

  // 完成导出压缩
  logfile.WriteEx("ok.\n"); 

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
 
  logfile.Write("backupdbuser exit.\n");
 
  exit(0);
}
