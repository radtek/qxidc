#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

// 删除系统数据字典中不存在，但T_ALLTABLE表中存在的表
long DeleteALLTABLE();
// 把数据字典中存在，但T_ALLTABLE表中不存在的记录插入T_ALLTABLE表中
long InsertALLTABLE();
// 把T_ALLTABLE表中存在，但T_APPTABLE表中不存在的记录插入T_APPTABLE表中
long InsertAPPTABLE();
// 把T_ALLTABLE表中存在，但T_DSYNCCFG表中不存在的记录插入T_DSYNCCFG表中
long InsertDSYNCCFG();
// 把T_ALLTABLE表中存在，但T_BDSYNCCFG表中不存在的记录插入T_BDSYNCCFG表中
long InsertBDSYNCCFG();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/managedict logfilename connstr \n");

    printf("Example:/htidc/htidc/bin/procctl 120 /htidc/htidc/bin/managedict /log/ssqx/managedict.log qxidc/pwdidc@EJETDB_221.179.6.136\n\n");

    printf("本程序用于自动维护T_ALLTABLE、T_APPTABLE、T_DSYNCCFG和T_BDSYNCCFG字典表的记录。\n");
    printf( "把数据字典中存在，但T_ALLTABLE表中不存在的记录插入T_ALLTABLE表中\n" );
    printf( "把T_ALLTABLE表中存在，但T_APPTABLE表中不存在的记录插入T_APPTABLE表中\n" );
    printf( "把T_ALLTABLE表中存在，但T_DSYNCCFG表中不存在的记录插入T_DSYNCCFG表中\n" );
    printf( "把T_ALLTABLE表中存在，但T_BDSYNCCFG表中不存在的记录插入T_BDSYNCCFG表中\n" );
    printf("本程序是数据中心的一个公共功能程序，由procctl调度。\n\n\n");
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

  //打开告警
  logfile.SetAlarmOpt("managedict");

  // 注意，连接数据库的超时时间是500秒
  ProgramActive.SetProgramInfo(&logfile,"managedict",500);

  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // 注意，程序超时之前是500秒，现在改为180秒，因为上面连数据库失败时可能要很长时间，如果把时间设为180秒，
  // 可能导致告警无法捕获。
  ProgramActive.SetProgramInfo(&logfile,"managedict",180);

  logfile.Write("begin ...");

  // 删除系统数据字典中不存在，但T_ALLTABLE表中存在的表
  if (DeleteALLTABLE() != 0) { logfile.Write("DeleteALLTABLE failed.\n"); CallQuit(-1); }
   
  // 把数据字典中存在，但T_ALLTABLE表中不存在的记录插入T_ALLTABLE表中
  if (InsertALLTABLE() != 0) { logfile.Write("InsertALLTABLE failed.\n"); CallQuit(-1); }

  // 把T_ALLTABLE表中存在，但T_APPTABLE表中不存在的记录插入T_APPTABLE表中
  if (InsertAPPTABLE() != 0) { logfile.Write("InsertAPPTABLE failed.\n"); CallQuit(-1); }

  // 把T_ALLTABLE表中存在，但T_DSYNCCFG表中不存在的记录插入T_DSYNCCFG表中
  if (InsertDSYNCCFG() != 0) { logfile.Write("InsertDSYNCCFG failed.\n"); CallQuit(-1); }

  // 把T_ALLTABLE表中存在，但T_BDSYNCCFG表中不存在的记录插入T_BDSYNCCFG表中
  if (InsertBDSYNCCFG() != 0) { logfile.Write("InsertBDSYNCCFG failed.\n"); CallQuit(-1); }

  conn.commitwork();

  logfile.WriteEx("end.\n\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("managedict exit.\n");

  exit(0);
}

// 删除系统数据字典中不存在，但T_ALLTABLE表中存在的表
long DeleteALLTABLE()
{
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    BEGIN\
      delete T_DSYNCCFG\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
      delete T_BDSYNCCFG\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
      delete T_APPTABLE\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
      delete T_ALLTABLE\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
    END;");
  if (stmt.execute() != 0)
  {
     logfile.Write("DeleteALLTABLE T_ALLTABLE failed.\n%s\n",stmt.cda.message);
  }
    
  return stmt.cda.rc;
}

