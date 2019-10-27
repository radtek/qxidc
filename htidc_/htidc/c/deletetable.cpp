#include "_public.h"
#include "_oracle.h"

char logfilename[301];
char connstr[101];
char tname[51];
char where[1024];
char hourstr[101];
char localhour[21];

connection conn;
CLogFile logfile;
CProgramActive ProgramActive;

BOOL deletetable();

void CallQuit(int sig);

int  maxcounts=500;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\nUsing:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/deletetable \"<logfilename>/log/sqxj/deletetable_ALLAWSDATA.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><tname>T_ALLAWSDATA</tname><where>where ddatetime<sysdate-5</where><hourstr>01,02,03</hourstr>\"\n\n");

    printf("����һ�����߳�������������е����ݡ�\n");
    printf("<logfilename>/log/sqxj/deletetable_ALLAWSDATA.log</logfilename> ������������־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> Ŀ�����ݿ�����Ӳ�����\n");
    printf("<tname>T_ALLAWSDATA</tname> ������ı�����\n");
    printf("<where>where ddatetime<sysdate-5</where> ���������ݵ�������\n");
    printf("<hourstr>01,02,03</hourstr> ������������ʱ�Σ�Сʱ��ʱ��֮���ð�ǵĶ��ŷָ�����\n\n");

    return -1;
  }

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
  if (strlen(tname) == 0)    { printf("tname is null.\n"); return -1; }
  if (strlen(where) == 0)    { printf("where is null.\n"); return -1; }
  if (strlen(hourstr) == 0)    { printf("hourstr is null.\n"); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // ����־�ļ�
  if (logfile.Open(logfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",logfilename); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("deletetable");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"deletetable",1200);

  // �жϵ�ǰʱ���Ƿ�������ʱ��֮��
  memset(localhour,0,sizeof(localhour));
  LocalTime(localhour,"hh24");
  if (strstr(hourstr,localhour)==0) exit(0);

  // �������ݿ�
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    logfile.Write("connect database %s failed.\n",connstr); return -1;
  }

  logfile.Write("delete table %s.\n",tname); 
    
  if (deletetable() == FALSE) logfile.Write("deletetable failed.\n"); 

  return 0;
}

BOOL deletetable()
{
  int  ccount=0;
  char strrowid[51],strrowidn[maxcounts][51];

  sqlstatement selstmt;

  selstmt.connect(&conn);
  selstmt.prepare("select rowid from %s %s",tname,where);
  selstmt.bindout(1, strrowid,50);
  
  if (selstmt.execute() != 0)
  {
    logfile.Write("%s failed.\n%s\n",selstmt.m_sql,selstmt.cda.message); return FALSE;
  }

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
  
  sqlstatement delstmt;
  delstmt.connect(&conn);
  delstmt.prepare(strDeleteSQL);

  for (ii=0; ii<maxcounts; ii++)
  {
    delstmt.bindin(ii+1,strrowidn[ii],50);
  }

  while (TRUE)
  {
    memset(strrowid,0,sizeof(strrowid));

    if (selstmt.next() != 0) break;

    strcpy(strrowidn[ccount],strrowid);

    ccount++;

    if (ccount == maxcounts)
    {
      if (delstmt.execute() != 0) 
      {
        logfile.Write("delete %s failed.\n%s\n",tname,delstmt.cda.message); return FALSE;
      }

      memset(strrowidn,0,sizeof(strrowidn));

      ccount=0;
    }

    if (fmod(selstmt.cda.rpc,100000) < 1) 
    {
      ProgramActive.WriteToFile();

      // �жϵ�ǰʱ���Ƿ�������ʱ��֮��
      memset(localhour,0,sizeof(localhour));
      LocalTime(localhour,"hh24");
      if (strstr(hourstr,localhour)==0) exit(0);

      logfile.Write("rows %d deleted.\n",selstmt.cda.rpc);
    }
  }

  // ������ѭ�������ʱ���������maxcounts���������ﴦ��
  for (ii=0; ii<ccount; ii++)
  {
    delstmt.prepare("delete from %s where rowid=:1",tname);
    delstmt.bindin(1,strrowidn[ii],50);
    if (delstmt.execute() != 0) 
    {
      if (delstmt.cda.rc != 1)
      {
        logfile.Write("delete %s failed.\n%s\n",tname,delstmt.cda.message); return FALSE;
      }
    }
  }

  logfile.Write("rows %d deleted.completed.\n",selstmt.cda.rpc);

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("deletetable exit.\n");

  exit(0);
}


