#include "idcapp.h"

void CallQuit(int sig);

char strSRCConnStr[201]; // 数据源库连接参数
char strDSTConnStr[201]; // 目的数据库连接参数

CIDCCFG IDCCFG;

connection     connsrc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;

char strSyncName[31];
char strDBLink[31];
char strTNameStr[4096];

CSYNCTABLE   SYNCTABLE;

// 检查数据源，源表是否存在，同步的日志表是否存在，同步的日志表的序列是否存在，基于数据源表的触发器是否存在。
// 如果数据源表不存在，返回失败，如果日志表、序列或触发器不存在，则创建它。
BOOL CheckTABSEQTR();

int main(int argc,char *argv[])
{
  if (argc != 8)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/synctable inifile src:username/password@tnsname dst:username/password@tnsname dblink tablenames syncname logname\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable /htidc/sqxt/ini/sqxt.xml idc/oracle@EJETDB_172.22.11.112 idc/oracle@EJETDB_10.12.12.61 EJETDB_172_22_11_112 T_GD_AO_201010,T_GD_AO_201011,T_OBTDATA_201010,T_OBTDATA_201011,T_SW_OCEAN,T_SW_SURF,T_ASIAN_CITY_7DAYS,T_ASIAN_GAMES,T_ASIAN_GAMES_SG,T_ASIAN_GAMES_SW _12_61 _11.112.log\n\n");
 
    printf("这是一个工具程序，它可以把某ORACLE数据库中的表的数据同步到另一个ORACLE数据库。\n");
    printf("在数据源的数据库用户中，本程序自动创建同步日志表和在被每个被同步的表上创建一个触发器。\n");
    printf("src:username/password@tnsname是数据源数据库连接字符串。\n");
    printf("dst:username/password@tnsname是目的数据库连接字符串。\n");
    printf("dblink是目的数据库中到数据源数据库的数据链路，需要DBA手工创建。\n");
    printf("tablenames是需要同步的表名，用逗号分隔开，注意，tablenames是源表表名，如果源表表名不是以T_打头，\n");
    printf("目的表名必须是源表表名之前加上T_，如果源表表名是以T_打头就算了，目的表需要DBA手工创建。\n");
    printf("syncname是同步标志，如果要把同一数据源表往多个目的数据库同步，必须使用不同的标志。\n");
    printf("logname是该程序运行时产生的日志文件名的后缀。\n");
    printf("本程序运行异常时，会发生LOGINDB和SYNCTABLE告警。\n\n");

    printf("注意：\n");
    printf("1、本程序要求数据源表和目的表的结构完全相同（除了sync_rowid、crttime和keyid字段），并且，目的表\n");
    printf("   一定要有sync_rowid、crttime和keyid字段，源表有没有这三个字段无所谓。\n\n");

    printf("2、目的表的sync_rowid字段一定要创建索引，最好是唯一索引，但对crttime字段不作要求。\n\n");

    printf("3、目的表的keyid字段一定要创建唯一索引，程序从目的数据库的SEQ_表名.nextval取值。\n\n");

    printf("4、该程序启用后，只同步数据源表中本程序启动的时间点之后变动的记录，原有的记录不会同步，如果希望\n");
    printf("   数据源表中以前的记录也被同步，需要手工把源表的rowid插入到同步日志表中，可参考syncscript.txt。\n\n");

    printf("5、当本程序停用后，DBA一定要手工删除数据源用户中的同步日志表（T_SYNCLOG_同步标志）。\n\n");

    printf("6、当本程序停用后，DBA一定要手工删除数据源用户中的同步表上的触发器（TR_表名_同步标志）。\n\n");

    printf("7、/htidc/sqxt/sql/syncscript.txt是省气象台数据同步的例子，包括操作流程，可参考。\n\n");

    printf("8、注意不要禁用或删除了别人创建的触发器，否则麻烦大了。\n\n");

    return -1;
  }

  memset(strSRCConnStr,0,sizeof(strSRCConnStr));
  memset(strDSTConnStr,0,sizeof(strDSTConnStr));
  memset(strDBLink,0,sizeof(strDBLink));
  memset(strTNameStr,0,sizeof(strTNameStr));
  memset(strSyncName,0,sizeof(strSyncName));

  strcpy(strSRCConnStr,argv[2]);
  strcpy(strDSTConnStr,argv[3]);
  strcpy(strDBLink,argv[4]);
  strcpy(strTNameStr,argv[5]);
  strcpy(strSyncName,argv[6]);

  ToUpper(strTNameStr);

  // 从参数文件中加载全部的参数
  if (IDCCFG.LoadIniFile(argv[1]) == FALSE)
  {
    printf("IDCCFG.LoadIniFile(%s) failed.\n",argv[1]); return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; 
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,200,"%s/synctable%s",IDCCFG.m_logpath,argv[7]);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("synctable");

  // 注意，程序超时是500秒
  ProgramActive.SetProgramInfo(&logfile,"synctable",500);

  strcpy(SYNCTABLE.m_dblink,strDBLink);

  // 连接数据源数据库
  if (connsrc.connecttodb(strSRCConnStr) != 0)
  {
    logfile.Write("connsrc.connecttodb(%s) failed\n",strSRCConnStr); 

    CallQuit(-1);
  }

  // 连接目的数据库
  if (conndst.connecttodb(strDSTConnStr) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed\n",strDSTConnStr); 

    CallQuit(-1);
  }

  // 注意，程序超时之前是500秒，现在改为180秒，因为上面连数据库失败时可能要很长时间，如果把时间设为180秒，
  // 可能导致告警无法捕获。
  ProgramActive.SetProgramInfo(&logfile,"synctable",180);

  SYNCTABLE.BindConnLog(&conndst,&connsrc,&logfile);

  // 检查数据源，源表是否存在，同步的日志表是否存在，同步的日志表的序列是否存在，基于数据源表的触发器是否存在。
  // 如果数据源表不存在，返回失败，如果日志表、序列或触发器不存在，则创建它。
  if (CheckTABSEQTR() == FALSE)
  {
    CallQuit(-1);
  }

  // 调用SYNCTABLE.LoadTInfo的时候，如果同步日志表的记录太多，需要的时间就越长，所以这里改为1200秒
  ProgramActive.SetProgramInfo(&logfile,"synctable",1200);

  // 从数据源数据库的数据字典中载入待同步的表的结构，用小写存放
  if (SYNCTABLE.LoadTInfo(strTNameStr) != 0)
  {
    CallQuit(-1);
  }

  // 改回180秒
  ProgramActive.SetProgramInfo(&logfile,"synctable",180);

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (SYNCTABLE.LoadSyncLog(strSyncName) != 0)
    {
      CallQuit(-1);
    }

    ProgramActive.WriteToFile();

    if (SYNCTABLE.SyncDATA(&ProgramActive,strSyncName) != 0)
    {
      CallQuit(-1);
    }

    ProgramActive.WriteToFile();

    sleep(5);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  connsrc.rollbackwork(); conndst.rollbackwork(); 

  //connalm.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable exit.\n");

  exit(0);
}

