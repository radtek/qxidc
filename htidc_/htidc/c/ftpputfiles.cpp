#include "_public.h"
#include "ftp.h"

void CallQuit(int sig);

CDir           Dir;
Cftp           ftp;
CLogFile       logfile;
CProgramActive ProgramActive;

char strremoteip[21];        // 远程服务器的IP
UINT uport;                  // 远程服务器FTP的端口
char strmode[11];            // 传输模式，port和pasv
char strusername[31];        // 远程服务器FTP的用户名
char strpassword[31];        // 远程服务器FTP的密码
char strlocalpath[201];      // 本地文件存放的目录
char strlocalpathbak[201];   // 本地文件发送成功后，存放的备份目录，如果该目录为空，文件发送成功后直接删除
char strremotepath[201];     // 远程服务器存放文件的最终目录
char strmatchname[301];      // 待发送文件匹配的文件名
UINT utimeout;               // FTP发送文件的超时时间
int  itimetvl;
char strtrlog[11];              // 是否切换日志
char strdeleteremote[21];
 
// 建立与对方服务器的连接，并登录
BOOL ftplogin();

// 把文件发送到对方服务器目录
BOOL ftpputfiles();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpputfiles logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpputfiles /tmp/htidc/log/ftpputfiles_211.139.255.23_oracle_gzradpic.log \"<remoteip>211.139.255.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>oracle1234HOTA</password><localpath>/tmp/htidc/ftpget/gzrad</localpath><localpathbak>/tmp/htidc/ftpget/gzradbak</localpathbak><remotepath>/tmp/radpic/gzrad</remotepath><matchname>*.GIF</matchname><timeout>1800</timeout><timetvl>-8</timetvl><trlog>TRUE</trlog><deleteremote>TRUE</deleteremote>\"\n\n\n");

    printf("本程序是数据中心的公共功能模块，用于把本地文件发送到远程的FTP服务器。\n");
    printf("logfilename是本程序运行的日志文件，一般放在/tmp/htidc/log目录下，"\
           "采用ftpputfiles_远程地址_ftp用户名.log命名。\n");
    printf("xmlbuffer为文件传输的参数，如下：\n");
    printf("<remoteip>211.139.255.23</remoteip> 远程服务器的IP。\n");
    printf("<port>21</port> 远程服务器FTP的端口。\n");
    printf("<mode>pasv</mode> 传输模式，pasv和port，可选字段，缺省采用pasv模式，port模式一般不用。\n");
    printf("<username>oracle</username> 远程服务器FTP的用户名。\n");
    printf("<password>oracle1234HOTA</password> 远程服务器FTP的密码。\n");
    printf("<localpath>/tmp/htidc/ftpget/gzrad</localpath> 本地文件存放的目录。\n");
    printf("<localpathbak>/tmp/htidc/ftpget/gzradbak</localpathbak> 本地文件发送成功后，存放的备份目录，"\
           "如果该目录为空，文件发送成功后直接删除，如果localpath等于localpathbak，"\
           "文件发送成功后将保留源文件不变。正常情况下，本地文件传输后应该删除或备份，"\
           "不删除不备份只适用于传输日志文件的情况。\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> 远程服务器存放文件的目录。\n");
    printf("<matchname>*.GIF</matchname> 待发送文件匹配的文件名，采用大写匹配，不匹配的文件"\
           "不会被发送，本字段尽可能设置精确。\n");
    printf("<timeout>1800</timeout> FTP发送文件的超时时间，单位：秒，注意，必须确保传输某"\
           "个文件的时间小于timeout的取值，否则会造成传输失败的情况。\n");
    printf("<trlog>TRUE</trlog> 当日志文件大于100M时，是否切换日志文件，TRUE-切换；FALSE-不切换。\n");
    printf("<deleteremote>TRUE</deleteremote> 如果远程目录存在该文件，是否删除远程目录的临时文件和正式文件\n");
    printf("xmlbuffer可以处理时间变量，<timetvl>-8</timetvl> 为时间变量的偏移时间。"\
           "目前可以处理以下时间变量：{YYYY}（4位的年）、{YYY}（后三位的年）、"\
           "{YY}（后两位的年）、{MM}（月月）、{DD}（日日）、{HH}（时时）、{MI}（分分）、{SS}（秒秒）。\n");
    printf("增量采集字段标志 <incfield>keyid</incfield> 它必须是fieldstr中的字段名，并且只能是整数。\n");
    printf("以上的参数只有mode、timetvl、localpathbak、trlog为可选参数，其它的都必填。\n\n");

    printf("port 模式是建立从服务器高端口连到客户端20端口数据连接。\n");
    printf("pasv 模式是建立客户高端口连到服务器返回的数据端口的数据连接。\n\n");

    printf("port（主动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，"\
           "服务器接受连接，建立一条命令链路。\n");
    printf("当需要传送数据时，服务器从20端口向客户端的空闲端口发送连接请求，建立一条数据链路来传送数据。\n\n");

    printf("pasv（被动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，"\
           "服务器接受连接，建立一条命令链路。\n");
    printf("当需要传送数据时，客户端向服务器的空闲端口发送连接请求，建立一条数据链路来传送数据。\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001]; 

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // 判断是否切换日志
  BOOL btrlog=TRUE;
  if (strcmp(strtrlog,"FALSE")==0) btrlog=FALSE;

  if (logfile.Open(argv[1],"a+",btrlog) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("ftpputfiles");

  memset(strremoteip,0,sizeof(strremoteip));          // 远程服务器的IP
  uport=21;                                           // 远程服务器FTP的端口
  memset(strmode,0,sizeof(strmode));                  // 传输模式，port和pasv
  memset(strusername,0,sizeof(strusername));          // 远程服务器FTP的用户名
  memset(strpassword,0,sizeof(strpassword));          // 远程服务器FTP的密码
  memset(strlocalpath,0,sizeof(strlocalpath));        // 本地文件存放的目录
  memset(strlocalpathbak,0,sizeof(strlocalpathbak));  // 本地文件发送成功后，存放的备份目录，如果该目录为空，文件发送成功后直接删除
  memset(strremotepath,0,sizeof(strremotepath));      // 远程服务器存放文件的最终目录
  memset(strmatchname,0,sizeof(strmatchname));        // 待发送文件匹配的文件名
  utimeout=0;                                         // FTP发送文件的超时时间
  itimetvl=0;
  memset(strtrlog,0,sizeof(strtrlog));        
  memset(strdeleteremote,0,sizeof(strdeleteremote));

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);
  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);

  // 处理xmlbuffer中的时间变量
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"localpath",strlocalpath,200);
  GetXMLBuffer(strXmlBuffer,"localpathbak",strlocalpathbak,200);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,200);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);
  GetXMLBuffer(strXmlBuffer,"trlog",strtrlog,8);
  GetXMLBuffer(strXmlBuffer,"deleteremote",strdeleteremote,8);

  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);

  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (uport == 0) { logfile.Write("port is null.\n"); return -1; }
  // mode是可以为空的，如果为空，就采用pasv模式
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strpassword) == 0) { logfile.Write("password is null.\n"); return -1; }
  if (strlen(strlocalpath) == 0) { logfile.Write("localpath is null.\n"); return -1; }
  // localpathbak是可以为空的，如果为空，发送成功后，直接删除源文件
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (utimeout == 0) { logfile.Write("timeout is null.\n"); return -1; }

  // 处理目录的匹配
  /*
  ProcMatchDTime(strlocalpath);
  ProcMatchDTime(strlocalpathbak);
  ProcMatchDTime(strremotepath);
  */

  /*
  logfile.Write("remoteip=%s\n",strremoteip);
  logfile.Write("port=%lu\n",uport);
  logfile.Write("mode=%s\n",strmode);
  logfile.Write("username=%s\n",strusername);
  logfile.Write("password=%s\n",strpassword);
  logfile.Write("localpath=%s\n",strlocalpath);
  logfile.Write("localpathbak=%s\n",strlocalpathbak);
  logfile.Write("remotepath=%s\n",strremotepath);
  logfile.Write("matchname=%s\n",strmatchname);
  logfile.Write("timeout=%lu\n",utimeout);
  */

  // 注意，程序超时是utimeout秒
  ProgramActive.SetProgramInfo(&logfile,"ftpputfiles",utimeout);

  BOOL bConnected=FALSE;

  // 本地文件存放的目录
  if (Dir.OpenDir(strlocalpath) == FALSE)
  {
    logfile.Write("Dir.OpenDir(%s) failed.\n",strlocalpath); return -1;
  }

  ProgramActive.WriteToFile();

  while (Dir.ReadDir() == TRUE)
  {
    // 判断文件名是否和MatchStr匹配，如果不匹配，不分发该文件
    if (MatchFileName(Dir.m_FileName,strmatchname) == FALSE) continue;

    // 如果待分发的文件名匹配*TMP,*TEMP,*TOPWALK*,*SWP，就跳过这个文件
    if (MatchFileName(Dir.m_FileName,"*TMP,*TEMP,*TOPWALK*,*SWP") == TRUE) continue;

    // 如果没有连上对方的FTP服务器，就连接
    if (bConnected==FALSE)
    {
      // 连接服务器
      if (ftplogin() == FALSE) return FALSE;

      ProgramActive.WriteToFile();

      bConnected=TRUE;
    }

    // 把文件发送到对方服务器目录
    ftpputfiles();

    ProgramActive.WriteToFile();
  }

  ftp.logout();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  logfile.Write("ftpputfiles exit.\n");

  exit(0);
}

