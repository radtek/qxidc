#include "idcapp.h"

char strlogfilename[301];
char strconnstr[101];
char strtname[51];
char strlogtname[51];
char strlogidxname1[51];
char strlogidxname2[51];
char strtablespace[51];
char strlogseqname[51];
char strtriggername[51];
char strsyncinsert[11];
char strsyncupdate[11];
char strsyncdelete[11];
char strinitdata[11];

void CallQuit(int sig);

CLogFile   logfile;
connection conn;
CProgramActive ProgramActive;

// 创建同步日志表、索引、序列
BOOL CrtLogTable();

// 创建触发器
BOOL CrtTrigger();

// 把数据源表中的记录更新到日志表中
BOOL InitData();

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./crtsynctrigger xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/crtsynctrigger \"<logfilename>/log/sqxj/crtsynctrigger_ALLAWSDATA.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><tname>T_ALLAWSDATA</tname><logtname>T_ALLAWSDATA_SYNC</logtname><logidxname1>IDX_ALLAWSDATA_SYNC_1</logidxname1><logidxname2>IDX_ALLAWSDATA_SYNC_2</logidxname2><tablespace>USERS</tablespace><logseqname>SEQ_ALLAWSDATA_SYNC</logseqname><triggername>TR_ALLAWSDATA_SYNC</triggername><syncinsert>true</syncinsert><syncupdate>true</syncupdate><syncdelete>false</syncdelete><initdata>true</initdata>\"\n\n");

    printf("本程序用于在数据源表中创建触发器、创建同步日志表和初始化同步日志表中的数据。\n");
    printf("<logfilename>/log/sqxj/crtsynctrigger_ALLAWSDATA.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 数据源数据库的连接参数。\n");
    printf("<tname>T_ALLAWSDATA</tname> 数据源表名。\n");
    printf("<logtname>T_ALLAWSDATA_SYNC</logtname> 同步日志表名。\n");
    printf("<logidxname1>IDX_ALLAWSDATA_SYNC_1</logidxname1> 同步日志表的sync_rowid列的索引名。\n");
    printf("<logidxname2>IDX_ALLAWSDATA_SYNC_2</logidxname2> 同步日志表的keyid列的索引名。\n");
    printf("<tablespace>USERS</tablespace> 日志表和索引创建时分配的表空间。\n");
    printf("<logseqname>SEQ_ALLAWSDATA_SYNC</logseqname> 同步日志表的keyid采用的序列生成器。\n");
    printf("<triggername>TR_ALLAWSDATA_SYNC</triggername> 数据源表上用于生成日志表数据的触发器名称。\n");
    printf("<syncinsert>true</syncinsert> 是否同步insert操作。\n");
    printf("<syncupdate>true</syncupdate> 是否同步update操作。\n");
    printf("<syncdelete>false</syncdelete> 是否同步delete操作。\n");
    printf("<initdata>true</initdata> 是否把数据源表的全部记录初始化到日志表中。\n\n");

    printf("注意，当数据源表有删除操作的时候，同步日志表中会产生无效记录，这些无效记录会占用数据库表空间，"\
           "但并不会影响同步。\n");
    printf("crtsynctrigger 程序的以下参数\n"\
           "<syncinsert>true</syncinsert>\n"\
           "<syncupdate>true</syncupdate>\n"\
           "<syncdelete>false</syncdelete>\n"\
           "一般来说，syncinsert设置为true，如果不需要同步源表的update和delete操作，都是设置为false\n"\
           "特别是syncdelete，如果不需要同步删除操作，syncdelete必须设置为false，"\
           "否则在同步日志表中会生成太多的无效记录。\n"\
           "如果syncdelete设置为true，管理员可以用clearsynclog程序不定期清理日志表中的无效记录。\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strtname,0,sizeof(strtname));
  memset(strlogtname,0,sizeof(strlogtname));
  memset(strtablespace,0,sizeof(strtablespace));
  memset(strlogidxname1,0,sizeof(strlogidxname1));
  memset(strlogidxname2,0,sizeof(strlogidxname2));
  memset(strlogseqname,0,sizeof(strlogseqname));
  memset(strtriggername,0,sizeof(strtriggername));
  memset(strsyncinsert,0,sizeof(strsyncinsert));
  memset(strsyncupdate,0,sizeof(strsyncupdate));
  memset(strsyncdelete,0,sizeof(strsyncdelete));
  memset(strinitdata,0,sizeof(strinitdata));


  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"tname",strtname,50);
  GetXMLBuffer(argv[1],"logtname",strlogtname,50);
  GetXMLBuffer(argv[1],"logidxname1",strlogidxname1,50);
  GetXMLBuffer(argv[1],"logidxname2",strlogidxname2,50);
  GetXMLBuffer(argv[1],"tablespace",strtablespace,50);
  GetXMLBuffer(argv[1],"logseqname",strlogseqname,50);
  GetXMLBuffer(argv[1],"triggername",strtriggername,50);
  GetXMLBuffer(argv[1],"syncinsert",strsyncinsert,5);
  GetXMLBuffer(argv[1],"syncupdate",strsyncupdate,5);
  GetXMLBuffer(argv[1],"syncdelete",strsyncdelete,5);
  GetXMLBuffer(argv[1],"initdata",strinitdata,5);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strtname) == 0)    { printf("tname is null.\n"); return -1; }
  if (strlen(strlogtname) == 0) { printf("logtname is null.\n"); return -1; }
  if (strlen(strlogidxname1) == 0) { printf("logidxname1 is null.\n"); return -1; }
  if (strlen(strlogidxname2) == 0) { printf("logidxname2 is null.\n"); return -1; }
  if (strlen(strtablespace) == 0) { printf("tablespace is null.\n"); return -1; }
  if (strlen(strlogseqname) == 0) { printf("logseqname is null.\n"); return -1; }
  if (strlen(strtriggername) == 0) { printf("triggername is null.\n"); return -1; }
  if (strlen(strsyncinsert) == 0)  { printf("syncinsert is null.\n"); return -1; }
  if (strlen(strsyncupdate) == 0)  { printf("syncupdate is null.\n"); return -1; }
  if (strlen(strsyncdelete) == 0)  { printf("syncdelete is null.\n"); return -1; }
  if (strlen(strinitdata) == 0)    { printf("initdata is null.\n"); return -1; }

  if ( (strcmp(strsyncinsert,"true") != 0) && 
       (strcmp(strsyncupdate,"true") != 0) &&
       (strcmp(strsyncdelete,"true") != 0) )
  {
    printf("syncinsert、syncupdate、syncdelete不能都为空。\n");
  }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 打开日志文件
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("crtsynctrigger");

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&logfile,"crtsynctrigger",300);

  // 连接数据库
  if (conn.connecttodb(strconnstr,FALSE) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // 创建同步日志表、索引、序列
  if (CrtLogTable()==FALSE) CallQuit(-1);

  // 创建触发器
  if (CrtTrigger()==FALSE) CallQuit(-1);

  // 把数据源表中的记录更新到日志表中
  if (InitData()==FALSE) CallQuit(-1);

  exit(0);
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("crtsynctrigger exit.\n");

  exit(0);
}