// 检查数据源，源表是否存在，同步的日志表是否存在，同步的日志表的序列是否存在，基于数据源表的触发器是否存在。
// 如果数据源表不存在，返回失败，如果日志表、序列或触发器不存在，则创建它。
BOOL CheckTABSEQTR()
{
  char strDstTName[51],strTRName[51],strTmpName[31];

  sqlstatement stmt;
  stmt.connect(&connsrc);

  // 检查序列是否存在
  memset(strTmpName,0,sizeof(strTmpName));
  snprintf(strTmpName,30,"SEQ_SYNCLOG%s",strSyncName);
  if (CheckSEQExist(&connsrc,strTmpName) == FALSE) 
  {
    stmt.prepare("CREATE SEQUENCE %s increment by 1 minvalue 1 nocycle",strTmpName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE SEQUENCE %s failed.\n%s\n",strTmpName,stmt.cda.message); return FALSE;
    }
    logfile.Write("CREATE SEQUENCE %s ok.\n",strTmpName);
  }

  // 检查日志表T_SYNCLOG_SyncName是否存在
  memset(strTmpName,0,sizeof(strTmpName));
  snprintf(strTmpName,30,"T_SYNCLOG%s",strSyncName);
  if (CheckTExist(&connsrc,strTmpName) == FALSE) 
  {
    stmt.prepare("CREATE TABLE %s(keyid number(18),tname varchar2(50),ctype number(1),sync_rowid rowid,crttime date default sysdate)",strTmpName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE TABLE %s failed.\n%s\n",strTmpName,stmt.cda.message); return FALSE;
    }

    stmt.prepare("CREATE UNIQUE INDEX IDX_SYNCLOG%s_1 on T_SYNCLOG%s(keyid)",strSyncName,strSyncName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE UNIQUE INDEX IDX_SYNCLOG%s_1 failed.\n%s\n",strSyncName,stmt.cda.message); return FALSE;
    }

    stmt.prepare("comment on table T_SYNCLOG%s is \
                  '此表用于数据同步，禁止任何人对该表做任何操作，如有疑问，请与数据库管理员沟通。'",strSyncName);
    stmt.execute();

    stmt.prepare("CREATE INDEX IDX_SYNCLOG%s_2 on T_SYNCLOG%s(tname)",strSyncName,strSyncName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE INDEX IDX_SYNCLOG%s_2 failed.\n%s\n",strSyncName,stmt.cda.message); return FALSE;
    }

    logfile.Write("CREATE TABLE T_SYNCLOG%s ok.\n",strSyncName);
  }

   // 检查数据源表和目的数据库的表是否存在
  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strTNameStr,",");

  for (UINT ii=0;ii<CmdStr.m_vCmdStr.size();ii++)
  { 
    // 检查数据源表是否存在
    if (CheckTExist(&connsrc,(char *)CmdStr.m_vCmdStr[ii].c_str()) == FALSE) 
    {
      logfile.Write("数据源数据库的表不存在（%s）.\n",CmdStr.m_vCmdStr[ii].c_str()); return FALSE;
    }

    // 检查目的数据库表是否存在，目的表在源表名前加“T_”，如果源表已经是“T_”打头，就不加了
    memset(strDstTName,0,sizeof(strDstTName));
    if (strncmp(CmdStr.m_vCmdStr[ii].c_str(),"T_",2) == 0)
      strcpy(strDstTName,CmdStr.m_vCmdStr[ii].c_str());
    else
      sprintf(strDstTName,"T_%s",CmdStr.m_vCmdStr[ii].c_str());
    
    if (CheckTExist(&conndst,strDstTName) == FALSE) 
    {
      logfile.Write("目的数据库的表不存在（%s）.\n",strDstTName); return FALSE;
    }

    // 检查数据源表的触发器是否存在，如果不存在，就创建它。
    memset(strTRName,0,sizeof(strTRName));
    if (strncmp(CmdStr.m_vCmdStr[ii].c_str(),"T_",2) == 0)
      sprintf(strTRName,"TR_%s%s",CmdStr.m_vCmdStr[ii].c_str()+2,strSyncName);
    else
      sprintf(strTRName,"TR_%s%s",CmdStr.m_vCmdStr[ii].c_str(),strSyncName);
    if (CheckTRExist(&connsrc,strTRName) == FALSE) 
    {
      stmt.prepare("\
        CREATE OR REPLACE TRIGGER %s\
         AFTER INSERT OR UPDATE OR DELETE\
            ON %s FOR EACH ROW\
        BEGIN\
          if inserting then\
            insert into T_SYNCLOG%s(keyid,tname,ctype,sync_rowid) values(SEQ_SYNCLOG%s.nextval,'%s',1,:new.rowid);\
          end if;\
          if updating then\
            insert into T_SYNCLOG%s(keyid,tname,ctype,sync_rowid) values(SEQ_SYNCLOG%s.nextval,'%s',2,:new.rowid);\
          end if;\
          if deleting then\
            insert into T_SYNCLOG%s(keyid,tname,ctype,sync_rowid) values(SEQ_SYNCLOG%s.nextval,'%s',3,:old.rowid);\
          end if;\
        END;",strTRName,CmdStr.m_vCmdStr[ii].c_str(),strSyncName,strSyncName,CmdStr.m_vCmdStr[ii].c_str(),strSyncName,strSyncName,CmdStr.m_vCmdStr[ii].c_str(),strSyncName,strSyncName,CmdStr.m_vCmdStr[ii].c_str());
      if (stmt.execute() != 0)
      {
        logfile.Write("CREATE TRIGGER %s.\n%s\n",strTRName,stmt.cda.message); return FALSE;
      }

      logfile.Write("CREATE TRIGGER %s ok.\n",strTRName);
    }
  }

  return TRUE;
}

