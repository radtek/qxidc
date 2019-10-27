#include "idcapp.h"

void CallQuit(int sig);

char strLogPath[201]; // ��־�ļ�Ŀ¼
char strTmpPath[201]; // ���̻��Ϣ�ļ�Ŀ¼
char strIDCConnStr[201]; // ���ݿ����Ӳ���
char strHostName[31]; 
char strAlarmValue[11];
double dusep=0.00;   // CPU��������

class CCPUSTAT
{
public:
  char   m_hostname[31];
  struct st_CPUSTAT
  {
    char cpu[11];
    UINT user;
    UINT system;
    UINT nice;
    char idle[16];
    UINT iowait;
    UINT irq;
    UINT softirq;
  };

  st_CPUSTAT m_stCPUSTAT;
  vector<struct st_CPUSTAT> m_vCPUSTAT;

  connection *m_conn;
  CLogFile   *m_logfile;

  CCPUSTAT();
 ~CCPUSTAT();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // ��ȡCPU����
  BOOL GETCPUSTAT();

  // ��CPU������µ�����
  long UptCPUSTAT();

  // ����CPU�������Ƿ�ﵽm_AlarmValue������У������ɸ澯
  UINT m_AlarmValue;

  char m_AlarmSTR[1024];
  void AlanyCPUSTAT();
};

connection     conn;
CLogFile       logfile;
CCPUSTAT       CPUSTAT;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./cpustat username/password@tnsname hostname alarmvalue\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/cpustat hssms/smspwd@EJETDB_221.179.6.136 61.235.114.95 50\n\n");
    printf("�˳������ڰѱ���������CPUʹ������Ϣ��������ݿ�T_CPUSTAT������������Ҫ��صķ������ϡ�\n");
    printf("username/password@tnsname��ָ�������ݿ������ݿ�����Ӳ�����\n");
    printf("hostname�Ǳ�����������������Ϊ�˷���ʶ��Ҳ������IP��\n");
    printf("alarmvalue�ǲ����澯�ķ�ֵ����CPUʹ���ʳ������ֵʱ�������澯��\n\n\n");
 
    return -1;
  }

  memset(strHostName,0,sizeof(strHostName));
  memset(strAlarmValue,0,sizeof(strAlarmValue));

  strncpy(strHostName,argv[2],20);
  strncpy(strAlarmValue,argv[3],10);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open("/tmp/htidc/log/cpustat.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/cpustat.log) failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("cpustat");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"cpustat",180);

  // �������ݿ⣬�����Զ��ύ�ķ�ʽ������Ҫ������ơ�
  if (conn.connecttodb(argv[1],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[1]); return -1;
  }

  CPUSTAT.BindConnLog(&conn,&logfile);

  CPUSTAT.m_AlarmValue=atoi(strAlarmValue);

  strcpy(CPUSTAT.m_hostname,strHostName);

  // ��ȡCPU����
  if (CPUSTAT.GETCPUSTAT() == FALSE)
  {
    logfile.Write("CPUSTAT.GETCPUSTAT failed.\n"); return -1;
  }

  // ��CPU������µ�����
  if (CPUSTAT.UptCPUSTAT() != 0)
  {
    logfile.Write("CPUSTAT.UptCPUSTAT failed.\n"); return -1;
  }

  // �����Ƿ��г�����ֵ��CPU�ռ������ʣ�����У������ɸ澯
  CPUSTAT.AlanyCPUSTAT();

  if ( strlen(CPUSTAT.m_AlarmSTR) > 0)
  {
    logfile.Write("��������%s��%s\n",CPUSTAT.m_hostname,CPUSTAT.m_AlarmSTR);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("cpustat exit.\n");

  exit(0);
}


CCPUSTAT::CCPUSTAT()
{
  memset(m_hostname,0,sizeof(m_hostname));
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));
  m_conn=0; m_logfile=0;
}

CCPUSTAT::~CCPUSTAT()
{
  m_vCPUSTAT.clear();
}

void CCPUSTAT::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn; m_logfile=in_logfile;
}

BOOL CCPUSTAT::GETCPUSTAT()
{
  FILE *fp=0;

  if ( (fp=FOPEN("/proc/stat","r")) == NULL )
  {
    m_logfile->Write("open /proc/stat failed.\n"); return FALSE;
  }

  m_vCPUSTAT.clear();

  char strLine[512];

  CCmdStr CmdStr;

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
    memset(&m_stCPUSTAT,0,sizeof(m_stCPUSTAT));

    if (FGETS(strLine,500,fp) == FALSE) break;

    // ɾ���ַ���ǰ��Ŀո�
    Trim(strLine);

    // ���ַ����м�Ķ���ո�ȫ��ת��Ϊһ���ո�
    UpdateStr(strLine,"  "," ");

    CmdStr.SplitToCmd(strLine," ");

    CmdStr.GetValue(0, m_stCPUSTAT.cpu,10);
    CmdStr.GetValue(1,&m_stCPUSTAT.user);
    CmdStr.GetValue(2,&m_stCPUSTAT.system);
    CmdStr.GetValue(3,&m_stCPUSTAT.nice);
    CmdStr.GetValue(4, m_stCPUSTAT.idle,15);
    CmdStr.GetValue(5,&m_stCPUSTAT.iowait);
    CmdStr.GetValue(6,&m_stCPUSTAT.irq);
    CmdStr.GetValue(7,&m_stCPUSTAT.softirq);

    // ��CPU������ת����Сд
    ToLower(m_stCPUSTAT.cpu);

    if (strcmp(m_stCPUSTAT.cpu,"cpu") != 0) continue;

    m_vCPUSTAT.push_back(m_stCPUSTAT);
  }

  fclose(fp);

  return TRUE;
}

