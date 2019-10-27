#include "idcapp.h"

void CallQuit(int sig);

class CSESSIONSTAT
{
public:
  char   m_hostname[31];
  struct st_SESSIONSTAT
  {
    char hostname[31];
    int  sid;
    char machine[51];
    char osuser[31];
    char schemaname[31];
    char process[31];
    char program[101];
    char status[31];
    char logontime[20];
  };

  st_SESSIONSTAT m_stSESSIONSTAT;
  vector<struct st_SESSIONSTAT> m_vSESSIONSTAT;

  connection *m_conn;
  CLogFile   *m_logfile;

  CSESSIONSTAT();
 ~CSESSIONSTAT();


  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // ��ȡ���ݿ�ĻỰ��Ϣ
  BOOL GETSESSIONSTAT();

  // �����ݿ�ĻỰ��Ϣ�����������ĵı���
  long UptSESSIONSTAT();

  // ���������ļ��ռ��������Ƿ�ﵽm_AlarmValue������У������ɸ澯
  UINT m_AlarmValue;
  UINT m_sessions;
};


char strDSTConnStr[201]; // ���ݿ����Ӳ���
char strIDCConnStr[201]; // ���ݿ����Ӳ���
char strHostName[31]; 
char strAlarmValue[11];

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
CSESSIONSTAT   SESSIONSTAT;

int main(int argc,char *argv[])
{
  if (argc != 5) 
  {
    printf("\n");
    printf("Using:./sessionstat hostname src:username/password@tnsname idc:username/password@tnsname alarmvalue\n");
    printf("Example:/htidc/htidc/bin/procctl 80 /htidc/htidc/bin/sessionstat 10.153.97.14 szidc/pwdidc@SZQX_10.153.97.14 hssms/smspwd@EJETDB_221.179.6.136 200\n\n");

    printf("�˳���������ȡ��������ݿ�ĻỰ��Ϣ�����ѽ��������������ĵ�T_SESSIONSTAT��\n");
    printf("hostname�Ǵ���ط���������������Ϊ�˷���ʶ��Ҳ������IP��\n");
    printf("src:username/password@tnsname�Ǵ�������ݿ�����Ӳ�����\n");
    printf("idc:username/password@tnsname�������������ݿ�����Ӳ�����\n");
    printf("alarmvalue�ǲ����澯�ķ�ֵ�����Ự�����������ֵʱ�������澯��\n\n\n");

    return -1;
  }

  memset(strHostName,0,sizeof(strHostName));
  memset(strDSTConnStr,0,sizeof(strDSTConnStr));
  memset(strIDCConnStr,0,sizeof(strIDCConnStr));
  memset(strAlarmValue,0,sizeof(strAlarmValue));

  strncpy(strHostName,argv[1],30);
  strncpy(strDSTConnStr,argv[2],50);
  strncpy(strIDCConnStr,argv[3],50);
  strncpy(strAlarmValue,argv[4],10);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open("/tmp/htidc/log/sessionstat.log","a+") == FALSE)
  {
    printf("/tmp/htidc/log/sessionstat.log failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("sessionstat");

  ProgramActive.SetProgramInfo(&logfile,"sessionstat",180);

  // �����������ĵ����ݿ�
  if (connidc.connecttodb(strIDCConnStr,TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) idc db failed.\n",strIDCConnStr); CallQuit(-1);
  }

  // ����Զ�����ݿ�
  if (conndst.connecttodb(strDSTConnStr) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) dst db failed.\n",strDSTConnStr); CallQuit(-1);
  }

  SESSIONSTAT.BindConnLog(&conndst,&logfile);

  // ��ȡԶ�����ݿ�Ự����������
  if (SESSIONSTAT.GETSESSIONSTAT() == FALSE)
  {
    logfile.Write("SESSIONSTAT.GETSESSIONSTAT failed.\n"); CallQuit(-1);
  }

  // ��ʹ���������ĵ����ݿ�����
  SESSIONSTAT.BindConnLog(&connidc,&logfile);

  SESSIONSTAT.m_AlarmValue=atoi(strAlarmValue);

  strcpy(SESSIONSTAT.m_hostname,strHostName);

  // �����ݿ�Ự���������µ�����
  if (SESSIONSTAT.UptSESSIONSTAT() != 0)
  {
    logfile.Write("SESSIONSTAT.UptSESSIONSTAT failed.\n"); CallQuit(-1);
  }

  // �����Ƿ��г�����ֵ�����ݿ�Ự��������У������ɸ澯
  if (SESSIONSTAT.m_sessions > (UINT)atoi(strAlarmValue))
  {
    logfile.Write("failed.%s�����ݿ������������%lu,�����˸澯��ֵ%s��\n",SESSIONSTAT.m_hostname,SESSIONSTAT.m_sessions,strAlarmValue);
  }
  else
  {
    logfile.Write("%s�����ݿ������������%lu,�澯��ֵ%s��\n",SESSIONSTAT.m_hostname,SESSIONSTAT.m_sessions,strAlarmValue);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("sessionstat exit.\n");

  exit(0);
}


CSESSIONSTAT::CSESSIONSTAT()
{
  m_conn=0; m_logfile=0;
  memset(m_hostname,0,sizeof(m_hostname));
  m_AlarmValue=0;
}

CSESSIONSTAT::~CSESSIONSTAT()
{
  m_vSESSIONSTAT.clear();
}

void CSESSIONSTAT::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn = in_conn; m_logfile = in_logfile;
}

