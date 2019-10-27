#include "_public.h"
#include "ftp.h"

void CallQuit(int sig);

Cftp           ftp;
CLogFile       logfile;
CProgramActive ProgramActive;

FILE *listfp;

char strremoteip[21];        // 远程服务器的IP
UINT uport;                  // 远程服务器FTP的端口
char strmode[11];            // 传输模式，port和pasv
char strusername[31];        // 远程服务器FTP的用户名
char strpassword[31];        // 远程服务器FTP的密码
char strremotepath[301];     // 远程服务器存放文件的目录
char strmatchname[301];      // 待采集文件匹配的文件名
UINT utimeout;               // FTP采集文件的超时时间

// 建立与远程服务器的连接，并登录
BOOL ftplogin();

// 从远程服务器get文件
BOOL ftpdelfiles();

// 文件列表数据结构
struct st_filelist
{
  char remotefilename[301];
  char remotefilenamebak[301];
  char localfilename[301];
  char modtime[21];
  int  filesize;
  int  ftpsts;   // 1-计划采集；2-无需采集。
};

// 文件信息的临时数据结构，在任何函数中都可以直接用
struct st_filelist stfilelist;

// 远程服务器文件清单的容器
vector<struct st_filelist> m_vrfilelist;

// 获取远程服务器的文件清单
BOOL nlist();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpdelfiles logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpdelfiles /tmp/htidc/log/ftpdelfiles_211.139.255.23_oracle_gzradpic.log \"<remoteip>211.139.255.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>oracle</password><remotepath>/tmp/radpic/gzrad</remotepath><matchname>CSQX*.GIF</matchname><listfilename>/tmp/htidc/list/ftpdelfiles_211.139.255.23_oracle_gzradpic.xml</listfilename><timeout>1800</timeout>\"\n\n\n");

    printf("本程序是数据中心的公共功能模块，用于删除远程FTP服务器的文件。\n");
    printf("logfilename是本程序运行的日志文件，一般放在/tmp/htidc/log目录下，"\
           "采用ftpdelfiles_远程地址_ftp用户名_文件类型.log的命名方式。\n");
    printf("xmlbuffer为文件传输的参数，如下：\n");
    printf("<remoteip>211.139.255.23</remoteip> 远程服务器的IP。\n");
    printf("<port>21</port> 远程服务器FTP的端口。\n");
    printf("<mode>pasv</mode> 传输模式，pasv和port，可选字段，缺省采用pasv模式。\n");
    printf("<username>oracle</username> 远程服务器FTP的用户名。\n");
    printf("<password>oracle1234HOTA</password> 远程服务器FTP的密码。\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> 远程服务器存放文件的目录。\n");
    printf("<matchname>*.GIF</matchname> 待删除文件匹配的文件名，采用大写匹配，"\
           "不匹配的文件不会被采集，本字段尽可能设置精确，不允许用*匹配全部的文件。\n");
    printf("<timeout>1800</timeout> FTP删除文件的超时时间，单位：秒，注意，必须确保传输某单"\
           "个文件的时间小于timeout的取值，否则会造成传输失败的情况。\n");
  
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

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("ftpdelfiles");

  memset(strremoteip,0,sizeof(strremoteip));          // 远程服务器的IP
  uport=21;                                           // 远程服务器FTP的端口
  memset(strmode,0,sizeof(strmode));                  // 传输模式，port和pasv
  memset(strusername,0,sizeof(strusername));          // 远程服务器FTP的用户名
  memset(strpassword,0,sizeof(strpassword));          // 远程服务器FTP的密码
  memset(strremotepath,0,sizeof(strremotepath));      // 远程服务器存放文件的目录
  memset(strmatchname,0,sizeof(strmatchname));        // 待采集文件匹配的文件名
  utimeout=0;                                         // FTP采集文件的超时时间

  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);

  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);

  // mode是可以为空的，如果为空，就采用pasv模式
  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);
  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (uport == 0) { logfile.Write("port is null.\n"); return -1; }
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (strcmp(strmatchname,"*") == 0) { logfile.Write("matchname is vailed(* only).\n"); return -1; }
  if (utimeout == 0) { logfile.Write("timeout is null.\n"); return -1; }

  // 注意，程序超时是utimeout秒
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",utimeout);

  // 登录远程FTP服务器
  if (ftplogin() == FALSE) return FALSE;

  // 获取远程服务器的文件清单
  if (nlist() == FALSE) return FALSE;

  for (UINT ii=0;ii<m_vrfilelist.size();ii++)
  {
    if (ftp.ftpdelete(stfilelist.remotefilename) == FALSE)
    {
      logfile.Write("ftp.ftpdelete(%s) failed.\n%s",stfilelist.remotefilename,ftp.response()); continue;
    }
  }

  ftp.logout();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  logfile.Write("ftpdelfiles exit.\n");

  exit(0);
}

