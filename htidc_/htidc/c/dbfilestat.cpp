#include "idcapp.h"

void CallQuit(int sig);

class CTBSPACESTAT
{
public:
  char   m_hostname[31];
  struct st_TBSPACESTAT
  {
    char tablespace[101];
    char total[31];
    char used[31];
    char available[31];
    char usep[31];
  };

  st_TBSPACESTAT m_stTBSPACESTAT;
  vector<struct st_TBSPACESTAT> m_vTBSPACESTAT;

  connection *m_conn;
  CLogFile   *m_logfile;

  CTBSPACESTAT();
 ~CTBSPACESTAT();


  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // ��ȡ�����ļ�����
  BOOL GETTBSPACESTAT();

  // �������ļ�������µ�����
  long UptTBSPACESTAT();

  // ���������ļ��ռ��������Ƿ�ﵽm_AlarmValue������У������ɸ澯
  UINT m_AlarmValue;
  char m_AlarmSTR[1024];
};


char strDSTConnStr[201]; // ���ݿ����Ӳ���
char strIDCConnStr[201]; // ���ݿ����Ӳ���
char strHostName[31]; 
char strAlarmValue[11];

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
CTBSPACESTAT   TBSPACESTAT;

int main(int argc,char *argv[])
{
  if (argc != 5)
  {
    printf("\n");
    printf("Using:./dbfilestat hostname dst:username/password@tnsname idc:username/password@tnsname alarmvalue\n");
    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/dbfilestat 10.153.97.14 szidc/pwdidc@SZQX_10.153.97.14 hssms/smspwd@EJETDB_221.179.6.136 80\n");

    printf("�˳���������ȡ��������ݿ�ı�ռ���Ϣ�����ѽ��������������ĵ�T_TBSPACESTAT��\n");
    printf("hostname�Ǵ���ط���������������Ϊ�˷���ʶ��Ҳ������IP��\n");
    printf("dst:username/password@tnsname�Ǵ�������ݿ�����Ӳ�����\n");
    printf("idc:username/password@tnsname�������������ݿ�����Ӳ�����\n");
    printf("alarmvalue�ǲ����澯�ķ�ֵ������ռ�ʹ���ʳ������ֵʱ�������澯��\n\n\n");
 
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

  if (logfile.Open("/tmp/htidc/log/dbfilestat.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/dbfilestat.log) failed.\n"); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("dbfilestat");

  ProgramActive.SetProgramInfo(&logfile,"dbfilestat",180);

  // �����������ĵ����ݿ�
  if (connidc.connecttodb(strIDCConnStr,TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) idc db failed\n",strIDCConnStr); CallQuit(-1);
  }

  // ����Զ�����ݿ�
  if (conndst.connecttodb(strDSTConnStr) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) dst db failed\n",strDSTConnStr); CallQuit(-1);
  }

  TBSPACESTAT.BindConnLog(&conndst,&logfile);

  // ��ȡԶ�������ļ�ʹ���ʱ���
  if (TBSPACESTAT.GETTBSPACESTAT() == FALSE)
  {
    logfile.Write("TBSPACESTAT.GETTBSPACESTAT failed.\n"); CallQuit(-1);
  }

  // ��ʹ���������ĵ����ݿ�����
  TBSPACESTAT.BindConnLog(&connidc,&logfile);

  TBSPACESTAT.m_AlarmValue=atoi(strAlarmValue);

  strcpy(TBSPACESTAT.m_hostname,strHostName);

  // �������ļ�������µ�����
  if (TBSPACESTAT.UptTBSPACESTAT() != 0)
  {
    logfile.Write("TBSPACESTAT.UptTBSPACESTAT failed.\n"); CallQuit(-1);
  }

  if (strlen(TBSPACESTAT.m_AlarmSTR) > 0)
  {
    logfile.Write("%s��%s\n",TBSPACESTAT.m_hostname,TBSPACESTAT.m_AlarmSTR);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("dbfilestat exit.\n");

  exit(0);
}


CTBSPACESTAT::CTBSPACESTAT()
{
  m_conn=0; m_logfile=0;
  memset(m_hostname,0,sizeof(m_hostname));
  m_AlarmValue=0;
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));
}

CTBSPACESTAT::~CTBSPACESTAT()
{
  m_vTBSPACESTAT.clear();
}

