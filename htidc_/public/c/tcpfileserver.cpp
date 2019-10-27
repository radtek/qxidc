/*
 ����һ��ͨ�õĹ���ģ�飬����TCPЭ�鷢���ļ��Ŀͻ��ˡ�
*/

#include "_public.h"

struct st_arg
{
  char ip[31];              // �������˵�IP��ַ��
  int  port;                // �������˵Ķ˿ڡ�
  int  ptype;               // �ļ����ͳɹ����ļ��Ĵ���ʽ��1-�����ļ���2-�ƶ�������Ŀ¼��3-ɾ���ļ���
  char clientpath[301];     // �����ļ���ŵĸ�Ŀ¼��
  char clientpathbak[301];  // �ļ��ɹ����ͺ󣬱����ļ����ݵĸ�Ŀ¼����ptype==2ʱ��Ч��
  char srvpath[301];        // ������ļ���ŵĸ�Ŀ¼��
  char srvpathbak[301];     // �ļ��ɹ����պ󣬷�����ļ����ݵĸ�Ŀ¼����ptype==2ʱ��Ч��
  bool andchild;            // �Ƿ���clientpathĿ¼�¸�����Ŀ¼���ļ���true-�ǣ�false-��
  char matchname[301];      // �������ļ�����ƥ�䷽ʽ����"*.TXT,*.XML"��ע���ô�д��
  char okfilename[301];     // �ѷ��ͳɹ��ļ����嵥��
  int  timetvl;             // ɨ�豾��Ŀ¼�ļ���ʱ��������λ���롣
} starg;

// ��xml����������starg�ṹ��
bool _xmltoarg(char *strxmlbuffer);

CTcpServer TcpServer;
CLogFile logfile;

char strRecvBuffer[TCPBUFLEN+10]; // ���ձ��ĵĻ�����
char strSendBuffer[TCPBUFLEN+10]; // ���ͱ��ĵĻ�����

int clienttype=0;

// �ȴ���¼
bool ClientLogin();

// �г�srvpathĿ¼���ļ����嵥�����ظ��ͻ��ˡ�
bool ListFile();

// �����˳�ʱ���õĺ���
void FathEXIT(int sig);
void ChldEXIT(int sig);

// �����ļ�������
void RecvFilesMain();

// �����ļ�������
void SendFilesMain();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/public/bin/tcpfileserver logfilename port\n");

    printf("Example:/htidc/public/bin/tcpfileserver /log/shqx/tcpfileserver.log 5010\n\n");
    printf("��������һ����������ģ�飬����TCP/IP�����ļ��ķ���ˡ�\n");
    printf("logfilename ��־�ļ�����\n");
    printf("port ���ڴ����ļ���TCP�˿ڡ�\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,FathEXIT); signal(SIGTERM,FathEXIT);

  // �򿪳���������־������һ������̳�����־�����Զ��л�
  if (logfile.Open(argv[1],"a+",false) == false)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.Write("fileserver started(%s).\n",argv[2]);

  if (TcpServer.InitServer(atoi(argv[2])) == false)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[2]); exit(-1);
  }

  while (true)
  {
    // �ȴ��ͻ��˵�����
    if (TcpServer.Accept() == false)
    {
      logfile.Write("TcpServer.Accept() failed.\n"); continue;
    }

    // �µĿͻ�������
    if (fork() > 0)
    {
      // �����̹رոս���������sock���ӣ����ص�Accept��������
      TcpServer.CloseClient(); continue;
    }

    // �����ӽ��̵�����
    signal(SIGINT,ChldEXIT); signal(SIGTERM,ChldEXIT);

    // �ӽ�����Ҫ�ص�������sock
    TcpServer.CloseListen();

    // �ȴ���¼
    if (ClientLogin() == false) ChldEXIT(0);

    // �����ļ�������
    if (clienttype==1) RecvFilesMain();

    // �����ļ�������
    if (clienttype==2) SendFilesMain();

    ChldEXIT(0);
  }

  return 0;
}

