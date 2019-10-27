/*
    �ļ�����Ŀͻ�����
*/

#include "_cmpublic.h"
#include "_public.h"

// �����˳�ʱ���õĺ���
void EXIT(int sig);

CDir Dir;

CLogFile       logfile;
CProgramActive ProgramActive;

struct st_FileInfo
{
  char filename[301];    // ȫ·�����ļ���
  UINT filesize;         // �ļ��Ĵ�С
  char filenametmp[301]; // ȫ·������ʱ�ļ���
  char mtime[21];        // �ļ���ʱ��
} stFileInfo;

char strlocalrootpath[301];
char strlocalrootpathbak[301];
char strdstrootpath[301];
UINT uptype;
char strdstrootpathbak[301];
char strurl[101];
char strmatchname[301];
char strandchild[11];
UINT utimetvl;
UINT ucheckcount;
UINT uconntype=0;
BOOL bandchild=FALSE;
char strstarttime[301];
char strXmlBuffer[4001];

char strRecvBuffer[TCPBUFLEN+10]; // ���ձ��ĵĻ�����
char strSendBuffer[TCPBUFLEN+10]; // ���ͱ��ĵĻ�����

// TCP��ز�������
CTcpClient TcpClient;

// ���ʹ�������ļ������ļ���С��Ϣ
BOOL WriteFileInfo();

// ���ʹ�������ļ�����
int  fd=-1;
BOOL PutFile();

// ������������
BOOL WriteActiveTest();

// ��¼������
BOOL ClientLogin();

#define MAXFILECOUNT 100

UINT totalcount=0;
char totalfiles[MAXFILECOUNT][301];
// �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
BOOL CheckSendFiles();

// �����ļ�������
void SendFilesMain();
// �����ļ�������
void RecvFilesMain();

// ���մ�������ļ������ļ���С��Ϣ
BOOL ReadFileInfo();