void CTBSPACESTAT::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn = in_conn; m_logfile = in_logfile;
}

// ��ȡ�����ļ�����
BOOL CTBSPACESTAT::GETTBSPACESTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select f.tablespace_name,round(a.total,2),round(u.used,2),round(f.free,2),round((u.used/a.total)*100,2) from\
        (select tablespace_name, sum(bytes/(1024*1024)) total from dba_data_files group by tablespace_name) a,\
        (select tablespace_name, round(sum(bytes/(1024*1024))) used from dba_extents group by tablespace_name) u,\
        (select tablespace_name, round(sum(bytes/(1024*1024))) free from dba_free_space group by tablespace_name) f\
    where a.tablespace_name = f.tablespace_name\
      and a.tablespace_name = u.tablespace_name\
      and a.tablespace_name not in ('UNDOTBS1','UNDOTBS2','UNDOTBS3','TEMP')");
  stmt.bindout(1,m_stTBSPACESTAT.tablespace,30);
  stmt.bindout(2,m_stTBSPACESTAT.total,30);
  stmt.bindout(3,m_stTBSPACESTAT.used,30);
  stmt.bindout(4,m_stTBSPACESTAT.available,30);
  stmt.bindout(5,m_stTBSPACESTAT.usep,10);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("GETTBSPACESTAT select DBA_FREE_SPACE,DBA_DATA_FILES failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  m_vTBSPACESTAT.clear();

  while (TRUE)
  {
    memset(&m_stTBSPACESTAT,0,sizeof(m_stTBSPACESTAT));

    if (stmt.next() != 0) break;

    m_vTBSPACESTAT.push_back(m_stTBSPACESTAT);
  }

  return TRUE;
}

// �������ļ�������µ�����
long CTBSPACESTAT::UptTBSPACESTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("delete T_TBSPACESTAT where hostname=:1");
  stmt.bindin(1,m_hostname,30);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptTBSPACESTAT delete T_TBSPACESTAT failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }

  stmt.prepare("insert into T_TBSPACESTAT(hostname,total,used,available,usep,tablespace,alarmvalue,ddatetime) values(:1,:2,:3,:4,:5,:6,:7,sysdate)");
  stmt.bindin(1, m_hostname,30);
  stmt.bindin(2, m_stTBSPACESTAT.total,30);
  stmt.bindin(3, m_stTBSPACESTAT.used,30);
  stmt.bindin(4, m_stTBSPACESTAT.available,30);
  stmt.bindin(5, m_stTBSPACESTAT.usep,30);
  stmt.bindin(6, m_stTBSPACESTAT.tablespace,30);
  stmt.bindin(7,&m_AlarmValue);

  for (UINT ii=0;ii<m_vTBSPACESTAT.size();ii++)
  {
    memcpy(&m_stTBSPACESTAT,&m_vTBSPACESTAT[ii],sizeof(m_stTBSPACESTAT));

    m_logfile->Write("������%s����ռ�%s����%sM����ʹ��%sM��Լռ%s%%��\n",\
                      m_hostname,m_stTBSPACESTAT.tablespace,m_stTBSPACESTAT.total,\
                      m_stTBSPACESTAT.used,m_stTBSPACESTAT.usep);


    if ( (UINT)atoi(m_stTBSPACESTAT.usep) >= m_AlarmValue)
    {
      char strTemp[1024];
      memset(strTemp,0,sizeof(strTemp));
      snprintf(strTemp,256,"failed.��ռ�%s����%sM����ʹ��%sM��Լռ%s%%��",m_stTBSPACESTAT.tablespace,\
              m_stTBSPACESTAT.total,m_stTBSPACESTAT.used,m_stTBSPACESTAT.usep);
      strcat(m_AlarmSTR,strTemp);
    }
  
    if (stmt.execute() != 0)
    {
      m_logfile->Write("UptTBSPACESTAT insert T_TBSPACESTAT failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
    }
  }

  // ������ȫ�Ƕ����滻��ȫ�ǵľ��
  if (strlen(m_AlarmSTR) > 0) strcpy(m_AlarmSTR+strlen(m_AlarmSTR)-2,"��");

  logfile.WriteEx("\n");

  return 0;
}


