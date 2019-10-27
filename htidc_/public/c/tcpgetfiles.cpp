/*
 ����һ��ͨ�õĹ���ģ�飬����TCPЭ���ȡ�ļ��Ŀͻ��ˡ�
*/

#include "_public.h"

struct st_arg
{
  char ip[31];              // �������˵�IP��ַ��
  int  port;                // �������˵Ķ˿ڡ�
  int  ptype;               // �ļ���ȡ�ɹ����ļ��Ĵ���ʽ��1-�����ļ���2-ɾ���ļ���3-�ƶ�������Ŀ¼��
  char clientpath[301];     // �����ļ���ŵĸ�Ŀ¼��
  char srvpath[301];        // ������ļ���ŵĸ�Ŀ¼��
  char srvpathbak[301];     // �ļ��ɹ���ȡ�󣬷�����ļ����ݵĸ�Ŀ¼����ptype==3ʱ��Ч��
  bool andchild;            // �Ƿ��ȡsrvpathĿ¼�¸�����Ŀ¼���ļ���true-�ǣ�false-��
  char matchname[301];      // ����ȡ�ļ�����ƥ�䷽ʽ����"*.TXT,*.XML"��ע���ô�д��
  char okfilename[301];     // �ѻ�ȡ�ɹ��ļ����嵥��
  int  timetvl;             // ɨ�豾��Ŀ¼�ļ���ʱ��������λ���롣
} starg;

char strRecvBuffer[TCPBUFLEN+10]; // ���ձ��ĵĻ�����
char strSendBuffer[TCPBUFLEN+10]; // ���ͱ��ĵĻ�����

vector<struct st_fileinfo> vlistfile,vlistfile1;
vector<struct st_fileinfo> vokfilename,vokfilename1;

// �ѷ����srvpathĿ¼�µ��ļ����ص�vlistfile������
bool LoadListFile();

// ��okfilename�ļ����ݼ��ص�vokfilename������
bool LoadOKFileName();

// ��vlistfile�����е��ļ���vokfilename�������ļ��Աȣ��õ���������
// һ����vlistfile�д��ڣ����Ѿ��ɼ��ɹ����ļ�vokfilename1
// ������vlistfile�д��ڣ����ļ�����Ҫ���²ɼ����ļ�vlistfile1
bool CompVector();

// ��vokfilename1�����е�������д��okfilename�ļ��У�����֮ǰ�ľ�okfilename�ļ�
bool WriteToOKFileName();

// ���ptype==1���Ѳɼ��ɹ����ļ���¼׷�ӵ�okfilename�ļ���
bool AppendToOKFileName(struct st_fileinfo *stfileinfo);

CTcpClient TcpClient;

CLogFile logfile;

// �������ҵ������������
bool _tcpgetfiles();

void EXIT(int sig);

// ��ʾ����İ���
void _help(char *argv[]);
  
// ��xml����������starg�ṹ��
bool _xmltoarg(char *strxmlbuffer);

// ��¼������
bool ClientLogin(const char *argv);

// �����˷�����������
bool ActiveTest();

// ʵ���ļ���ȡ�Ĺ���
bool _tcpgetfiles();

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // �ر�ȫ�����źź��������
  CloseIOAndSignal();

  // ��������˳����ź�
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("����־�ļ�ʧ�ܣ�%s����\n",argv[1]); return -1;
  }

  // ��xml����������starg�ṹ��
  if (_xmltoarg(argv[2])==false) return -1;

  while (true)
  {
    // ��������������Ӳ���¼
    ClientLogin(argv[2]);

    // ʵ���ļ���ȡ�Ĺ���
    _tcpgetfiles();

    if (vlistfile.size()==0)
    {
      // �����˷�����������
      ActiveTest(); 
    
      sleep(starg.timetvl);
    }
  }

  return 0;
}

void EXIT(int sig)
{
  logfile.Write("�����˳���sig=%d\n\n",sig);

  TcpClient.Close();

  exit(0);
}

