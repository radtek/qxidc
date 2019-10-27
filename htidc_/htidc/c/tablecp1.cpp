#include "idcapp.h"

char connstr[101];
char orgtname[51];
char totname[51];
char where[1024];
connection conn;

BOOL _TableCP();

// 删除目的表中的全部数据
BOOL TruncateTable();

void CallQuit(int sig);

BOOL deleteopt=FALSE;
int  maxcounts=1;

char exceptionfieldstr[301];

int main(int argc,char *argv[])
{
  if ( (argc != 7)  && (argc != 8) )
  {
    printf("\nUsing:/htidc/htidc/bin/tablecp1 username/password@tnsname orgtname@dblink totname where deleteopt[TRUE|FALSE] maxcounts [exceptionfieldstr]\n\n"); 

    printf("这是一个工具程序，用于在数据库集群之间迁移数据，主要用于对当前表的迁移。\n");
    printf("username/password@tnsname是接收数据的数据库连接参数，即toname表所在的数据库。\n");
    printf("orgtname@dblink是数据源的表名，加上dblink后支持远程表。\n");
    printf("totname是数据目的表名。\n");
    printf("where是需要复制数据的条件，是一个完整的where子句，如果没有where子句，必须用\"\"。\n");
    printf("deleteopt是否清除目的表中现有的记录。\n");
    printf("maxcounts是每次执行数据复制的记录数，取值在1-500之间。\n");
    printf("exceptionfieldstr是数据复制时不用理会的字段，字段之间用半角的逗号分隔开，这是个可选字段。\n\n");

    printf("该程序是根据rowid来复制数据的，不要求有keyid字段。\n");
    printf("如果被操作的表有keyid字段，用tableupt程序可保证数据的完整性，但该程序复制数据比tableupt要快。\n\n\n");

    return -1;
  }

  memset(connstr,0,sizeof(connstr));
  memset(orgtname,0,sizeof(orgtname));
  memset(totname,0,sizeof(totname));
  memset(where,0,sizeof(where));
  deleteopt=FALSE;
  memset(exceptionfieldstr,0,sizeof(exceptionfieldstr));

  strcpy(connstr,argv[1]);
  strcpy(orgtname,argv[2]);
  strcpy(totname,argv[3]);
  strcpy(where,argv[4]);
  if ( (strcmp(argv[5],"TRUE") == 0) || (strcmp(argv[5],"true") == 0) ) deleteopt=TRUE;
  maxcounts=atoi(argv[6]);
  if (argc == 8) strncpy(exceptionfieldstr,argv[7],300);

  if ( (maxcounts<1) || (maxcounts>1000) ) { printf("maxcounts %s is invalid,should in 1-1000.\n",argv[6]); return -1; }

  // 如果目的表中有"@"字符，就一定搞错了源表和目的表的关系
  if (strstr(totname,"@") > 0) { printf("totname %s is invalid.\n",totname); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 连接数据库
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime);
  fprintf(stdout,"%s:copy table %s to %s...",strLocalTime,orgtname,totname); fflush(stdout);
    
  if (_TableCP() == FALSE)
  {
    printf("failed.\n"); fflush(stdout);
  }


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
    printf("table is not exist.\n"); return FALSE;
  }

  // 删除目的表中的全部数据
  if (TruncateTable() == FALSE) return FALSE;

  CTimer Timer;

  int  ccount=0;
  char strrowid[51],strrowidn[maxcounts][51];

  sqlstatement selidc,insstmt;

  selidc.connect(&conn);
  selidc.prepare("select rowid from %s %s",orgtname,where);
  selidc.bindout(1, strrowid,50);
  
  if (selidc.execute() != 0)
  {
    printf("%s\n",selidc.m_sql);
    printf("_TableCP select %s failed.\n%s\n",orgtname,selidc.cda.message); return FALSE;
  }

  int ii=0;
  insstmt.connect(&conn);
  char strSQL[10241];
  memset(strSQL,0,sizeof(strSQL));
  sprintf(strSQL,"insert into %s(%s) select %s from %s where rowid in (",\
                  totname,strColumnStr,strColumnStr,orgtname);
  char strtemp[11];
  for (ii=0; ii<maxcounts; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%d",ii+1);
    if (ii >0) sprintf(strtemp,",:%d",ii+1);
    strcat(strSQL,strtemp);
  }
  strcat(strSQL,")");
  
  insstmt.prepare(strSQL);

  for (ii=0; ii<maxcounts; ii++)
  {
    insstmt.bindin(ii+1,strrowidn[ii],50);
  }

  // 每maxcounts记录就插入一次
  while (TRUE)
  {
    memset(strrowid,0,sizeof(strrowid));

    if (selidc.next() != 0) break;

    strcpy(strrowidn[ccount],strrowid);

    ccount++;

    if (ccount == maxcounts)
    {
      if (insstmt.execute() != 0) 
      {
        if (insstmt.cda.rc != 1)
        {
          printf("_TableCP insert %s failed.\n%s\n",totname,insstmt.cda.message); return FALSE;
        }
      }

      conn.commitwork();

      memset(strrowidn,0,sizeof(strrowidn));

      ccount=0;
    }
  }

  // 在以上循环处理的时候，如果不足maxcounts，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      insert into %s(%s) select %s from %s where rowid=:1",\
             totname,strColumnStr,strColumnStr,orgtname);
    insstmt.bindin(1,strrowidn[ii],50);
    if (insstmt.execute() != 0) 
    {
      if (insstmt.cda.rc != 1)
      {
        printf("_TableCP insert %s failed.\n%s\n",totname,insstmt.cda.message); return FALSE;
      }
    }

    conn.commitwork();
  }

  printf("ok,rows %ld,%ld seconds elapsed.\n",selidc.cda.rpc,Timer.Elapsed()); fflush(stdout);

  return TRUE;
}

// 删除目的表中的全部数据
BOOL TruncateTable()
{
  if (deleteopt == FALSE) return TRUE;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("truncate table %s",totname);

  if (stmt.execute() != 0)
  {
    if ( (stmt.cda.rc==2266) || (stmt.cda.rc==54) )
    {
      stmt.prepare("delete %s",totname);
      if (stmt.execute() != 0)
      {
        printf("delete %s failed.\n%s\n",totname,stmt.cda.message); return FALSE;
      }
    }
    else
    {
      printf("truncate table %s failed.\n%s\n",totname,stmt.cda.message); return FALSE;
    }
  }

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  printf("catching the signal(%d).\n",sig);

  printf("tablecp1 exit.\n");

  exit(0);
}