/*
select count(*) from T_QPEAUTOSTATION@szqx_10_153_97_14;
select count(*) from idcty.QPEAUTOSTATION@ejetdb_10_153_100_3;

select count(*) from T_QPEAUTOSTATIONDAYS@szqx_10_153_97_14;
select count(*) from idcty.QPEAUTOSTATIONDAYS@ejetdb_10_153_100_3;

select count(*) from T_QPEWIND@szqx_10_153_97_14;
select count(*) from idcty.QPEWIND@ejetdb_10_153_100_3;

select count(*) from T_QPEAUTOSTATION@szqx_10_153_97_14;
select count(*) from idcty.QPEAUTOSTATION@ejetdb_10_153_100_3;

select count(*) from T_SERVICE_PRODUCT@szqx_10_153_97_14;
select count(*) from idcty.SERVICE_PRODUCT@ejetdb_10_153_100_3;


select count(*) from T_VENUEGROUP@szqx_10_153_97_14;
select count(*) from forecast.T_VENUEGROUP@ejetdb_10_153_98_3;

select count(*) from T_VENUEINFO@szqx_10_153_97_14;
select count(*) from forecast.T_VENUEINFO@ejetdb_10_153_98_3;


select count(*) from T_VENUEFILE@szqx_10_153_97_14;
select count(*) from forecast.T_VENUEFILE@ejetdb_10_153_98_3;


select count(*) from T_PRODUCETRACE@szqx_10_153_97_14;
select count(*) from forecast.T_PRODUCETRACE@ejetdb_10_153_98_3;


select count(*) from T_BYTHEREHOURFORCAST@szqx_10_153_97_14;
select count(*) from forecast.BYTHEREHOURFORCAST@ejetdb_10_153_98_3;


select count(*) from T_BYHOURPARTONE@szqx_10_153_97_14;
select count(*) from forecast.T_BYHOURPARTONE@ejetdb_10_153_98_3;

select count(*) from T_BYHOURPARTTWO@szqx_10_153_97_14;
select count(*) from forecast.T_BYHOURPARTTWO@ejetdb_10_153_98_3;


select count(*) from T_DBYS_INDEX@szqx_10_153_97_14;
select count(*) from ybdb.DBYS_INDEX@exoa_10_153_98_6;

select count(*) from T_DBYS_DETAILS@szqx_10_153_97_14;
select count(*) from ybdb.DBYS_DETAILS@exoa_10_153_98_6;

select count(*) from T_REPLACEDIC@szqx_10_153_97_14;
select count(*) from ybdb.REPLACEDIC@exoa_10_153_98_6;


select count(*) from T_GDFPICTURE@szqx_10_153_97_14;
select count(*) from idcty.GDFPICTURE@ejetdb_10_153_98_3;

select count(*) from T_GDFPICTURE2@szqx_10_153_97_14;
select count(*) from idcty.GDFPICTURE2@ejetdb_10_153_98_3;


select count(*) from T_SMSTASK@szqx_10_153_97_14;
select count(*) from szalarm.SMSTASK@szalarm_10_153_97_30;

select count(*) from T_SMSUSERGROUP@szqx_10_153_97_14;
select count(*) from szalarm.SMSUSERGROUP@szalarm_10_153_97_30;


*/
