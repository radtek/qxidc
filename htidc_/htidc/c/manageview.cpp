#include "idcapp.h"

void CallQuit(int sig);

char strDstConnStr[201]; // 数据库连接参数

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;
UINT           uAPPID=0;

// 在数据中心集群数据库中，只能创建V_TNAME和V_TNAME_ALL视图，其它的视图会被删除。
long DropOthView();

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./manageview logfilename connstr appid\n\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/manageview /log/ssqx/manageview_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/manageview /log/ssqx/manageview_2.log qxidc/pwdidc@EJETDB_221.179.6.136 2\n\n");

    printf("管理数据中心集群数据库全部表的视图，任何表都会被创建视图。\n");
    printf("V_TNAME是联合当前表和历史表，V_TNAME_ALL是联合当前表、历史表和归档表。\n");
    printf("如果不存在历史表或归档表，V_TNAME和V_TNAME_ALL就是表自身的视图。\n");
    printf("在数据中心集群数据库中，只能创建V_TNAME和V_TNAME_ALL视图，其它的视图会被删除。\n");
    printf("appid是待管理的数据库，它必须是集群中的数据库之一。\n");
    printf("如果需要对多个集群数据库进行管理，就必须运行多个manageview程序。\n\n\n");
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
  logfile.SetAlarmOpt("manageview");

  // 注意，程序超时是60秒
  ProgramActive.SetProgramInfo(&logfile,"manageview",60);

  // 连接数据中心数据库
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

  // 连接数据中心的集群数据库
  if (conndst.connecttodb(strDstConnStr,TRUE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed.\n",strDstConnStr); CallQuit(-1);
  }

  // 在数据中心集群数据库中，只能创建V_TNAME和V_TNAME_ALL视图，其它的视图会被删除。
  if (DropOthView() != 0) CallQuit(-1);

  // 注意，连上数据库后，程序超时改为180秒
  ProgramActive.SetProgramInfo(&logfile,"manageview",180);

  ALLTABLE.BindConnLog(&connidc,&conndst,&logfile,&ProgramActive);

  CTABFIELD TABFIELD;

  // 设置集群服务器的ID
  ALLTABLE.m_appid=uAPPID;

  // 从T_APPTABLE表中载入某集群服务器全部的当前表，用于创建视图
  if (ALLTABLE.LoadForCrtView() != 0)
  {
    logfile.Write("ALLTABLE.LoadForCrtView failed.\n"); CallQuit(-1);
  }

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (ALLTABLE.LoadAllRecordNext() != 0) break;
  
    if (CrtView(&conndst,ALLTABLE.m_stALLTABLE.tname) != 0)
    {
      logfile.Write("create view %s FAILED.\n",ALLTABLE.m_stALLTABLE.tname); continue;
    }

    logfile.Write("create view %s ok.\n",ALLTABLE.m_stALLTABLE.tname);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("manageview exit.\n");

  exit(0);
}


// 在数据中心集群数据库中，只能创建V_TNAME和V_TNAME_ALL和V_TNAME_SHARE视图，其它的视图会被删除。
long DropOthView()
{
  char view_name[31];

  sqlstatement stmtsel;
  stmtsel.connect(&connidc);  // 连数据中心核心库
  stmtsel.prepare(\
    "select view_name from USER_VIEWS\
      where view_name not in\
            (select 'V'||substr(tname,2,30) from t_APPTABLE\
              where appid=:1 and tname not like '%%HIS' and tname not like '%%ARH%%')\
        and view_name not in\
            (select 'V'||substr(tname,2,30)||'_ALL' from t_APPTABLE\
              where appid=:2 and tname not like '%%HIS' and tname not like '%%ARH%%')\
        and view_name not in\
            (select 'V'||substr(tname,2,30)||'_SHARE' from t_APPTABLE\
              where appid=:3 and tname not like '%%HIS' and tname not like '%%ARH%%')");
  stmtsel.bindin(1,&uAPPID);
  stmtsel.bindin(2,&uAPPID);
  stmtsel.bindin(3,&uAPPID);
  stmtsel.bindout(1,view_name,30);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("DropOthView select USER_VIEWS failed.\n%s\n",stmtsel.cda.message);
    return stmtsel.cda.rc;
  }

  sqlstatement stmtdel;
  stmtdel.connect(&conndst);  // 连数据中心目的数据库
  
  while (TRUE)
  {
    memset(view_name,0,sizeof(view_name));

    if (stmtsel.next() != 0) break;

    stmtdel.prepare("drop view %s",view_name);
    stmtdel.execute();

    logfile.Write("%s\n",stmtdel.m_sql);
  }

  return 0;
}