// ���մ������ļ�����
BOOL ReadFile();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/fileclient logfilename xmlbuffer\n");

    printf("Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/bin/fileclient /htidc/htidc/log/fileclient_aaa.log \"<localrootpath>/tmp/aaa</localrootpath><ptype>3</ptype><localrootpathbak>/tmp/aaa</localrootpathbak><dstrootpath>/tmp/bbb</dstrootpath><url>127.0.0.1,5005</url><matchname>*.cpp</matchname><andchild>FALSE</andchild><timetvl>120</timetvl><checkcount>80</checkcount><conntype>1</conntype><timeoffset>0</timeoffset>\"\n");

    printf("        /htidc/htidc/bin/procctl 10 /htidc/htidc/bin/fileclient /htidc/htidc/log/fileclient_bbb.log \"<localrootpath>/tmp/aaa</localrootpath><dstrootpath>/tmp/bbb</dstrootpath><ptype>2</ptype><dstrootpathbak>/tmp/ccc/</dstrootpathbak><url>127.0.0.1,5005</url><matchname>*.cpp</matchname><andchild>FALSE</andchild><timetvl>120</timetvl><checkcount>80</checkcount><conntype>2</conntype><timeoffset>0</timeoffset>\"\n\n");

    /*
    /htidc/htidc/c/fileclient /tmp/fileclient.log "<localrootpath>/qxfwclient</localrootpath>><dstrootpath>/qxfw</dstrootpath><dstrootpathbak>/qxfwbak</dstrootpathbak><url>127.0.0.1,5005</url><matchname>*.JPG</matchname><andchild>TRUE</andchild><timetvl>120</timetvl><checkcount>80</checkcount><conntype>2</conntype>"
    */

    printf("��������fileserverͨѶ������TCP��socketͨѶ��ʵ���ļ����շ����ܡ�\n\n");

    printf("logfilename ��־�ļ�����\n");
    printf("xmlbuffer   �ļ������ȫ��������ÿ���ֶεĺ������£�ע�⣬�����ļ��ͽ����ļ����ֶ�ȡֵ�ǲ�ͬ�ġ�\n\n");

    printf("1�������ļ�\n");
    printf("localrootpath    �����ļ���ŵĸ�Ŀ¼��\n");
    printf("ptype            �ļ����ͳɹ��󣬿ͻ����ļ��Ĵ���ʽ��1-�����ļ���2-�ƶ�������Ŀ¼��3-ɾ���ļ���ֻ����2��3��"\
                            "1��ʱ�����á�\n");
    printf("localrootpathbak �ļ��ɹ����ͺ󣬱����ļ����ݵĸ�Ŀ¼��ע�⣬���localrootpathbakΪ�գ�"\
                            "�ļ��ɹ����ͺ󽫱�ɾ�������localrootpathbak����localrootpath���ļ��ɹ����ͺ����κδ���"\
                            "������������������ļ�����ô��������һ��ִ�е�ʱ��֮ǰ���͹����ļ��ᱻ�ط���"\
                            "localrootpathbak��һ����ѡ�ֶΣ����Բ����ڡ�\n");
    printf("dstrootpath      ������ļ���ŵĸ�Ŀ¼��\n");
    printf("url              �������˵�IP��ַ��ͨѶ�˿ڣ���10.148.124.85,5005��\n");
    printf("matchname        �������ļ�����ƥ�䷽ʽ����\"*.TXT,*.XML\"��ע���ô�д��\n");
    printf("andchild         �Ƿ���localrootpathĿ¼�¸�����Ŀ¼���ļ���TRUE-�ǣ�FALSE-�����ΪTRUE��"\
                            "�ڷ�������dstrootpathĿ¼�У���������localrootpath��ͬ��Ŀ¼�ṹ��\n");
    printf("timetvl          �����ļ���ʱ��������λ���롣\n");
    printf("checkcount       ÿ�η����ļ���ʱ�������˺˶��ļ��ĸ�������СΪ1�����Ϊ100���������100��ǿ��Ϊ100��\n");
    printf("conntype         �����ļ��̶�ȡֵΪ1��\n");
    printf("starttime        �����ļ���ʱ�䣬���Ǹ���ѡ�ֶΣ����Ϊ�գ������κ�ʱ�䶼���ͣ�"\
                            "�����Ϊ�գ�ֻ�е�ǰʱ���Сʱ��starttime�ڣ��Żᷢ���ļ�������ʱ�䲻���͡�"\
                            "starttime��ȡֵΪСʱ������\"00,01,12\"��\n\n");
    printf("timeoffset      �����ʱ��ƫ�ƣ���λ�����ӡ������������ƥ��{yyyy}��{yyy}��{yy}��{mm}��{dd}��{hh}��{mi}��"\
                           "�ֱ����4λ���ꡢ����λ���ꡢ����λ���ꡢ�¡��졢Сʱ�����ӡ�\n");
    printf("���ϵĲ����У�ֻ��localrootpathbak��starttime�ǿ�ѡ�����������Ķ����\n\n\n");

    printf("2�������ļ�\n");
    printf("localrootpath  �����ļ���ŵĸ�Ŀ¼��\n");
    printf("dstrootpath    ������ļ���ŵĸ�Ŀ¼��\n");
    printf("ptype          �ļ����ճɹ��󣬷�����ļ��Ĵ���ʽ��1-�����ļ���2-�ƶ�������Ŀ¼��3-ɾ���ļ���ֻ����2��3��"\
                          "1��ʱ�����á�\n");
    printf("dstrootpathbak �ļ����ճɹ��󣬷�����ļ��ı���Ŀ¼��ֻ�е�ptype=2ʱ�����ֶβ������塣\n");
    printf("url            �������˵�IP��ַ��ͨѶ�˿ڣ���10.148.124.85,5005��\n");
    printf("matchname      �������ļ�����ƥ�䷽ʽ����\"*.TXT,*.XML\"��ע���ô�д��\n");
    printf("andchild       �Ƿ����dstrootpathĿ¼�¸�����Ŀ¼���ļ���TRUE-�ǣ�FALSE-�����ΪTRUE��"\
                          "�ڷ�������localrootpathĿ¼�У���������dstrootpath��ͬ��Ŀ¼�ṹ��\n");
    printf("timetvl        �����ļ���ʱ��������λ���롣\n");
    printf("checkcount     ÿ�ν����ļ���ʱ�������˺˶��ļ��ĸ�������СΪ1�����Ϊ100���������100��ǿ��Ϊ100��\n");
    printf("conntype       �����ļ��̶�ȡֵΪ2��\n");
    printf("starttime      �����ļ���ʱ�䣬���Ǹ���ѡ�ֶΣ����Ϊ�գ������κ�ʱ�䶼���գ�"\
                          "�����Ϊ�գ�ֻ�е�ǰʱ���Сʱ��starttime�ڣ��Ż�����ļ�������ʱ�䲻���ա�"\
                          "starttime��ȡֵΪСʱ������\"00,01,12\"��\n\n");
    printf("timeoffset     �����ʱ��ƫ�ƣ���λ�����ӡ������������ƥ��{yyyy}��{yyy}��{yy}��{mm}��{dd}��{hh}��{mi}��"\
                          "�ֱ����4λ���ꡢ����λ���ꡢ����λ���ꡢ�¡��졢Сʱ�����ӡ�\n");
    printf("���ϵĲ����У�ֻ��dstrootpathbak��starttime�ǿ�ѡ�����������Ķ����\n\n\n");

    return -1;
  }

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));
  strncpy(strXmlBuffer,argv[2],4000);

  memset(strlocalrootpath,0,sizeof(strlocalrootpath));
  memset(strlocalrootpathbak,0,sizeof(strlocalrootpathbak));
  memset(strdstrootpath,0,sizeof(strdstrootpath));
  memset(strurl,0,sizeof(strurl));
  memset(strmatchname,0,sizeof(strmatchname));
  memset(strandchild,0,sizeof(strandchild));
  utimetvl=0;
  uptype=0;
  ucheckcount=0;
  uconntype=0;
  memset(strstarttime,0,sizeof(strstarttime));

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // �򿪳���������־
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("fileclient");

  GetXMLBuffer(strXmlBuffer,"localrootpath",strlocalrootpath,300);
  GetXMLBuffer(strXmlBuffer,"localrootpathbak",strlocalrootpathbak,300);
  GetXMLBuffer(strXmlBuffer,"dstrootpath",strdstrootpath,300);
  GetXMLBuffer(strXmlBuffer,"url",strurl,100);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"andchild",strandchild,10);
  GetXMLBuffer(strXmlBuffer,"timetvl",&utimetvl);
  GetXMLBuffer(strXmlBuffer,"checkcount",&ucheckcount);
  GetXMLBuffer(strXmlBuffer,"conntype",&uconntype);
  GetXMLBuffer(strXmlBuffer,"starttime",strstarttime,300);
  GetXMLBuffer(strXmlBuffer,"ptype",&uptype);

  // Ϊ�˼��ݾɵĽű�����ʱ��ô����
  uptype=3;
  if (strlen(strlocalrootpathbak)!=0) uptype=2;

  if (strlen(strlocalrootpath) == 0) { logfile.Write("localrootpath is null.\n"); return -1; }
  // ���localrootpathbakΪ�գ��ļ��ڴ����ɾ��
  // if (strlen(strlocalrootpathbak) == 0) { logfile.Write("localrootpathbak is null.\n"); return -1; }
  if (strlen(strdstrootpath) == 0) { logfile.Write("dstrootpath is null.\n"); return -1; }
  if (strlen(strurl) == 0) { logfile.Write("url is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (strlen(strandchild) == 0) { logfile.Write("andchild is null.\n"); return -1; }
  if (utimetvl == 0) { logfile.Write("timetvl is null.\n"); return -1; }
  if (ucheckcount == 0) { logfile.Write("checkcount is null.\n"); return -1; }
  if (uconntype == 0) { logfile.Write("conntype is null.\n"); return -1; }
  // starttime�ǿ���Ϊ�յġ�
  // if (strlen(strstarttime) == 0) { logfile.Write("starttime is null.\n"); return -1; }

  if (ucheckcount>MAXFILECOUNT) ucheckcount=MAXFILECOUNT;

  if ( (strcmp(strandchild,"TRUE") == 0) || (strcmp(strandchild,"true") == 0) ) bandchild=TRUE;

  // ��ȡ������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬���˳�
  if (strlen(strstarttime) != 0)
  {
    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"hh24mi");
    strLocalTime[2]=0;
    if (strstr(strstarttime,strLocalTime) == 0) { sleep(50); return 0; }
  }

  logfile.Write("fileclient started...");

  char strProgramName[51];
  memset(strProgramName,0,sizeof(strProgramName));
  snprintf(strProgramName,50,"fileclient_%05d",getpid());
  ProgramActive.SetProgramInfo(&logfile,strProgramName,300);

  TcpClient.SetConnectOpt(strurl);

  // ���ӷ����
  if (TcpClient.ConnectToServer() == FALSE)
  {
    logfile.Write("TcpClient.ConnectToServer(%s) failed.\n",strurl); EXIT(-1);
  }

  // ��¼������
  if (ClientLogin() == FALSE)
  {
    logfile.Write("ClientLogin() failed.\n"); EXIT(-1);
  }

  // �����ļ�
  if (uconntype==1) SendFilesMain();

  // �����ļ�
  if (uconntype==2) RecvFilesMain();

  EXIT(0);
}

