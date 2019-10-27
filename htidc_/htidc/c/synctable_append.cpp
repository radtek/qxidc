#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

int lobfieldcount=0;

// 判断目的表是否存在，如果表不存在，返回失败
// 判断目的表是否有sync_rowid列，如果没有，返回失败
// 判断sync_rowid列是否创建了唯一索引，如果没有，返回失败
BOOL CheckDstTName();

// 执行数据同步
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcfields[2001];
char strdsttname[51];
char strdstfields[2001];
char strand[501];
char strignoreerror[11];

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_append xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_append \"<logfilename>/log/sqxj/synctable_append_ALLAWSDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_ALLAWSDATA@SZIDC</srctname><srcfields></srcfields><dsttname>T_ALLAWSDATA</dsttname><dstfields></dstfields><and>and ossmocode like '5%%'</and><ignoreerror>FALSE</ignoreerror>\"\n\n");

    printf("这是一个工具程序，用于在Oracle数据库之间同步数据，作为synctable_append程序的补充，当目的数据库表与源数据库表的数据不一致时，运行本程序可以把源数据库表中的数据补入目的数据表中。\n\n");
    printf("<logfilename>/log/sqxj/synctable_append_ALLAWSDATA.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 目的数据库的连接参数。\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> 数据库的字符集，必须要与<connstr>"\
           "参数描述的数据库相同。\n");
    printf("<srctname>T_ALLAWSDATA@SZIDC</srctname> 数据源表名，在表名后加数据库链路参数，就可以指向远程数据库。\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> 数据源表的字段列表，用于填充在select和from之间，"\
           "所以，srcfields可以是真实的字段，也可以是任何Oracle的函数或运算。\n");
    printf("<dsttname>T_ALLAWSDATA</dsttname> 接收数据的目的表名。\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> 接收数据的目的表的字段列表，与"\
           "<srcfields>不同，它必须是真实存在的字段。\n");
    printf("<and>and ossmocode like '5%%'</and> 同步的条件，即select语句where后面的部分，注意，要以and 开头。\n");
    printf("<ignoreerror>FALSE</ignoreerror> 当操作数据错误时，是否继续，一般情况下取FALSE。"\
           "只有当数据源表的某些记录有坏块时，才采用TRUE。\n\n");

    printf("注意：\n");
    printf("  1、<srcfields>和<dstfields>可以为空，如果为空，将以目的表的数据字典中的字段填充。\n");
    printf("     如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。\n");
    printf("  2、<charset>可以为空，如果为空，本程序将不会设置字符集环境参数，那么在操作系统环境中必须已\n");
    printf("     设置正确的NLS_LANG环境变量。\n");
    printf("  3、<dsttname>参数的目的表必须存在sync_rowid字段，数据类型为rowid，\n");
    printf("     并且，sync_rowid字段必须是唯一索引。\n");
    printf("  4、本程序可以用synctable_increment程序的参数，但日志文件名一定不同。\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  memset(strand,0,sizeof(strand));
  memset(strignoreerror,0,sizeof(strignoreerror));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"and",strand,500);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)    { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)   { printf("dstfields is null.\n"); return -1; }
  // if (strlen(strand) == 0)       { printf("and is null.\n"); return -1; }
  // if (strlen(strignoreerror) == 0)       { printf("ignoreerror is null.\n"); return -1; }

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
  logfile.SetAlarmOpt("synctable_append");

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"synctable_append",1200);

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
  // 判断目的表是否有sync_rowid列，如果没有，返回失败
  // 判断sync_rowid列是否创建了唯一索引，如果没有，返回失败
  if (CheckDstTName()==FALSE) CallQuit(-1);

  ProgramActive.WriteToFile();

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

  // 判断目的表是否有sync_rowid列，如果没有，返回失败
  if (strstr(TABFIELD.m_allfieldstr,"sync_rowid") == 0)
  {
    logfile.Write("%s表没有sync_rowid列，请创建它。\n",strdsttname); return FALSE;
  }

  // 判断sync_rowid列是否创建了唯一索引，如果没有，返回失败
  char stroutput[51];
  memset(stroutput,0,sizeof(stroutput));
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select column_name from USER_IND_COLUMNS where table_name=upper(:1) and column_name='SYNC_ROWID'");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,stroutput,50);
  stmt.execute();
  stmt.next();

  if (strlen(stroutput)==0)
  {
    logfile.Write("%s表的sync_rowid列没有创建唯一索引，请创建它。\n",strdsttname); return FALSE;
  }

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

  // 为strsrcfields加上rowid列，为strdstfields加上sync_rowid列
  strcat(strsrcfields,",rowid"); strcat(strdstfields,",sync_rowid");

  lobfieldcount=0;
  // 判断目的表是否有二进制字段，如果有，把字段的个数存放在lobfieldcount变量中
  stmt.prepare(\
     "select count(*) from USER_TAB_COLUMNS where table_name=upper(:1) and data_type in ('BLOB','CLOB')");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&lobfieldcount);
  stmt.execute();
  stmt.next();

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_append exit.\n");

  exit(0);
}

// 执行数据同步
BOOL SyncTable()
{
  char rowid[31];

  sqlstatement selstmt,insstmt;

  selstmt.connect(&conn);
  selstmt.prepare("select rowid from %s",strsrctname);
  selstmt.bindout(1,rowid,30);

  if (selstmt.execute() != 0)
  {
    logfile.Write("%s\n%s failed.\n",selstmt.m_sql,selstmt.cda.message); return FALSE;
  }
  
  insstmt.connect(&conn);
  insstmt.prepare("\
    BEGIN\
      delete from %s where sync_rowid=:1;\
      insert into %s(%s) select %s from %s where rowid=:2 %s;\
    END;",strdsttname,strdsttname,strdstfields,strsrcfields,strsrctname,strand);
  insstmt.bindin(1,rowid,30);
  insstmt.bindin(2,rowid,30);

  while (TRUE)
  {
    memset(rowid,0,sizeof(rowid));

    if ( (selstmt.cda.rpc>1) && (fmod(selstmt.cda.rpc,5000) < 1) )
    {
      logfile.Write("%d rows append.\n",selstmt.cda.rpc);
      ProgramActive.WriteToFile();
      conn.commitwork();
    }

    if (selstmt.next() != 0) break;

    if (insstmt.execute() != 0)
    {
      logfile.Write("\n%s\n%s failed.\n",insstmt.m_sql,insstmt.cda.message); 

      if ( (insstmt.cda.rc != 1) && (insstmt.cda.rc != 1410) )
      {
        if (strcmp(strignoreerror,"TRUE") != 0) return FALSE;
      }
    }
  }

  conn.commitwork();

  logfile.Write("%d rows append.\n",selstmt.cda.rpc);

  return TRUE;
}