// �������˳�ʱ���õĺ���
void FathEXIT(int sig)
{
  if (sig > 0)
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  TcpServer.CloseListen();

  kill(0,15);

  logfile.Write("fileserver EXIT.\n");

  exit(0);
}

// �ӽ����˳�ʱ���õĺ���
void ChldEXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  TcpServer.CloseClient();

  exit(0);
}

// �ȴ���¼
bool ClientLogin()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  if (TcpServer.Read(strRecvBuffer,20) == false)
  {
    logfile.Write("1 TcpServer.Read() failed.\n"); return false;
  }
  // logfile.Write("1 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  GetXMLBuffer(strRecvBuffer,"clienttype",&clienttype);

  if ( (clienttype==1) || (clienttype==2) )
    strcpy(strSendBuffer,"ok");
  else
    strcpy(strSendBuffer,"failed");

  // logfile.Write("1 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
  if (TcpServer.Write(strSendBuffer) == false)
  {
    logfile.Write("1 TcpServer.Write() failed.\n"); return false;
  }

  logfile.Write("%s login %s.\n",TcpServer.GetIP(),strSendBuffer);

  if (strcmp(strSendBuffer,"failed") == 0) return false;

  // �Ѳ�����������
  _xmltoarg(strRecvBuffer);

  return true;
}

// �����ļ�������
void RecvFilesMain()
{
  while (true)
  {
    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    memset(strSendBuffer,0,sizeof(strSendBuffer));

    if (TcpServer.Read(strRecvBuffer,80) == false)
    {
      logfile.Write("2 TcpServer.Read() failed.\n"); ChldEXIT(-1);
    }
    // logfile.Write("2 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

    // ������������
    if (strstr(strRecvBuffer,"activetest")!=0)
    {
      strcpy(strSendBuffer,"ok");
      // logfile.Write("2 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
      if (TcpServer.Write(strSendBuffer) == false)
      {
        logfile.Write("2 TcpServer.Write() failed.\n"); ChldEXIT(-1);
      }

      continue;
    }

    struct st_fileinfo stfileinfo;
    memset(&stfileinfo,0,sizeof(struct st_fileinfo));

    // ��ȡ�����յ��ļ���ʱ��ʹ�С
    GetXMLBuffer(strRecvBuffer,"filename",stfileinfo.filename);
    GetXMLBuffer(strRecvBuffer,"filesize",&stfileinfo.filesize);
    GetXMLBuffer(strRecvBuffer,"mtime",stfileinfo.mtime);

    // ���ļ����е�clientpath�滻��srvpath��ҪС�ĵ���������
    UpdateStr(stfileinfo.filename,starg.clientpath,starg.srvpath,false);

    // �����ļ�������
    if (RecvFile(&logfile,TcpServer.m_connfd,&stfileinfo)== false)
    {
      logfile.Write("RecvFile() failed.\n"); ChldEXIT(-1);
    }

    logfile.Write("recv %s ok.\n",stfileinfo.filename);
  }
}


