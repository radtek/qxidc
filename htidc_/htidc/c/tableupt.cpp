#include "idcapp.h"

char connstr[101];
char orgtname[51];
char totname[51];
char where[1024];
connection conn;

BOOL _TableUpt();

// ɾ��Ŀ�ı��е�ȫ������
BOOL TruncateTable();

void CallQuit(int sig);

BOOL deleteopt=FALSE;
int  maxcounts=1;

char exceptionfieldstr[301];

int main(int argc,char *argv[])
{
  /*
  char str[50];
  memset(str,0,sizeof(str));
  dfmtochar("114��30'30\"",str);
  printf("=%s=\n",str);
  dfmtochar("114��30",str);
  printf("=%s=\n",str);
  dfmtochar("114��",str);
  printf("=%s=\n",str);

  dfmtochar("14��30'30\"",str);
  printf("=%s=\n",str);
  dfmtochar("14��30",str);
  printf("=%s=\n",str);
  dfmtochar("14��",str);
  printf("=%s=\n",str);
  exit(0);
  */

  if ( (argc != 7)  && (argc != 8) )
  {
    printf("\nUsing:/htidc/htidc/bin/tableupt username/password@tnsname orgtname@dblink totname where deleteopt[TRUE|FALSE] maxcounts [exceptionfieldstr]\n\n"); 

    printf("����һ�����߳������������ݿ⼯Ⱥ֮��Ǩ�����ݣ���Ҫ���ڶԵ�ǰ���Ǩ�ơ�\n");
    printf("username/password@tnsname�ǽ������ݵ����ݿ����Ӳ�������toname�����ڵ����ݿ⡣\n");
    printf("orgtname@dblink������Դ�ı���������dblink��֧��Զ�̱�\n");
    printf("totname������Ŀ�ı�����\n");
    printf("where����Ҫ�������ݵ���������һ��������where�Ӿ䣬���û��where�Ӿ䣬������\"\"��\n");
    printf("deleteopt�Ƿ����Ŀ�ı������е����м�¼��\n");
    printf("maxcounts��ÿ��ִ�����ݸ��Ƶļ�¼����ȡֵ��1-500֮�䡣\n");
    printf("exceptionfieldstr�����ݸ���ʱ���������ֶΣ��ֶ�֮���ð�ǵĶ��ŷָ��������Ǹ���ѡ�ֶΡ�\n\n");
    printf("�ó���Ҫ�󱻲����ı������keyid�ֶΣ�������������ȫ����tablecp1�Ĺ��ܡ�\n\n\n");
    printf("�ó����ȸ����������ҳ���ҪǨ�����ݵ�keyid��ÿ�ΰ���maxcounts��ɾ��Ŀ�ı��ԭ�����ݣ������²�������\n" );

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

  // ���Ŀ�ı�����"@"�ַ�����һ�������Դ���Ŀ�ı�Ĺ�ϵ
  if (strstr(totname,"@") > 0) { printf("totname %s is invalid.\n",totname); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // �������ݿ�
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime);
  fprintf(stdout,"%s:copy table %s to %s...",strLocalTime,orgtname,totname); fflush(stdout);
    
  if (_TableUpt() == FALSE)
  {
    printf("failed.\n"); fflush(stdout);
  }


  return 0;
}

BOOL _TableUpt()
{
  char strColumnStr[2048];
  memset(strColumnStr,0,sizeof(strColumnStr));

  // ��ȡ���ȫ��������Ϣ
  CTABFIELD TABFIELD;
  TABFIELD.GetALLField(&conn,totname,exceptionfieldstr);
  strcpy(strColumnStr,TABFIELD.m_allfieldstr);

  // �жϱ��Ƿ���ڣ����û������Ϣ����Ϳ϶�������
  if (strlen(strColumnStr) == 0) 
  {
    printf("table is not exist.\n"); return FALSE;
  }

  // ɾ��Ŀ�ı��е�ȫ������
  if (TruncateTable() == FALSE) return FALSE;

  CTimer Timer;

  int  ccount=0;
  long ikeyid,ikeyidn[maxcounts];

  sqlstatement selidc,insstmt;

  selidc.connect(&conn);
  selidc.prepare("select keyid from %s %s",orgtname,where);
  selidc.bindout(1,&ikeyid);
  
  if (selidc.execute() != 0)
  {
    printf("%s\n",selidc.m_sql);
    printf("_TableUpt select %s failed.\n%s\n",orgtname,selidc.cda.message); return FALSE;
  }

  int ii=0;
  insstmt.connect(&conn);
  char strSQL1[10241],strSQL2[10241];
  memset(strSQL1,0,sizeof(strSQL1)); memset(strSQL2,0,sizeof(strSQL2));
  sprintf(strSQL1,"delete %s where keyid in (",totname);
  sprintf(strSQL2,"insert into %s(%s) select %s from %s where keyid in (",\
                  totname,strColumnStr,strColumnStr,orgtname);
  char strtemp[11];
  for (ii=0; ii<maxcounts; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%d",ii+1);
    if (ii >0) sprintf(strtemp,",:%d",ii+1);
    strcat(strSQL1,strtemp); strcat(strSQL2,strtemp);
  }
  strcat(strSQL1,")"); strcat(strSQL2,")");
  
  insstmt.prepare(" \
    BEGIN \
      %s; \
      %s; \
    END;",strSQL1,strSQL2);

  for (ii=0; ii<maxcounts; ii++)
  {
    insstmt.bindin(ii+1,&ikeyidn[ii]);
  }

  // ÿmaxcounts��¼�Ͳ���һ��
  while (TRUE)
  {
    if (selidc.next() != 0) break;

    ikeyidn[ccount]=ikeyid;

    ccount++;

    if (ccount == maxcounts)
    {
      if (insstmt.execute() != 0) 
      {
        if (insstmt.cda.rc != 1)
        {
          printf("_TableUpt failed.\n%s\n%s\n",insstmt.cda.message,insstmt.m_sql); return FALSE;
        }
      }

      conn.commitwork();

      ccount=0;
    }
  }

  // ������ѭ�������ʱ���������maxcounts���������ﴦ��
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare(" \
      BEGIN \
        delete %s where keyid=:1; \
        insert into %s(%s) select %s from %s where keyid=:2; \
      END;",totname,totname,strColumnStr,strColumnStr,orgtname);
    insstmt.bindin(1,&ikeyidn[ii]);
    insstmt.bindin(2,&ikeyidn[ii]);
    if (insstmt.execute() != 0) 
    {
      if (insstmt.cda.rc != 1)
      {
        printf("_TableUpt failed.\n%s\n%s\n",insstmt.cda.message,insstmt.m_sql); return FALSE;
      }
    }

    conn.commitwork();
  }

  printf("ok,rows %ld,%ld seconds elapsed.\n",selidc.cda.rpc,Timer.Elapsed()); fflush(stdout);

  return TRUE;
}

// ɾ��Ŀ�ı��е�ȫ������
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

  printf("tableupt exit.\n");

  exit(0);
}


