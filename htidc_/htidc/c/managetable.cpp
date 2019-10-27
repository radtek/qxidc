#include "idcapp.h"

void CallQuit(int sig);

char strDstConnStr[201]; // 数据库连接参数

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;
UINT           uAPPID=0;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/managetable logfilename connstr appid\n\n");

    printf("Example:/htidc/htidc/bin/procctl 14400 /htidc/htidc/bin/managetable /log/ssqx/managetable_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 14400 /htidc/htidc/bin/managetable /log/ssqx/managetable_2.log qxidc/pwdidc@EJETDB_221.179.6.136 5\n\n");

    printf("管理数据中心集群服务器全部表数据的备份、归档和清理。\n");
    printf("数据管理的参数由数据字典表T_APPTABLE中的hdataptype和hdatapcfg决定。\n");
    printf("appid是待管理的数据库，它必须是集群中的数据库之一。\n");
    printf("如果需要对多个集群数据库进行管理，就必须运行多个managetable程序。\n\n\n");
    printf("注意：appid必须在T_DAPPSERVER表里存在\n\n\n");
 
    return -1;
  }

  uAPPID=atoi(argv[3]);

  memset(strDstConnStr,0,sizeof(strDstConnStr));

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("managetable");

  // 注意，程序超时是60秒
  ProgramActive.SetProgramInfo(&logfile,"managetable",60);

  // 连接数据中心的集群数据库
  if (connidc.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  long irsts=0;
  // 从数据中心的T_DAPPSERVER中提取待管理的服务器的状态参数
  if (findbypk(&connidc,"T_DAPPSERVER","appid","rsts",uAPPID,&irsts) != 0)
  {
    logfile.Write("Call findbypk failed\n"); CallQuit(-1);
  }
  if (irsts!=1)
  {
    logfile.Write("appserver no exist or stopped.\n"); connidc.disconnect(); sleep(30); return 0;
  }

  // 从数据中心的T_DAPPSERVER中提取待管理的服务器的数据库连接参数
  if (findbypk(&connidc,"T_DAPPSERVER","appid","tnsname",uAPPID,strDstConnStr,50) != 0)
  {
    logfile.Write("Call findbypk failed\n"); CallQuit(-1);
  }

  logfile.Write("DstConnStr=%s\n",strDstConnStr);

  // 连接待管理数据的数据库
  if (conndst.connecttodb(strDstConnStr,TRUE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed.\n",strDstConnStr); CallQuit(-1);
  }

  // 注意，连上数据库后，程序超时改为3600秒
  ProgramActive.SetProgramInfo(&logfile,"managetable",3600);

  ALLTABLE.BindConnLog(&connidc,&conndst,&logfile,&ProgramActive);

  CTABFIELD TABFIELD;

  // 设置集群服务器的ID
  ALLTABLE.m_appid=uAPPID;

  // 从T_APPTABLE表中载入某集群服务器需要管理的表的记录，用于数据管理
  if (ALLTABLE.LoadForManager() != 0)
  {
    logfile.Write("ALLTABLE.LoadForManager failed.\n"); CallQuit(-1);
  }

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (ALLTABLE.LoadAllRecordNext() != 0) break;

    // 获取表的全部的列信息
    if (TABFIELD.GetALLField(&conndst,ALLTABLE.m_stALLTABLE.tname) == FALSE)
    {
      logfile.Write("TABFIELD.GetALLField failed.\n"); CallQuit(-1);
    }

    memset(ALLTABLE.m_COLUMNSSTR,0,sizeof(ALLTABLE.m_COLUMNSSTR));
    strcpy(ALLTABLE.m_COLUMNSSTR,TABFIELD.m_allfieldstr);

    logfile.Write("%s\n",ALLTABLE.m_stALLTABLE.tname);

    while (TRUE)
    {
      ProgramActive.WriteToFile();

      // 把基数据表的记录装入容器
      if (ALLTABLE.LoadALLROWID() != 0)
      {
        logfile.Write("ALLTABLE.LoadALLROWID failed.\n"); CallQuit(-1);
      }

      ProgramActive.WriteToFile();

      // 归档
      if (ALLTABLE.m_stALLTABLE.hdataptype == 0)
      {
        if (ALLTABLE.BackupToATable() != 0)
        {
          logfile.Write("ALLTABLE.BackupToATable failed.\n"); CallQuit(-1);
        }
      }
   
      // 备份
      if (ALLTABLE.m_stALLTABLE.hdataptype == 1)
      {
        if (ALLTABLE.BackupToHTable() != 0)
        {
          logfile.Write("ALLTABLE.BackupToHTable failed.\n"); CallQuit(-1);
        }
      }

      // 删除
      if (ALLTABLE.m_stALLTABLE.hdataptype == 2)
      {
        if (ALLTABLE.DeleteTable() != 0)
        {
          logfile.Write("ALLTABLE.DeleteTable failed.\n"); CallQuit(-1);
        }
      }

      // 如果被处理的记录小于50000，表示该表需要处理的数据已被处理完，就退出这个子循环
      if (ALLTABLE.m_stALLTABLE.totalcount<50000 ) break;
    }

    logfile.WriteEx("\n");
  }

  logfile.Write("ok.\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("managetable exit.\n");

  exit(0);
}

