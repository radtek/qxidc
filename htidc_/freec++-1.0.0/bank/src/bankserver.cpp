//
// ������Ϊ���й�̨ҵ��ĺ�̨�������
// 

#include "bank.h"

// �����˳�ʱ���õĺ���
void MAINEXIT(int sig);

CLogFile logfile;

// ���ڴ���ͻ��˵����ӵ�������
void *pthid_main(void *arg);

// ���ڴ��ȫ���ͻ������ӵ�sock
vector<int> v_sock;

// TCP�����
CTcpServer TcpServer;

// ���ݿ����ӳ�
pthread_mutex_t mutex[5];
connection conn[5];

// ����ͻ��˱��ĵ�������
BOOL procmessage(char *strRecvBuffer,char *strSendBuffer,vector<struct st_bank> &vbank);

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./bankserver port connstr logfilename\n\n");

    printf("Example:/home/wucz/bank/bin/bankserver 5010 scott/tiger@orcl11g_193.112.167.234 /home/wucz/bank/log/bankserver.log\n\n");

    printf("������Ϊ���й�̨ҵ��ĺ�̨�������չʾ�˶��̡߳��߳�ͬ�������ݿ����ӳؼ�����\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,MAINEXIT); signal(SIGTERM,MAINEXIT);

  // ����־�ļ�
  if (logfile.Open(argv[3],"a+")==FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[3]); return -1;
  }

  // TCP����˳�ʼ��
  if (TcpServer.InitServer(atoi(argv[1])) == FALSE)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  // ��ʼ�����ݿ����ӳ�
  for (int ii=0;ii<5;ii++)
  {
     pthread_mutex_init(&mutex[ii],NULL);
    if (conn[ii].connecttodb(argv[2],"Simplified Chinese_China.ZHS16GBK") != 0)
    {
      logfile.Write("connect database failed.\n%s\n",conn[ii].m_cda.message); return -1;
    }
  }

  while (TRUE)
  {
    // �ȴ��ͻ��˵�����
    if (TcpServer.Accept() == FALSE)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); MAINEXIT(-1);
    }

    // �ѿͻ��˵�sock��������
    v_sock.push_back(TcpServer.m_connfd);

    logfile.Write("���ܵ�һ���µ����ӡ�\n");

    pthread_t pthid;

    // �������̣߳����ڴ���ͻ��˵�����
    if ( pthread_create(&pthid,NULL,pthid_main,(void*)TcpServer.m_connfd) != 0)
    {
      logfile.Write("pthread_create failed.\n"); MAINEXIT(-1);
    }
  }

  return 0;
}

// ���ڴ���ͻ��˵����ӵ�������
void *pthid_main(void *arg)
{
  int sockfd = (long)arg;

  int  irecvlen=0;
  char strRecvBuffer[1024],strSendBuffer[1024];

  while (TRUE)
  {
    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    memset(strSendBuffer,0,sizeof(strSendBuffer));
  
    // ��ȡ�ͻ��˵ı��ģ���ʱ����80��
    if (TcpRead(sockfd,strRecvBuffer,&irecvlen,80)==FALSE)
    {
      logfile.Write("TcpRead() failed.\n"); break;
    }

    logfile.Write("recv ok:%s\n",strRecvBuffer);

    vector<struct st_bank> vbank;

    // ����ͻ��˱��ĵ�������
    procmessage(strRecvBuffer,strSendBuffer,vbank);

    // ��ͻ��˷�����Ӧ����
    if (TcpWrite(sockfd,strSendBuffer)==FALSE)
    {
      logfile.Write("TcpWrite() failed.\n"); break;
    }

    logfile.Write("send ok:%s\n",strSendBuffer);

    // ��������н�����������ﷵ��
    if ( vbank.size()>0 )
    {
      for (UINT ii=0;ii<vbank.size();ii++)
      {
        memset(strSendBuffer,0,sizeof(strSendBuffer));
        sprintf(strSendBuffer,"<opertype>%d</opertype><je>%.2f</je><crttime>%s</crttime>",vbank[ii].opertype,vbank[ii].je,vbank[ii].crttime);
        if (TcpWrite(sockfd,strSendBuffer)==FALSE)
        {
          logfile.Write("TcpWrite() failed.\n"); break;
        }
      }
    }
  }

  pthread_exit(NULL);
}

void MAINEXIT(int sig)
{
  signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

  if (sig>0) signal(sig,SIG_IGN);

  logfile.Write("main exit,sig=%d.",sig);

  logfile.Write("�ر����ݿ����ӳء�\n");

  // �ر����ݿ����ӳء�
  for (int ii=0;ii<5;ii++)
  {
    pthread_mutex_destroy(&mutex[ii]);
    conn[ii].disconnect();
  }

  logfile.Write("�رռ�����sock��\n");

  // �رռ�����sock
  TcpServer.CloseListen();

  logfile.Write("�رտͻ��˵�sock��\n");

  // �رտͻ��˵�sock��
  for (UINT ii=0;ii<v_sock.size();ii++)
  {
    close(v_sock[ii]);
  }

  logfile.Write("�����˳���\n");

  // �ر���־�ļ�
  logfile.Close();

  exit(0);
}

