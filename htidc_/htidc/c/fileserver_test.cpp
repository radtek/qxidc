/*
    �ļ�����ķ������
*/

#include "_cmpublic.h"
#include "_public.h"

// �����˳�ʱ���õĺ���
void FathEXIT(int sig);
void ChldEXIT(int sig);

char strlocalrootpath[301];
char strlocalrootpathbak[301];
char strdstrootpath[301];
char strmatchname[301];
char strandchild[11];
int  iptype=0;
UINT utimetvl;
UINT ucheckcount;
UINT uconntype=0;
BOOL bandchild=FALSE;
char strXmlBuffer[4001];

// ���ʹ�������ļ������ļ���С��Ϣ
BOOL WriteFileInfo();

// ���ʹ�������ļ�����
int  fd=-1;
BOOL PutFile();

// ������������
BOOL WriteActiveTest();

CDir Dir;

CLogFile logfile;

char strRecvBuffer[TCPBUFLEN+10]; // ���ձ��ĵĻ�����
char strSendBuffer[TCPBUFLEN+10]; // ���ͱ��ĵĻ�����

struct st_FileInfo
{
  char filename[301];    // ȫ·�����ļ���
  UINT filesize;         // �ļ��Ĵ�С
  char mtime[21];        // �ļ���ʱ��
  char filenametmp[301]; // ȫ·������ʱ�ļ���
} stFileInfo;

CProgramActive ProgramActive;
   
// ���մ�������ļ������ļ���С��Ϣ
BOOL ReadFileInfo();

// ���մ������ļ�����
BOOL ReadFile();

// TCP��ز�������
CTcpServer TcpServer;

// �ȴ���¼
int conntype=0;      // 1-���ڷ��ͣ�2-���ڽ���
BOOL ClientLogin();

#define MAXFILECOUNT 100

UINT totalcount=0;
char totalfiles[MAXFILECOUNT][301];

// �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
BOOL CheckSendFiles();

// �����ļ�������
void RecvFilesMain();

// �����ļ�������
void SendFilesMain();

// ������¼��XML
BOOL AXMLBuffer();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/fileserver logfilename port\n");

    printf("Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/bin/fileserver /htidc/htidc/log/fileserver.log 5005\n\n");
    printf("logfilename ��־�ļ�����\n");
    printf("port ���ڴ����ļ���TCP�˿ڡ�\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,FathEXIT); signal(SIGTERM,FathEXIT);

  // �򿪳���������־������һ������̳�����־���ܱ���
  if (logfile.Open(argv[1],"a+",FALSE) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("fileserver");

  logfile.Write("fileserver started(%s).\n",argv[2]);

  if (TcpServer.InitServer(atoi(argv[2])) == FALSE)
  {
    logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[2]); exit(-1);
  }

  while (TRUE)
  {
    // �ȴ��ͻ��˵�����
    if (TcpServer.Accept() == FALSE) 
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

    char strProgramName[51];
    memset(strProgramName,0,sizeof(strProgramName));
    snprintf(strProgramName,50,"fileserver_%05d",getpid());
    ProgramActive.SetProgramInfo(&logfile,strProgramName,300);

    // �ȴ���¼
    if (ClientLogin() == FALSE) ChldEXIT(0);

    // �����ļ�������
    if (conntype==1) RecvFilesMain();

    // �����ļ�������
    if (conntype==2) SendFilesMain();

    logfile.Write("�Ƿ����ӣ�%s\n",strRecvBuffer);

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

  if (fd>0) { close(fd); fd=-1; }

  TcpServer.CloseClient();

  exit(0);
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

    if (TcpServer.Read(strRecvBuffer,180) == FALSE)
    {
      logfile.Write("TcpServer.Read() FAILED.\n"); return FALSE;
    }

    logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

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
            logfile.Write("recv %s ok(%ld).\n",strfilename,FileSize(strfilename));
          }
        }
      }
      else
      {
        // �˶�ʧ�ܣ�ɾ����ʱ�ļ�
        for (UINT ii=0;ii<totalcount;ii++) REMOVE(totalfiles[ii]);
      }

      if (TcpServer.Write(strSendBuffer) == FALSE)
      {
        logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
      }

      logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

      totalcount=0;
      memset(totalfiles,0,sizeof(totalfiles));
  
      continue;
    }

    // �����ȡ�������������ģ��ͼ����ȶԷ������ļ���Ϣ
    if (strstr(strRecvBuffer,"<activetest>") != 0) 
    {
      strcpy(strSendBuffer,"ok");

      if (TcpServer.Write(strSendBuffer) == FALSE)
      {
        logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
      }

      logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

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

    if ( (bytes=recv(TcpServer.m_connfd,buffer,onread,0)) <= 0 ) 
    { 
      logfile.Write("recv() FAILED.\n"); close(fd); fd=-1; return FALSE; 
    }

    write(fd,buffer,bytes);

    total_bytes = total_bytes + bytes;

    if ((UINT)total_bytes == stFileInfo.filesize) break;
  }

  close(fd);

  // �����ļ���ʱ��
  UTime(stFileInfo.filenametmp,stFileInfo.mtime);

  fd=-1;

  return TRUE;
}