// �����ļ�������
void SendFilesMain()
{
  while (true)
  {
    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    if (TcpServer.Read(strRecvBuffer,80) == false)
    {
      logfile.Write("3 TcpServer.Read() failed.\n"); ChldEXIT(-1);
    }
    // logfile.Write("3 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

    // ������������
    if (strstr(strRecvBuffer,"activetest")!=0)
    {
      memset(strSendBuffer,0,sizeof(strSendBuffer));
      strcpy(strSendBuffer,"ok");
      // logfile.Write("3 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
      if (TcpServer.Write(strSendBuffer) == false)
      {
        logfile.Write("3 TcpServer.Write() failed.\n"); ChldEXIT(-1);
      }

      continue;
    }

    // �����ȡ�ļ��б���
    if (strcmp(strRecvBuffer,"<list>")==0)
    {
      if (ListFile()==false)
      {
        logfile.Write("ListFile() failed.\n"); ChldEXIT(-1);
      }

      continue;
    }

    // ȡ�ļ�����
    if (strncmp(strRecvBuffer,"<filename>",10)==0)
    {
      // ��ȡ�����յ��ļ���ʱ��ʹ�С
      struct st_fileinfo stfileinfo;
      memset(&stfileinfo,0,sizeof(struct st_fileinfo));
      GetXMLBuffer(strRecvBuffer,"filename",stfileinfo.filename);
      GetXMLBuffer(strRecvBuffer,"filesize",&stfileinfo.filesize);
      GetXMLBuffer(strRecvBuffer,"mtime",stfileinfo.mtime);

      // ���ļ����͸��ͻ���
      if (SendFile(&logfile,TcpServer.m_connfd,&stfileinfo)==false) ChldEXIT(-1);

      logfile.Write("put %s ...ok.\n",stfileinfo.filename);

      // ɾ������˵��ļ�
      if (starg.ptype==2) REMOVE(stfileinfo.filename);

      // ���ݷ���˵��ļ�
      if (starg.ptype==3) 
      {
        char strfilenamebak[301];
        memset(strfilenamebak,0,sizeof(strfilenamebak));
        strcpy(strfilenamebak,stfileinfo.filename);
        UpdateStr(strfilenamebak,starg.srvpath,starg.srvpathbak,false);  // ҪС�ĵ���������
        if (RENAME(stfileinfo.filename,strfilenamebak)==false)
        {
          logfile.Write("RENAME %s to %s failed.\n",stfileinfo.filename,strfilenamebak); ChldEXIT(-1);
        }
      }
    }
  }
}

// ��xml����������starg�ṹ��
bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg,0,sizeof(struct st_arg));
  GetXMLBuffer(strxmlbuffer,"ip",starg.ip);
  GetXMLBuffer(strxmlbuffer,"port",&starg.port);
  GetXMLBuffer(strxmlbuffer,"ptype",&starg.ptype);
  GetXMLBuffer(strxmlbuffer,"clientpath",starg.clientpath);
  GetXMLBuffer(strxmlbuffer,"clientpathbak",starg.clientpathbak);
  GetXMLBuffer(strxmlbuffer,"srvpath",starg.srvpath);
  GetXMLBuffer(strxmlbuffer,"srvpathbak",starg.srvpathbak);
  GetXMLBuffer(strxmlbuffer,"andchild",&starg.andchild);
  GetXMLBuffer(strxmlbuffer,"matchname",starg.matchname);
  GetXMLBuffer(strxmlbuffer,"okfilename",starg.okfilename);
  GetXMLBuffer(strxmlbuffer,"timetvl",&starg.timetvl);

  return true;
}

// �г�srvpathĿ¼���ļ����嵥�����ظ��ͻ��ˡ�
bool ListFile()
{
  CDir Dir;

  // ע�⣬���Ŀ¼�µ����ļ�������50000�����������ļ����ܽ�������
  if (Dir.OpenDir(starg.srvpath,starg.matchname,50000,starg.andchild,false)==false)
  {
    logfile.Write("Dir.OpenDir(%s) ʧ�ܡ�\n",starg.srvpath); return false;
  }

  // �Ȱ��ļ��������ظ��ͻ���
  memset(strSendBuffer,0,sizeof(strSendBuffer));
  sprintf(strSendBuffer,"<totalfile>%d</totalfile>",Dir.m_vFileName.size());
  // logfile.Write("4 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
  if (TcpServer.Write(strSendBuffer) == false)
  {
    logfile.Write("4 TcpServer.Write() failed.\n"); return false;
  }

  // ���ļ���Ϣһ�����ķ��ظ��ͻ���
  while (true)
  {
    if (Dir.ReadDir()==false) break;

    memset(strSendBuffer,0,sizeof(strSendBuffer));
    sprintf(strSendBuffer,"<filename>%s</filename><mtime>%s</mtime><filesize>%d</filesize>",Dir.m_FullFileName,Dir.m_ModifyTime,Dir.m_FileSize);
    // logfile.Write("5 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
    if (TcpServer.Write(strSendBuffer) == false)
    {
      logfile.Write("5 TcpServer.Write() failed.\n"); return false;
    }
  }

  return true;
}
