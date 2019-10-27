#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

// 判断目的表是否存在，如果表不存在，返回失败
BOOL CheckDstTName();

// 执行数据同步
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[101];
char strsrcfields[2001];
char strdsttname[101];
char strdstfields[2001];
char strwhere[501];
char stralarmbz[21];

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_update2 xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/synctable_update2 \"<logfilename>/log/sqxj/synctable_update2_INDEXINFO.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_INDEXINFO@SZIDC</srctname><srcfields>cityid,cityname,pubdate,gm</srcfields><dsttname>T_INDEXINFO</dsttname><dstfields>cityid,cityname,pubdate,gm</dstfields><where>where pubdate>sysdate-1</where><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("这是一个工具程序，用于在Oracle数据库之间同步数据，同步的方法是全表刷新或按条件刷新。\n");
    printf("synctable_update2程序不要求目的表有主键，也不需要创建sync_rowid列，执行一个存储过程，删除-插入目的表。\n");
    printf("所以，本程序不宜用于同步数据量太大的任务。\n");
    printf("还有，如果源表没有delete操作，用synctable_update1和synctable_update2都可以，如果有delete操作，就只能用本程序了。\n");
    printf("<logfilename>/log/sqxj/synctable_update2_INDEXINFO.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 目的数据库的连接参数。\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> 数据库的字符集，必须要与<connstr>"\
           "参数描述的数据库相同。\n");
    printf("<srctname>T_INDEXINFO@SZIDC</srctname> 数据源表名，在表名后加数据库链路参数，就可以指向远程数据库。\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> 数据源表的字段列表，用于填充在select和from之间，"\
           "所以，srcfields可以是真实的字段，也可以是任何Oracle的函数或运算。\n");
    printf("<dsttname>T_INDEXINFO</dsttname> 接收数据的目的表名。\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> 接收数据的目的表的字段列表，与"\
           "<srcfields>不同，它必须是真实存在的字段。\n");
    printf("<where>where pubdate>sysdate-1</where> 同步的条件，即select语句的where部分。\n");
    printf("<alarmbz>FALSE</alarmbz> 程序运行是否发出告警，缺省是TRUE。\n\n");

    printf("注意：\n");
    printf("  1、<srcfields>和<dstfields>可以为空，如果为空，将以目的表的数据字典中的字段填充。\n");
    printf("     如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。\n");
    printf("  2、<where>可以为空，如果为空，每次都会刷新表中全部的记录。\n");
    printf("  3、<charset>可以为空，如果为空，本程序将不会设置字符集环境参数，那么在操作系统环境中必须已\n");
    printf("     设置正确的NLS_LANG环境变量。\n");
    printf("  4、<dsttname>参数的目的表必须存在sync_rowid字段，数据类型为rowid，\n");
    printf("     并且，sync_rowid字段必须是唯一索引。\n");
    printf("  5、本程序由procctl调度，运行一次即执行一次同步。\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  memset(strwhere,0,sizeof(strwhere));
  memset(stralarmbz,0,sizeof(stralarmbz));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,100);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,100);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"where",strwhere,500);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);

  if (strlen(strlogfilename) == 0)      { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)          { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)          { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)         { printf("srctname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)     { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)         { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)     { printf("dstfields is null.\n"); return -1; }
  // if (strlen(strwhere) == 0)         { printf("where is null.\n"); return -1; }
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");

  // 规范表名和字段名的大小写
  ToUpper(strsrctname);  ToUpper(strdsttname);
  ToLower(strsrcfields); ToLower(strdstfields);

  // 如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。
  if ( (strlen(strsrcfields)==0) || (strlen(strdstfields)==0) )
  {
    memset(strsrcfields,0,sizeof(strsrcfields)); memset(strdstfields,0,sizeof(strdstfields));
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);
 

  // 打开日志文件
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_update2");

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"synctable_update2",1200);

  // 如果目的表中有"@"字符，就一定搞错了源表和目的表的关系
  if (strstr(strdsttname,"@") > 0) 
  { 
    logfile.Write("%s表必段是%s连接中指定的数据库的本地表，不允许指向远程数据库的表。\n",\
                   strdsttname,strconnstr);
    return -1;
  }

  // 连接数据库
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // 判断目的表是否存在，如果表不存在，返回失败
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // 杀死与同步目的表有关的会话，清理出锁
  KillLocked(&conn,strdsttname);

  // 执行数据同步
  if (SyncTable()==FALSE) CallQuit(-1);

  return 0;
}

// 判断目的表是否存在，如果表不存在，返回失败
// 判断目的表是否有sync_rowid列，如果没有，返回失败
// 判断sync_rowid列是否创建了唯一索引，如果没有，返回失败
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // 获取该表全部的字段
  TABFIELD.GetALLField(&conn,strdsttname);

  if (TABFIELD.m_fieldcount==0)
  {
    logfile.Write("%s表不存在，请创建它。\n",strdsttname); return FALSE;
  }

  // 不处理sync_rowid字段
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rowid","");
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rowid,","");

  // 旧的同步表中有sync_rownum字段，排除它
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rownum,","");
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rownum","");

  // <srcfields>和<dstfields>为空，将以目的表的数据字典中的字段填充。
  if ( (strlen(strsrcfields)==0) && (strlen(strdstfields)==0) )
  {
    strcpy(strsrcfields,TABFIELD.m_allfieldstr); strcpy(strdstfields,TABFIELD.m_allfieldstr);
  }

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // 杀死与同步目的表有关的会话，清理出锁
  KillLocked(&conn,strdsttname);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_update2 exit.\n");

  exit(0);
}

// 执行数据同步
BOOL SyncTable()
{
  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);
 
  char strLocalTime[21];
  char strsysdate[101];
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strsysdate,0,sizeof(strsysdate));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  sprintf(strsysdate,"to_date('%s','yyyymmddhh24miss')",strLocalTime);
  
  UpdateStr(strwhere,"sysdate",strsysdate,TRUE);

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    BEGIN\
       delete from %s %s;\
       insert into %s(%s) select %s from %s %s;\
    END;",strdsttname,strwhere,strdsttname,strdstfields,strsrcfields,strsrctname,strwhere);
  if (stmt.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",stmt.m_sql,stmt.cda.message); return FALSE;
  }

  logfile.WriteEx("ok,table refreshed(%s)\n",strwhere);

  conn.commitwork();

  return TRUE;
}

