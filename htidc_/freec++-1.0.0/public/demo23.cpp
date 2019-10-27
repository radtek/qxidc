//
// ��������ʾ���̵߳�TCP�ͻ��˵��߳�ͬ��������
// 

#include "_public.h"

pthread_mutex_t mutex;

// ���ͱ��ĵ��߳�������
void *send_main(void *arg);
// ���ձ��ĵ��߳�������
void *recv_main(void *arg);

CTcpClient TcpClient;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./demo23 ip,port\n\n");

    printf("Example:./demo23 193.112.167.234,5010\n\n");

    printf("��������ʾ���̵߳�TCP�ͻ��˵��߳�ͬ��������\n\n");

    return -1;
  }

  // ���������������
  if (TcpClient.ConnectToServer(argv[1]) == FALSE)
  {
    printf("TcpClient.ConnectToServer(%s) failed.\n",argv[1]); return -1;
  }

  pthread_t psend1,psend2,precv;

  // ��ʼ��������
  pthread_mutex_init(&mutex,NULL);

  // ���������߳�һ
  if ( pthread_create(&psend1,NULL,send_main,(void *)1) != 0)
  {
    printf("pthread_create psend1 failed.\n"); pthread_mutex_destroy(&mutex); return -1;
  }
  
  // ���������̶߳�
  if ( pthread_create(&psend2,NULL,send_main,(void *)2) != 0)
  {
    printf("pthread_create psend2 failed.\n"); pthread_mutex_destroy(&mutex); return -1;
  }

  // ���������߳�
  if ( pthread_create(&precv,NULL,recv_main,(void*)4) != 0)
  {
    printf("pthread_create precv failed.\n"); pthread_mutex_destroy(&mutex); return -1;
  }

  // �ȴ��߳��˳�
  pthread_join(psend1,NULL);
  pthread_join(psend2,NULL);
  pthread_join(precv ,NULL);

  // ������
  pthread_mutex_destroy(&mutex);

  return 0;
}

void *send_main(void *arg)
{
  long ll=(long)arg;
  
  printf("�߳�%ld��ʼ���С�\n",ll);

  char strSendBuffer[1024];

  for (int ii=0;ii<100000;ii++)
  {
    memset(strSendBuffer,0,sizeof(strSendBuffer));
    sprintf(strSendBuffer,"%ld:%03d:%s",ll,ii,\
          "Ӣ�����һ�֣�����������������������һ����ʽ����������ζ�ź�ħ�ϴγư�ŷ�ڵĻƽ�һ��ȫ���˳���");

    pthread_mutex_lock(&mutex);

    // ��strSendBuffer���ݷ��͸������
    if ( TcpWrite(TcpClient.m_sockfd,strSendBuffer) == FALSE)
    {
      pthread_mutex_unlock(&mutex); printf("TcpClient.Write() failed.\n"); exit(-1);
    }

    pthread_mutex_unlock(&mutex);
  }

  printf("�߳�%ld���н�����\n",ll);

  pthread_exit(NULL);
}

void *recv_main(void *arg)
{
  char strRecvBuffer[1024];

  while (TRUE)
  {
    memset(strRecvBuffer,0,sizeof(strRecvBuffer));

    // ���շ���˷��صı���
    if (TcpClient.Read(strRecvBuffer,20)==FALSE)
    {
      printf("TcpClient.Read() failed.\n"); exit(-1);
    }
  }

  pthread_exit(NULL);
}