// 创建同步日志表、索引、序列
BOOL CrtLogTable()
{
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("create table %s(sync_rowid rowid,keyid number(15)) tablespace %s",strlogtname,strtablespace);
  if (stmt.execute() != 0)
  {
    if (stmt.cda.rc != 955)
    {
      logfile.Write("create table %s failed.\n%s\n%s\n",strlogtname,stmt.m_sql,stmt.cda.message); 
      return FALSE;
    }
  }

  logfile.Write("create table %s ok.\n",strlogtname);

  stmt.prepare("create unique index %s on %s(sync_rowid) tablespace %s",strlogidxname1,strlogtname,strtablespace);
  if (stmt.execute() != 0)
  {
    if ( (stmt.cda.rc != 955) && (stmt.cda.rc != 1408) )
    {
      logfile.Write("create index %s failed.\n%s\n%s\n",strlogidxname1,stmt.m_sql,stmt.cda.message); 
      return FALSE;
    }
  }

  logfile.Write("create index %s ok.\n",strlogidxname1);

  stmt.prepare("create unique index %s on %s(keyid) tablespace %s",strlogidxname2,strlogtname,strtablespace);
  if (stmt.execute() != 0)
  {
    if ( (stmt.cda.rc != 955) && (stmt.cda.rc != 1408) )
    {
      logfile.Write("create index %s failed.\n%s\n%s\n",strlogidxname2,stmt.m_sql,stmt.cda.message); 
      return FALSE;
    }
  }

  logfile.Write("create index %s ok.\n",strlogidxname2);

  stmt.prepare("create sequence %s increment by 1 minvalue 1",strlogseqname);
  if (stmt.execute() != 0)
  {
    if (stmt.cda.rc != 955)
    {
      logfile.Write("create sequence %s failed.\n%s\n%s\n",strlogseqname,stmt.m_sql,stmt.cda.message); 
      return FALSE;
    }
  }

  logfile.Write("create sequence %s ok.\n",strlogseqname);

  return TRUE;
}