// 建立与远程服务器的连接，并登录
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // 设置传输模式
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP连接和登录的超时时间设为80就够了。
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) FAILED.\n\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // 程序的超时时间再设为utimeout秒。
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",utimeout);

  return TRUE;
}


// 获取远程服务器的文件清单
BOOL nlist()
{
  m_vrfilelist.clear();

  char strListFileName[301];
  memset(strListFileName,0,sizeof(strListFileName));
  snprintf(strListFileName,300,"/tmp/htidc/tmp/ftp_%s_%s_%d.list",strremoteip,strusername,getpid());
  MKDIR("/tmp/htidc/tmp",FALSE);

  // 进入目标目录
  if (ftp.chdir(strremotepath) == FALSE)
  {
    // 如果进入目标目录失败，有可能是目标目录尚未创建
    logfile.Write("ftp.chdir(%s) failed.\n%s",strremotepath,ftp.response()); return FALSE;
  }

  // logfile.Write("chdir ok.\n");

  char strMatch[2];
  memset(strMatch,0,sizeof(strMatch));

  // 获取对方的文件清单时，往往需要更长的时间，所以这里设置为1200秒
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",1200);

  // 获取远程服务器的文件清单，分别用"."、"*"和" "尝试。
  for (int ii=0;ii<3;ii++)
  {
    if (ii==0) strMatch[0]='*';
    if (ii==1) strMatch[0]='.';
    if (ii==2) strMatch[0]=0;

    REMOVE(strListFileName); 

    ftp.nlist(strMatch,strListFileName);

    ProgramActive.WriteToFile();

    if (FileSize(strListFileName) > 2) break;

    REMOVE(strListFileName); 
  }

  // 判断清单文件是否为空，为空就直接返回
  if (FileSize(strListFileName) <= 0) return FALSE;

  char strLine[301];

  listfp=0;

  // 把远程服务器的文件清单文件加载到m_vrfilelist
  if ( (listfp=FOPEN(strListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strListFileName); REMOVE(strListFileName); return FALSE; 
  }

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    // 从文件中获取一行
    if (FGETS(strLine,300,listfp) == FALSE) break;

    // 如果远程的文件名匹配TMP,TEMP,*TOPWALK*,*SWP，就跳过这个文件
    if (MatchFileName(strLine,"*TMP,*TEMP,*TOPWALK*,*SWP") == TRUE) continue;

    // 如果远程的文件名不匹配strmatchname，就跳过这个文件
    if (MatchFileName(strLine,strmatchname) == FALSE) continue;

    memset(&stfilelist,0,sizeof(stfilelist));

    snprintf(stfilelist.remotefilename,300,"%s/%s",strremotepath,strLine);

    stfilelist.ftpsts=1;

    m_vrfilelist.push_back(stfilelist);
  }

  fclose(listfp); 

  // 删除已使用的远程服务器的文件清单文件。
  REMOVE(strListFileName);

  return TRUE;
}

