#include "qxidc.h"

CIDCCFG IDCCFG;

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CTIMECHECK     TIMECHECK;

char strTableName[51];

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\nUsing:/htidc/pyqx/c/timecheck inifile tname\n");
    printf("Example:/htidc/pyqx/c/timecheck /htidc/pyqx/ini/pyqx.xml T_OBTMIND\n");
    printf("        /htidc/htidc/bin/procctl 10 /htidc/pyqx/c/timecheck /htidc/pyqx/ini/pyqx.xml T_OBTMIND\n\n\n");

    printf("�ó������ڶ����ݱ���ʱ��һ�����������ơ�\n\n\n");

    return -1;
  }

  // �Ӳ����ļ��м���ȫ���Ĳ���
  if (IDCCFG.LoadIniFile(argv[1]) == FALSE)
  {
    printf("IDCCFG.LoadIniFile(%s) failed.\n",argv[1]); return -1;
  }

  strcpy(strTableName,argv[2]);

  memset(strTableName,0,sizeof(strTableName));

  strcpy(strTableName,argv[2]);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; 
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,201,"%s/timecheck_%s.log",IDCCFG.m_logpath,strTableName);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }

  // ע�⣬����ʱ��1800��
  ProgramActive.SetProgramInfo(&logfile,"timecheck",1800);

  // �������ݿ⣬�����ݿ������������������ݴ���
  if (conn.connecttodb(IDCCFG.m_idcconnstr,TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",IDCCFG.m_idcconnstr); CallQuit(-1);
  }

  logfile.Write("timecheck(%s) beging.\n",strTableName);

  TIMECHECK.BindConnLog(&conn,&logfile);

  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    if (TIMECHECK.CheckTable(strTableName) != 0)
    {
      logfile.Write("TIMECHECK","�������timecheck������CheckTable(%s)ʧ��!",strTableName); CallQuit(-1);
    }

    conn.commitwork();

    sleep(10);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("timecheck exit.\n");

  exit(0);
}

