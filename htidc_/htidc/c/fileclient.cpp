/*
    文件传输的客户程序
*/

#include "_cmpublic.h"
#include "_public.h"

// 程序退出时调用的函数
void EXIT(int sig);

CDir Dir;

CLogFile       logfile;
CProgramActive ProgramActive;

struct st_FileInfo
{
  char filename[301];    // 全路径的文件名
  UINT filesize;         // 文件的大小
  char filenametmp[301]; // 全路径的临时文件名
  char mtime[21];        // 文件的时间
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

char strRecvBuffer[TCPBUFLEN+10]; // 接收报文的缓冲区
char strSendBuffer[TCPBUFLEN+10]; // 发送报文的缓冲区

// TCP相关操作的类
CTcpClient TcpClient;

// 发送待传输的文件名和文件大小信息
BOOL WriteFileInfo();

// 发送待传输的文件内容
int  fd=-1;
BOOL PutFile();

// 发送心跳报文
BOOL WriteActiveTest();

// 登录服务器
BOOL ClientLogin();

#define MAXFILECOUNT 100

UINT totalcount=0;
char totalfiles[MAXFILECOUNT][301];
// 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
BOOL CheckSendFiles();

// 发送文件主函数
void SendFilesMain();
// 接收文件主函数
void RecvFilesMain();

// 接收待传输的文件名和文件大小信息
BOOL ReadFileInfo();

// 接收待传输文件内容
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

    printf("本程序与fileserver通讯，基于TCP的socket通讯，实现文件的收发功能。\n\n");

    printf("logfilename 日志文件名。\n");
    printf("xmlbuffer   文件传输的全部参数，每个字段的函义如下，注意，发送文件和接收文件的字段取值是不同的。\n\n");

    printf("1、发送文件\n");
    printf("localrootpath    本地文件存放的根目录。\n");
    printf("ptype            文件发送成功后，客户端文件的处理方式：1-保留文件；2-移动到备份目录；3-删除文件，只启用2和3，"\
                            "1暂时不启用。\n");
    printf("localrootpathbak 文件成功发送后，本地文件备份的根目录，注意，如果localrootpathbak为空，"\
                            "文件成功发送后将被删除，如果localrootpathbak等于localrootpath，文件成功发送后不做任何处理，"\
                            "继续保留，如果保留文件，那么任务在下一次执行的时候，之前发送过的文件会被重发。"\
                            "localrootpathbak是一个可选字段，可以不存在。\n");
    printf("dstrootpath      服务端文件存放的根目录。\n");
    printf("url              服务器端的IP地址和通讯端口，如10.148.124.85,5005。\n");
    printf("matchname        待发送文件名的匹配方式，如\"*.TXT,*.XML\"，注意用大写。\n");
    printf("andchild         是否发送localrootpath目录下各级子目录的文件，TRUE-是；FALSE-否。如果为TRUE，"\
                            "在服务器的dstrootpath目录中，将创建与localrootpath相同的目录结构。\n");
    printf("timetvl          发送文件的时间间隔，单位：秒。\n");
    printf("checkcount       每次发送文件的时候，与服务端核对文件的个数，最小为1，最大为100，如果超过100，强制为100。\n");
    printf("conntype         发送文件固定取值为1。\n");
    printf("starttime        发送文件的时间，这是个可选字段，如果为空，就在任何时间都发送，"\
                            "如果不为空，只有当前时间的小时在starttime内，才会发送文件，其它时间不发送。"\
                            "starttime的取值为小时，例如\"00,01,12\"。\n\n");
    printf("timeoffset      宏变量时间偏移，单位：分钟。本组参数可以匹配{yyyy}、{yyy}、{yy}、{mm}、{dd}、{hh}、{mi}，"\
                           "分别代表4位的年、后三位的年、后两位的年、月、天、小时、分钟。\n");
    printf("以上的参数中，只有localrootpathbak和starttime是可选参数，其它的都必填。\n\n\n");

