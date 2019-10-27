#include "idcapp.h"

void CallQuit(int sig);

char strDBLinkName[51]; 
UINT uAPPID;

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./counttable logfilename connstr appid\n");

    printf("Example:/htidc/htidc/bin/procctl 86400 /htidc/htidc/bin/counttable /log/ssqx/counttable_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 86400 /htidc/htidc/bin/counttable /log/ssqx/counttable_2.log qxidc/pwdidc@EJETDB_221.179.6.136 2\n\n");

    printf("此程序用于自动统计数据中心集群中各数据库的表信息。\n");
    printf("inifile中指定了数据中心主数据库的连接参数，日志文件和临时文件的目录等参数。\n");
    printf("appid是待管理的数据库，它必须是集群中的数据库之一。\n");
    printf("如果需要对多个集群数据库进行管理，就必须运行多个counttable程序。\n");
    printf("注意：appid必须在T_DAPPSERVER中存在。\n\n\n");
 
    return -1;
  }

  uAPPID=atoi(argv[3]);

  memset(strDBLinkName,0,sizeof(strDBLinkName));

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("counttable");

  // 注意，连接数据库的超时时间是60秒
  ProgramActive.SetProgramInfo(&logfile,"counttable",60);

  ALLTABLE.BindConnLog(&conn,0,&logfile,&ProgramActive);

  if (conn.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // 注意，程序超时是21600秒
  ProgramActive.SetProgramInfo(&logfile,"counttable",21600);

  // 从数据中心的T_DAPPSERVER中提取待管理的服务器的数据库连接参数
  if (findbypk(&conn,"T_DAPPSERVER","appid","dblinkname",uAPPID,strDBLinkName,50) != 0)
  {
    logfile.Write("Call findbypk failed\n"); CallQuit(-1);
  }

  logfile.Write("DBLinkName=%s\n",strDBLinkName);

  // 设置集群服务器的ID
  ALLTABLE.m_appid=uAPPID;

  // 从T_APPTABLE表中载入某集群服务器的全部记录
  if (ALLTABLE.LoadForCount() != 0)
  {
    logfile.Write("ALLTABLE.LoadForCount failed.\n"); CallQuit(-1);
  }

  /*
  char strIDCUserName[51];
  memset(strIDCUserName,0,sizeof(strIDCUserName));
  strcpy(strIDCUserName,argv[2]);
  char *pos=strstr(strIDCUserName,"/");
  if (pos<=0) CallQuit(-1);
  pos[0]=0;
  ToUpper(strIDCUserName);
  */

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (ALLTABLE.LoadAllRecordNext() != 0) break;

    // 统计表的记录数
    if (ALLTABLE.CountTable(strDBLinkName) != 0)
    {
      logfile.Write("ALLTABLE.CountTable FAILED.\n");
    }

    // 更新T_ALLTABLE表的记录总数、统计时间字段、表的大小、索引的大小
    if (ALLTABLE.UptCountTime(strDBLinkName) != 0)
    {
      logfile.Write("ALLTABLE.UptCountTime failed.\n");
    }

    logfile.Write("tname=%s,%d\n",ALLTABLE.m_stALLTABLE.tname,ALLTABLE.m_stALLTABLE.totalcount);
  }

  logfile.WriteEx("\n\n\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("counttable exit.\n");

  exit(0);
}

