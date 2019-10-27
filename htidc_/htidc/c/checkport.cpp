#include "_public.h"

CLogFile logfile;
CProgramActive ProgramActive;

void EXIT(int sig);

char stripandport[51];
CTcpClient TcpClient;

int main( int argc,char *argv[])
{
  if (argc!=3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/checkport logfilename ipandport\n\n");
    printf("Sample:/htidc/htidc/bin/procctl 800 /htidc/htidc/bin/checkport /log/hssms/checkport_10_153_97_30_80.log 10.153.97.30,80\n");
    printf("���������ڼ��Ŀ���ַ�Ͷ˿��Ƿ������ϣ�ֻ����������ͨ�ԣ��޷��ж϶Է��ķ����Ƿ�����������\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
  
  if (logfile.Open(argv[1], "a+" ) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("checkport");

  // ע�⣬����ʱ��80��
  ProgramActive.SetProgramInfo(&logfile,"checkport",80);

  memset(stripandport,0,sizeof(stripandport));
  strcpy(stripandport,argv[2]);

  TcpClient.SetConnectOpt(stripandport);

  if (TcpClient.ConnectToServer()==FALSE)
  {
    logfile.Write("�������Ӳ���ʧ��(%s failed).\n");  TcpClient.Close(); return 0;
  }

  logfile.Write("connect %s ok.\n",stripandport);

  TcpClient.Close();

  return 0;
}

void EXIT(int sig)
{
  if (sig > 0)
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  logfile.Write("�������Ӳ���ʧ��(%s failed).\n"); 

  TcpClient.Close(); 

  exit(0);
} 
 
