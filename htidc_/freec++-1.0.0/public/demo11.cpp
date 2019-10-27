//
// ��������ʾ����CTcpClient�࣬ʵ��socketͨѶ�Ŀͻ���
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./demo11 ip,port\n\n");

    printf("Example:./demo11 193.112.167.234,5010\n\n");

    printf("��������ʾ����CTcpClient�࣬ʵ��socketͨѶ�Ŀͻ��ˡ�\n\n");

    return -1;
  }

  CTcpClient TcpClient;

  // ���������������
  if (TcpClient.ConnectToServer(argv[1]) == FALSE)
  {
    printf("TcpClient.ConnectToServer(%s) failed.\n",argv[1]); return -1;
  }

  char strRecvBuffer[1024],strSendBuffer[1024];

  memset(strSendBuffer,0,sizeof(strSendBuffer));
  strcpy(strSendBuffer,\
        "Ӣ�����һ�֣�����������������������һ����ʽ����������ζ�ź�ħ�ϴγư�ŷ�ڵĻƽ�һ��ȫ���˳���");

  // ��strSendBuffer���ݷ��͸������
  if (TcpClient.Write(strSendBuffer)==FALSE)
  {
    printf("TcpClient.Write() failed.\n"); return -1;
  }

  printf("send ok:%s\n",strSendBuffer);

  memset(strRecvBuffer,0,sizeof(strRecvBuffer));

  // ���շ���˷��صı���
  if (TcpClient.Read(strRecvBuffer)==FALSE)
  {
    printf("TcpClient.Read() failed.\n"); return -1;
  }

  printf("recv ok:%s\n",strRecvBuffer);

  return 0;
}