// ��ʾ����İ���
void _help(char *argv[])
{
  printf("\n");
  printf("Using:/htidc/public/bin/tcpgetfiles logfilename xmlbuffer\n\n");

  printf("Sample:/htidc/public/bin/tcpgetfiles /log/shqx/tcpgetfiles_surfdata.log \"<ip>172.16.0.15</ip><port>5010</port><ptype>1</ptype><clientpath>/data/shqx/sdata/surfdata</clientpath><srvpath>/data/shqx/tcp/surfdata</srvpath><srvpathbak>/data/shqx/tcp/surfdatabak</srvpathbak><andchild>true</andchild><matchname>SURF_*.TXT,*.DAT</matchname><okfilename>/data/shqx/tcplist/tcpgetfiles_surfdata.xml</okfilename><timetvl>10</timetvl>\"\n\n\n");

  printf("����һ��ͨ�õĹ���ģ�飬����TCPЭ���ȡ�ļ��Ŀͻ��ˡ�\n");
  printf("logfilename   ���������е���־�ļ���\n");
  printf("xmlbuffer     ���������еĲ��������£�\n");
  printf("ip            �������˵�IP��ַ��\n");
  printf("port          �������˵Ķ˿ڡ�\n");
  printf("clientpath    �ͻ����ļ���ŵĸ�Ŀ¼��\n");
  printf("srvpath       ������ļ���ŵĸ�Ŀ¼��\n");
  printf("ptype         �ļ���ȡ�ɹ��������ļ��Ĵ���ʽ��1-�����ļ���2-ɾ���ļ���3-�ƶ�������Ŀ¼��\n");
  printf("srvpathbak    �ļ��ɹ���ȡ�󣬷�����ļ����ݵĸ�Ŀ¼����ptype==3ʱ��Ч��ȱʡΪ�ա�\n");
  printf("andchild      �Ƿ��ȡsrvpathĿ¼�¸�����Ŀ¼���ļ���true-�ǣ�false-��ȱʡΪfalse��\n");
  printf("matchname     ����ȡ�ļ�����ƥ�䷽ʽ����\"*.TXT,*.XML\"��ע���ô�д��\n");
  printf("okfilename    �ѻ�ȡ�ɹ��ļ����嵥��ȱʡΪ�ա�\n");
  printf("timetvl       ɨ�豾��Ŀ¼�ļ���ʱ��������λ���룬ȡֵ��1-50֮�䡣\n\n\n");
}

