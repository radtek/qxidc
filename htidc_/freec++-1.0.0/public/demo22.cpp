//
// ��������ʾ����CTcpServer�࣬���ö��̵߳ķ�ʽ��ʵ��socketͨѶ�ķ����
// 

#include "_public.h"

CLogFile logfile;

void *pthid_main(void *arg);

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./demo22 port logfilename\n\n");

    printf("Example:./demo22 5010 demo22.log\n\n");

    printf("��������ʾ����CTcpServer�࣬���ö��̵߳ķ�ʽ��ʵ��socketͨѶ�ķ���ˡ�\n\n");

    return -1;
  }

  if (logfile.Open(argv[2],"a+")==FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[2]); return -1;
  }

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

    logfile.Write("���ܵ�һ���µ����ӡ�\n");

    pthread_t pthid;

    // �������߳�
    if ( pthread_create(&pthid,NULL,pthid_main,(void*)TcpServer.m_connfd) != 0)
    {
      logfile.Write("pthread_create failed.\n"); return -1;
    }
  }

  return 0;
}

void *pthid_main(void *arg)
{
  int sockfd = (long)arg;

  int  irecvlen=0;
  char strRecvBuffer[1024],strSendBuffer[1024];

  while (TRUE)
  {
    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  
    // ��ȡ�ͻ��˵ı��ģ���ʱ����20��
    if (TcpRead(sockfd,strRecvBuffer,&irecvlen,20)==FALSE)
    {
      logfile.Write("TcpRead() failed.\n"); pthread_exit(NULL);
    }

    logfile.Write("recv ok:%s\n",strRecvBuffer);

    memset(strSendBuffer,0,sizeof(strSendBuffer));
    strcpy(strSendBuffer,"ok");

    // ��ͻ��˷�����Ӧ����
    if (TcpWrite(sockfd,strSendBuffer)==FALSE)
    {
      logfile.Write("TcpWrite() failed.\n"); pthread_exit(NULL);
    }

    logfile.Write("send ok:%s\n",strSendBuffer);
  }

  pthread_exit(NULL);
}
