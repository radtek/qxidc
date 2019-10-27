#include "idcapp.h"

char connstr[101];
char orgtname[51];
char dsttname[51];
connection conn;

BOOL _TableCP();

void CallQuit(int sig);

int  maxcounts=1;

long DstMaxKeyID,OrgBeginKeyID,OrgMaxCPKeyID=0;

// ��Ŀ�ı��л�ȡ�Ѹ��Ƽ�¼������keyid
long FetchDstMaxKeyID();

// ������Դ���л�ȡ��һ��Ӧ�ø��Ƶ����ݵ���ʼλ��
long FetchOrgBeginKeyID();

// ��T_TABLECP��ȡ��Ҫ���Ƶļ�¼������keyid
long FetchOrgMaxCPKeyID();

char exceptionfieldstr[301];

int main(int argc,char *argv[])
{
  if ( (argc != 5) && (argc != 6) )
  {
    printf("\nUsing:/htidc/htidc/bin/tablecp2 username/password@tnsname orgtname@dblink dsttname maxcounts [exceptionfieldstr]\n\n"); 

    printf("����һ�����߳������������ݿ⼯Ⱥ֮��Ǩ�����ݣ���Ҫ���ڶ���ʷ����ֹ�ı�������Ǩ�ơ�\n");
    printf("username/password@tnsname�ǽ������ݵ����ݿ����Ӳ�������toname�����ڵ����ݿ⡣\n");
    printf("orgtname@dblink������Դ�ı���������dblink��֧��Զ�̱�\n");
    printf("dsttname������Ŀ�ı�����\n");
    printf("maxcounts��ÿ��ִ�����ݸ��Ƶļ�¼����ȡֵ��1-500֮�䡣\n");
    printf("exceptionfieldstr�����ݸ���ʱ���������ֶΣ��ֶ�֮���ð�ǵĶ��ŷָ��������Ǹ���ѡ�ֶΡ�\n\n\n");

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

  // ���Ŀ�ı�����"@"�ַ�����һ�������Դ���Ŀ�ı�Ĺ�ϵ
  if (strstr(dsttname,"@") > 0) { printf("dsttname %s is invalid.\n",dsttname); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // �������ݿ�
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  // ��T_TABLECP��ȡ��Ҫ���Ƶļ�¼������keyid
  if (FetchOrgMaxCPKeyID() != 0) return -1;

  //printf("OrgMaxCPKeyID=%ld\n",OrgMaxCPKeyID);

  while (TRUE)
  {
    // ��Ŀ�ı��л�ȡ�Ѹ��Ƽ�¼������keyid
    if (FetchDstMaxKeyID() != 0) break;

    //printf("DstMaxKeyID=%ld\n",DstMaxKeyID);

    // ������Դ���л�ȡ��һ��Ӧ�ø��Ƶ����ݵ���ʼλ��
    if (FetchOrgBeginKeyID() != 0) break;

    //printf("OrgBeginKeyID=%ld\n",OrgBeginKeyID);

    // ���OrgBeginKeyIDΪ0����ʾ�������ݿɸ��ơ�
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

  // ��ȡ���ȫ��������Ϣ
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

  // ÿmaxcounts��¼�Ͳ���һ��
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

  // ������ѭ�������ʱ���������maxcounts���������ﴦ��
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


// ��Ŀ�ı��л�ȡ�Ѹ��Ƽ�¼������keyid
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

// ������Դ���л�ȡ��һ��Ӧ�ø��Ƶ����ݵ���ʼλ��
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

// ��T_TABLECP��ȡ��Ҫ���Ƶļ�¼������keyid
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