// ��ȡ���ݿ�ĻỰ��Ϣ
BOOL CSESSIONSTAT::GETSESSIONSTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
   select sid,machine,osuser,schemaname,process,program,status,\
          to_char(logon_time,'yyyymmddhh24miss') from V$SESSION\
    where type!='BACKGROUND' and status!='KILLED'order by machine,osuser,schemaname,process,program,status,logon_time");
  stmt.bindout(1,&m_stSESSIONSTAT.sid);
  stmt.bindout(2, m_stSESSIONSTAT.machine,50);
  stmt.bindout(3, m_stSESSIONSTAT.osuser,30);
  stmt.bindout(4, m_stSESSIONSTAT.schemaname,30);
  stmt.bindout(5, m_stSESSIONSTAT.process,30);
  stmt.bindout(6, m_stSESSIONSTAT.program,100);
  stmt.bindout(7, m_stSESSIONSTAT.status,30);
  stmt.bindout(8, m_stSESSIONSTAT.logontime,20);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("GETSESSIONSTAT select V$SESSION failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  m_vSESSIONSTAT.clear();

  m_sessions=0;

  while (TRUE)
  {
    memset(&m_stSESSIONSTAT,0,sizeof(m_stSESSIONSTAT));

    if (stmt.next() != 0) break;

    m_vSESSIONSTAT.push_back(m_stSESSIONSTAT);

    m_sessions++;
  }

  return TRUE;
}

// �����ݿ�ĻỰ��Ϣ�����������ĵı���
long CSESSIONSTAT::UptSESSIONSTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("delete T_SESSIONSTAT where hostname=:1");
  stmt.bindin(1,m_hostname,30);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptSESSIONSTAT delete T_SESSIONSTAT failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(m_conn,strLocalTime,"yyyymmddhh24miss");

  stmt.prepare("insert into T_SESSIONSTAT(hostname,sid,machine,osuser,schemaname,process,program,status,logontime,alarmvalue,ddatetime) values(:1,:2,:3,:4,:5,:6,:7,:8,to_date(:9,'yyyymmddhh24miss'),:10,to_date(:11,'yyyymmddhh24miss'))");
  stmt.bindin( 1, m_hostname,30);
  stmt.bindin( 2,&m_stSESSIONSTAT.sid);
  stmt.bindin( 3, m_stSESSIONSTAT.machine,50);
  stmt.bindin( 4, m_stSESSIONSTAT.osuser,30);
  stmt.bindin( 5, m_stSESSIONSTAT.schemaname,30);
  stmt.bindin( 6, m_stSESSIONSTAT.process,30);
  stmt.bindin( 7, m_stSESSIONSTAT.program,100);
  stmt.bindin( 8, m_stSESSIONSTAT.status,30);
  stmt.bindin( 9, m_stSESSIONSTAT.logontime,20);
  stmt.bindin(10,&m_AlarmValue);
  stmt.bindin(11, strLocalTime,14);

  for (UINT ii=0;ii<m_vSESSIONSTAT.size();ii++)
  {
    memcpy(&m_stSESSIONSTAT,&m_vSESSIONSTAT[ii],sizeof(m_stSESSIONSTAT));

    if (stmt.execute() != 0)
    {
      m_logfile->Write("UptSESSIONSTAT insert T_SESSIONSTAT failed.\n%s\n",stmt.cda.message);
      return stmt.cda.rc;
    }
  }

  return 0;
}


