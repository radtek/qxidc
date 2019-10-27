#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

BOOL bFirstStart=TRUE;

int lobfieldcount=0;

// 判断目的表是否存在，如果表不存在，返回失败
// 判断目的表是否有dstpkfieldname列，如果没有，返回失败
// 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
BOOL CheckDstTName();

// 判断程序的启动月份
void CheckMonth();

BOOL bContinue;
// 执行数据同步
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcpkfieldname[51];
//char strdstpkfieldname[51];
char strsrcfields[2001];
char strdsttname[51];
char strdstfields[2001];
int  timetvl;
char strand[1001];
char stralarmbz[21];
char strignoreerror[11];
int  maxcount;
char strmonth[3];
char strdblink[101];
char strtablename[51];

UINT uMaxKeyIDOld;  // 源数据库中，已同步成功的记录的最大keyid
UINT uMinKeyID;     // 本次同步的记录中，最小的keyid。
UINT uMaxKeyID;     // 本次同步的记录中，最大的keyid。

// 获取uMaxKeyIDOld、uMinKeyID、uMaxKeyID的值
BOOL LoadSyncKeyID();
// 把uMaxKeyID的值更新到T_SYNCLOG表中
BOOL UpdateSyncKeyID();  

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_increment3 xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_increment3 \"<logfilename>/log/szqx/synctable_increment2_LOCALOBTDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.98.24</connstr><srctname>T_LOCALOBTDATA@SZIDC</srctname><srcpkfieldname>keyid</srcpkfieldname><srcfields></srcfields><dsttname>T_LOCALOBTDATA</dsttname><dstfields></dstfields><timetvl>10</timetvl><and></and><ignoreerror>FALSE</ignoreerror><maxcount>300</maxcount><month>01</month><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("这是一个工具程序，用于在Oracle数据库之间同步数据，同步的方法增量，不能同步修改和删除操作。\n");
    printf("本程序不需要在源数据库中创建触发器和日志表，但是，要求源表的主键字段一定是整数，并且为自增型字段,目的表可以没有这个字段。\n");
    printf("synctable_increment3 程序为了目的表没有源表相同的主键字段而增加，当然有也可以，用法基本同synctable_increment2使用相同。\n");
    printf("<logfilename>/log/szqx/synctable_increment3_LOCALOBTDATA.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 本地数据库的连接参数。\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> 数据库的字符集，必须要与<connstr>"\
           "参数描述的数据库相同。\n");
    printf("<srctname>T_LOCALOBTDATA</srctname> 数据源表名。\n");
    printf("<srcpkfieldname>keyid</srcpkfieldname> 数据源表的主键字段名。\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> 数据源表的字段列表，用于填充在select和from之间，"\
           "所以，srcfields可以是真实的字段，也可以是任何Oracle的函数或运算。\n");
    printf("<dsttname>T_LOCALOBTDATA@SZIDC</dsttname> 接收数据的目的表名,在表名后加数据库链路参数，就指向远程数据库。\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> 接收数据的目的表的字段列表，与"\
           "<srcfields>不同，它必须是真实存在的字段。\n");
    printf("<timetvl>10</timetvl> 执行同步的时间间隔，单位是秒，如果为空，缺省用30。\n");
    printf("<and>and ossmocode like '5%%'</and> 同步的条件，即select语句where后面的部分，注意，要以and 开头。\n");
    printf("<ignoreerror>FALSE</ignoreerror> 当操作数据错误时，是否继续，一般情况下取FALSE。"\
           "只有当数据源表的某些记录有坏块时，才采用TRUE。\n");
    printf("<maxcount>300</maxcount> 执行一次同步操作的记录数，建议采用300。\n\n");
    printf("<month>01</month> 程序启动有月份，这是一个可选参数，只适用于把数据从总表同步月表的场合。"\
           "如果指定了该参数，那么程序只在指定的月份启动，其它的月份程序进入休眠状态。\n");
    printf("<alarmbz>FALSE</alarmbz> 程序运行是否发出告警，缺省是TRUE。\n\n");


    printf("注意：\n");
    printf("  1、<srcfields>和<dstfields>可以为空，如果为空，将以目的表的数据字典中的字段填充。\n");
    printf("     如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。\n");
    printf("  2、<charset>可以为空，如果为空，本程序将不会设置字符集环境参数，那么在操作系统环境中必须已\n");
    printf("     设置正确的NLS_LANG环境变量。\n");
    printf("  3、如果<timetvl>不超过30秒，本程序常驻内存，如果超过了30秒（包括30秒），执行完一次同步后将退出。\n\n");
    printf("  4、<maxcount>参数非常重要，如果数据源表有非法数据，当<maxcount>为1时，失败的记录无法同步，但不\n");
    printf("     会影响其它的记录，如果<maxcount>大于1，包含失败记录的同一批次的同步将全部失败。\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcpkfieldname,0,sizeof(strsrcpkfieldname));
  //memset(strdstpkfieldname,0,sizeof(strdstpkfieldname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  timetvl=0;
  memset(strand,0,sizeof(strand));
  memset(strignoreerror,0,sizeof(strignoreerror));
  maxcount=0;
  memset(strmonth,0,sizeof(strmonth));
  memset(stralarmbz,0,sizeof(stralarmbz));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcpkfieldname",strsrcpkfieldname,50);
  //GetXMLBuffer(argv[1],"dstpkfieldname",strdstpkfieldname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"timetvl",&timetvl);
  GetXMLBuffer(argv[1],"and",strand,1000);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);
  GetXMLBuffer(argv[1],"month",strmonth,2);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
 
  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  if (strlen(strsrcpkfieldname) == 0)    { printf("srcpkfieldname is null.\n"); return -1; }
  //if (strlen(strdstpkfieldname) == 0)    { printf("dstpkfieldname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)    { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)   { printf("dstfields is null.\n"); return -1; }
  if (timetvl<=0)   timetvl=30;  // 缺省30秒
  if (timetvl<10)   timetvl=10;  // 最小不能小于10
  // if (strlen(strand) == 0)       { printf("and is null.\n"); return -1; }
  // if (strlen(strignoreerror) == 0)       { printf("ignoreerror is null.\n"); return -1; }
  if ( (maxcount==0) || (maxcount>300) ) maxcount=300;
  // if (strlen(strmonth) == 0)       { printf("month is null.\n"); return -1; }
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");

  // 规范表名和字段名的大小写
  ToUpper(strsrctname);       ToUpper(strdsttname);
  ToLower(strsrcpkfieldname); 
  //ToLower(strdstpkfieldname); 
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
 

  // 判断程序的启动月份
  CheckMonth();

  // 打开日志文件
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_increment3");

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"synctable_increment3",1200);

  // 如果源表中有"@"字符，就一定搞错了源表和目的表的关系
  if (strstr(strsrctname,"@") > 0) 
  { 
    logfile.Write("%s表必段是%s连接中指定的数据库的本地表，不允许指向远程数据库的表。\n",\
                   strsrctname,strconnstr);
    return -1;
  }

  // 连接数据库
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // 目的表表名和链路拆分
  char *strpos=0;
  memset(strdblink,0,sizeof(strdblink));
  memset(strtablename,0,sizeof(strtablename));

  if ((strpos = strstr(strdsttname,"@")) == 0) return FALSE;

  strncpy(strtablename,strdsttname,strlen(strdsttname)-strlen(strpos));
  strncpy(strdblink,strpos,100);  

  // 判断目的表是否存在，如果表不存在，返回失败
  // 不做以下两点判断
  // 判断目的表是否有dstpkfieldname列，如果没有，返回失败
  // 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // 杀死与同步目的表有关的会话，清理出锁
  KillLocked(&conn,strdsttname);

  // 执行数据同步
  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (SyncTable()==FALSE) CallQuit(-1);

    // 如果最长超过了30秒，干脆退出去算了，本程序没必要呆在内存中，浪费资源
    if (timetvl>=30) break;
  }

  return 0;
}