    printf("2、接收文件\n");
    printf("localrootpath  本地文件存放的根目录。\n");
    printf("dstrootpath    服务端文件存放的根目录。\n");
    printf("ptype          文件接收成功后，服务端文件的处理方式：1-保留文件；2-移动到备份目录；3-删除文件，只启用2和3，"\
                          "1暂时不启用。\n");
    printf("dstrootpathbak 文件接收成功后，服务端文件的备份目录，只有当ptype=2时，本字段才有意义。\n");
    printf("url            服务器端的IP地址和通讯端口，如10.148.124.85,5005。\n");
    printf("matchname      待接收文件名的匹配方式，如\"*.TXT,*.XML\"，注意用大写。\n");
    printf("andchild       是否接收dstrootpath目录下各级子目录的文件，TRUE-是；FALSE-否。如果为TRUE，"\
                          "在服务器的localrootpath目录中，将创建与dstrootpath相同的目录结构。\n");
    printf("timetvl        接收文件的时间间隔，单位：秒。\n");
    printf("checkcount     每次接收文件的时候，与服务端核对文件的个数，最小为1，最大为100，如果超过100，强制为100。\n");
    printf("conntype       接收文件固定取值为2。\n");
    printf("starttime      接收文件的时间，这是个可选字段，如果为空，就在任何时间都接收，"\
                          "如果不为空，只有当前时间的小时在starttime内，才会接收文件，其它时间不接收。"\
                          "starttime的取值为小时，例如\"00,01,12\"。\n\n");
    printf("timeoffset     宏变量时间偏移，单位：分钟。本组参数可以匹配{yyyy}、{yyy}、{yy}、{mm}、{dd}、{hh}、{mi}，"\
                          "分别代表4位的年、后三位的年、后两位的年、月、天、小时、分钟。\n");
    printf("以上的参数中，只有dstrootpathbak和starttime是可选参数，其它的都必填。\n\n\n");

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

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开程序运行日志
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

  // 为了兼容旧的脚本，暂时这么处理
  uptype=3;
  if (strlen(strlocalrootpathbak)!=0) uptype=2;

  if (strlen(strlocalrootpath) == 0) { logfile.Write("localrootpath is null.\n"); return -1; }
  // 如果localrootpathbak为空，文件在传输后将删除
  // if (strlen(strlocalrootpathbak) == 0) { logfile.Write("localrootpathbak is null.\n"); return -1; }
  if (strlen(strdstrootpath) == 0) { logfile.Write("dstrootpath is null.\n"); return -1; }
  if (strlen(strurl) == 0) { logfile.Write("url is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (strlen(strandchild) == 0) { logfile.Write("andchild is null.\n"); return -1; }
  if (utimetvl == 0) { logfile.Write("timetvl is null.\n"); return -1; }
  if (ucheckcount == 0) { logfile.Write("checkcount is null.\n"); return -1; }
  if (uconntype == 0) { logfile.Write("conntype is null.\n"); return -1; }
  // starttime是可以为空的。
  // if (strlen(strstarttime) == 0) { logfile.Write("starttime is null.\n"); return -1; }

  if (ucheckcount>MAXFILECOUNT) ucheckcount=MAXFILECOUNT;

  if ( (strcmp(strandchild,"TRUE") == 0) || (strcmp(strandchild,"true") == 0) ) bandchild=TRUE;

  // 获取本机的时间，判断是否该启用程序，如果时间中的小时不匹配，就退出
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

  // 连接服务端
  if (TcpClient.ConnectToServer() == FALSE)
  {
    logfile.Write("TcpClient.ConnectToServer(%s) failed.\n",strurl); EXIT(-1);
  }

  // 登录服务器
  if (ClientLogin() == FALSE)
  {
    logfile.Write("ClientLogin() failed.\n"); EXIT(-1);
  }

  // 发送文件
  if (uconntype==1) SendFilesMain();

  // 接收文件
  if (uconntype==2) RecvFilesMain();

  EXIT(0);
}

// 发送文件
void SendFilesMain()
{
  CTimer ActiveTimer;

  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 获取本机的时间，判断是否该启用程序，如果时间中的小时不匹配，就return，不用睡眠，主函数中有睡眠
    if (strlen(strstarttime) != 0)
    {
      char strLocalTime[21];
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"hh24mi");
      strLocalTime[2]=0;
      if (strstr(strstarttime,strLocalTime) == 0) return;
    }

    // 设置文件时间的格式
    Dir.SetDateFMT("yyyymmddhh24miss");

    // 打开待发送文件的目录
    if (Dir.OpenDirNoSort(strlocalrootpath,bandchild) == FALSE)
    {
      logfile.Write("Dir.OpenDirNoSort(%s) failed.\n",strlocalrootpath); EXIT(-1);
    }

    totalcount=0;
    memset(totalfiles,0,sizeof(totalfiles));

    while (TRUE)
    {
      // 写入进程活动信息
      ProgramActive.WriteToFile();

      // 读取一个文件
      if (Dir.ReadDir() == FALSE) break;

      // 如果文件名不匹配或后缀为TMP，就不传输，跳过它
      if ( (MatchFileName(Dir.m_FileName,strmatchname)==FALSE) ||
           (MatchFileName(Dir.m_FileName,"*.TMP,*.SWP")== TRUE) ) continue;

      // 发送待传输的文件名和文件大小信息
      if (WriteFileInfo() == FALSE)
      {
        logfile.Write("FAILED.WriteFileInfo FAILED.\n"); EXIT(-1);
      }

      // 发送待传输的文件内容
      if (PutFile() == FALSE)
      {
        logfile.Write("FAILED.PutFile FAILED.\n"); EXIT(-1);
      }

      strncpy(totalfiles[totalcount],Dir.m_FullFileName,300);
      totalcount++;

      if (totalcount>=ucheckcount)
      {
        // 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
        if (CheckSendFiles() == FALSE)
        {
          logfile.Write("CheckSendFiles() FAILED.\n"); EXIT(-1);
        }
      }
    }

    if (totalcount>0)
    {
      // 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
      if (CheckSendFiles() == FALSE)
      {
        logfile.Write("CheckSendFiles() FAILED.\n"); EXIT(-1);
      }
    }

    // 睡眠utimetvl秒
    for (UINT ii=0;ii<utimetvl;ii++)
    {
      sleep(1);

      if (ActiveTimer.Elapsed() > 30 ) 
      {
        ActiveTimer.Beginning();

        // 发送心跳报文
        if (WriteActiveTest() == FALSE) { logfile.Write("WriteActiveTest FAILED.\n"); EXIT(-1); }
      }
    }
  }
}

