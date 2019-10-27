#include "idcapp.h"

char strlogfilename[301];
char strconnstr[101];
char strtname[51];
char strlogtname[51];

void CallQuit(int sig);

CLogFile   logfile;
connection conn;
CProgramActive ProgramActive;

// ����ͬ����־�����Ѿ���Ч����־��¼
BOOL InitData();

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./clearsynclog xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 259200 /htidc/htidc/bin/clearsynclog \"<logfilename>/log/sqxj/clearsynclog_ALLAWSDATA_LOG.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><tname>T_ALLAWSDATA</tname><logtname>T_ALLAWSDATA_LOG</logtname>\"\n\n");

    printf("��������������ͬ����־�����Ѿ���Ч����־��¼��\n");
    printf("<logfilename>/log/sqxj/clearsynclog_ALLAWSDATA.log</logfilename> ���������е���־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> ����Դ���ݿ�����Ӳ�����\n");
    printf("<tname>T_ALLAWSDATA</tname> ����Դ������\n");
    printf("<logtname>T_ALLAWSDATA_LOG</logtname> ͬ����־������\n\n");
    printf("�����������ϵͳ����Ա����������Ҳ������procctl���ȣ�����������ʱ����Ҫ�������������̫��"\
           "�����ݿ���Դ��\n\n");

    printf("ע�⣬������Դ����ɾ��������ʱ��ͬ����־���л������Ч��¼����Щ��Ч��¼��ռ�����ݿ��ռ䣬"\
           "��������Ӱ��ͬ����\n");
    printf("crtsynctrigger ��������²���\n"\
           "<syncinsert>true</syncinsert>\n"\
           "<syncupdate>false</syncupdate>\n"\
           "<syncdelete>false</syncdelete>\n"\
           "һ����˵��syncinsert����Ϊtrue���������Ҫͬ��Դ���update��delete��������������Ϊfalse\n"\
           "�ر���syncdelete���������Ҫͬ��ɾ��������syncdelete�������Ϊfalse��"\
           "������ͬ����־���л�����̫�����Ч��¼��\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strtname,0,sizeof(strtname));
  memset(strlogtname,0,sizeof(strlogtname));


  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"tname",strtname,50);
  GetXMLBuffer(argv[1],"logtname",strlogtname,50);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strtname) == 0)    { printf("tname is null.\n"); return -1; }
  if (strlen(strlogtname) == 0) { printf("logtname is null.\n"); return -1; }

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // ����־�ļ�
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("clearsynclog");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"clearsynclog",180);

  // �������ݿ�
  if (conn.connecttodb(strconnstr,FALSE) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // ����ͬ����־�����Ѿ���Ч����־��¼
  if (InitData()==FALSE) CallQuit(-1);

  exit(0);
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("clearsynclog exit.\n");

  exit(0);
}

// ����ͬ����־�����Ѿ���Ч����־��¼
BOOL InitData()
{
  char sync_rowid[31];

  sqlstatement sellog;
  sellog.connect(&conn);
  sellog.prepare("select sync_rowid from %s",strlogtname);
  sellog.bindout(1,sync_rowid,30);

  int ccount=0;
  sqlstatement selchk;
  selchk.connect(&conn);
  selchk.prepare("select count(*) from %s where rowid=:1",strtname);
  selchk.bindin(1,sync_rowid,30);
  selchk.bindout(1,&ccount);

  sqlstatement dellog;
  dellog.connect(&conn);
  dellog.prepare("delete from %s where sync_rowid=:1",strlogtname);
  dellog.bindin(1,sync_rowid,30);

  if (sellog.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strlogtname,sellog.cda.message);
  }

  int idelrows=0;

  logfile.Write("check %s ...\n",strlogtname);

  while (TRUE)
  {
    memset(sync_rowid,0,sizeof(sync_rowid));

    if ( (sellog.cda.rpc>1) && (fmod(sellog.cda.rpc,10000) < 1) )
    {
      if (idelrows>0) logfile.Write("%d rows deleted.\n",idelrows);
      ProgramActive.WriteToFile(); 
      conn.commitwork();
      idelrows=0;
    }

    if (sellog.next() != 0) break;

    selchk.execute();

    ccount=0;

    selchk.next();

    if (ccount==1) continue;

    idelrows++;

    if (dellog.execute() != 0)
    {
      logfile.Write("insert %s failed.\n%s\n",strlogtname,dellog.cda.message);
    }
  }

  logfile.Write("%d rows deleted.\n",idelrows);

  conn.commitwork();

  return TRUE;
}


