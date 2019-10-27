//
// 本程序演示采用CTcpClient类，实现socket通讯的客户端
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./demo11 ip port\n\n");

    printf("Example:./demo11 118.89.50.198 5010\n\n");

    printf("本程序演示采用CTcpClient类，实现socket通讯的客户端。\n\n");

    return -1;
  }

  CTcpClient TcpClient;

  // 向服务器发起连接
  if (TcpClient.ConnectToServer(argv[1],atoi(argv[2])) == false)
  {
    printf("TcpClient.ConnectToServer(%s,%d) failed.\n",argv[1],atoi(argv[2])); return -1;
  }

  char strRecvBuffer[1024],strSendBuffer[1024];

  memset(strSendBuffer,0,sizeof(strSendBuffer));
  strcpy(strSendBuffer,\
        "英超最后一轮，卡里克踢完了在曼联的最后一场正式比赛，这意味着红魔上次称霸欧冠的黄金一代全部退场。");

  // 把strSendBuffer内容发送给服务端
  if (TcpClient.Write(strSendBuffer)==false)
  {
    printf("TcpClient.Write() failed.\n"); return -1;
  }

  printf("send ok:%s\n",strSendBuffer);

  memset(strRecvBuffer,0,sizeof(strRecvBuffer));

  // 接收服务端返回的报文
  if (TcpClient.Read(strRecvBuffer)==false)
  {
    if (TcpClient.m_btimeout==true) printf("timeout\n");
    printf("TcpClient.Read() failed.\n"); return -1;
  }

  printf("recv ok:%s\n",strRecvBuffer);

  return 0;
}

