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
    printf("本程序用于检查目标地址和端口是否能连上，只测试网络连通性，无法判断对方的服务是否正常工作。\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
  
  if (logfile.Open(argv[1], "a+" ) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("checkport");

  // 注意，程序超时是80秒
  ProgramActive.SetProgramInfo(&logfile,"checkport",80);

  memset(stripandport,0,sizeof(stripandport));
  strcpy(stripandport,argv[2]);

  TcpClient.SetConnectOpt(stripandport);

  if (TcpClient.ConnectToServer()==FALSE)
  {
    logfile.Write("网络连接测试失败(%s failed).\n");  TcpClient.Close(); return 0;
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

  logfile.Write("网络连接测试失败(%s failed).\n"); 

  TcpClient.Close(); 

  exit(0);
} 
 
