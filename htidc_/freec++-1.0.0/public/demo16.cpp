//
// ��������ʾ����CTcpServer�࣬���ö���̵ķ�ʽ��ʵ��socketͨѶ�ķ����
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./demo16 port logfilename\n\n");

    printf("Example:./demo16 5010 demo16.log\n\n");

    printf("��������ʾ����CTcpServer�࣬���ö���̵ķ�ʽ��ʵ��socketͨѶ�ķ���ˡ�\n");

    return -1;
  }

  CLogFile logfile;

  if (logfile.Open(argv[2],"a+")==FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[2]); return -1;
  }

  signal(SIGCHLD,SIG_IGN);

  CTcpServer TcpServer;

  // ����˳�ʼ��
  if (TcpServer.InitServer(atoi(argv[1])) == FALSE)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  while (TRUE)
  {
    // �ȴ��ͻ��˵�����
    if (TcpServer.Accept() == FALSE)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); return -1;
    }

    if (fork()>0)
    {
      TcpServer.CloseClient(); continue;
    }

    logfile.Write("���ܵ�һ���µ����ӡ�\n");

    TcpServer.CloseListen();

    char strRecvBuffer[1024],strSendBuffer[1024];

    while (TRUE)
    {
      memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  
      // ��ȡ�ͻ��˵ı��ģ���ʱ����20��
      if (TcpServer.Read(strRecvBuffer,20)==FALSE) 
      {
        logfile.Write("TcpServer.Read() failed.\n"); return -1;
      }

      logfile.Write("recv ok:%s\n",strRecvBuffer);

      memset(strSendBuffer,0,sizeof(strSendBuffer));
      strcpy(strSendBuffer,"ok");

      // ��ͻ��˷�����Ӧ����
      if (TcpServer.Write(strSendBuffer)==FALSE) 
      {
        logfile.Write("TcpServer.Write() failed.\n"); return -1;
      }
    }

    logfile.Write("send ok:%s\n",strSendBuffer);
  }

  return 0;
}