// �����ļ�
void SendFilesMain()
{
  CTimer ActiveTimer;

  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ��ȡ������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬��return������˯�ߣ�����������˯��
    if (strlen(strstarttime) != 0)
    {
      char strLocalTime[21];
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"hh24mi");
      strLocalTime[2]=0;
      if (strstr(strstarttime,strLocalTime) == 0) return;
    }

    // �����ļ�ʱ��ĸ�ʽ
    Dir.SetDateFMT("yyyymmddhh24miss");

    // �򿪴������ļ���Ŀ¼
    if (Dir.OpenDirNoSort(strlocalrootpath,bandchild) == FALSE)
    {
      logfile.Write("Dir.OpenDirNoSort(%s) failed.\n",strlocalrootpath); EXIT(-1);
    }

    totalcount=0;
    memset(totalfiles,0,sizeof(totalfiles));

    while (TRUE)
    {
      // д����̻��Ϣ
      ProgramActive.WriteToFile();

      // ��ȡһ���ļ�
      if (Dir.ReadDir() == FALSE) break;

      // ����ļ�����ƥ����׺ΪTMP���Ͳ����䣬������
      if ( (MatchFileName(Dir.m_FileName,strmatchname)==FALSE) ||
           (MatchFileName(Dir.m_FileName,"*.TMP,*.SWP")== TRUE) ) continue;

      // ���ʹ�������ļ������ļ���С��Ϣ
      if (WriteFileInfo() == FALSE)
      {
        logfile.Write("FAILED.WriteFileInfo FAILED.\n"); EXIT(-1);
      }

      // ���ʹ�������ļ�����
      if (PutFile() == FALSE)
      {
        logfile.Write("FAILED.PutFile FAILED.\n"); EXIT(-1);
      }

      strncpy(totalfiles[totalcount],Dir.m_FullFileName,300);
      totalcount++;

      if (totalcount>=ucheckcount)
      {
        // �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
        if (CheckSendFiles() == FALSE)
        {
          logfile.Write("CheckSendFiles() FAILED.\n"); EXIT(-1);
        }
      }
    }

    if (totalcount>0)
    {
      // �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
      if (CheckSendFiles() == FALSE)
      {
        logfile.Write("CheckSendFiles() FAILED.\n"); EXIT(-1);
      }
    }

    // ˯��utimetvl��
    for (UINT ii=0;ii<utimetvl;ii++)
    {
      sleep(1);

      if (ActiveTimer.Elapsed() > 30 ) 
      {
        ActiveTimer.Beginning();

        // ������������
        if (WriteActiveTest() == FALSE) { logfile.Write("WriteActiveTest FAILED.\n"); EXIT(-1); }
      }
    }
  }
}

