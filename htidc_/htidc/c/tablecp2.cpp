#include "idcapp.h"

char connstr[101];
char orgtname[51];
char dsttname[51];
connection conn;

BOOL _TableCP();

void CallQuit(int sig);

int  maxcounts=1;

long DstMaxKeyID,OrgBeginKeyID,OrgMaxCPKeyID=0;

// 从目的表中获取已复制记录的最大的keyid
long FetchDstMaxKeyID();

// 从数据源表中获取这一次应该复制的数据的起始位置
long FetchOrgBeginKeyID();

// 从T_TABLECP提取需要复制的记录的最大的keyid
long FetchOrgMaxCPKeyID();

char exceptionfieldstr[301];

int main(int argc,char *argv[])
{
  if ( (argc != 5) && (argc != 6) )
  {
    printf("\nUsing:/htidc/htidc/bin/tablecp2 username/password@tnsname orgtname@dblink dsttname maxcounts [exceptionfieldstr]\n\n"); 

    printf("这是一个工具程序，用于在数据库集群之间迁移数据，主要用于对历史表（静止的表）的增量迁移。\n");
    printf("username/password@tnsname是接收数据的数据库连接参数，即toname表所在的数据库。\n");
    printf("orgtname@dblink是数据源的表名，加上dblink后支持远程表。\n");
    printf("dsttname是数据目的表名。\n");
    printf("maxcounts是每次执行数据复制的记录数，取值在1-500之间。\n");
    printf("exceptionfieldstr是数据复制时不用理会的字段，字段之间用半角的逗号分隔开，这是个可选字段。\n\n\n");

    return -1;
  }

  memset(connstr,0,sizeof(connstr));
  memset(orgtname,0,sizeof(orgtname));
  memset(dsttname,0,sizeof(dsttname));
  memset(exceptionfieldstr,0,sizeof(exceptionfieldstr));

  strcpy(connstr,argv[1]);
  strcpy(orgtname,argv[2]);
  strcpy(dsttname,argv[3]);
  maxcounts=atoi(argv[4]);
  if (argc == 6) strncpy(exceptionfieldstr,argv[5],300);

  if ( (maxcounts<1) || (maxcounts>1000) ) { printf("maxcounts %s is invalid,should in 1-1000.\n",argv[4]); return -1; }

  // 如果目的表中有"@"字符，就一定搞错了源表和目的表的关系
  if (strstr(dsttname,"@") > 0) { printf("dsttname %s is invalid.\n",dsttname); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 连接数据库
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  // 从T_TABLECP提取需要复制的记录的最大的keyid
  if (FetchOrgMaxCPKeyID() != 0) return -1;

  //printf("OrgMaxCPKeyID=%ld\n",OrgMaxCPKeyID);

  while (TRUE)
  {
    // 从目的表中获取已复制记录的最大的keyid
    if (FetchDstMaxKeyID() != 0) break;

    //printf("DstMaxKeyID=%ld\n",DstMaxKeyID);

    // 从数据源表中获取这一次应该复制的数据的起始位置
    if (FetchOrgBeginKeyID() != 0) break;

    //printf("OrgBeginKeyID=%ld\n",OrgBeginKeyID);

    // 如果OrgBeginKeyID为0，表示已无数据可复制。
    if (OrgBeginKeyID==0) break;

    if (_TableCP() == FALSE)
    {
      printf("failed.\n"); break;
    }
  }

  fprintf(stdout,"finish.\n");

  fflush(stdout);

  return 0;
}

BOOL _TableCP()
{
  char strColumnStr[2048];
  memset(strColumnStr,0,sizeof(strColumnStr));

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime);

  printf("%s:copy table %s to %s...",strLocalTime+11,orgtname,dsttname); 

  // 获取表的全部的列信息
  CTABFIELD TABFIELD;
  TABFIELD.GetALLField(&conn,dsttname,exceptionfieldstr);
  strcpy(strColumnStr,TABFIELD.m_allfieldstr);

  CTimer Timer;

  int  ccount=0;
  long ikeyid,ikeyidn[maxcounts];

  sqlstatement selidc,insstmt;

  selidc.connect(&conn);
  selidc.prepare("select keyid from %s where keyid>=%ld and keyid<%ld+100000 and keyid<=%ld order by keyid",orgtname,OrgBeginKeyID,OrgBeginKeyID,OrgMaxCPKeyID);
  selidc.bindout(1,&ikeyid);
  
  if (selidc.execute() != 0)
  {
    printf("%s failed.\n%s\n",selidc.m_sql,selidc.cda.message); return FALSE;
  }

  int ii=0;
  insstmt.connect(&conn);
  char strSQL[10241];
  memset(strSQL,0,sizeof(strSQL));
  sprintf(strSQL,"insert into %s(%s) select %s from %s where keyid in (",\
                  dsttname,strColumnStr,strColumnStr,orgtname);
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
    insstmt.bindin(ii+1,&ikeyidn[ii]);
  }

  // 每maxcounts记录就插入一次
  while (TRUE)
  {
    ikeyid=0;

    if (selidc.next() != 0) break;

    ikeyidn[ccount]=ikeyid;

    ccount++;

    if (ccount == maxcounts)
    {
      if (insstmt.execute() != 0) 
      {
        if (insstmt.cda.rc != 1)
        {
          printf("%s failed.\n%s\n",insstmt.m_sql,insstmt.cda.message); return FALSE;
        }
      }

      memset(&ikeyidn,0,sizeof(ikeyidn));

      ccount=0;
    }
  }

  // 在以上循环处理的时候，如果不足maxcounts，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      insert into %s(%s) select %s from %s where keyid=:1",\
             dsttname,strColumnStr,strColumnStr,orgtname);
    insstmt.bindin(1,&ikeyidn[ii]);
    if (insstmt.execute() != 0) 
    {
      if (insstmt.cda.rc != 1)
      {
        printf("%s failed.\n%s\n",insstmt.m_sql,insstmt.cda.message); return FALSE;
      }
    }
  }

  fprintf(stdout,"ok,rows %ld,%ld seconds elapsed.\n",selidc.cda.rpc,Timer.Elapsed()); 

  fflush(stdout);

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  printf("catching the signal(%d).\n",sig);

  printf("tablecp2 exit.\n");

  exit(0);
}


