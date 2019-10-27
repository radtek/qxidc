#include "_public.h"
#include "_ooci.h"

char logfilename[301];
char connstr[101];
char tname[51];
char where[1024];
char hourstr[101];
char localhour[21];

connection conn;
CLogFile logfile;

void EXIT(int sig);

// 显示程序的帮助
void _help(char *argv[]);

// 每批删除的记录数
int  maxcounts=1;

bool _deletetables();

int main(int argc,char *argv[])
{
  if (argc != 2) { _help(argv); return -1; }

  memset(logfilename,0,sizeof(logfilename));
  memset(connstr,0,sizeof(connstr));
  memset(tname,0,sizeof(tname));
  memset(where,0,sizeof(where));
  memset(hourstr,0,sizeof(hourstr));

  GetXMLBuffer(argv[1],"logfilename",logfilename,300);
  GetXMLBuffer(argv[1],"connstr",connstr,100);
  GetXMLBuffer(argv[1],"tname",tname,50);
  GetXMLBuffer(argv[1],"where",where,1000);
  GetXMLBuffer(argv[1],"hourstr",hourstr,2000);

  if (strlen(logfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(connstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(tname) == 0)       { printf("tname is null.\n"); return -1; }
  if (strlen(where) == 0)       { printf("where is null.\n"); return -1; }
  if (strlen(hourstr) == 0)     { printf("hourstr is null.\n"); return -1; }

  // 关闭全部的信号和输入输出
  CloseIOAndSignal();

  // 处理程序退出的信号
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开日志文件
  if (logfile.Open(logfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",logfilename); return -1;
  }

  while (true)
  {
    // 判断当前时间是否在启动时间之内
    memset(localhour,0,sizeof(localhour));
    LocalTime(localhour,"hh24");
    if (strstr(hourstr,localhour)==0) { sleep(60); continue; }

    // 连接数据库
    if (conn.connecttodb(connstr,"Simplified Chinese_China.ZHS16GBK") != 0)
    {
      logfile.Write("connect database %s failed.\n",connstr); sleep(60); continue; 
    }

    logfile.Write("delete table %s.\n",tname);

    if (_deletetables() == false) logfile.Write("deletetables failed.\n"); 

    conn.disconnect();

    sleep(60); 
  }

  return 0;
}

void EXIT(int sig)
{
  printf("程序退出，sig=%d\n\n",sig);

  exit(0);
}


// 显示程序的帮助
void _help(char *argv[])
{
  printf("\nUsing:/htidc/public/bin/deletetables \"<logfilename>/log/shqx/deletetables_SURFDATA.log</logfilename><connstr>shqx/pwdidc@snorcl11g_198</connstr><tname>T_SURFDATA</tname><where>where ddatetime<sysdate-0.7</where><hourstr>11,12,13</hourstr>\"\n\n");

  printf("这是一个工具程序，用于清理表中的数据。\n");
  printf("<logfilename>/log/shqx/deletetables_ALLAWSDATA.log</logfilename> 本程序运行日志文件名。\n");
  printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 目的数据库的连接参数。\n");
  printf("<tname>T_ALLAWSDATA</tname> 待清理的表名。\n");
  printf("<where>where ddatetime<sysdate-5</where> 待清理数据的条件。\n");
  printf("<hourstr>01,02,03</hourstr> 本程序启动的时次，小时，时次之间用半角的逗号分隔开。\n\n");

  return;
}

bool _deletetables()
{
  int  ccount=0;
  char strrowid[51],strrowidn[maxcounts][51];

  // 获取符合条件的记录的rowid
  sqlstatement selstmt(&conn);
  selstmt.prepare("select rowid from %s %s",tname,where);
  selstmt.bindout(1, strrowid,50);

  if (selstmt.execute() != 0)
  {
    logfile.Write("%s failed.\n%s\n",selstmt.m_sql,selstmt.m_cda.message); return false;
  }

  // 生成删除数据的SQL语句，一次删除maxcounts条记录。
  int ii=0;
  char strDeleteSQL[10241];
  memset(strDeleteSQL,0,sizeof(strDeleteSQL));

  sprintf(strDeleteSQL,"delete from %s where rowid in (",tname);

  char strtemp[11];
  for (ii=0; ii<maxcounts; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%d",ii+1);
    if (ii >0) sprintf(strtemp,",:%d",ii+1);
    strcat(strDeleteSQL,strtemp);
  }
  strcat(strDeleteSQL,")");

  sqlstatement delstmt(&conn);
  delstmt.prepare(strDeleteSQL);

  for (ii=0; ii<maxcounts; ii++)
  {
    delstmt.bindin(ii+1,strrowidn[ii],50);
  }

  while (true)
  {
    memset(strrowid,0,sizeof(strrowid));

    if (selstmt.next() != 0) break;

    strcpy(strrowidn[ccount],strrowid);

    ccount++;

    if (ccount == maxcounts)
    {
      if (delstmt.execute() != 0)
      {
        logfile.Write("delete %s failed.\n%s\n",tname,delstmt.m_cda.message); return false;
      }

      memset(strrowidn,0,sizeof(strrowidn));

      ccount=0;
    }

    if (fmod(selstmt.m_cda.rpc,10000) < 1)
    {
      logfile.Write("rows %d deleted.\n",selstmt.m_cda.rpc);

      conn.commit();

      // 判断当前时间是否在启动时间之内
      memset(localhour,0,sizeof(localhour));
      LocalTime(localhour,"hh24");
      if (strstr(hourstr,localhour)==0) return true;
    }
  }

  // 在以上循环处理的时候，如果不足maxcounts，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    delstmt.prepare("delete from %s where rowid=:1",tname);
    delstmt.bindin(1,strrowidn[ii],50);

    if (delstmt.execute() != 0)
    {
      if (delstmt.m_cda.rc != 1)
      {
        logfile.Write("delete %s failed.\n%s\n",tname,delstmt.m_cda.message); return false;
      }
    }
  }

  logfile.Write("rows %d deleted.completed.\n",selstmt.m_cda.rpc);

  conn.commit();

  return true;
}

