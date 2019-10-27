#include "idcapp.h"

void CallQuit(int sig);

char strLogPath[201]; // ��־�ļ�Ŀ¼
char strTmpPath[201]; // ���̻��Ϣ�ļ�Ŀ¼
char strIniFile[201];
char strIDCConnStr[201]; // ���ݿ����Ӳ���

CALARMLOG ALARMLOG;

CDir Dir;
connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if ( (argc != 2) && (argc != 3) )
  {
    printf("\n");
    printf("Using:./alarmtotab username/password@tnsname [NLS_LANG]\n\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/alarmtotab hssms/smspwd@EJETDB_192.168.1.10\n\n");
    printf("�˳������ڰѴ����/tmp/htidc/alarmxmlĿ¼�к�̨����ĸ澯��־�ļ����µ�T_ALARMLOG���С�\n\n");
 
    return -1;
  }

  memset(strIniFile,0,sizeof(strIniFile));

  if (argc == 3)  setenv("NLS_LANG",argv[2],1);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open("/tmp/htidc/log/alarmtotab.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/alarmtotab.log) failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("alarmtotab");

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&logfile,"alarmtotab",300);

  // �������ݿ⣬�����Զ��ύ�ķ�ʽ������Ҫ������ơ�
  if (conn.connecttodb(argv[1],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[1]); return -1;
  }

  ALARMLOG.BindConnLog(&conn,&logfile);

  // �򿪱�׼��ʽ�ļ�Ŀ¼
  if (Dir.OpenDir("/tmp/htidc/alarmxml",TRUE) == FALSE)
  {
    logfile.Write("Dir.OpenDir /tmp/htidc/alarmxml failed.\n"); CallQuit(-1);
  }

  while (TRUE)
  {
    if (Dir.ReadDir() == FALSE) break;

    // ����ļ���������.XML�������Ͳ�������ļ�
    if (MatchFileName(Dir.m_FileName,"*.XML") == FALSE)
    {
      REMOVE(Dir.m_FullFileName); continue;
    }

    logfile.Write("%s\n",Dir.m_FileName);

    ALARMLOG.ReadXMLFile(Dir.m_FullFileName);

    // ������־��������logfile.Writeȥд�գ���������failed����ѭ��
    fprintf(logfile.m_tracefp,"leasttime=%s\n",ALARMLOG.m_stALARMLOG.leasttime);
    fprintf(logfile.m_tracefp,"progname=%s\n",ALARMLOG.m_stALARMLOG.progname);
    fprintf(logfile.m_tracefp,"alarmtext=%s\n\n",ALARMLOG.m_stALARMLOG.alarmtext);
    fflush(logfile.m_tracefp);

    // ������־�����Ƿ�ɹ��������ж�UptAlarmLog�Ľ�����������ɱ������ѭ���澯��
    ALARMLOG.UptAlarmLog();

    REMOVE(Dir.m_FullFileName);

    conn.commitwork();
  }

  // ɾ����������֮ǰ�ļ�¼
  ALARMLOG.DelAlarmLog();

  conn.commitwork();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("alarmtotab exit.\n");

  exit(0);
}


