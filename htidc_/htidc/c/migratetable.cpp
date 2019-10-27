#include "idcapp.h"

char connstr[101];
char orgtname[51];
char totname[51];
char where[1024];
connection conn;

CLogFile logfile;

BOOL _TableCP();

void CallQuit(int sig);

int  maxcounts=1;

char exceptionfieldstr[301];

char hourstr[101],localhour[21];

int main(int argc,char *argv[])
{
  if (argc != 7)
  {
    printf("\nUsing:/htidc/htidc/bin/migratetable username/password@tnsname orgtname@dblink totname where maxcounts hourstr\n\n"); 

    printf("这是一个工具程序，用于在数据库集群之间迁移数据，主要用于对当前表的迁移。\n");
    printf("username/password@tnsname是接收数据的数据库连接参数，即toname表所在的数据库。\n");
    printf("orgtname@dblink是数据源的表名，加上dblink后支持远程表。\n");
    printf("totname是数据目的表名。\n");
    printf("where是需要复制数据的条件，是一个完整的where子句，如果没有where子句，必须用\"\"。\n");
    printf("maxcounts是每次执行数据复制的记录数，取值在1-500之间。\n");
    printf("hourstr是程序启动的小时，小时之间用半角的逗号分隔开，如\"19,20,21,22,23,00,01,02,03,04,05,06,07,08\"\n\n\n");

    return -1;
  }

  memset(connstr,0,sizeof(connstr));
  memset(orgtname,0,sizeof(orgtname));
  memset(totname,0,sizeof(totname));
  memset(where,0,sizeof(where));
  memset(hourstr,0,sizeof(hourstr));

  strcpy(connstr,argv[1]);
  strcpy(orgtname,argv[2]);
  strcpy(totname,argv[3]);
  strcpy(where,argv[4]);
  maxcounts=atoi(argv[5]);
  strcpy(hourstr,argv[6]);

  if ( (maxcounts<1) || (maxcounts>1000) ) { printf("maxcounts %s is invalid,should in 1-1000.\n",argv[6]); return -1; }

  // 如果目的表中有"@"字符，就一定搞错了源表和目的表的关系
  if (strstr(totname,"@") > 0) { printf("totname %s is invalid.\n",totname); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[301];
  memset(strLogFileName,0,sizeof(strLogFileName));
  sprintf(strLogFileName,"/tmp/htidc/log/migratetable_%s.log",totname);
  logfile.Open(strLogFileName,"a+");

  // 判断当前时间是否在启动时间之内
  memset(localhour,0,sizeof(localhour));
  LocalTime(localhour,"hh24");
  if (strstr(hourstr,localhour)==0) exit(0);

  // 连接数据库
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    logfile.Write("connect database %s failed.\n",connstr); return -1;
  }

  logfile.Write("migrate table %s to %s.\n",orgtname,totname); 
    
  if (_TableCP() == FALSE) logfile.Write("_TableCP failed.\n"); 

  return 0;
}

BOOL _TableCP()
{
  char strColumnStr[2048];
  memset(strColumnStr,0,sizeof(strColumnStr));

  // 获取表的全部的列信息
  CTABFIELD TABFIELD;
  TABFIELD.GetALLField(&conn,totname,exceptionfieldstr);
  strcpy(strColumnStr,TABFIELD.m_allfieldstr);

  // 判断表是否存在，如果没有列信息，表就肯定不存在
  if (strlen(strColumnStr) == 0) 
  {
    logfile.Write("table is not exist.\n"); return FALSE;
  }

  int  ccount=0;
  char strrowid[51],strrowidn[maxcounts][51];

  sqlstatement selstmt;

  selstmt.connect(&conn);
  selstmt.prepare("select rowid from %s %s",orgtname,where);
  selstmt.bindout(1, strrowid,50);
  
  if (selstmt.execute() != 0)
  {
    logfile.Write("%s failed.\n%s\n",selstmt.m_sql,selstmt.cda.message); return FALSE;
  }

  int ii=0;
  char strInsertSQL[10241],strDeleteSQL[10241];
  memset(strInsertSQL,0,sizeof(strInsertSQL));
  memset(strDeleteSQL,0,sizeof(strDeleteSQL));

  sprintf(strInsertSQL,"insert into %s(%s) select %s from %s where rowid in (",totname,strColumnStr,strColumnStr,orgtname);
  sprintf(strDeleteSQL,"delete from %s where rowid in (",orgtname);

  char strtemp[11];
  for (ii=0; ii<maxcounts; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%d",ii+1);
    if (ii >0) sprintf(strtemp,",:%d",ii+1);
    strcat(strInsertSQL,strtemp);
    strcat(strDeleteSQL,strtemp);
  }
  strcat(strInsertSQL,")");
  strcat(strDeleteSQL,")");
  
  sqlstatement insstmt,delstmt;
  insstmt.connect(&conn);
  insstmt.prepare(strInsertSQL);

  delstmt.connect(&conn);
  delstmt.prepare(strDeleteSQL);

  for (ii=0; ii<maxcounts; ii++)
  {
    insstmt.bindin(ii+1,strrowidn[ii],50);
    delstmt.bindin(ii+1,strrowidn[ii],50);
  }

  // 每maxcounts记录就插入一次
  while (TRUE)
  {
    memset(strrowid,0,sizeof(strrowid));

    if (selstmt.next() != 0) break;

    strcpy(strrowidn[ccount],strrowid);

    ccount++;

    if (ccount == maxcounts)
    {
      if (insstmt.execute() != 0) 
      {
        if (insstmt.cda.rc != 1)
        {
          logfile.Write("_TableCP insert %s failed.\n%s\n",totname,insstmt.cda.message); return FALSE;
        }
      }

      if (delstmt.execute() != 0) 
      {
        logfile.Write("_TableCP delete %s failed.\n%s\n",totname,insstmt.cda.message); return FALSE;
      }

      conn.commitwork();

      memset(strrowidn,0,sizeof(strrowidn));

      ccount=0;
    }

    if (fmod(selstmt.cda.rpc,100000) < 1) 
    {
      // 判断当前时间是否在启动时间之内
      memset(localhour,0,sizeof(localhour));
      LocalTime(localhour,"hh24");
      if (strstr(hourstr,localhour)==0) exit(0);

      logfile.Write("%s to %s ok(%d).\n",orgtname,totname,selstmt.cda.rpc);
    }
  }

  // 在以上循环处理的时候，如果不足maxcounts，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      BEGIN\
        insert into %s(%s) select %s from %s where rowid=:1;\
        delete from %s where rowid=:2;\
      END;",totname,strColumnStr,strColumnStr,orgtname,orgtname);
    insstmt.bindin(1,strrowidn[ii],50);
    insstmt.bindin(2,strrowidn[ii],50);
    if (insstmt.execute() != 0) 
    {
      if (insstmt.cda.rc != 1)
      {
        logfile.Write("_TableCP insert %s or delete %s failed.\n%s\n",totname,orgtname,insstmt.cda.message); return FALSE;
      }
    }

    conn.commitwork();
  }

  logfile.Write("%s to %s finish(%d).\n",orgtname,totname,selstmt.cda.rpc);

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("migratetable exit.\n");

  exit(0);
}