// 从目的表中获取已复制记录的最大的keyid
long FetchDstMaxKeyID()
{
  DstMaxKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select max(keyid) from %s where keyid<=%ld",dsttname,OrgMaxCPKeyID);
  stmt.bindout(1,&DstMaxKeyID);
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n%s",stmt.m_sql,stmt.cda.message); return stmt.cda.rc;
  }

  stmt.next();

  return 0;
}

// 从数据源表中获取这一次应该复制的数据的起始位置
long FetchOrgBeginKeyID()
{
  OrgBeginKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select min(keyid) from %s where keyid>%ld and keyid<=%ld",orgtname,DstMaxKeyID,OrgMaxCPKeyID);
  stmt.bindout(1,&OrgBeginKeyID);
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n%s",stmt.m_sql,stmt.cda.message); return stmt.cda.rc;
  }

  stmt.next();

  return 0;
}

// 从T_TABLECP提取需要复制的记录的最大的keyid
long FetchOrgMaxCPKeyID()
{
  sqlstatement stmt;
  stmt.connect(&conn);

  stmt.prepare("select keyid from T_TABLECP where tname=upper('%s')",dsttname);
  stmt.bindout(1,&OrgMaxCPKeyID);
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n%s",stmt.m_sql,stmt.cda.message); return stmt.cda.rc;
  }

  if (stmt.next()!=0)
  {
    stmt.prepare("insert into T_TABLECP(tname,keyid) select upper('%s'),max(keyid) from %s",dsttname,orgtname);
    if (stmt.execute() != 0)
    {
      printf("%s failed.\n%s",stmt.m_sql,stmt.cda.message); return stmt.cda.rc;
    }

    stmt.prepare("select keyid from T_TABLECP where tname=upper(:1)");
    stmt.bindin(1,dsttname,30);
    stmt.bindout(1,&OrgMaxCPKeyID);
    if (stmt.execute() != 0)
    {
      printf("%s failed.\n%s",stmt.m_sql,stmt.cda.message); return stmt.cda.rc;
    }
    stmt.next();
  }
  
  return 0;
}