// �ȴ���¼
BOOL ClientLogin()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  if (TcpServer.Read(strRecvBuffer,180) == FALSE)
  {
    logfile.Write("TcpServer.Read() FAILED.\n"); return FALSE;
  }

  logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  GetXMLBuffer(strRecvBuffer,"conntype",&conntype);

  if ( (conntype==1) || (conntype==2) ) 
    strcpy(strSendBuffer,"ok");
  else
    strcpy(strSendBuffer,"failed");

  if (TcpServer.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
  }

  logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  logfile.Write("%s login %s.\n%s\n",TcpServer.GetIP(),strSendBuffer,strRecvBuffer);

  if (strcmp(strSendBuffer,"failed") == 0) return FALSE;

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));
  strcpy(strXmlBuffer,strRecvBuffer);

  // ������¼��XML
  AXMLBuffer();
  
  // Ϊ�˼��ݾɵĿͻ��˳����������д�����ʱ��ô����
  iptype=2;
  if (strlen(strlocalrootpathbak)==0) iptype=3;

  if (ucheckcount>MAXFILECOUNT) ucheckcount=MAXFILECOUNT;

  if ( (strcmp(strandchild,"TRUE") == 0) || (strcmp(strandchild,"true") == 0) ) bandchild=TRUE;

  return TRUE;
}

// �����ļ�������
void RecvFilesMain()
{
  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ���մ������ļ������ļ���С
    if (ReadFileInfo() == FALSE) 
    {
      logfile.Write("ReadFileInfo() FAILED.\n"); ChldEXIT(0);
    }

    // ���մ������ļ�����
    if (ReadFile() == FALSE) ChldEXIT(0);

    strncpy(totalfiles[totalcount],stFileInfo.filenametmp,300);

    totalcount++;
  }
}

// �����ļ�
void SendFilesMain()
{
  CTimer ActiveTimer;

  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // �����ļ�ʱ��ĸ�ʽ
    Dir.SetDateFMT("yyyymmddhh24miss");

    // �򿪴������ļ���Ŀ¼
    if (Dir.OpenDirNoSort(strlocalrootpath,bandchild) == FALSE)
    {
      logfile.Write("Dir.OpenDirNoSort(%s) failed.\n",strlocalrootpath); ChldEXIT(-1);
    }

    totalcount=0;
    memset(totalfiles,0,sizeof(totalfiles));

    BOOL bHaveFiles=FALSE;

    while (TRUE)
    {
      // д����̻��Ϣ
      ProgramActive.WriteToFile();

      // ��ȡһ���ļ�
      if (Dir.ReadDir() == FALSE) break;

      // ����ļ�����ƥ����׺ΪTMP���Ͳ����䣬������
      if ( (MatchFileName(Dir.m_FileName,strmatchname)==FALSE) ||
           (MatchFileName(Dir.m_FileName,"*.TMP,*.SWP")== TRUE) ) continue;

      bHaveFiles=TRUE;

      // ���ʹ�������ļ������ļ���С��Ϣ
      if (WriteFileInfo() == FALSE)
      {
        logfile.Write("WriteFileInfo() FAILED.\n"); ChldEXIT(-1);
      }

      // ���ʹ�������ļ�����
      if (PutFile() == FALSE)
      {
        logfile.Write("PutFile() FAILED.\n"); ChldEXIT(-1);
      }

      strncpy(totalfiles[totalcount],Dir.m_FullFileName,300);
      totalcount++;

      if (totalcount>=ucheckcount)
      {
        // �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
        if (CheckSendFiles() == FALSE)
        {
          logfile.Write("CheckSendFiles() FAILED.\n"); ChldEXIT(-1);
        }
      }
    }

    if (totalcount>0)
    {
      // �����˺˶��ļ�����������˶Գɹ�����ɾ��totalfiles�����е��ļ�
      if (CheckSendFiles() == FALSE)
      {
        logfile.Write("CheckSendFiles() FAILED.\n"); ChldEXIT(-1);
      }
    }

    if (bHaveFiles==FALSE)
    {
      // ˯��utimetvl��
      for (UINT ii=0;ii<utimetvl;ii++)
      {
        sleep(1);

        if (ActiveTimer.Elapsed() > 30 )
        {
          ActiveTimer.Beginning();

          // ������������
          if (WriteActiveTest() == FALSE) { logfile.Write("WriteActiveTest FAILED.\n"); ChldEXIT(-1); }
        }
      }
    }
  }
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
  if (TcpServer.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
  }

  logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  return TRUE;
}