// 判断目的表是否存在，如果表不存在，返回失败
// 不做以下两步判断
// 判断目的表是否有dstpkfieldname列，如果没有，返回失败
// 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // 获取该表全部的字段
  TABFIELD.GetALLField(&conn,strdsttname,0,TRUE);

  if (TABFIELD.m_fieldcount==0)
  {
    logfile.Write("%s表不存在，请创建它。\n",strdsttname); return FALSE;
  }

  // 旧的同步表中有sync_rownum字段，排除它
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rownum,","");
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rownum","");

  // <srcfields>和<dstfields>为空，将以目的表的数据字典中的字段填充。
  if ( (strlen(strsrcfields)==0) && (strlen(strdstfields)==0) )
  {
    strcpy(strsrcfields,TABFIELD.m_allfieldstr); strcpy(strdstfields,TABFIELD.m_allfieldstr);
  }

  lobfieldcount=0;
  // 判断目的表是否有二进制字段，如果有，把字段的个数存放在lobfieldcount变量中
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare(\
     "select count(*) from USER_TAB_COLUMNS%s where table_name=upper('%s') and data_type in ('BLOB','CLOB')",strdblink,strtablename);
  //stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&lobfieldcount);
  stmt.execute();
  stmt.next();

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // 杀死与同步目的表有关的会话，清理出锁
  KillLocked(&conn,strdsttname);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_increment3 exit.\n");

  exit(0);
}