// 创建触发器
BOOL CrtTrigger()
{
  char strtemp[1024],strSQL[1024];

  memset(strSQL,0,sizeof(strSQL));

  memset(strtemp,0,sizeof(strtemp));
  sprintf(strtemp,"CREATE OR REPLACE TRIGGER %s AFTER \n",strtriggername);
    
  if (strcmp(strsyncinsert,"true") == 0) strcat(strtemp,"INSERT OR ");
  if (strcmp(strsyncupdate,"true") == 0) strcat(strtemp,"UPDATE OR ");
  strcat(strtemp,"DELETE OR ");

  strtemp[strlen(strtemp)-4]=0;
  strcat(strSQL,strtemp);

  sprintf(strtemp," ON %s FOR EACH ROW\nBEGIN\n",strtname);
  strcat(strSQL,strtemp);

  if (strcmp(strsyncinsert,"true") == 0)
  {
    sprintf(strtemp,"  if inserting then\n");
    strcat(strSQL,strtemp);
      
    sprintf(strtemp,"    insert into %s(sync_rowid,keyid) values(:new.rowid,%s.nextval);\n",strlogtname,strlogseqname);
    strcat(strSQL,strtemp);

    sprintf(strtemp,"  end if;\n");
    strcat(strSQL,strtemp);
  }

  if (strcmp(strsyncupdate,"true") == 0)
  {
    sprintf(strtemp,"  if updating then\n");
    strcat(strSQL,strtemp);
      
    sprintf(strtemp,"    update %s set keyid=%s.nextval where sync_rowid=:new.rowid;\n",strlogtname,strlogseqname);
    strcat(strSQL,strtemp);

    sprintf(strtemp,"  end if;\n");
    strcat(strSQL,strtemp);
  }

  if (strcmp(strsyncdelete,"true") == 0)
  {
    sprintf(strtemp,"  if deleting then\n");
    strcat(strSQL,strtemp);
      
    sprintf(strtemp,"    update %s set keyid=%s.nextval where sync_rowid=:new.rowid;\n",strlogtname,strlogseqname);
    strcat(strSQL,strtemp);

    sprintf(strtemp,"  end if;\n");
    strcat(strSQL,strtemp);
  }
  else
  {
    sprintf(strtemp,"  if deleting then\n");
    strcat(strSQL,strtemp);
      
    sprintf(strtemp,"    delete from %s where sync_rowid=:new.rowid;\n",strlogtname);
    strcat(strSQL,strtemp);

    sprintf(strtemp,"  end if;\n");
    strcat(strSQL,strtemp);
  }
  

  sprintf(strtemp,"exception\n"\
                  "when others then\n"\
                  "return;\n"\
                  "END;\n");
  strcat(strSQL,strtemp);

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare(strSQL);
  if (stmt.execute() != 0)
  {
    logfile.Write("create trigger %s failed.\n%s\n%s\n",strtriggername,stmt.m_sql,stmt.cda.message);
  }

  logfile.WriteEx("\n%s\n",strSQL);

  return TRUE;
}

// 把数据源表中的记录更新到日志表中
BOOL InitData()
{
  if (strcmp(strinitdata,"true") != 0) return TRUE;

  char rowid[31];

  sqlstatement selsrc;
  selsrc.connect(&conn);
  selsrc.prepare("select rowid from %s",strtname);
  selsrc.bindout(1,rowid,30);

  int ccount=0;
  sqlstatement selchk;
  selchk.connect(&conn);
  selchk.prepare("select count(*) from %s where sync_rowid=:1",strlogtname);
  selchk.bindin(1,rowid,30);
  selchk.bindout(1,&ccount);

  sqlstatement inslog;
  inslog.connect(&conn);
  inslog.prepare("insert into %s(sync_rowid,keyid) values(:1,%s.nextval)",strlogtname,strlogseqname);
  inslog.bindin(1,rowid,30);

  if (selsrc.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strlogtname,selsrc.cda.message);
  }

  while (TRUE)
  {
    memset(rowid,0,sizeof(rowid));

    if ( (selsrc.cda.rpc>1) && (fmod(selsrc.cda.rpc,5000) < 1) )
    {
      logfile.Write("%d rows inserted.\n",selsrc.cda.rpc);
      ProgramActive.WriteToFile(); 
      conn.commitwork();
    }

    if (selsrc.next() != 0) break;

    if (selchk.execute() != 0)
    {
      logfile.Write("select %s failed.\n%s\n",strlogtname,selchk.cda.message);
    }

    ccount=0;

    selchk.next();

    if (ccount==1) continue;

    if (inslog.execute() != 0)
    {
      logfile.Write("insert %s failed.\n%s\n",strlogtname,inslog.cda.message);
    }
  }

  conn.commitwork();

  return TRUE;
}