// ��CPU������µ�����
long CCPUSTAT::UptCPUSTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("update T_CPUSTAT set user1=user2,system1=system2,nice1=nice2,idle1=idle2,iowait1=iowait2,irq1=irq2,softirq1=softirq2 where hostname=:1");
  stmt.bindin(1,m_hostname,30);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptCPUSTAT update1 T_CPUSTAT failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }

  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(m_conn,strLocalTime,"yyyymmddhh24miss");

  sqlstatement stmtins;
  stmtins.connect(m_conn);
  stmtins.prepare("insert into T_CPUSTAT(hostname,cpu,user2,system2,nice2,idle2,iowait2,irq2,softirq2,alarmvalue,ddatetime) values(:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,to_date(:11,'yyyymmddhh24miss'))");
  stmtins.bindin( 1, m_hostname,30);
  stmtins.bindin( 2, m_stCPUSTAT.cpu,10);
  stmtins.bindin( 3,&m_stCPUSTAT.user);
  stmtins.bindin( 4,&m_stCPUSTAT.system);
  stmtins.bindin( 5,&m_stCPUSTAT.nice);
  stmtins.bindin( 6, m_stCPUSTAT.idle,15);
  stmtins.bindin( 7,&m_stCPUSTAT.iowait);
  stmtins.bindin( 8,&m_stCPUSTAT.irq);
  stmtins.bindin( 9,&m_stCPUSTAT.softirq);
  stmtins.bindin(10,&m_AlarmValue);
  stmtins.bindin(11, strLocalTime,14);

  sqlstatement stmtupt;
  stmtupt.connect(m_conn);
  stmtupt.prepare("update T_CPUSTAT set user2=:1,system2=:2,nice2=:3,idle2=:4,iowait2=:5,irq2=:6,softirq2=:7,alarmvalue=:8,ddatetime=to_date(:9,'yyyymmddhh24miss'),usep=:10 where hostname=:11 and cpu=:12");
  stmtupt.bindin( 1,&m_stCPUSTAT.user);
  stmtupt.bindin( 2,&m_stCPUSTAT.system);
  stmtupt.bindin( 3,&m_stCPUSTAT.nice);
  stmtupt.bindin( 4, m_stCPUSTAT.idle,15);
  stmtupt.bindin( 5,&m_stCPUSTAT.iowait);
  stmtupt.bindin( 6,&m_stCPUSTAT.irq);
  stmtupt.bindin( 7,&m_stCPUSTAT.softirq);
  stmtupt.bindin( 8,&m_AlarmValue);
  stmtupt.bindin( 9, strLocalTime,14);
  stmtupt.bindin(10,&dusep);
  stmtupt.bindin(11, m_hostname,30);
  stmtupt.bindin(12, m_stCPUSTAT.cpu,10);

  for (UINT ii=0;ii<m_vCPUSTAT.size();ii++)
  {
    memcpy(&m_stCPUSTAT,&m_vCPUSTAT[ii],sizeof(m_stCPUSTAT));

    if (stmtins.execute() != 0)
    {
      if (stmtins.cda.rc == 1)
      {
        if (stmtupt.execute() != 0)
        {
          m_logfile->Write("UptCPUSTAT update2 T_CPUSTAT failed.\n%s\n",stmtupt.cda.message);
          return stmtupt.cda.rc;
        }
      }
      else
      {
        m_logfile->Write("UptCPUSTAT insert T_CPUSTAT failed.\n%s\n",stmtins.cda.message);
        return stmtins.cda.rc;
      }
    }
  }

  return 0;
}

// ����CPU�������Ƿ�ﵽm_AlarmValue������У������ɸ澯
void CCPUSTAT::AlanyCPUSTAT()
{
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));


  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select 100.00-sum(idle2-idle1)*100/sum(user2+system2+nice2+idle2+iowait2+irq2+softirq2-user1-system1-nice1-idle1-iowait1-irq1-softirq1) from T_CPUSTAT where hostname=:1 and cpu='cpu'");
  stmt.bindin(1, m_hostname,30);
  stmt.bindout(1,&dusep);

  stmt.execute();

  stmt.next();

  if ( dusep >= m_AlarmValue )
  {
    snprintf(m_AlarmSTR,1024,"failed.CPUʹ����%.2f,�ﵽ�˷�ֵ%lu��",dusep,m_AlarmValue);
  }
  else
  {
    m_logfile->Write("CPUʹ����%.2f,��ֵ%lu��\n",dusep,m_AlarmValue);
  }

  // ����CPU��ʹ�����ֶ�
  stmt.prepare("update T_CPUSTAT set usep=:1 where hostname=:2 and cpu='cpu'");
  stmt.bindin(1,&dusep);
  stmt.bindin(2, m_hostname,30);
  stmt.execute();
}