// ������������
BOOL WriteActiveTest()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,500,"<activetest>");

  // �����˷�����������
  if (TcpServer.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
  }

  logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  // �ȴ�����˵�������Ӧ����
  if (TcpServer.Read(strRecvBuffer,180) == FALSE)
  {
    logfile.Write("TcpServer.Read() FAILED.\n"); return FALSE;
  }

  logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

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
    logfile.Write("open %s failed.\n",Dir.m_FullFileName); return FALSE;
  }

  while (TRUE)
  {
    memset(buffer,0,sizeof(buffer));

    if ((Dir.m_FileSize-total_bytes) > 1000) onread=1000;
    else onread=Dir.m_FileSize-total_bytes;

    bytes=read(fd,buffer,onread);

    if (bytes > 0)
    {
      if (Writen(TcpServer.m_connfd,buffer,bytes) == FALSE)
      {
        logfile.Write("Writen() FAILED.\n"); close(fd); fd=-1; return FALSE;
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

  if (TcpServer.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
  }

  logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  if (TcpServer.Read(strRecvBuffer,180) == FALSE)
  {
    logfile.Write("TcpServer.Read() FAILED.\n"); return FALSE;
  }

  logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  if (strcmp(strRecvBuffer,"ok") != 0) return TRUE;

  char strbakfilename[301];

  // �˶Գɹ���ɾ�����ص��ļ�
  for (UINT ii=0;ii<totalcount;ii++)
  {
    // �����ѷ��͹����ļ�
    if ( iptype==2 )
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

    // ɾ���ѷ��͹����ļ�
    if ( iptype==3 )
    {
      // ���ɾ��ʧ�ܣ�д��ʧ����־�������򲻿��˳�������������
      if (REMOVE(totalfiles[ii]) == FALSE)
      {
        logfile.Write("REMOVE %s failed.\n",totalfiles[ii]);
      }
    }

    logfile.Write("send %s ok.\n",totalfiles[ii]);
  }

  totalcount=0;
  memset(totalfiles,0,sizeof(totalfiles));

  return TRUE;
}

// ������¼��XML
BOOL AXMLBuffer()
{
  int itimeoffset=0;

  GetXMLBuffer(strXmlBuffer,"timeoffset",&itimeoffset);

  char strTMPBuffer[4001];
  memset(strTMPBuffer,0,sizeof(strTMPBuffer));
  strncpy(strTMPBuffer,strXmlBuffer,4000);

  // ����xmlbuffer�е�ʱ�����
  MatchBuffer(strTMPBuffer,itimeoffset);
  
  GetXMLBuffer(strTMPBuffer,"dstrootpath",strlocalrootpath,300);
  GetXMLBuffer(strTMPBuffer,"dstrootpathbak",strlocalrootpathbak,300);
  GetXMLBuffer(strTMPBuffer,"localrootpath",strdstrootpath,300);
  GetXMLBuffer(strTMPBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strTMPBuffer,"andchild",strandchild,10);
  GetXMLBuffer(strTMPBuffer,"timetvl",&utimetvl);    
  GetXMLBuffer(strTMPBuffer,"checkcount",&ucheckcount);
  GetXMLBuffer(strTMPBuffer,"ptype",&iptype);

  return TRUE;
}