// 建立与对方服务器的连接，并登录
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // 设置传输模式
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP连接和登录的超时时间设为80就够了。
  ProgramActive.SetProgramInfo(&logfile,"ftpputfiles",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) failed.\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // 程序的超时时间再设为utimeout秒。
  ProgramActive.SetProgramInfo(&logfile,"ftpputfiles",utimeout);

  // 在对方服务器上创建目录
  ftp.mkdir(strremotepath);

  return TRUE;
}


// 把文件发送到对方服务器目录
BOOL ftpputfiles()
{
  char strFullLocalFileName[301];
  char strFullLocalFileNameBak[301];
  char strFullRemoteFileName[301];
  char strFullRemoteFileNameTmp[301];

  memset(strFullLocalFileName,0,sizeof(strFullLocalFileName));
  memset(strFullLocalFileNameBak,0,sizeof(strFullLocalFileNameBak));
  memset(strFullRemoteFileName,0,sizeof(strFullRemoteFileName));
  memset(strFullRemoteFileNameTmp,0,sizeof(strFullRemoteFileNameTmp));
  
  snprintf(strFullLocalFileName,300,"%s/%s",strlocalpath,Dir.m_FileName);
  snprintf(strFullLocalFileNameBak,300,"%s/%s",strlocalpathbak,Dir.m_FileName);
  snprintf(strFullRemoteFileName,300,"%s/%s",strremotepath,Dir.m_FileName);
  snprintf(strFullRemoteFileNameTmp,300,"%s/%s.tmp",strremotepath,Dir.m_FileName);
  
  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-10*60); // 当前时间点10分钟之前的时间点

  BOOL bIsLogFile=MatchFileName(Dir.m_FileName,"*.LOG");

  // 如果本地目录该文件的大小是0，并且文件的创建时间在10分钟之前，就认为是非法的文件，删除它
  // 如果在10分钟之内，就不要删除，因为它很可能是中间状态的文件，如果删除，会影响文件的生成
  // 但是，日志文件除外
  if ( (bIsLogFile == FALSE) && (FileSize(strFullLocalFileName) == 0) )
  {
    if (strcmp(Dir.m_ModifyTime,strLocalTime)<=0) { REMOVE(strFullLocalFileName); return FALSE; }
    
    return TRUE; 
  }

  // 删除远程目录的临时文件和正式文件，因为如果对方目录已存在该文件，可能会导致文件传输不成功能情况
  // 但是，新的类库不知道还会不会这样
  if (strcmp(strdeleteremote,"TRUE")==0) { ftp.ftpdelete(strFullRemoteFileNameTmp); ftp.ftpdelete(strFullRemoteFileName); }

  CTimer Timer;

  Timer.Beginning();

  logfile.Write("put %s(size=%ld,mtime=%s)...",strFullLocalFileName,Dir.m_FileSize,Dir.m_ModifyTime);

  // 发送文件
  if (ftp.put(strFullLocalFileName,strFullRemoteFileName,bIsLogFile) == FALSE)
  {
    logfile.WriteEx("failed.\n%s",ftp.response()); return FALSE;
  }

  if (strlen(strlocalpathbak) == 0)
  {
    // 删除本地目录的该文件
    if (REMOVE(strFullLocalFileName) == FALSE) 
    { 
      logfile.WriteEx("failed.REMOVE(%s) failed.\n",strFullLocalFileName); return FALSE; 
    }
  }
  else
  {
    // 把本地目录的该文件移到备份的目录去，如果localpath等于localpathbak，保留源文件不变
    if (strcmp(strlocalpath,strlocalpathbak) != 0)
    {
      if (RENAME(strFullLocalFileName,strFullLocalFileNameBak) == FALSE) 
      { 
        logfile.WriteEx("failed.RENAME(%s,%s) failed.\n",strFullLocalFileName,strFullLocalFileNameBak); return FALSE; 
      }
    }
  }

  logfile.WriteEx("ok(time=%d).\n",Timer.Elapsed());

  return TRUE;
}

