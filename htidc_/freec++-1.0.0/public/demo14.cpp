//
// ��������ʾ����CTcpServer�࣬ʵ��socketͨѶ�ķ���˺��ļ����䡣
// 

#include "_public.h"

// �����ļ�������
BOOL RecvFile(char *strRecvBuffer,int sockfd,char *strfilename);

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./demo14 port filename\n\n");

    printf("Example:./demo14 5010 test2.jpg\n\n");

    printf("��������ʾ����CTcpServer�࣬ʵ��socketͨѶ�ķ���˺��ļ����䡣\n\n");

    return -1;
  }

  CTcpServer TcpServer;

  // ����˳�ʼ��
  if (TcpServer.InitServer(atoi(argv[1])) == FALSE)
  {
    printf("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  // �ȴ��ͻ��˵�����
  if (TcpServer.Accept() == FALSE)
  {
    printf("TcpServer.Accept() failed.\n"); return -1;
  }

  char strRecvBuffer[1024],strSendBuffer[1024];

  memset(strRecvBuffer,0,sizeof(strRecvBuffer));

  // ��ȡ�ͻ��˵ı��ģ���ʱ����20��
  if (TcpServer.Read(strRecvBuffer,20)==FALSE) 
  {
    printf("TcpServer.Read() failed.\n"); return -1;
  }

  printf("recv:%s\n",strRecvBuffer);

  printf("recv file ...");

  memset(strSendBuffer,0,sizeof(strSendBuffer));

  // �����ļ�������
  if (RecvFile(strRecvBuffer,TcpServer.m_connfd,argv[2])==TRUE)
  {
    strcpy(strSendBuffer,"ok");
    printf("ok.\n");
  }
  else
  {
    strcpy(strSendBuffer,"failed");
    printf("failed.\n");
  }

  // ��ͻ��˷�����Ӧ����
  if (TcpServer.Write(strSendBuffer)==FALSE) 
  {
    printf("TcpServer.Write() failed.\n"); return -1;
  }

  printf("send:%s\n",strSendBuffer);

  return 0;
}

// �����ļ�������
BOOL RecvFile(char *strRecvBuffer,int sockfd,char *strfilename)
{
  UINT  ufilesize=0;
  char strmtime[20]; 

  memset(strmtime,0,sizeof(strmtime));

  // ��ȡ�����յ��ļ���ʱ��ʹ�С
  GetXMLBuffer(strRecvBuffer,"mtime",strmtime);
  GetXMLBuffer(strRecvBuffer,"size",&ufilesize);

  FILE *fp=NULL;

  if ( (fp=fopen(strfilename,"wb")) ==NULL)
  {
    printf("create %s failed.\n",strfilename); return FALSE;
  }

  int  total_bytes=0;
  int  onread=0;
  char buffer[1000];

  while (TRUE)
  {
    memset(buffer,0,sizeof(buffer));

    if ((ufilesize-total_bytes) > 1000) onread=1000;
    else onread=ufilesize-total_bytes;

    if (Readn(sockfd,buffer,onread) == FALSE)
    {
      printf("Readn() failed.\n"); fclose(fp); fp=NULL; return FALSE;
    }

    fwrite(buffer,1,onread,fp);

    total_bytes = total_bytes + onread;

    if ((UINT)total_bytes == ufilesize) break;
  }

  fclose(fp);

  // �����ļ���ʱ��
  UTime(strfilename,strmtime);

  return TRUE;
}