// 获取uMaxKeyIDOld、uMinKeyID、uMaxKeyID的值
BOOL LoadSyncKeyID()
{
  bContinue=FALSE;
  uMaxKeyIDOld=uMinKeyID=uMaxKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select keyid from T_SYNCLOG%s where tname=upper('%s')",strdblink,strtablename);
  //stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&uMaxKeyIDOld);

  if (stmt.execute() != 0)
  {
    // 如果是T_SYNCLOG表不存在，就不理返回失败，在下面会创建它
    if (stmt.cda.rc!=942)
    {
      logfile.Write("select T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
    }

    // T_SYNCLOG表不存在，就创建它
    stmt.prepare("create table T_SYNCLOG%s(tname varchar2(30),keyid number(15),primary key(tname))",strdblink);
    stmt.execute();
  }
  else
  {
    if (stmt.next() != 0) 
    {
      stmt.prepare("insert into T_SYNCLOG%s(tname,keyid) values(upper('%s'),0)",strdblink,strtablename);
      //stmt.bindin(1,strdsttname,50);
      if (stmt.execute() != 0)
      {
        logfile.Write("insert T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
      }

      conn.commitwork();
    }
  }

  // 从数据源表的日志表中获取大于uMaxKeyIDOld的最小keyid
  stmt.prepare("select min(%s) from %s where %s>:1",strsrcpkfieldname,strsrctname,strsrcpkfieldname);
  stmt.bindin(1,&uMaxKeyIDOld);
  stmt.bindout(1,&uMinKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrctname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  // 如果uMinKeyID==0，则表示没有需要同步的记录。
  if (uMinKeyID==0) return TRUE;

  // 从数据源表的日志表中获取大于uMinKeyID+100000的最大keyid
  stmt.prepare("select max(%s) from %s where %s<=:1+100000 %s",strsrcpkfieldname,strsrctname,strsrcpkfieldname,strand);
  stmt.bindin(1,&uMinKeyID);
  stmt.bindout(1,&uMaxKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrctname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  uMinKeyID=uMaxKeyIDOld+1;

  //logfile.Write("uMaxKeyIDOld=%lu\n",uMaxKeyIDOld);
  //logfile.Write("uMinKeyID=%lu\n",uMinKeyID);
  //logfile.Write("uMaxKeyID=%lu\n",uMaxKeyID);

  return TRUE;
}

BOOL UpdateSyncKeyID()
{
  // 更新已同步数据的位置
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("update T_SYNCLOG%s set keyid=:1 where tname=upper('%s')",strdblink,strtablename);
  stmt.bindin(1,&uMaxKeyID);
  //stmt.bindin(2,strdsttname,50);
  if (stmt.execute() != 0)
  {
    logfile.Write("update T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  // 注意，在这里不要提交事务，T_SYNCLOG必须要和数据同步在同一事务中
  return TRUE;
}

#define MAXCOUNT 300

// 执行数据同步
BOOL SyncTable()
{
  // 获取uMaxKeyIDOld、uMinKeyID、uMaxKeyID的值
  if (LoadSyncKeyID() == FALSE) return FALSE;

  // 如果没有需要同步的记录，就直接返回
  if (uMinKeyID==0) return TRUE;

  // 如果数据源数据库对同一张表的插入是并行操作，就有可能造成keyid混乱的情况。
  // 在这里sleep，等待并行事务都提交完成。
  // 但是，sleep(timetvl)并不一定能真的等待全部事务都完成，却可以尽可能避免这种混乱
  if (bFirstStart==TRUE)
  {
    bFirstStart=FALSE; sleep(20);
  }
  else
  {
    if (timetvl<=30) sleep(timetvl);
  }

  CTimer Timer;

  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);

  UINT uMaxCount=0;
  UINT ccount=0;
  UINT keyid,keyidn[MAXCOUNT];

  if (lobfieldcount == 0) uMaxCount=maxcount;
  if (lobfieldcount >  0) uMaxCount=1;

  sqlstatement selsrc,delstmt,insstmt;

  UINT synccount=0;

  selsrc.connect(&conn);
  selsrc.prepare("select %s from %s where %s>=:1 and %s<=:2",strsrcpkfieldname,strsrctname,strsrcpkfieldname,strsrcpkfieldname);
  selsrc.bindin(1,&uMinKeyID);
  selsrc.bindin(2,&uMaxKeyID);
  selsrc.bindout(1,&keyid);

  if (selsrc.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",selsrc.m_sql,selsrc.cda.message); return FALSE;
  }
  
  UINT ii=0;
  char strtemp[11],strBindStr[4096];

  memset(strBindStr,0,sizeof(strBindStr));

  for (ii=0; ii<uMaxCount; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%lu",ii+1);
    if (ii >0) sprintf(strtemp,",:%lu",ii+1);
    strcat(strBindStr,strtemp);
  }

  insstmt.connect(&conn);
  insstmt.prepare("insert into %s(%s) select %s from %s where %s in (%s) %s",strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname,strBindStr,strand);\

  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1,&keyidn[ii]);
  }

  while (TRUE)
  {
    if (selsrc.next() != 0) break;

    keyidn[ccount]=keyid;

    ccount++;

    // 每uMaxCount条记录就插入一次
    if (ccount == uMaxCount)
    {
      if (insstmt.execute() != 0)
      {
        logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

        if ( (insstmt.cda.rc != 1) && (insstmt.cda.rc != 1410) )
        {
          if (strcmp(strignoreerror,"TRUE") != 0) return FALSE;
        }
      }
      else
      {
        synccount=synccount+insstmt.cda.rpc;

        // 如果有二进制字段，就改为一条条提交
        if (lobfieldcount >  0) conn.commitwork();
      }

      ProgramActive.WriteToFile();

      memset(&keyidn,0,sizeof(keyidn));

      ccount=0;
    }
  }

  // 在以上循环处理的时候，如果不足uMaxCount，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("insert into %s(%s) select %s from %s where %s=:1 %s",strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname,strand);
    insstmt.bindin(1,&keyidn[ii]);

    if (insstmt.execute() != 0)
    {
      logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

      if ( (insstmt.cda.rc != 1) && (insstmt.cda.rc != 1410) )
      {
        if (strcmp(strignoreerror,"TRUE") != 0) return FALSE;
      }
    }
    else
    {
      synccount=synccount+insstmt.cda.rpc;

      // 如果有二进制字段，就改为一条条提交
      if (lobfieldcount >  0) conn.commitwork();
    }
  }

  logfile.WriteEx("ok,%lu,rows incrementd(%d).\n",synccount,Timer.Elapsed());

  if (UpdateSyncKeyID()==FALSE) return FALSE;

  if (synccount>10) bContinue=TRUE;

  conn.commitwork();

  return TRUE;
}


// 判断程序的启动月份
void CheckMonth()
{
  if (strlen(strmonth)==0) return;

  char strLocalTime[21];

  // 判断是否匹配当前时间点的月
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd");
  if (strncmp(strLocalTime+4,strmonth,2)==0) return;
  
  // 判断是否匹配当前时间点减一天的月
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd",0-1*24*60*60);
  if (strncmp(strLocalTime+4,strmonth,2)==0) return;
  
  // 休眠一小时后退出
  sleep(1*60*60);
  
  exit(0);
}