// ����ͻ��˱��ĵ�������
BOOL procmessage(char *strRecvBuffer,char *strSendBuffer,vector<struct st_bank> &vbank)
{
  connection *pconn;

  int iconn=0;

  for (iconn=0;iconn<5;iconn++)
  {
    if (pthread_mutex_trylock(&mutex[iconn])==0)
    {
      logfile.Write("��ȡ���ӳ�%d\n",iconn);
      pconn=&conn[iconn]; break;
    }
  }

  if (iconn==5)
  {
    strcpy(strSendBuffer,"<retno>-1</retno><retmesg>ϵͳ��æ��</retmesg>"); return FALSE;
  }

  struct st_bank stbank;
  memset(&stbank,0,sizeof(struct st_bank));

  GetXMLBuffer(strRecvBuffer,"msgid",&stbank.msgid);
  GetXMLBuffer(strRecvBuffer,"oper_id",stbank.oper_id,30);
  GetXMLBuffer(strRecvBuffer,"password",stbank.password,30);
  GetXMLBuffer(strRecvBuffer,"userid",stbank.userid,20);
  GetXMLBuffer(strRecvBuffer,"cardid",stbank.cardid,18);
  GetXMLBuffer(strRecvBuffer,"username",stbank.username,30);
  GetXMLBuffer(strRecvBuffer,"tel",stbank.tel,30);
  GetXMLBuffer(strRecvBuffer,"opertype",&stbank.opertype);
  GetXMLBuffer(strRecvBuffer,"je",&stbank.je);
  GetXMLBuffer(strRecvBuffer,"userid1",stbank.userid1,20);

  // ����Ա��¼ҵ��
  if (stbank.msgid==1)
  {
    if (msgid_1(pconn,&stbank,&logfile)==0)
    {
      strcpy(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg>");
    }
    else
    {
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>����Ա�����֤ʧ�ܡ�</retmesg>");
    }
  }

  // ����ҵ��
  if (stbank.msgid==2)
  {
    if (msgid_2(pconn,&stbank,&logfile)==0)
    {
      pconn->commit();
      strcpy(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg>");
    }
    else
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>ϵͳ���ϡ�</retmesg>");
    }
  }

  // �ͻ������֤
  if (stbank.msgid==3)
  {
    if (msgid_3(pconn,&stbank,&logfile)==0)
    {
      strcpy(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg>");
    }
    else
    {
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>�ͻ������֤ʧ�ܡ�</retmesg>");
    }
  }

  // ���ҵ��
  if (stbank.msgid==4)
  {
    if (msgid_4(pconn,&stbank,&logfile)==0)
    {
      pconn->commit();
      strcpy(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg>");
    }
    else
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>ϵͳ���ϡ�</retmesg>");
    }
  }

  // ȡ��ҵ��
  if (stbank.msgid==5)
  {
    int iret=msgid_5(pconn,&stbank,&logfile);

    // �ɹ�
    if (iret==0)
    {
      conn->commit();
      strcpy(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg>");
    }

    // ����
    if (iret==-1)
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>���㡣</retmesg>");
    }

    // ϵͳ����
    if (iret>0)
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>ϵͳ���ϡ�</retmesg>");
    }
  }

  // ת��ҵ��
  if (stbank.msgid==6)
  {
    // �ж�ת���ʻ������
    // �ж������ʻ��Ƿ���ڣ������ʻ���״̬������3-������
    int iret=msgid_6(pconn,&stbank,&logfile);

    // �ɹ�
    if (iret==0)
    {
      conn->commit();
      strcpy(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg>");
    }

    // ����
    if (iret==-1)
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>���㡣</retmesg>");
    }

    // ת���ʻ��Ƿ�
    if (iret==-2)
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-2</retno><retmesg>ת���ʻ��Ƿ���</retmesg>");
    }

    // ϵͳ����
    if (iret>0)
    {
      pconn->rollback();
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>ϵͳ���ϡ�</retmesg>");
    }

  }
  
  // �����
  if (stbank.msgid==7)
  {
    if (msgid_7(pconn,&stbank,&logfile)==0)
    {
      sprintf(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg><ye>%.2f</ye>",stbank.ye);
    }
    else
    {
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>�ͻ���Ϣ�����ڡ�</retmesg>");
    }
  }

  // ����ˮ
  if (stbank.msgid==8)
  {
    if (msgid_8(pconn,&stbank,&logfile,vbank)==0)
    {
      sprintf(strSendBuffer,"<retno>0</retno><retmesg>ok</retmesg><totalcount>%lu</totalcount>",vbank.size());
    }
    else
    {
      strcpy(strSendBuffer,"<retno>-1</retno><retmesg>ϵͳ���ϡ�</retmesg>");
    }
  }

  pthread_mutex_unlock(&mutex[iconn]);

  return TRUE;
}