// �����˳�ʱ���õĺ���
void EXIT(int sig)
{
  if (sig > 0)
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  TcpClient.Close();

  if (fd>0) { close(fd); fd=0; }

  logfile.Write("fileclient EXIT.\n");

  exit(0);
}

// ���ʹ�������ļ������ļ���С��Ϣ
BOOL WriteFileInfo()
{
  char strDstFileName[301];
  memset(strDstFileName,0,sizeof(strDstFileName));
  snprintf(strDstFileName,300,"%s%s",strdstrootpath,Dir.m_FullFileName+strlen(strlocalrootpath));

  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,500,\
          "<filename>%s</filename><filesize>%lu</filesize><mtime>%s</mtime>",\
           strDstFileName,Dir.m_FileSize,Dir.m_ModifyTime);

  // �����˷����ļ�������Ϣ
  if (TcpClient.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  return TRUE;
}

// ������������
BOOL WriteActiveTest()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,500,"<activetest>");

  // �����˷�����������
  if (TcpClient.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  // �ȴ�����˵�������Ӧ����
  if (TcpClient.Read(strRecvBuffer,80) == FALSE)
  {
    logfile.Write("TcpClient.Read() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  if (strcmp(strRecvBuffer,"ok") != 0) return FALSE;

  return TRUE;
}

// ���ʹ�������ļ�����
BOOL PutFile()
{
  int  bytes=0;
  int  total_bytes=0;
  int  onread=0;
  char buffer[1000];

  fd=-1;

  if (Dir.m_FileSize == 0) return TRUE;

  if ( (fd=open(Dir.m_FullFileName,O_RDONLY)) < 0 ) 
  {
    logfile.Write("open(%s) failed.\n",Dir.m_FullFileName); return FALSE;
  }

  while (TRUE)
  {
    memset(buffer,0,sizeof(buffer));

    if ((Dir.m_FileSize-total_bytes) > 1000) onread=1000;
    else onread=Dir.m_FileSize-total_bytes;

    bytes=read(fd,buffer,onread);

    if (bytes > 0)
    {
      if (Writen(TcpClient.m_sockfd,buffer,bytes) == FALSE)
      {
        logfile.Write("Writen FAILED.\n"); close(fd); fd=-1; return FALSE;
      }
    }

    total_bytes = total_bytes + bytes;

    if ((UINT)total_bytes == Dir.m_FileSize) break;
  }

  close(fd);

  fd=-1;

  return TRUE;
}

// �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
BOOL CheckSendFiles()
{
  // �����˺˶��ļ�����
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,500,"<totalcount>%lu</totalcount>",totalcount);

  if (TcpClient.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  if (TcpClient.Read(strRecvBuffer,80) == FALSE)
  {
    logfile.Write("TcpClient.Read() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  if (strcmp(strRecvBuffer,"ok") != 0) return TRUE;

  char strbakfilename[301];

  // �˶Գɹ���ɾ�����ص��ļ�
  for (UINT ii=0;ii<totalcount;ii++)
  {
    // ɾ���ѷ��͹����ļ�
    if ( uptype == 3)
    {
      // ���ɾ��ʧ�ܣ�д��ʧ����־�������򲻿��˳�������������
      if (REMOVE(totalfiles[ii]) == FALSE)
      {
        logfile.Write("REMOVE %s failed.\n",totalfiles[ii]); 
      }
    }

    // �����ѷ��͹����ļ�
    if ( uptype == 2)
    {
      memset(strbakfilename,0,sizeof(strbakfilename));
      strcpy(strbakfilename,totalfiles[ii]);
      UpdateStr(strbakfilename,strlocalrootpath,strlocalrootpathbak,FALSE);

      // ����ƶ�ʧ�ܣ�д��ʧ����־�������򲻿��˳�������������
      if (RENAME(totalfiles[ii],strbakfilename) == FALSE)
      {
        logfile.Write("RENAME %s to %s failed.\n",totalfiles[ii],strbakfilename);
      }
    }

    logfile.Write("send %s ok.\n",totalfiles[ii]);
  }

  totalcount=0;
  memset(totalfiles,0,sizeof(totalfiles));

  return TRUE;
}

// ��¼������
BOOL ClientLogin()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  strcpy(strSendBuffer,strXmlBuffer);

  if (TcpClient.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  if (TcpClient.Read(strRecvBuffer,80) == FALSE)
  {
    logfile.Write("TcpClient.Read() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  if (strcmp(strRecvBuffer,"ok") == 0) { logfile.WriteEx("login ok.\n"); return TRUE; }

  logfile.WriteEx("login failed.\n");

  return FALSE;
}

// �����ļ�������
void RecvFilesMain()
{
  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ��ȡ������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬��return������˯�ߣ�����������˯��
    if (strlen(strstarttime) != 0)
    {
      char strLocalTime[21];
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"hh24mi");
      strLocalTime[2]=0;
      if (strstr(strstarttime,strLocalTime) == 0) return;
    }

    // ���մ������ļ������ļ���С
    if (ReadFileInfo() == FALSE)
    {
      logfile.Write("ReadFileInfo() FAILED.\n"); EXIT(0);
    }

    // ���մ������ļ�����
    if (ReadFile() == FALSE) EXIT(0);

    strncpy(totalfiles[totalcount],stFileInfo.filenametmp,300);

    totalcount++;
  }
}

// ���մ�������ļ������ļ���С��Ϣ
BOOL ReadFileInfo()
{
  // �ȴ��ͻ��˵�������
  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    memset(strSendBuffer,0,sizeof(strSendBuffer));

    if (TcpClient.Read(strRecvBuffer,80) == FALSE)
    {
      logfile.Write("TcpClient.Read() FAILED.\n"); return FALSE;
    }

    //logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

    // ����Ǻ˶��ļ���ָ��
    if (strstr(strRecvBuffer,"<totalcount>") != 0)
    {
      UINT totalcountdst=0;
      GetXMLBuffer(strRecvBuffer,"totalcount",&totalcountdst);

      if (totalcountdst==totalcount)
      {
        strcpy(strSendBuffer,"ok");
      }
      else
      {
        strcpy(strSendBuffer,"failed");
      }

      if (totalcountdst==totalcount)
      {
        char strfilename[301];

        for (UINT ii=0;ii<totalcount;ii++)
        {
          memset(strfilename,0,sizeof(strfilename));
          strncpy(strfilename,totalfiles[ii],strlen(totalfiles[ii])-4);
          if (RENAME(totalfiles[ii],strfilename) == FALSE)
          {
            strcpy(strSendBuffer,"failed");
            logfile.Write("RENAME %s to %s FAILED.\n",totalfiles[ii],strfilename);
          }
          else
          {
            logfile.Write("recv %s ok.\n",strfilename);
          }
        }
      }
      else
      {
        // �˶�ʧ�ܣ�ɾ����ʱ�ļ�
        for (UINT ii=0;ii<totalcount;ii++) REMOVE(totalfiles[ii]);
      }

      if (TcpClient.Write(strSendBuffer) == FALSE)
      {
        logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
      }

      //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

      totalcount=0;
      memset(totalfiles,0,sizeof(totalfiles));

      continue;
    }

    // �����ȡ�������������ģ��ͼ����ȶԷ������ļ���Ϣ
    if (strstr(strRecvBuffer,"<activetest>") != 0)
    {
      strcpy(strSendBuffer,"ok");

      if (TcpClient.Write(strSendBuffer) == FALSE)
      {
        logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
      }

      //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

      continue;
    }

    // �����ľ����ļ���Ϣ��
    break;
  }

  memset(&stFileInfo,0,sizeof(stFileInfo));

  GetXMLBuffer(strRecvBuffer,"filename", stFileInfo.filename,300);
  GetXMLBuffer(strRecvBuffer,"filesize",&stFileInfo.filesize);
  GetXMLBuffer(strRecvBuffer,"mtime",stFileInfo.mtime,14);

  snprintf(stFileInfo.filenametmp,300,"%s.tmp",stFileInfo.filename);

  //logfile.Write("filename=%s,size=%lu.\n",stFileInfo.filename,stFileInfo.filesize);

  return TRUE;
}

// ���մ������ļ�����
BOOL ReadFile()
{
  fd=-1;

  if ( (fd=OPEN(stFileInfo.filenametmp,O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IXUSR)) < 0)
  {
    logfile.Write("OPEN %s failed.\n",stFileInfo.filenametmp); return FALSE;
  }

  int  bytes=0;
  int  total_bytes=0;
  int  onread=0;
  char buffer[1000];

  while (TRUE)
  {
    // ����ļ���СΪ0����ʲô������ȡ��ֱ���˳�ѭ��
    if (stFileInfo.filesize==0) break;

    memset(buffer,0,sizeof(buffer));

    if ((stFileInfo.filesize-total_bytes) > 1000) onread=1000;
    else onread=stFileInfo.filesize-total_bytes;

    if ( (bytes=recv(TcpClient.m_sockfd,buffer,onread,0)) <= 0 )
    {
      logfile.Write("recv() FAILED.\n"); close(fd); fd=-1; return FALSE;
    }

    write(fd,buffer,bytes);

    total_bytes = total_bytes + bytes;

    if ((UINT)total_bytes == stFileInfo.filesize) break;
  }

  close(fd);

  fd=-1;

  // �����ļ���ʱ��
  UTime(stFileInfo.filenametmp,stFileInfo.mtime);

  return TRUE;
}