// ��xml����������starg�ṹ��
bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg,0,sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer,"ip",starg.ip);
  if (strlen(starg.ip)==0) { logfile.Write("ip is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"port",&starg.port);
  if ( starg.port==0) { logfile.Write("port is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"ptype",&starg.ptype);
  if ((starg.ptype!=1)&&(starg.ptype!=2)&&(starg.ptype!=3) ) { logfile.Write("ptype not in (1,2,3).\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"clientpath",starg.clientpath);
  if (strlen(starg.clientpath)==0) { logfile.Write("clientpath is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"srvpathbak",starg.srvpathbak);
  if ((starg.ptype==3)&&(strlen(starg.srvpathbak)==0)) { logfile.Write("srvpathbak is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"srvpath",starg.srvpath);
  if (strlen(starg.srvpath)==0) { logfile.Write("srvpath is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"andchild",&starg.andchild);

  GetXMLBuffer(strxmlbuffer,"matchname",starg.matchname);
  if (strlen(starg.matchname)==0) { logfile.Write("matchname is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"okfilename",starg.okfilename);
  if ((starg.ptype==1)&&(strlen(starg.okfilename)==0)) { logfile.Write("okfilename is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"timetvl",&starg.timetvl);
  if (starg.timetvl==0) { logfile.Write("timetvl is null.\n"); return false; }

  if (starg.timetvl>50) starg.timetvl=50;

  return true;
}


// ��¼������
bool ClientLogin(const char *argv)
{
  if (TcpClient.m_sockfd>0) return true;

  int ii=0;

  while (true)
  {
    if (ii++>0) sleep(20);    // ��һ�ν���ѭ��������

    // ���������������
    if (TcpClient.ConnectToServer(starg.ip,starg.port) == false)
    {
      logfile.Write("TcpClient.ConnectToServer(%s,%d) failed.\n",starg.ip,starg.port); continue;
    }

    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    memset(strSendBuffer,0,sizeof(strSendBuffer));

    strcpy(strSendBuffer,argv); strcat(strSendBuffer,"<clienttype>2</clienttype>");

    // logfile.Write("1 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
    if (TcpClient.Write(strSendBuffer) == false)
    {
      logfile.Write("1 TcpClient.Write() failed.\n"); continue;
    }

    if (TcpClient.Read(strRecvBuffer,20) == false)
    {
      logfile.Write("1 TcpClient.Read() failed.\n"); continue;
    }
    // logfile.Write("1 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

    break;
  }

  logfile.Write("login(%s,%d) ok.\n",starg.ip,starg.port);

  return true;
}

// �����˷�����������
bool ActiveTest()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  strcpy(strSendBuffer,"<activetest>ok</activetest>");

  // logfile.Write("2 strSendBuffer=%s\n",strSendBuffer);  // xxxxxx
  if (TcpClient.Write(strSendBuffer) == false)
  {
    logfile.Write("2 TcpClient.Write() failed.\n"); TcpClient.Close(); return false;
  }

  if (TcpClient.Read(strRecvBuffer,20) == false)
  {
    logfile.Write("2 TcpClient.Read() failed.\n"); TcpClient.Close(); return false;
  }
  // logfile.Write("2 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  if (strcmp(strRecvBuffer,"ok") != 0) { TcpClient.Close(); return false; }

  return true;
}

// ʵ���ļ���ȡ�Ĺ���
bool _tcpgetfiles()
{
  // �ѷ����srvpathĿ¼�µ��ļ����ص�vlistfile������
  if (LoadListFile()==false)
  {
    logfile.Write("LoadListFile() failed.\n"); TcpClient.Close(); return false;
  }

  if (starg.ptype==1)
  {
    // ����okfilename�ļ��е����ݵ�����vokfilename��
    LoadOKFileName();

    // ��vlistfile�����е��ļ���vokfilename�������ļ��Աȣ��õ���������
    // һ����vlistfile�д��ڣ����Ѿ��ɼ��ɹ����ļ�vokfilename1
    // ������vlistfile�д��ڣ����ļ�����Ҫ���²ɼ����ļ�vlistfile1
    CompVector();

    // ��vokfilename1�����е�������д��okfilename�ļ��У�����֮ǰ�ľ�okfilename�ļ�
    WriteToOKFileName();
   
    // ��vlistfile1�����е����ݸ��Ƶ�vlistfile������
    vlistfile.clear(); vlistfile.swap(vlistfile1);
  }

  // �ӷ���������ȡ���ļ����ѸĶ������ļ�
  for (int ii=0;ii<vlistfile.size();ii++)
  {
    // �����˷��ͽ���ȡ�����أ����ļ�����Ϣ
    memset(strSendBuffer,0,sizeof(strSendBuffer));
    sprintf(strSendBuffer,"<filename>%s</filename><filesize>%d</filesize><mtime>%s</mtime>",vlistfile[ii].filename,vlistfile[ii].filesize,vlistfile[ii].mtime);
    // logfile.Write("3 strSendBuffer=%s\n",strSendBuffer);     // xxxxxx  
    if (TcpClient.Write(strSendBuffer) == false)
    {
      logfile.Write("3 TcpClient.Write() failed.\n"); TcpClient.Close(); return false;
    }

    // �˱�����Щ���࣬����Ϊ�˼���SendFile��RecvFile���������Ƕ����ܲ�����Ӱ�졣
    if (TcpClient.Read(strRecvBuffer) == false)
    {
      logfile.Write("3 TcpClient.Read() failed.\n"); TcpClient.Close(); return false;
    }
    // logfile.Write("3 strRecvBuffer=%s\n",strRecvBuffer);     // xxxxxx  
    
    // ���ļ����е�clientpath�滻��srvpath��ҪС�ĵ���������
    struct st_fileinfo stfileinfo;
    memset(&stfileinfo,0,sizeof(struct st_fileinfo));
    strcpy(stfileinfo.filename,vlistfile[ii].filename);
    strcpy(stfileinfo.mtime,vlistfile[ii].mtime);
    stfileinfo.filesize=vlistfile[ii].filesize;
    UpdateStr(stfileinfo.filename,starg.srvpath,starg.clientpath);

    logfile.Write("get %s ...",stfileinfo.filename);

    // �����ļ�������
    if (RecvFile(&logfile,TcpClient.m_sockfd,&stfileinfo)== false)
    {
      logfile.Write("RecvFile() failed.\n"); TcpClient.Close(); return false;
    }

    logfile.WriteEx("ok.\n");

    // ���ptype==1���Ѳɼ��ɹ����ļ���¼׷�ӵ�okfilename�ļ���
    if (starg.ptype==1) AppendToOKFileName(&vlistfile[ii]);
  }

  return true;
}

// �ѷ����srvpathĿ¼�µ��ļ����ص�vlistfile������
bool LoadListFile()
{
  // 1���ͻ������˷���һ�������ģ�
  // 2�������Dirɨ��srvpath�����ļ����õ�һ���ļ��嵥��
  // 3���������ͻ��˷���һ���ļ������ı��ģ�
  // 4�����ļ���Ϣһ��һ���Ĵ����ͻ��ˣ�
  // 5���ͻ��˽��յ��ļ�¼ֱ�Ӵ����vlistfile�����С�

  vlistfile.clear();

  memset(strSendBuffer,0,sizeof(strSendBuffer));
  strcpy(strSendBuffer,"<list>");
  // logfile.Write("4 strSendBuffer=%s\n",strSendBuffer);     // xxxxxx  
  if (TcpClient.Write(strSendBuffer) == false)
  {
    logfile.Write("4 TcpClient.Write() failed.\n"); return false;
  }

  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  if (TcpClient.Read(strRecvBuffer,20) == false)
  {
    logfile.Write("4 TcpClient.Read() failed.\n"); return false;
  }
  // logfile.Write("4 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  int totalfile=0;
  GetXMLBuffer(strRecvBuffer,"totalfile",&totalfile);

  struct st_fileinfo stfileinfo;

  for (int ii=0;ii<totalfile;ii++)
  {
    memset(&stfileinfo,0,sizeof(struct st_fileinfo));

    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    if (TcpClient.Read(strRecvBuffer,20) == false)
    {
      logfile.Write("5 TcpClient.Read() failed.\n"); return false;
    }
    // logfile.Write("5 strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

    GetXMLBuffer(strRecvBuffer,"filename",stfileinfo.filename);
    GetXMLBuffer(strRecvBuffer,"filesize",&stfileinfo.filesize);
    GetXMLBuffer(strRecvBuffer,"mtime",stfileinfo.mtime);
    
    vlistfile.push_back(stfileinfo);
    // logfile.Write("vlistfile filename=%s,mtime=%s\n",stfileinfo.filename,stfileinfo.mtime);
  }

  return true;
}

// ��okfilename�ļ����ݼ��ص�vokfilename������
bool LoadOKFileName()
{
  vokfilename.clear();

  CFile File;

  // ע�⣺��������ǵ�һ�βɼ���okfilename�ǲ����ڵģ������Ǵ�������Ҳ����true��
  if (File.Open(starg.okfilename,"r") == false) return true;

  struct st_fileinfo stfileinfo;

  char strbuffer[301];

  while (true)
  {
    memset(&stfileinfo,0,sizeof(struct st_fileinfo));

    if (File.Fgets(strbuffer,300,true)==false) break;

    GetXMLBuffer(strbuffer,"filename",stfileinfo.filename,300);
    GetXMLBuffer(strbuffer,"mtime",stfileinfo.mtime,20);

    vokfilename.push_back(stfileinfo);

    // logfile.Write("vokfilename filename=%s,mtime=%s\n",stfileinfo.filename,stfileinfo.mtime);
  }

  return true;
}

// ��vlistfile�����е��ļ���vokfilename�������ļ��Աȣ��õ���������
// һ����vlistfile�д��ڣ����Ѿ��ɼ��ɹ����ļ�vokfilename1
// ������vlistfile�д��ڣ����ļ�����Ҫ���²ɼ����ļ�vlistfile1
bool CompVector()
{
  vokfilename1.clear();  vlistfile1.clear();

  for (int ii=0;ii<vlistfile.size();ii++)
  {
    int jj=0;
    for (jj=0;jj<vokfilename.size();jj++)
    {
      if ( (strcmp(vlistfile[ii].filename,vokfilename[jj].filename)==0) &&
           (strcmp(vlistfile[ii].mtime,vokfilename[jj].mtime)==0) )
      {
        vokfilename1.push_back(vlistfile[ii]); break;
      }
    }

    if (jj==vokfilename.size())
    {
      vlistfile1.push_back(vlistfile[ii]);
    }
  }

  /*
  for (int ii=0;ii<vokfilename1.size();ii++)
  {
    logfile.Write("vokfilename1 filename=%s,mtime=%s\n",vokfilename1[ii].filename,vokfilename1[ii].mtime);
  }

  for (int ii=0;ii<vlistfile1.size();ii++)
  {
    logfile.Write("vlistfile1 filename=%s,mtime=%s\n",vlistfile1[ii].filename,vlistfile1[ii].mtime);
  }
  */

  return true;
}

// ��vokfilename1�����е�������д��okfilename�ļ��У�����֮ǰ�ľ�okfilename�ļ�
bool WriteToOKFileName()
{
  CFile File;

  // ע�⣬���ļ���Ҫ���û������
  if (File.Open(starg.okfilename,"w",false) == false)
  {
    logfile.Write("File.Open(%s) failed.\n",starg.okfilename); return false;
  }

  for (int ii=0;ii<vokfilename1.size();ii++)
  {
    File.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n",vokfilename1[ii].filename,vokfilename1[ii].mtime);
  }

  return true;
}

// ���ptype==1���Ѳɼ��ɹ����ļ���¼׷�ӵ�okfilename�ļ���
bool AppendToOKFileName(struct st_fileinfo *stfileinfo)
{
  CFile File;

  // ע�⣬���ļ���Ҫ���û������
  if (File.Open(starg.okfilename,"a",false) == false)
  {
    logfile.Write("File.Open(%s) failed.\n",starg.okfilename); return false;
  }

  File.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n",stfileinfo->filename,stfileinfo->mtime);

  return true;
}

