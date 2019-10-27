/*
    文件传输的服务程序
*/

#include "_cmpublic.h"
#include "_public.h"

// 程序退出时调用的函数
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

// 发送待传输的文件名和文件大小信息
BOOL WriteFileInfo();

// 发送待传输的文件内容
int  fd=-1;
BOOL PutFile();

// 发送心跳报文
BOOL WriteActiveTest();

CDir Dir;

CLogFile logfile;

char strRecvBuffer[TCPBUFLEN+10]; // 接收报文的缓冲区
char strSendBuffer[TCPBUFLEN+10]; // 发送报文的缓冲区

struct st_FileInfo
{
  char filename[301];    // 全路径的文件名
  UINT filesize;         // 文件的大小
  char mtime[21];        // 文件的时间
  char filenametmp[301]; // 全路径的临时文件名
} stFileInfo;

CProgramActive ProgramActive;
   
// 接收待传输的文件名和文件大小信息
BOOL ReadFileInfo();

// 接收待传输文件内容
BOOL ReadFile();

// TCP相关操作的类
CTcpServer TcpServer;

// 等待登录
int conntype=0;      // 1-用于发送；2-用于接收
BOOL ClientLogin();

#define MAXFILECOUNT 100

UINT totalcount=0;
char totalfiles[MAXFILECOUNT][301];

// 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
BOOL CheckSendFiles();

// 接收文件主函数
void RecvFilesMain();

// 发送文件主函数
void SendFilesMain();

// 解析登录的XML
BOOL AXMLBuffer();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/fileserver logfilename port\n");

    printf("Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/bin/fileserver /htidc/htidc/log/fileserver.log 5005\n\n");
    printf("logfilename 日志文件名。\n");
    printf("port 用于传输文件的TCP端口。\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,FathEXIT); signal(SIGTERM,FathEXIT);

  // 打开程序运行日志，这是一个多进程程序，日志不能备份
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
    // 等待客户端的连接
    if (TcpServer.Accept() == FALSE) 
    {
      logfile.Write("TcpServer.Accept() failed.\n"); continue;
    }

    // 新的客户端连接
    if (fork() > 0) 
    { 
      // 父进程关闭刚建立起来的sock连接，并回到Accept继续监听
      TcpServer.CloseClient(); continue; 
    }

    // 进入子进程的流程
    signal(SIGINT,ChldEXIT); signal(SIGTERM,ChldEXIT);

    // 子进程需要关掉监听的sock
    TcpServer.CloseListen();

    char strProgramName[51];
    memset(strProgramName,0,sizeof(strProgramName));
    snprintf(strProgramName,50,"fileserver_%05d",getpid());
    ProgramActive.SetProgramInfo(&logfile,strProgramName,300);

    // 等待登录
    if (ClientLogin() == FALSE) ChldEXIT(0);

    // 接收文件主函数
    if (conntype==1) RecvFilesMain();

    // 发送文件主函数
    if (conntype==2) SendFilesMain();

    logfile.Write("非法连接：%s\n",strRecvBuffer);

    ChldEXIT(0);
  }

  return 0;
}

// 父进程退出时调用的函数
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

// 子进程退出时调用的函数
void ChldEXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  if (fd>0) { close(fd); fd=-1; }

  TcpServer.CloseClient();

  exit(0);
}

// 接收待传输的文件名和文件大小信息
BOOL ReadFileInfo()
{
  // 等待客户端的请求报文
  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    memset(strRecvBuffer,0,sizeof(strRecvBuffer));
    memset(strSendBuffer,0,sizeof(strSendBuffer));

    if (TcpServer.Read(strRecvBuffer,180) == FALSE)
    {
      logfile.Write("TcpServer.Read() FAILED.\n"); return FALSE;
    }

    logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

    // 如果是核对文件数指令
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
        // 核对失败，删除临时文件
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

    // 如果读取到的是心跳报文，就继续等对方发送文件信息
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

    // 其它的就是文件信息了
    break;
  }

  memset(&stFileInfo,0,sizeof(stFileInfo));

  GetXMLBuffer(strRecvBuffer,"filename", stFileInfo.filename,300);
  GetXMLBuffer(strRecvBuffer,"filesize",&stFileInfo.filesize);
  GetXMLBuffer(strRecvBuffer,"mtime",stFileInfo.mtime,14);

  snprintf(stFileInfo.filenametmp,300,"%s.tmp",stFileInfo.filename);

  return TRUE;
}

// 接收待传输文件内容
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
    // 如果文件大小为0，就什么都不读取，直接退出循环
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

  // 重置文件的时间
  UTime(stFileInfo.filenametmp,stFileInfo.mtime);

  fd=-1;

  return TRUE;
}

// 等待登录
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

  // 解析登录的XML
  AXMLBuffer();
  
  // 为了兼容旧的客户端程序，以下两行代码暂时这么处理
  iptype=2;
  if (strlen(strlocalrootpathbak)==0) iptype=3;

  if (ucheckcount>MAXFILECOUNT) ucheckcount=MAXFILECOUNT;

  if ( (strcmp(strandchild,"TRUE") == 0) || (strcmp(strandchild,"true") == 0) ) bandchild=TRUE;

  return TRUE;
}

