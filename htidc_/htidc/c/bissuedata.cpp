#include <math.h>
#include "idcapp.h"
 
void CallQuit(int sig);
 
UINT uAPPID=0;
UINT uMaxListID=0;
UINT uTotalCount=0;
char strDstConnStr[201];  // 远程数据库连接参数

connection     conndst,connidc;
CLogFile       logfile;
CProgramActive ProgramActive;
CBDSYNCCFG     BDSYNCCFG;
UINT           OrgKeyID;

BOOL _bissuedata(struct st_BDSYNCCFG stBDSYNCCFG);

#define MAXCOUNT 100
UINT    uMaxCount;

// 删除远程数据库用户表的外键
BOOL DropFKey(char *strDstConnStr);

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./bissuedata logfilename connstr appid\n");
 
    printf("Example:/htidc/htidc/c/procctl 30 /htidc/htidc/c/bissuedata /log/ssqx/bissuedata_2.log qxidc/pwdidc@EJETDB_221.179.6.136  2\n");
    printf("        /htidc/htidc/c/procctl 30 /htidc/htidc/c/bissuedata /log/ssqx/bissuedata_3.log qxidc/pwdidc@EJETDB_221.179.6.136  3\n\n");
 
    printf("数据同步的服务程序，把数据定时批量的分发到各业务应用的数据库中。\n");
    printf("appid是接收数据的数据库的ID，它是在数据中心的T_DAPPSERVER表中定义。\n");
    printf("如果需要往多个目的数据库同步数据，就必须启动多个bissuedata程序。\n");
    printf("注意：1.appid必须在T_DAPPSERVER中存在。\n");
    printf("      2.需要同步的表要在T_ALLTABLE表中设置好批量同步。\n");
    printf("      3.需要同步的表要在T_BDSYNCCFG。\n\n\n");
 
    return -1;
  }

  uAPPID = atoi(argv[3]);

  memset(strDstConnStr,0,sizeof(strDstConnStr));
 
  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);
 
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }
 
  logfile.SetAlarmOpt("bissuedata");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"bissuedata",180);
 
  // 连接数据中心数据库，用于读取同步参数和更新同步时间以及错误告警
  if (connidc.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  BDSYNCCFG.BindConnLog (&connidc,&logfile);

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
 
  // 连接远程数据库，用于数据同步
  if (conndst.connecttodb(strDstConnStr,FALSE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed\n",strDstConnStr); CallQuit(-1);
  }

  // 删除远程数据库用户表的外键
  DropFKey(strDstConnStr);
 
  // 从T_BDSYNCCFG表中载入需要执行批量同步的表
  if (BDSYNCCFG.LoadBSyncTable(uAPPID) != 0)
  {
    logfile.Write("BDSYNCCFG.LoadBSyncTable failed.\n"); CallQuit(-1);
  } 

  // logfile.Write( "Info: get %d record\n", BDSYNCCFG.m_vBDSYNCCFG.size() );
  for (UINT ii=0;ii<BDSYNCCFG.m_vBDSYNCCFG.size();ii++)
  {
    logfile.Write("copy %-20s ... ",\
                   BDSYNCCFG.m_vBDSYNCCFG[ii].tname);

    if ( (BDSYNCCFG.m_vBDSYNCCFG[ii].bsynctype!=1) && (BDSYNCCFG.m_vBDSYNCCFG[ii].bsynctype!=2) )
    {
      logfile.WriteEx("failed.bsynctype is invalid.\n");
    }

    // 更新已同步表的执行时间为当前时间
    if (BDSYNCCFG.UptBSyncTable(uAPPID,BDSYNCCFG.m_vBDSYNCCFG[ii].tname) != 0)
    {
      logfile.Write("BDSYNCCFG.UptBSyncTable failed.\n"); CallQuit(-1);
    }   
  
    if (_bissuedata(BDSYNCCFG.m_vBDSYNCCFG[ii]) == FALSE) { logfile.WriteEx("failed.\n"); continue; }

    // 如果同步类型是增量更新，就更新已同步表的已同步记录的位置
    if (BDSYNCCFG.m_vBDSYNCCFG[ii].bsynctype == 2)
    {
      if (BDSYNCCFG.UptBSyncTable(uAPPID,BDSYNCCFG.m_vBDSYNCCFG[ii].tname,OrgKeyID) != 0)
      {
        logfile.Write("BDSYNCCFG.UptBSyncTable failed.\n"); CallQuit(-1);
      }   
    }

    ProgramActive.WriteToFile();
  }

  return 0;
}

BOOL _bissuedata(struct st_BDSYNCCFG stBDSYNCCFG)
{
  char strColumnStr[2048];
  memset(strColumnStr,0,sizeof(strColumnStr));

  // 获取表的全部的列信息
  CTABFIELD TABFIELD;
  TABFIELD.GetALLField(&conndst,stBDSYNCCFG.tname);
  strcpy(strColumnStr,TABFIELD.m_allfieldstr);

  // 把m_stDSYNCCFG.columnsstr中的crttime和sync_rowid字段剔除
  UpdateStr(strColumnStr,",crttime",""); UpdateStr(strColumnStr,"crttime,","");
  UpdateStr(strColumnStr,",sync_rowid",""); UpdateStr(strColumnStr,"sync_rowid,","");

  // 判断表是否存在，如果没有列信息，表就肯定不存在
  if (strlen(strColumnStr) == 0) 
  {
    logfile.Write("table %s no exist.\n",stBDSYNCCFG.tname); return FALSE;
  }

  CTimer Timer;

  // 如果同步类型是刷新，并且同步条件子句为空，就一定是同步该表全部的记录，
  // 这种表的记录一般不会太多，为了保证数据一致性和效率，用一个SQL过程搞定它。
  // 并且，这种同步不需要keyid字段。
  if ( (stBDSYNCCFG.bsynctype == 1) && (strlen(stBDSYNCCFG.bsyncterm) == 0) )
  {
    sqlstatement stmt;
    stmt.connect(&conndst);
    stmt.prepare("\
      BEGIN\
         delete from %s;\
         insert into %s(%s) select %s from %s@HTIDC;\
      END;",stBDSYNCCFG.tname,stBDSYNCCFG.tname,strColumnStr,strColumnStr,stBDSYNCCFG.tname);
    if (stmt.execute() != 0)
    {
      logfile.Write("_bissuedata insert %s failed.\n%s\n%s\n",stBDSYNCCFG.tname,stmt.cda.message, stmt.m_sql);
      logfile.Write("user:%s\npassword:%s\ntnsname:%s\n", conndst.env.user,conndst.env.pass,conndst.env.tnsname );
    }
         
    logfile.WriteEx("ok,table refreshed(%ld).\n",Timer.Elapsed());

    conndst.commitwork();

    return TRUE;
  }

  char strterm[301];
  memset(strterm,0,sizeof(strterm));

  // 生成同步条件子句
  if (stBDSYNCCFG.bsynctype == 1)
  {
    OrgKeyID=0; 
    strcpy(strterm,stBDSYNCCFG.bsyncterm);
  }
  else
  {
    OrgKeyID=stBDSYNCCFG.orgkeyid;
    sprintf(strterm," where keyid>%lu and keyid>nvl((select max(keyid)-100000 from %s@HTIDC),0) order by keyid",\
            stBDSYNCCFG.orgkeyid,stBDSYNCCFG.tname);
  }

  UINT ccount=0;
  UINT keyid,keyidn[MAXCOUNT];

  if (stBDSYNCCFG.lobfieldcount == 0) uMaxCount=MAXCOUNT;
  if (stBDSYNCCFG.lobfieldcount >  0) uMaxCount=1;

  sqlstatement selidc,delstmt,insstmt;

  selidc.connect(&conndst);
  selidc.prepare("select keyid from %s@HTIDC %s",stBDSYNCCFG.tname,strterm);
  selidc.bindout(1,&keyid);
  
  if (selidc.execute() != 0)
  {
    logfile.Write("_bissuedata select keyid from %s@HTIDC failed.\n%s\n",stBDSYNCCFG.tname,selidc.cda.message); 
    logfile.Write("%s\nuser:%s\npassword:%s\ntnsname:%s\n",
                   selidc.m_sql, conndst.env.user,conndst.env.pass,conndst.env.tnsname );
    return FALSE;
  }

  UINT ii=0;
  char strtemp[11],strInStr[4096];

  memset(strInStr,0,sizeof(strInStr));

  for (ii=0; ii<uMaxCount; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%lu",ii+1);
    if (ii >0) sprintf(strtemp,",:%lu",ii+1);
    strcat(strInStr,strtemp);
  }

  insstmt.connect(&conndst);
  insstmt.prepare("\
    BEGIN\
      delete from %s where keyid in (%s);\
      insert into %s(%s) select %s from %s@HTIDC where keyid in (%s);\
    END;",stBDSYNCCFG.tname,strInStr,stBDSYNCCFG.tname,strColumnStr,strColumnStr,stBDSYNCCFG.tname,strInStr);\
  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1,&keyidn[ii]);
  }

  // 每uMaxCount条记录就插入一次
  while (TRUE)
  {
    keyid=0;

    if (selidc.next() != 0) break;

    keyidn[ccount]=keyid;

    if (stBDSYNCCFG.bsynctype == 2) OrgKeyID=keyid;

    ccount++;

    if (ccount == uMaxCount)
    {
      if (insstmt.execute() != 0) 
      {
        if (insstmt.cda.rc != 1)
        {
          logfile.Write("_bissuedata insert %s failed.\n%s\n",stBDSYNCCFG.tname,insstmt.cda.message);
          logfile.Write("%s\nuser:%s\npassword:%s\ntnsname:%s\n",
                     delstmt.m_sql, conndst.env.user,conndst.env.pass,conndst.env.tnsname );
          return FALSE;
        }
      }

      ProgramActive.WriteToFile();

      conndst.commitwork();

      memset(&keyidn,0,sizeof(keyidn));

      ccount=0;
    }
  }

  // 在以上循环处理的时候，如果不足uMaxCount，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      BEGIN\
        delete from %s where keyid=:1;\
        insert into %s(%s) select %s from %s@HTIDC where keyid=:2;\
      END;",stBDSYNCCFG.tname,stBDSYNCCFG.tname,strColumnStr,strColumnStr,stBDSYNCCFG.tname);
    insstmt.bindin(1,&keyidn[ii]);
    insstmt.bindin(2,&keyidn[ii]);
    if (insstmt.execute() != 0) 
    {
      if (insstmt.cda.rc != 1)
      {
        logfile.Write("_bissuedata insert %s failed.\n%s\n",stBDSYNCCFG.tname,insstmt.cda.message); return FALSE;
      }
    }

    conndst.commitwork();
  }

  if (stBDSYNCCFG.bsynctype == 1) 
  {
    logfile.WriteEx("ok,%ld rows refreshed(%ld).\n",selidc.cda.rpc,Timer.Elapsed());
  }
  else
  {
    logfile.WriteEx("ok,%ld rows inserted(%ld).\n",selidc.cda.rpc,Timer.Elapsed());
  }

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  printf("catching the signal(%d).\n",sig);

  printf("tablecp exit.\n");

  exit(0);
}


// 删除远程数据库用户表的外键
BOOL DropFKey(char *strDstConnStr)
{
  char strIDCUserName[51];
  memset(strIDCUserName,0,sizeof(strIDCUserName));
  strcpy(strIDCUserName,strDstConnStr);
  char *pos=strstr(strIDCUserName,"/");
  if (pos<=0) CallQuit(-1);
  pos[0]=0;
  ToUpper(strIDCUserName);

  char strSQL[1024];
  sqlstatement stmtsel,stmtdel;

  stmtsel.connect(&conndst);
  stmtsel.prepare("select 'alter table '||table_name||' drop constraint '||constraint_name from USER_CONSTRAINTS where OWNER=:1 and constraint_type='R' and substr(table_name,1,2)='T_'");
  stmtsel.bindin(1,strIDCUserName,20);
  stmtsel.bindout(1,strSQL,300);
  stmtsel.execute();

  while (TRUE)
  {
    memset(strSQL,0,sizeof(strSQL));
    if (stmtsel.next() != 0) break;

    stmtdel.connect(&conndst);
    stmtdel.prepare(strSQL);
    stmtdel.execute();

    logfile.Write("%s\n",strSQL);
  }

  return TRUE;
}

