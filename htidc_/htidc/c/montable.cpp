#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

CDMONCFG DMONCFG;

int _montable();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./montable logfilename connstr\n");

    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/montable /log/ssqx/montable.log qxidc/pwdidc@EJETDB_221.179.6.136\n\n");
 
    printf("�������ĵ��������澯���򣬳����ȡ���ݼ�ز�����T_DMONCFG������������־��T_DMONITEM�����С�\n");
    printf("����ÿ50������һ�Σ���procctl���ȡ�\n\n");
 
    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("montable");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"montable",300);

  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  DMONCFG.BindConnLog(&conn,&logfile);

  // ��T_DMONCFG���м�����Ҫȫ���ļ�¼�������m_vDMONCFG��
  if (DMONCFG.LoadDMONCFG("where rsts=1") == FALSE) CallQuit(-1);

  // ��ȡm_vDMONCFG��ȫ���Ĳ����������澯
  if (DMONCFG.MONTable() == FALSE) CallQuit(-1);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("montable exit.\n");

  exit(0);
}