// 程序退出时调用的函数
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
  if (TcpClient.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  return TRUE;
}

// 发送心跳报文
BOOL WriteActiveTest()
{
  memset(strRecvBuffer,0,sizeof(strRecvBuffer));
  memset(strSendBuffer,0,sizeof(strSendBuffer));

  snprintf(strSendBuffer,500,"<activetest>");

  // 向服务端发送心跳报文
  if (TcpClient.Write(strSendBuffer) == FALSE)
  {
    logfile.Write("TcpClient.Write() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strSendBuffer=%s\n",strSendBuffer);  // xxxxxx

  // 等待服务端的心跳回应报文
  if (TcpClient.Read(strRecvBuffer,80) == FALSE)
  {
    logfile.Write("TcpClient.Read() FAILED.\n"); return FALSE;
  }

  //logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

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

// 与服务端核对文件总数，如果核对成功，就删除totalfiles数组中的文件
BOOL CheckSendFiles()
{
  // 与服务端核对文件总数
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

  // 核对成功，删除本地的文件
  for (UINT ii=0;ii<totalcount;ii++)
  {
    // 删除已发送过的文件
    if ( uptype == 3)
    {
      // 如果删除失败，写下失败日志，但程序不可退出，函数不返回
      if (REMOVE(totalfiles[ii]) == FALSE)
      {
        logfile.Write("REMOVE %s failed.\n",totalfiles[ii]); 
      }
    }

    // 备份已发送过的文件
    if ( uptype == 2)
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

    logfile.Write("send %s ok.\n",totalfiles[ii]);
  }

  totalcount=0;
  memset(totalfiles,0,sizeof(totalfiles));

  return TRUE;
}

// 登录服务器
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

// 接收文件主函数
void RecvFilesMain()
{
  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 获取本机的时间，判断是否该启用程序，如果时间中的小时不匹配，就return，不用睡眠，主函数中有睡眠
    if (strlen(strstarttime) != 0)
    {
      char strLocalTime[21];
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"hh24mi");
      strLocalTime[2]=0;
      if (strstr(strstarttime,strLocalTime) == 0) return;
    }

    // 接收待传输文件名和文件大小
    if (ReadFileInfo() == FALSE)
    {
      logfile.Write("ReadFileInfo() FAILED.\n"); EXIT(0);
    }

    // 接收待传输文件内容
    if (ReadFile() == FALSE) EXIT(0);

    strncpy(totalfiles[totalcount],stFileInfo.filenametmp,300);

    totalcount++;
  }
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

    if (TcpClient.Read(strRecvBuffer,80) == FALSE)
    {
      logfile.Write("TcpClient.Read() FAILED.\n"); return FALSE;
    }

    //logfile.Write("strRecvBuffer=%s\n",strRecvBuffer);  // xxxxxx

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
            logfile.Write("recv %s ok.\n",strfilename);
          }
        }
      }
      else
      {
        // 核对失败，删除临时文件
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

    // 如果读取到的是心跳报文，就继续等对方发送文件信息
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

    // 其它的就是文件信息了
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

  // 重置文件的时间
  UTime(stFileInfo.filenametmp,stFileInfo.mtime);

  return TRUE;
}

