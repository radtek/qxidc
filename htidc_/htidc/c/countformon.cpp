#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

CDMONCFG DMONCFG;

// ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3-�������ݿ��ʧ��
int _countformon();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./countformon logfile connstr\n");

    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/countformon /log/ssqx/countformon.log qxidc/pwdidc@EJETDB_221.179.6.136\n\n");
 
    printf("�������ĵ�������ͳ�Ƴ��򣬳����ȡ���ݼ�ز�����T_DMONCFG��ͳ�ƽ���������������־��T_DMONITEM�����С�\n");
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

  logfile.SetAlarmOpt("countformon");

  // ע�⣬����ʱ��800��
  ProgramActive.SetProgramInfo(&logfile,"countformon",800);

  // �����������ݿ�Ĵ�����������Ŀ����Ϊ��ȷ�������ļ���Ҫ���ʱ���������ݿ�
  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  DMONCFG.BindConnLog(&conn,&logfile);

  // ��T_DMONCFG���м�����Ҫͳ�Ƶļ�¼�������m_vDMONCFG��
  if (DMONCFG.LoadDMONCFG("where rsts=1 and (exectime is null or ((sysdate-exectime)*1440>exectvl))") == FALSE) CallQuit(-1);

    // ��ȡm_vDMONCFG��ȫ���Ĳ�����ͳ��������
  if (DMONCFG.CountTable() == FALSE) CallQuit(-1);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("countformon exit.\n");

  exit(0);
}

