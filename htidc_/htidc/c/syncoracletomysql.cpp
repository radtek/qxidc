#include "idcapp.h"
#include "mysql.h"

MYSQL      *mysqlconn = NULL;
connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

BOOL bFirstStart=TRUE;

// 获取源表的所有字段
// 若dstfields为空，将以源表的数据字典中的字段填充。
BOOL CheckDstTName();

// 判断程序的启动月份
void CheckMonth();

// 执行数据同步
BOOL SyncTable();

char strlogfilename[301];
char strmysqlconnstr[201];
char stroracleconnstr[201];
char strcharset[51];
char strsrctname[51];
char strsrcpkfieldname[51];
char strdstpkfieldname[51];
char strsrcfields[4001];
char strdsttname[51];
char strdstfields[4001];
int  timetvl;
char strand[501];
char stralarmbz[21];
char strignoreerror[11];
int  maxcount;
char strmonth[3];

UINT uMaxKeyIDOld;  // 源数据库中，已同步成功的记录的最大keyid
UINT uMinKeyID;     // 本次同步的记录中，最小的keyid。
UINT uMaxKeyID;     // 本次同步的记录中，最大的keyid。

// 获取uMaxKeyIDOld、uMinKeyID、uMaxKeyID的值
BOOL LoadSyncKeyID();

