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
    printf("Using:./managesynonym logfilename connstr appid\n\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/managesynonym /log/ssqx/managesynonym_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/managesynonym /log/ssqx/managesynonym_2.log qxidc/pwdidc@EJETDB_221.179.6.136 2\n\n");

    printf("管理数据中心集群数据库全部的同义词，任何qxidc用户的表、视图和序列都会被创建公共同义词。\n");
    printf("appid是待管理的数据库，它必须是集群中的数据库之一。\n");
    printf("如果需要对多个集群数据库进行管理，就必须运行多个managesynonym程序。\n");
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
  logfile.SetAlarmOpt("managesynonym");

  // 注意，程序超时是60秒
  ProgramActive.SetProgramInfo(&logfile,"managesynonym",60);

  char strIDCUserName[51];
  memset(strIDCUserName,0,sizeof(strIDCUserName));
  strcpy(strIDCUserName,argv[2]);
  char *pos=strstr(strIDCUserName,"/");
  if (pos<=0) CallQuit(-1);
  pos[0]=0;
  ToUpper(strIDCUserName);

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

  // 注意，连上数据库后，程序超时改为180秒
  ProgramActive.SetProgramInfo(&logfile,"managesynonym",180);

  char strtable_name[51];  // qxidc用户的表、视图和序列名

  sqlstatement seltname;
  seltname.connect(&conndst);
  seltname.prepare("\
    (select table_name    table_name from    USER_TABLES)\
    union\
    (select view_name     table_name from    USER_VIEWS)\
    union\
    (select Sequence_name table_name from    USER_SEQUENCES)\
    union\
    (select object_name from USER_PROCEDURES where object_name in ('FINDCFIELDNAME','FINDCOMMENT','PATCHEXTVTIME','WDTOWDFW'))");
  seltname.bindout(1,strtable_name,50);

  int icount=0;
  sqlstatement selsynonym;
  selsynonym.connect(&conndst);
  selsynonym.prepare("\
    select count(*) from ALL_SYNONYMS\
     where owner='PUBLIC' and synonym_name=:1 and table_owner='%s' and table_name=synonym_name\
       and db_link is null",strIDCUserName);
  selsynonym.bindin(1,strtable_name,30);
  selsynonym.bindout(1,&icount);

  if (seltname.execute() != 0)
  {
    logfile.Write("select USER_TABLES,USER_VIEWS,USER_SEQUENCES failed.\n%s\n",seltname.cda.message); return seltname.cda.rc;
  }

  sqlstatement stmttmp;
  stmttmp.connect(&conndst);

  while (TRUE)
  {
    icount=0;

    memset(strtable_name,0,sizeof(strtable_name));

    if (seltname.next() != 0) break;

    if (selsynonym.execute() != 0)
    {
      logfile.Write("select ALL_SYNONYMS1 failed.\n%s\n",selsynonym.cda.message); return selsynonym.cda.rc;
    }
    
    selsynonym.next();

    if (icount==0)
    {
      // 虽然从数据字典中没有查询到对该对象命名的同义词，同时也执行删除操作，有备无患
      stmttmp.prepare("drop public synonym %s",strtable_name);
      stmttmp.execute();
  
      // 然后再创建同义词
      stmttmp.prepare("create public synonym %s for %s.%s",strtable_name,strIDCUserName,strtable_name);
      if (stmttmp.execute() != 0)
      {
        logfile.Write("create synonym %s failed.\n%s\n",strtable_name,stmttmp.cda.message); 
        return stmttmp.cda.rc;
      }

      logfile.Write("create public synonym %s ok.\n",strtable_name);
    }
  }

  // 删除数据中心的数据字典中不存在的同义词
  char synonym_name[51];
  selsynonym.prepare("\
    select synonym_name from ALL_SYNONYMS\
     where owner='PUBLIC'  and table_owner='%s'\
       and synonym_name not in \
           ((select table_name    table_name from    USER_TABLES)\
             union\
            (select view_name     table_name from    USER_VIEWS)\
             union\
            (select Sequence_name table_name from    USER_SEQUENCES)\
             union\
            (select object_name from USER_PROCEDURES where object_name in ('FINDCFIELDNAME','FINDCOMMENT','PATCHEXTVTIME','WDTOWDFW')))",strIDCUserName);
  selsynonym.bindout(1,synonym_name,30);
  
  if (selsynonym.execute() != 0)
  {
    logfile.Write("select ALL_SYNONYMS2 failed.\n%s\n",selsynonym.cda.message); return selsynonym.cda.rc;
  }
  while (TRUE)
  {
    memset(synonym_name,0,sizeof(synonym_name));
    if (selsynonym.next() != 0) break;

    stmttmp.prepare("drop public synonym %s",synonym_name);
    if (stmttmp.execute() != 0)
    {
      logfile.Write("drop synonym %s failed.\n%s\n",synonym_name,stmttmp.cda.message); 
      return stmttmp.cda.rc;
    }

    logfile.Write("drop public synonym %s ok.\n",synonym_name);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("managesynonym exit.\n");

  exit(0);
}

