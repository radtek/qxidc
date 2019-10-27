#include "idcapp.h"

char strlogfilename[301];
char strconnstr[101];
char strtname[51];
char strlogtname[51];

void CallQuit(int sig);

CLogFile   logfile;
connection conn;
CProgramActive ProgramActive;

// 清理同步日志表中已经无效的日志记录
BOOL InitData();

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./clearsynclog xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 259200 /htidc/htidc/bin/clearsynclog \"<logfilename>/log/sqxj/clearsynclog_ALLAWSDATA_LOG.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><tname>T_ALLAWSDATA</tname><logtname>T_ALLAWSDATA_LOG</logtname>\"\n\n");

    printf("本程序用于清理同步日志表中已经无效的日志记录。\n");
    printf("<logfilename>/log/sqxj/clearsynclog_ALLAWSDATA.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 数据源数据库的连接参数。\n");
    printf("<tname>T_ALLAWSDATA</tname> 数据源表名。\n");
    printf("<logtname>T_ALLAWSDATA_LOG</logtname> 同步日志表名。\n\n");
    printf("本程序可以由系统管理员定期启动，也可以由procctl调度，但是启动的时间间隔要长，否则会消耗太多"\
           "的数据库资源。\n\n");

    printf("注意，当数据源表有删除操作的时候，同步日志表中会产生无效记录，这些无效记录会占用数据库表空间，"\
           "但并不会影响同步。\n");
    printf("crtsynctrigger 程序的以下参数\n"\
           "<syncinsert>true</syncinsert>\n"\
           "<syncupdate>false</syncupdate>\n"\
           "<syncdelete>false</syncdelete>\n"\
           "一般来说，syncinsert设置为true，如果不需要同步源表的update和delete操作，都是设置为false\n"\
           "特别是syncdelete，如果不需要同步删除操作，syncdelete最好设置为false，"\
           "否则在同步日志表中会生成太多的无效记录。\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strtname,0,sizeof(strtname));
  memset(strlogtname,0,sizeof(strlogtname));


  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"tname",strtname,50);
  GetXMLBuffer(argv[1],"logtname",strlogtname,50);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strtname) == 0)    { printf("tname is null.\n"); return -1; }
  if (strlen(strlogtname) == 0) { printf("logtname is null.\n"); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 打开日志文件
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("clearsynclog");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"clearsynclog",180);

  // 连接数据库
  if (conn.connecttodb(strconnstr,FALSE) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // 清理同步日志表中已经无效的日志记录
  if (InitData()==FALSE) CallQuit(-1);

  exit(0);
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("clearsynclog exit.\n");

  exit(0);
}

// 清理同步日志表中已经无效的日志记录
BOOL InitData()
{
  char sync_rowid[31];

  sqlstatement sellog;
  sellog.connect(&conn);
  sellog.prepare("select sync_rowid from %s",strlogtname);
  sellog.bindout(1,sync_rowid,30);

  int ccount=0;
  sqlstatement selchk;
  selchk.connect(&conn);
  selchk.prepare("select count(*) from %s where rowid=:1",strtname);
  selchk.bindin(1,sync_rowid,30);
  selchk.bindout(1,&ccount);

  sqlstatement dellog;
  dellog.connect(&conn);
  dellog.prepare("delete from %s where sync_rowid=:1",strlogtname);
  dellog.bindin(1,sync_rowid,30);

  if (sellog.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strlogtname,sellog.cda.message);
  }

  int idelrows=0;

  logfile.Write("check %s ...\n",strlogtname);

  while (TRUE)
  {
    memset(sync_rowid,0,sizeof(sync_rowid));

    if ( (sellog.cda.rpc>1) && (fmod(sellog.cda.rpc,10000) < 1) )
    {
      if (idelrows>0) logfile.Write("%d rows deleted.\n",idelrows);
      ProgramActive.WriteToFile(); 
      conn.commitwork();
      idelrows=0;
    }

    if (sellog.next() != 0) break;

    selchk.execute();

    ccount=0;

    selchk.next();

    if (ccount==1) continue;

    idelrows++;

    if (dellog.execute() != 0)
    {
      logfile.Write("insert %s failed.\n%s\n",strlogtname,dellog.cda.message);
    }
  }

  logfile.Write("%d rows deleted.\n",idelrows);

  conn.commitwork();

  return TRUE;
}