// 把uMaxKeyID的值更新到T_SYNCORACLETOMYSQL表中
BOOL UpdateSyncKeyID();  

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/syncoracletomysql xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/syncoracletomysql \"<logfilename>/log/szqx/syncoracletomysql_LOCALOBTDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><oracleconnstr>szidc/pwdidc@SZQX_10.153.98.31</oracleconnstr><srctname>T_LOCALOBTDATA</srctname><srcpkfieldname>keyid</srcpkfieldname><dstpkfieldname>keyid</dstpkfieldname><srcfields></srcfields><mysqlconnstr>10.153.98.22,3306,admin,ssqx323,gbk,mas</mysqlconnstr><dsttname>T_LOCALOBTDATA</dsttname><dstfields></dstfields><timetvl>10</timetvl><and></and><ignoreerror>FALSE</ignoreerror><maxcount>300</maxcount><month>01</month><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("这是一个工具程序，用于从Oracle数据库同步到MYSQL数据库，同步的方法增量，不能同步修改和删除操作。\n");
    printf("本程序在源ORACLE数据库中创建日志表T_SYNCORACLETOMYS,要求表的主键字段一定是整数，并且为自增型字段。\n");
    printf("<logfilename>/log/szqx/syncoracletomysql_LOCALOBTDATA.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<oracleconnstr>szidc/pwdidc@SZQX_10.153.97.251</oracleconnstr> 源oracle数据库的连接参数。\n");
    printf("<mysqlconnstr>172.22.11.235,3306,gps,gps,gbk,gps_db</mysqlconnstr> 目的mysql数据库的连接参数\
             字段分别为：IP地址,端口,用户密码,字符集,打开的数据库。\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> 源oracle数据库的字符集。\n");
    printf("<srctname>T_LOCALOBTDATA</srctname> 数据源表名。\n");
    printf("<srcpkfieldname>keyid</srcpkfieldname> 数据源表的主键字段名。\n");
    printf("<dstpkfieldname>keyid</dstpkfieldname> 数据目的表的主键字段名。\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> 数据源表的字段列表，用于填充在select和from之间，"\
           "所以，srcfields可以是真实的字段，也可以是任何Oracle的函数或运算。\n");
    printf("<dsttname>T_LOCALOBTDATA</dsttname> 接收数据的目的表名。\n");
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
    printf("  3、如果<timetvl>不超过30秒，本程序常驻内存，如果超过了30秒（包括30秒），执行完一次同步后将退出。\n");
    printf("  4、<maxcount>参数非常重要，如果数据源表有非法数据，当<maxcount>为1时，失败的记录无法同步，但不\n");
    printf("     会影响其它的记录，如果<maxcount>大于1，包含失败记录的同一批次的同步将全部失败。\n");
    printf("  5、不支持同步大字段。\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(stroracleconnstr,0,sizeof(stroracleconnstr));
  memset(strmysqlconnstr,0,sizeof(strmysqlconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcpkfieldname,0,sizeof(strsrcpkfieldname));
  memset(strdstpkfieldname,0,sizeof(strdstpkfieldname));
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
  GetXMLBuffer(argv[1],"oracleconnstr",stroracleconnstr,200);
  GetXMLBuffer(argv[1],"mysqlconnstr",strmysqlconnstr,200);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcpkfieldname",strsrcpkfieldname,50);
  GetXMLBuffer(argv[1],"dstpkfieldname",strdstpkfieldname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,4000);
  GetXMLBuffer(argv[1],"timetvl",&timetvl);
  GetXMLBuffer(argv[1],"and",strand,500);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);
  GetXMLBuffer(argv[1],"month",strmonth,2);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
 
  if (strlen(strlogfilename) == 0)       { printf("logfilename is null.\n"); return -1;    }
  if (strlen(stroracleconnstr) == 0)     { printf("oracleconnstr is null.\n"); return -1;  }
  if (strlen(strcharset) == 0)           { printf("charset is null.\n"); return -1;        }
  if (strlen(strsrctname) == 0)          { printf("srctname is null.\n"); return -1;       }
  if (strlen(strsrcpkfieldname) == 0)    { printf("srcpkfieldname is null.\n"); return -1; }
  if (strlen(strdstpkfieldname) == 0)    { printf("dstpkfieldname is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)          { printf("dsttname is null.\n"); return -1;       }
  if (timetvl<=0)                        timetvl=30;  // 缺省30秒
  if (timetvl<10)                        timetvl=10;  // 最小不能小于10
  if ( (maxcount==0) || (maxcount>300) ) maxcount=300;
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");

  // 规范表名和字段名的大小写
  ToUpper(strsrctname);       ToUpper(strdsttname);
  ToLower(strsrcpkfieldname); ToLower(strdstpkfieldname); 
  ToLower(strsrcfields);      ToLower(strdstfields);

  // 如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。
  if ( (strlen(strsrcfields)==0) || (strlen(strdstfields)==0) )
  {
    memset(strsrcfields,0,sizeof(strsrcfields)); 
    memset(strdstfields,0,sizeof(strdstfields));
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
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("syncoracletomysql");

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"syncoracletomysql",1200);

  // 设置数据库的字符集
  if (strlen(strcharset) != 0) setenv("NLS_LANG",strcharset,1);

  // 获得地址端口,用户名,密码,打开的数据库
  char strdburl[51],strport[8],strdbuser[31],strpassword[31],strcharset[51],strdbname[31];
  int  port;
  memset(strdburl,0,sizeof(strdburl));
  memset(strport,0,sizeof(strport));
  memset(strdbuser,0,sizeof(strdbuser));
  memset(strpassword,0,sizeof(strpassword));
  memset(strcharset,0,sizeof(strcharset));
  memset(strdbname,0,sizeof(strdbname));

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strmysqlconnstr,",");;
  if (CmdStr.CmdCount() != 6)
  {
    logfile.Write("mysqlconnstr(%s) is invalied.\n",strmysqlconnstr); return -1;
  }

  CmdStr.GetValue(0,strdburl,30);
  CmdStr.GetValue(1,strport,8);
  CmdStr.GetValue(2,strdbuser,30);
  CmdStr.GetValue(3,strpassword,30);
  CmdStr.GetValue(4,strcharset,30);
  CmdStr.GetValue(5,strdbname,30);
  port = atoi(strport);

  // 连接MYSQL数据源数据库
  if ( (mysqlconn = mysql_init(NULL)) == NULL )
  {
    logfile.Write( "Init mysql failed\n" ); return -1;
  }

  if ( mysql_real_connect(mysqlconn,strdburl,strdbuser,strpassword,strdbname,port, NULL, 0 ) == NULL )
  {
    logfile.Write( "Connect to db(%s) failed.\n", strmysqlconnstr ); mysql_close(mysqlconn); return -1;
  }

  if ( mysql_set_character_set(mysqlconn, strcharset) != 0 )
  {
    logfile.Write("set character_set(%s) failed:%s\n",strcharset,mysql_error(mysqlconn));
    mysql_close(mysqlconn); return -1;
  }

  // 连接oracle数据库
  if (conn.connecttodb(stroracleconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",stroracleconnstr); return -1;
  }

  // 获取源表的所有字段
  // 若dstfields为空，将以源表的数据字典中的字段填充。
  if (CheckDstTName()==FALSE) CallQuit(-1);

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

// 获取源表的所有字段
// 若dstfields为空，将以源表的数据字典中的字段填充。
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // 获取该表全部的字段
  TABFIELD.GetALLField(&conn,strdsttname);

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

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("syncoracletomysql exit.\n");

  exit(0);
}

// 获取uMaxKeyIDOld、uMinKeyID、uMaxKeyID的值
BOOL LoadSyncKeyID()
{
  uMaxKeyIDOld=uMinKeyID=uMaxKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select keyid from T_SYNCORACLETOMYSQL where tname=upper(:1)");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&uMaxKeyIDOld);
  if (stmt.execute() != 0)
  {
    // 如果是T_SYNCORACLETOMYSQL表不存在，就不理返回失败，在下面会创建它
    if (stmt.cda.rc!=942)
    {
      logfile.Write("select T_SYNCORACLETOMYSQL failed.\n%s\n",stmt.cda.message); return FALSE;
    }

    // T_SYNCORACLETOMYSQL表不存在，就创建它
    stmt.prepare("create table T_SYNCORACLETOMYSQL(tname varchar2(30),keyid number(15),primary key(tname))");
    stmt.execute();
  }
  else
  {
    if (stmt.next() != 0) 
    {
      stmt.prepare("insert into T_SYNCORACLETOMYSQL(tname,keyid) values(upper(:1),0)");
      stmt.bindin(1,strdsttname,50);
      if (stmt.execute() != 0)
      {
        logfile.Write("insert T_SYNCORACLETOMYSQL failed.\n%s\n",stmt.cda.message); return FALSE;
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
  stmt.prepare("select max(%s) from %s where %s<=:1+100000",strsrcpkfieldname,strsrctname,strsrcpkfieldname);
  stmt.bindin(1,&uMinKeyID);
  stmt.bindout(1,&uMaxKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrctname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  uMinKeyID=uMaxKeyIDOld+1;

  return TRUE;
}

BOOL UpdateSyncKeyID()
{
  // 更新已同步数据的位置
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("update T_SYNCORACLETOMYSQL set keyid=:1 where tname=upper(:2)");
  stmt.bindin(1,&uMaxKeyID);
  stmt.bindin(2,strdsttname,50);
  if (stmt.execute() != 0)
  {
    logfile.Write("update T_SYNCORACLETOMYSQL failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  // 注意，在这里不要提交事务，T_SYNCORACLETOMYSQL必须要和数据同步在同一事务中
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
  UINT keyid;
  UINT synccount=0;

  uMaxCount=maxcount;

  sqlstatement selsrc,insstmt;

  selsrc.connect(&conn);
  selsrc.prepare("select %s from %s where %s>=:1 and %s<=:2 %s",strsrcpkfieldname,strsrctname,strsrcpkfieldname,strsrcpkfieldname,strand);
  selsrc.bindin(1,&uMinKeyID);
  selsrc.bindin(2,&uMaxKeyID);
  selsrc.bindout(1,&keyid);

  if (selsrc.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",selsrc.m_sql,selsrc.cda.message); return FALSE;
  }

  // 把字段名用CCmdStr类拆分开
  CCmdStr fieldstr;
  fieldstr.SplitToCmd(strsrcfields,",");

  // 用于存放字段值的数组,最多500个字段，每个字段的长度最多4000。
  char strfieldvalue[500][4001]; 
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  UINT ii=0;

  // 准备获取数据的SQL语句，绑定输出变量
  char strselectsql[4001],strtemp[4001];
  char strBindStr[409600],strinssql[4096];

  // 按照keyid，一条一条获取数据，再一条一条插入到mysql。
  while (TRUE)
  {
    // 获取keyid
    if (selsrc.next() != 0) break;

    memset(strselectsql,0,sizeof(strselectsql));
    snprintf(strselectsql,4000,"select %s from %s where %s=%ld",strsrcfields,strsrctname,strsrcpkfieldname,keyid);

    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare(strselectsql);

    // 数据源有多少个字段，就要绑定多少个输出。
    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      stmt.bindout(ii+1,strfieldvalue[ii],4000);
    }
    
    // 执行查询莸腟QL语句
    if (stmt.execute() != 0)
    {
      logfile.Write("%s\nexec sql failed.\n%s\n",strselectsql,stmt.cda.message); return stmt.cda.rc;
    }

    stmt.next();

    memset(strBindStr,0,sizeof(strBindStr));

    // 拼凑输入值。
    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      memset(strtemp,0,sizeof(strtemp));
      if (ii==0) snprintf(strtemp,4000,"'%s'",strfieldvalue[ii]);
      if (ii >0) snprintf(strtemp,4000,",'%s'",strfieldvalue[ii]);
      strcat(strBindStr,strtemp);
    }

    // 执行数据插入。
    memset(strinssql,0,sizeof(strinssql));
    snprintf(strinssql,4000,"insert into  %s(%s) values(%s)",strdsttname,strdstfields,strBindStr);

    if (mysql_query(mysqlconn,strinssql) != 0)
    {
      logfile.Write("Insert into %s failed: %s\n",strdsttname,mysql_error(mysqlconn));

      // 如果是主键冲突，则继续循环。
      if ( strstr(mysql_error(mysqlconn),"PRIMARY") != 0) continue;

      else
        return FALSE;
    }

    synccount++;
  }

  mysql_commit(mysqlconn);

  logfile.WriteEx("ok,%lu rows incrementd(%d).\n",synccount,Timer.Elapsed());

  if (UpdateSyncKeyID()==FALSE) return FALSE;

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