// 把数据字典中存在，但T_ALLTABLE表中不存在的记录插入T_ALLTABLE表中
long InsertALLTABLE()
{
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    insert into T_ALLTABLE(tname,tcname,keyid)\
     select upper(table_name),upper(table_name),SEQ_ALLTABLE.nextval from USER_TABLES\
      where substr(table_name,1,2)='T_' and table_name not in (select upper(tname) from T_ALLTABLE)");
  if (stmt.execute() != 0)
  {
     logfile.Write("InsertALLTABLE T_ALLTABLE failed.\n%s\n",stmt.cda.message);
  }
  
  return stmt.cda.rc;    
}

// 把T_ALLTABLE表中存在，但T_APPTABLE表中不存在的记录插入T_APPTABLE表中
long InsertAPPTABLE()
{
  int iappid=0;
  sqlstatement stmtsel,stmtins;
  stmtsel.connect(&conn);
  stmtsel.prepare("select appid from T_DAPPSERVER where servertype in (1,2) order by appid");
  stmtsel.bindout(1,&iappid);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("InsertAPPTABLE select T_DAPPSERVER failed.\n%s\n",stmtsel.cda.message); 
    return stmtsel.cda.rc;
  }

  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_APPTABLE(appid,tname,hdataptype,keyid)\
       select :1,tname,3,SEQ_APPTABLE.nextval from T_ALLTABLE\
        where tname not in (select tname from T_APPTABLE where appid=:2)");
  stmtins.bindin(1,&iappid);
  stmtins.bindin(2,&iappid);
 
  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  
    if (stmtins.execute() != 0)
    {
      logfile.Write("InsertAPPTABLE insert T_APPTABLE failed.\n%s\n",stmtins.cda.message); 
      return stmtins.cda.rc;
    }
  }

  return 0;    
}

// 把T_ALLTABLE表中存在，但T_DSYNCCFG表中不存在的记录插入T_DSYNCCFG表中
long InsertDSYNCCFG()
{
  int iappid=0;
  sqlstatement stmtsel,stmtins;
  stmtsel.connect(&conn);
  stmtsel.prepare("select appid from T_DAPPSERVER where servertype=2 order by appid");
  stmtsel.bindout(1,&iappid);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("InsertDSYNCCFG select T_DAPPSERVER failed.\n%s\n",stmtsel.cda.message); 
    return stmtsel.cda.rc;
  }

  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_DSYNCCFG(appid,tname,rsts,keyid)\
       select :1,tname,2,SEQ_DSYNCCFG.nextval from T_ALLTABLE\
        where tname not in (select tname from T_DSYNCCFG where appid=:2)");
  stmtins.bindin(1,&iappid);
  stmtins.bindin(2,&iappid);
 
  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  
    if (stmtins.execute() != 0)
    {
      logfile.Write("InsertDSYNCCFG insert T_DSYNCCFG failed.\n%s\n",stmtins.cda.message); 
      return stmtins.cda.rc;
    }
  }

  return 0;    
}

// 把T_ALLTABLE表中存在，但T_BDSYNCCFG表中不存在的记录插入T_BDSYNCCFG表中
long InsertBDSYNCCFG()
{
  // 为了兼容深圳的数据中心，servertype in (2,5,6)，新的只有2
  int iappid=0;
  sqlstatement stmtsel,stmtins;
  stmtsel.connect(&conn);
  stmtsel.prepare("select appid from T_DAPPSERVER where servertype in (2,5,6) order by appid");
  stmtsel.bindout(1,&iappid);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("InsertBDSYNCCFG select T_DAPPSERVER failed.\n%s\n",stmtsel.cda.message); 
    return stmtsel.cda.rc;
  }

  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_BDSYNCCFG(appid,tname,rsts,keyid)\
       select :1,tname,2,SEQ_BDSYNCCFG.nextval from T_ALLTABLE\
        where tname not in (select tname from T_BDSYNCCFG where appid=:2)");
  stmtins.bindin(1,&iappid);
  stmtins.bindin(2,&iappid);
 
  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  
    if (stmtins.execute() != 0)
    {
      logfile.Write("InsertBDSYNCCFG insert T_BDSYNCCFG failed.\n%s\n",stmtins.cda.message); 
      return stmtins.cda.rc;
    }
  }

  return 0;    
}