// 接收文件主函数
void RecvFilesMain()
{
  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 接收待传输文件名和文件大小
    if (ReadFileInfo() == FALSE) 
    {
      logfile.Write("ReadFileInfo() FAILED.\n"); ChldEXIT(0);
    }

    // 接收待传输文件内容
    if (ReadFile() == FALSE) ChldEXIT(0);

    strncpy(totalfiles[totalcount],stFileInfo.filenametmp,300);

    totalcount++;
  }
}

// 发送文件
void SendFilesMain()
{
  CTimer ActiveTimer;

  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 设置文件时间的格式
    Dir.SetDateFMT("yyyymmddhh24miss");

    // 打开待发送文件的目录
    if (Dir.OpenDirNoSort(strlocalrootpath,bandchild) == FALSE)
    {
      logfile.Write("Dir.OpenDirNoSort(%s) failed.\n",strlocalrootpath); ChldEXIT(-1);
    }

    totalcount=0;
    memset(totalfiles,0,sizeof(totalfiles));

    BOOL bHaveFiles=FALSE;

    while (TRUE)
    {
      // 写入进程活动信息
      ProgramActive.WriteToFile();

      // 读取一个文件
      if (Dir.ReadDir() == FALSE) break;

      // 如果文件名不匹配或后缀为TMP，就不传输，跳过它
      if ( (MatchFileName(Dir.m_FileName,strmatchname)==FALSE) ||
           (MatchFileName(Dir.m_FileName,"*.TMP,*.SWP")== TRUE) ) continue;

      bHaveFiles=TRUE;

      // 发送待传输的文件名和文件大小信息
      if (WriteFileInfo() == FALSE)
      {
        logfile.Write("WriteFileInfo() FAILED.\n"); ChldEXIT(-1);
      }

      // 发送待传输的文件内容
      if (PutFile() == FALSE)
      {
        logfile.Write("PutFile() FAILED.\n"); ChldEXIT(-1);
      }

      strncpy(totalfiles[totalcount],Dir.m_FullFileName,300);
      totalcount++;

      if (totalcount>=ucheckcount)
      {
        // 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
        if (CheckSendFiles() == FALSE)
        {
          logfile.Write("CheckSendFiles() FAILED.\n"); ChldEXIT(-1);
        }
      }
    }

    if (totalcount>0)
    {
      // 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
      if (CheckSendFiles() == FALSE)
      {
        logfile.Write("CheckSendFiles() FAILED.\n"); ChldEXIT(-1);
      }
    }

    if (bHaveFiles==FALSE)
    {
      // 睡眠utimetvl秒
      for (UINT ii=0;ii<utimetvl;ii++)
      {
        sleep(1);

        if (ActiveTimer.Elapsed() > 30 )
        {
          ActiveTimer.Beginning();

          // 发送心跳报文
          if (WriteActiveTest() == FALSE) { logfile.Write("WriteActiveTest FAILED.\n"); ChldEXIT(-1); }
        }
      }
    }
  }
}

// 发送待传输的文件名和文件大小信息
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

  // 向服务端发送文件基本信息
  if (TcpServer.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
  }

  logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  return TRUE;
}

// 发送心跳报文
BOOL WriteActiveTest()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,500,"<activetest>");

  // 向服务端发送心跳报文
  if (TcpServer.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpServer.Write() FAILED.\n"); return FALSE;
  }

  logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  // 等待服务端的心跳回应报文
  if (TcpServer.Read(strRecvBuffer,180) == FALSE)
  {
    logfile.Write("TcpServer.Read() FAILED.\n"); return FALSE;
  }

  logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

  if (strcmp(strRecvBuffer,"ok") != 0) return FALSE;

  return TRUE;
}

// 发送待传输的文件内容
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

// 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
BOOL CheckSendFiles()
{
  // 与服务端核对文件总数
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

  // 核对成功，删除本地的文件
  for (UINT ii=0;ii<totalcount;ii++)
  {
    // 备份已发送过的文件
    if ( iptype==2 )
    {
      memset(strbakfilename,0,sizeof(strbakfilename));
      strcpy(strbakfilename,totalfiles[ii]);
      UpdateStr(strbakfilename,strlocalrootpath,strlocalrootpathbak,FALSE);

      // 如果移动失败，写下失败日志，但程序不可退出，函数不返回
      if (RENAME(totalfiles[ii],strbakfilename) == FALSE)
      {
        logfile.Write("RENAME %s to %s failed.\n",totalfiles[ii],strbakfilename);
      }
    }

    // 删除已发送过的文件
    if ( iptype==3 )
    {
      // 如果删除失败，写下失败日志，但程序不可退出，函数不返回
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

// 解析登录的XML
BOOL AXMLBuffer()
{
  int itimeoffset=0;

  GetXMLBuffer(strXmlBuffer,"timeoffset",&itimeoffset);

  char strTMPBuffer[4001];
  memset(strTMPBuffer,0,sizeof(strTMPBuffer));
  strncpy(strTMPBuffer,strXmlBuffer,4000);

  // 处理xmlbuffer中的时间变量
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

