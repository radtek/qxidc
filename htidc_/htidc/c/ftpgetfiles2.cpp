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
char strlocalpath[301];      // 本地文件存放的目录
char strremotepathbin[301];     // 远程服务器存放雷达基文件的目录
char strremotepathgif[301];     // 远程服务器存放雷达图文件的目录
char strremotepathbak[301];  // 远程服务器存放文件的备份目录
char strmatchname[301];      // 待采集文件匹配的文件名
char strlistfilename[301];   // 存放已采集文件列表的xml文件
UINT udeleteremote;          // 采集成功后，是否删除远程服务器的文件，1-删除；2-不删除
UINT utimeout;               // FTP采集文件的超时时间
UINT umtime;                 // 文件mtime的有效时间
int  itimetvl;

// 建立与远程服务器的连接，并登录
 BOOL ftplogin();
//
// 从远程服务器get文件
 BOOL ftpgetfiles();

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

// 远程服务器雷达基文件清单的容器
vector<struct st_filelist> m_vrbinfilelist;

// 远程服务器雷达图文件清单的容器
vector<struct st_filelist> m_vrgiffilelist;

// 本地服务器文件清单的容器
vector<struct st_filelist> m_vlfilelist;

// 获取远程服务器的雷达基文件清单
BOOL getbinlist();

// 删除已经处理的雷达基文件 
BOOL removebinfile();

// 获取远程服务器的文件清单
BOOL nlist();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpgetfiles2 logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpgetfiles2 /tmp/htidc/log/ftpgetfiles_211.139.255.23_oracle_gzradpic.log \"<remoteip>211.139.255.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>oracle1234HOTA</password><localpath>/tmp/htidc/ftpget/gzrad</localpath><remotepathbin>/tmp/radpic/gzrad</remotepathbin><remotepathgif>/tmp/radpic/gzrad</remotepathgif><remotepathbak></remotepathbak><matchname>*.GIF</matchname><listfilename>/tmp/htidc/list/ftpgetfiles_211.139.255.23_oracle_gzradpic.xml</listfilename><deleteremote>2</deleteremote><timeout>1800</timeout><timetvl>-8</timetvl><mtime>24</mtime>\"\n\n\n");

    printf("本程序是数据中心的公共功能模块，用于把远程FTP服务器的文件采集到本地目录（将处理成雷达图的雷达基文件删掉，采集雷达图文件）。\n");
    printf("logfilename是本程序运行的日志文件，一般放在/tmp/htidc/log目录下，"\
		    "采用ftpgetfiles_远程地址_ftp用户名_文件类型.log的命名方式。\n");
    printf("xmlbuffer为文件传输的参数，如下：\n");
    printf("<remoteip>211.139.255.23</remoteip> 远程服务器的IP。\n");
    printf("<port>21</port> 远程服务器FTP的端口。\n");
    printf("<mode>pasv</mode> 传输模式，pasv和port，可选字段，缺省采用pasv模式。\n");
    printf("<username>oracle</username> 远程服务器FTP的用户名。\n");
    printf("<password>oracle1234HOTA</password> 远程服务器FTP的密码。\n");
    printf("<localpath>/tmp/htidc/ftpget</localpath> 本地文件存放的目录。\n");
    printf("<remotepathbin>/tmp/radpic/gzrad</remotepathbin> 远程服务器存放雷达基文件的目录。\n");
    printf("<remotepathgif>tmp/radpic/gzrad</remotepathgif> 远程服务器存放雷达图文件的目录。\n");
    printf("<remotepathbak>/tmp/radpic/gzradbak</remotepath> 远程服务器存放文件的备份目录，同时存放雷达基文件和雷达图文件，一般不用。"\
		    "如果为空，则表示不备份，但是否删除，由deleteremote参数决定。\n");
    printf("<deleteremote>2</deleteremote> 采集成功后，是否删除远程服务器的文件，"\
		    "1-删除；2-不删除，此参数的设置一定要询问远程服务器的系统管理员，"\
		    "只有当对方允许且分配了删除权限的情况下才能设置为1-删除。如果deleteremote为1，remotepathbak将失效。\n");
    printf("<matchname>*.GIF</matchname> 待采集文件匹配的文件名，采用大写匹配，"\
		    "不匹配的文件不会被采集，本字段尽可能设置精确，不允许用*匹配全部的文件。\n");
    printf("<listfilename>/tmp/htidc/list/ftpgetfiles_211.139.255.23_oracle_gzradpic.xml</listfilename> 存"\
		    "放已采集文件列表的xml文件，一般放在/tmp/htidc/list目录下，"\
		    "采用ftpgetfiles_远程地址_ftp用户名_文件类型.xml的命名方式。\n");
    printf("<timeout>1800</timeout> FTP采集文件的超时时间，单位：秒，注意，必须确保传输某单"\
		    "个文件的时间小于timeout的取值，否则会造成传输失败的情况。\n");
    printf("<mtime>24</mtime> 待传输文件时间的范围，单位：小时，如果文件时间在本参数之前，就不传输。\n");
    printf("xmlbuffer可以处理时间变量，<timetvl>-8</timetvl> 为时间变量的偏移时间。"\
		    "目前可以处理以下时间变量：{YYYY}（4位的年）、{YYY}（后三位的年）、"\
		    "{YY}（后两位的年）、{MM}（月月）、{DD}（日日）、{HH}（时时）、{MI}（分分）、{SS}（秒秒）。\n");
    printf("以上的参数只有mode、timetvl、remotepathbak、mtime为可选参数，其它的都必填。\n\n\n");

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
  logfile.SetAlarmOpt("ftpgetfiles");

  memset(strremoteip,0,sizeof(strremoteip));          // 远程服务器的IP
  uport=21;                                           // 远程服务器FTP的端口
  memset(strmode,0,sizeof(strmode));                  // 传输模式，port和pasv
  memset(strusername,0,sizeof(strusername));          // 远程服务器FTP的用户名
  memset(strpassword,0,sizeof(strpassword));          // 远程服务器FTP的密码
  memset(strlocalpath,0,sizeof(strlocalpath));        // 本地文件存放的目录
  memset(strremotepathbin,0,sizeof(strremotepathbin));// 远程服务器存放雷达基文件的目录
  memset(strremotepathgif,0,sizeof(strremotepathgif));// 远程服务器存放雷达图文件的目录
  memset(strremotepathbak,0,sizeof(strremotepathbak));// 远程服务器存放文件的备份目录
  memset(strmatchname,0,sizeof(strmatchname));        // 待采集文件匹配的文件名
  memset(strlistfilename,0,sizeof(strlistfilename));  // 存放已采集文件列表的xml文件
  udeleteremote=2;                                    // 采集成功后，是否删除远程服务器的文件，1-删除；2-不删除
  utimeout=0;                                         // FTP采集文件的超时时间
  umtime=0;
  itimetvl=0;

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);

  // 处理xmlbuffer中的时间变量
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);
  GetXMLBuffer(strXmlBuffer,"localpath",strlocalpath,300);
  GetXMLBuffer(strXmlBuffer,"remotepathbin",strremotepathbin,300);
  GetXMLBuffer(strXmlBuffer,"remotepathgif",strremotepathgif,300);
  GetXMLBuffer(strXmlBuffer,"remotepathbak",strremotepathbak,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"listfilename",strlistfilename,300);
  GetXMLBuffer(strXmlBuffer,"deleteremote",&udeleteremote);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);
  GetXMLBuffer(strXmlBuffer,"mtime",&umtime);

  // mode是可以为空的，如果为空，就采用pasv模式
  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);

  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (uport == 0) { logfile.Write("port is null.\n"); return -1; }
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strpassword) == 0) { logfile.Write("password is null.\n"); return -1; }
  if (strlen(strlocalpath) == 0) { logfile.Write("localpath is null.\n"); return -1; }
  if (strlen(strremotepathbin) == 0) { logfile.Write("remotepathbin is null.\n"); return -1; }
  if (strlen(strremotepathgif) == 0) { logfile.Write("remotepathgif is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (strcmp(strmatchname,"*") == 0) { logfile.Write("matchname is vailed(* only).\n"); return -1; }
  if (strlen(strlistfilename) == 0) { logfile.Write("listfilename is null.\n"); return -1; }
  if (udeleteremote == 0) { logfile.Write("deleteremote is null.\n"); return -1; }
  if (utimeout == 0) { logfile.Write("timeout is null.\n"); return -1; }


  // 注意，程序超时是utimeout秒
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles2",utimeout);

  // 登录远程FTP服务器
  if (ftplogin() == FALSE)  return FALSE;

  // 获取远程服务器的雷达图文件清单
  if (nlist() == FALSE) return FALSE;

  for (UINT ii=0;ii<m_vrgiffilelist.size();ii++)
  {
// 把文件采集到本地服务器目录
    if (m_vrgiffilelist[ii].ftpsts==1)
    {
      memcpy(&stfilelist,&m_vrgiffilelist[ii],sizeof(stfilelist));

      if (ftpgetfiles() == TRUE) m_vrgiffilelist[ii].ftpsts=2;
    }
  }

  // 获取远程服务器的雷达基文件清单
  if (getbinlist() == FALSE) 
  {
    logfile.Write(" get bin filelist failed.\n"); 
    return FALSE;
  }

  listfp=0;

  // 把已采集成功的文件信息文件从m_vrgiffilelist中写入本地清单文件
  if ( (listfp=FOPEN(strlistfilename,"w")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strlistfilename); return -1;
  }

  fprintf(listfp,"<data>\n");

  for (UINT ii=0;ii<m_vrgiffilelist.size();ii++)
  {
    if (m_vrgiffilelist[ii].ftpsts==2)
    {
      fprintf(listfp,"<filename>%s</filename><modtime>%s</modtime><filesize>%d</filesize><endl/>\n",m_vrgiffilelist[ii].remotefilename,m_vrgiffilelist[ii].modtime,m_vrgiffilelist[ii].filesize);
    }
  }

  fprintf(listfp,"</data>\n");

  // 如果m_vrbinfilelist和m_vrgiffilelist中文件名匹配， 删除远程目录下雷达基文件
  if (removebinfile() == FALSE) 
  {
    logfile.Write("no bin file to remove.\n"); return FALSE;
  }

  ftp.logout();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
  
  ftp.logout();

  logfile.Write("ftpgetfiles exit.\n");

  exit(0);
}

// 建立与远程服务器的连接，并登录
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // 设置传输模式
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP连接和登录的超时时间设为80就够了。
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) FAILED.\n\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // 程序的超时时间再设为utimeout秒。
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",utimeout);

  return TRUE;
}

// 获取远程雷基达服务器的文件清单
BOOL getbinlist()
{
  m_vrbinfilelist.clear();
  char strbinListFileName[301];
  memset(strbinListFileName,0,sizeof(strbinListFileName));
  snprintf(strbinListFileName,300,"/tmp/htidc/tmp/ftpbin_%s_%s_%d.list",strremoteip,strusername,getpid());
  MKDIR("/tmp/htidc/tmp",FALSE);

  // 进入目标目录
  if (ftp.chdir(strremotepathbin) == FALSE)
  {
    // 如果进入目标目录失败，有可能是目标目录尚未创建
    logfile.Write("ftp.chdir(%s) failed.\n%s",strremotepathbin,ftp.response()); return FALSE;
  }

  char strMatch[2];
  memset(strMatch,0,sizeof(strMatch));

  // 获取对方的文件清单时，往往需要更长的时间，所以这里设置为1200秒
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",1200);

  // 获取远程服务器的文件清单，分别用"."、"*"和" "尝试。
  for (int ii=0;ii<3;ii++)
  {
    if (ii==0) strMatch[0]='*';
    if (ii==1) strMatch[0]='.';
    if (ii==2) strMatch[0]=0;

    REMOVE(strbinListFileName); 

    ftp.nlist(strMatch,strbinListFileName);

    ProgramActive.WriteToFile();

    if (FileSize(strbinListFileName) > 2) break;

    REMOVE(strbinListFileName); 
  }

  // 判断清单文件是否为空，为空就直接返回
  if (FileSize(strbinListFileName) <= 0) return FALSE;

  char strLine[301];

  listfp=0;

  // 把远程服务器的雷达基文件清单文件加载到m_vrbinfilelist
  if ( (listfp=FOPEN(strbinListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strbinListFileName); REMOVE(strbinListFileName); return FALSE; 
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

    snprintf(stfilelist.remotefilename,300,"%s/%s",strremotepathbin,strLine);
    if (strlen(strremotepathbak) != 0)
    {
      snprintf(stfilelist.remotefilenamebak,300,"%s/%s",strremotepathbak,strLine);
    }
    snprintf(stfilelist.localfilename ,300,"%s/%s",strlocalpath ,strLine);

    stfilelist.ftpsts=1;

    m_vrbinfilelist.push_back(stfilelist);
  }

  fclose(listfp); 

  // logfile.Write("nlist load ok.\n");

  // 删除已使用的远程服务器的文件清单文件。
  REMOVE(strbinListFileName);

  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));
  LocalTime(strmtime,"yyyymmddhh24miss",0-umtime*60*60);

  // 获取远程服务器每个文件的修改时间和文件大小信息
  for (UINT ii=0;ii<m_vrbinfilelist.size();ii++)
  {
    // 如果获取远程服务器每个文件的修改时间和文件大小信息失败，返回失败
    if (ftp.mtime(m_vrbinfilelist[ii].remotefilename) == FALSE) 
    {
      logfile.Write("ftp.mtime(%s) FAILED.\n%s",m_vrbinfilelist[ii].remotefilename,ftp.response()); return FALSE; 
    }

    // 如果文件时间小于umtime，就丢弃这个文件
    if (umtime>0) 
    {
      if (strcmp(ftp.m_mtime,strmtime) < 0) 
      {
        m_vrbinfilelist.erase(m_vrbinfilelist.begin()+ii); ii--; continue;
      }
    }

    // 如果获取远程服务器每个文件的修改时间和文件大小信息失败，返回失败
    if (ftp.size(m_vrbinfilelist[ii].remotefilename) == FALSE) 
    {
      logfile.Write("ftp.size(%s) FAILED.\n%s",m_vrbinfilelist[ii].remotefilename,ftp.response()); return FALSE; 
    }

    strcpy(m_vrbinfilelist[ii].modtime,ftp.m_mtime);

    m_vrbinfilelist[ii].filesize=ftp.m_size;
  }
  return TRUE;
}

// 删除已经处理的雷达基文件 
BOOL removebinfile()
{
  if (m_vrbinfilelist.size() == 0)  return FALSE;

  CCmdStr CmdStr1; 
  CCmdStr CmdStr2;
 
  char strBuffer[301]; // 存放处理后的雷达基文件名
  for (UINT ii=0;ii<m_vrbinfilelist.size();ii++)
  {
    CmdStr1.SplitToCmd(m_vrbinfilelist[ii].remotefilename,"/");
    CmdStr2.SplitToCmd(CmdStr1.m_vCmdStr[CmdStr1.CmdCount()-1],".");
    
    memset(strBuffer,0,sizeof(strBuffer));
    sprintf(strBuffer,"%s_A", CmdStr2.m_vCmdStr[0].c_str());

    for (UINT jj=0;jj<m_vrgiffilelist.size();jj++) 
    {
      CmdStr1.SplitToCmd(m_vrgiffilelist[jj].remotefilename,"/");
      CmdStr2.SplitToCmd(CmdStr1.m_vCmdStr[CmdStr1.CmdCount()-1],".");

      if (strcmp(CmdStr2.m_vCmdStr[0].c_str(),strBuffer) == 0)
      {
        if (strlen(strremotepathbak) != 0)
        {
          if (ftp.ftprename(m_vrbinfilelist[ii].remotefilename,m_vrbinfilelist[ii].remotefilenamebak) == FALSE)
          {
	    logfile.Write("ftp.ftprename(%s,%s) failed.\n%s",m_vrbinfilelist[ii].remotefilename,m_vrbinfilelist[ii].remotefilenamebak,ftp.response());
          }
        }
        if (ftp.ftpdelete(m_vrbinfilelist[ii].remotefilename) == FALSE)
        {
          logfile.Write("ftp.ftpdelete(%s) failed.\n%s",m_vrbinfilelist[ii].remotefilename,ftp.response()); 
        }
        logfile.Write("delete  %s ...  %s",m_vrbinfilelist[ii].remotefilename,ftp.response()); 
      }
    }
  }
  return TRUE;
}

// 获取远程雷达图服务器的文件清单
BOOL nlist()
{
  m_vrgiffilelist.clear();
  char strgifListFileName[301];
  memset(strgifListFileName,0,sizeof(strgifListFileName));
  snprintf(strgifListFileName,300,"/tmp/htidc/tmp/ftpgif_%s_%s_%d.list",strremoteip,strusername,getpid());
  MKDIR("/tmp/htidc/tmp",FALSE);

  // 进入目标目录
  if (ftp.chdir(strremotepathgif) == FALSE)
  {
    // 如果进入目标目录失败，有可能是目标目录尚未创建
    logfile.Write("ftp.chdir(%s) failed.\n%s",strremotepathgif,ftp.response()); return FALSE;
  }

  char strMatch[2];
  memset(strMatch,0,sizeof(strMatch));

  // 获取对方的文件清单时，往往需要更长的时间，所以这里设置为1200秒
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles2",1200);

  // 获取远程服务器的文件清单，分别用"."、"*"和" "尝试。
  for (int ii=0;ii<3;ii++)
  {
    if (ii==0) strMatch[0]='*';
    if (ii==1) strMatch[0]='.';
    if (ii==2) strMatch[0]=0;

    REMOVE(strgifListFileName); 

    ftp.nlist(strMatch,strgifListFileName);

    ProgramActive.WriteToFile();

    if (FileSize(strgifListFileName) > 2) break;

    REMOVE(strgifListFileName); 
  }

  // 程序的超时时间再设为utimeout秒。
  // ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",utimeout);

  // 判断清单文件是否为空，为空就直接返回
  if (FileSize(strgifListFileName) <= 0) return FALSE;

  char strLine[301];

  listfp=0;

  // 把远程服务器的雷达图文件清单文件加载到m_vrgiffilelist
  if ( (listfp=FOPEN(strgifListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strgifListFileName); REMOVE(strgifListFileName); return FALSE; 
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

    snprintf(stfilelist.remotefilename,300,"%s/%s",strremotepathgif,strLine);
    if (strlen(strremotepathbak) != 0)
    {
      snprintf(stfilelist.remotefilenamebak,300,"%s/%s",strremotepathbak,strLine);
    }
    snprintf(stfilelist.localfilename ,300,"%s/%s",strlocalpath ,strLine);

    stfilelist.ftpsts=1;

    m_vrgiffilelist.push_back(stfilelist);
  }

  fclose(listfp); 

  // 删除已使用的远程服务器的文件清单文件。
  REMOVE(strgifListFileName);

  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));
  LocalTime(strmtime,"yyyymmddhh24miss",0-umtime*60*60);

  // 获取远程服务器每个文件的修改时间和文件大小信息
  for (UINT ii=0;ii<m_vrgiffilelist.size();ii++)
  {
    // 如果获取远程服务器每个文件的修改时间和文件大小信息失败，返回失败
    if (ftp.mtime(m_vrgiffilelist[ii].remotefilename) == FALSE) 
    {
      logfile.Write("ftp.mtime(%s) FAILED.\n%s",m_vrgiffilelist[ii].remotefilename,ftp.response()); return FALSE; 
    }

    // 如果文件时间小于umtime，就丢弃这个文件
    if (umtime>0) 
    {
      if (strcmp(ftp.m_mtime,strmtime) < 0) 
      {
        m_vrgiffilelist.erase(m_vrgiffilelist.begin()+ii); ii--; continue;
      }
    }

    // 如果获取远程服务器每个文件的修改时间和文件大小信息失败，返回失败
    if (ftp.size(m_vrgiffilelist[ii].remotefilename) == FALSE) 
    {
      logfile.Write("ftp.size(%s) FAILED.\n%s",m_vrgiffilelist[ii].remotefilename,ftp.response()); return FALSE; 
    }

    strcpy(m_vrgiffilelist[ii].modtime,ftp.m_mtime);

    m_vrgiffilelist[ii].filesize=ftp.m_size;

  }

  m_vlfilelist.clear();

  listfp=0;

  // 把本地服务器的文件清单文件加载到m_vlfilelist
  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // 从文件中获取一行
      if (FGETS(strLine,500,listfp,"<endl/>") == FALSE) break;

      memset(&stfilelist,0,sizeof(stfilelist));

      GetXMLBuffer(strLine,"filename", stfilelist.remotefilename,300);
      GetXMLBuffer(strLine,"modtime",  stfilelist.modtime,14);
      GetXMLBuffer(strLine,"filesize",&stfilelist.filesize);

      //logfile.Write("%s,%s,%d\n",stfilelist.remotefilename,stfilelist.modtime,stfilelist.filesize);

      m_vlfilelist.push_back(stfilelist);
    }

    fclose(listfp);
  }

  // logfile.Write("nlist local load ok.\n");

  for (UINT ii=0;ii<m_vrgiffilelist.size();ii++)
  {
    if (m_vlfilelist.size() == 0) continue;
    
    // 把远程目录的雷达图文件名、大小和日期和本地的清单文件比较一下，如果全部相同，就把ftpsts设置为2，不再采集
    for (UINT jj=0;jj<m_vlfilelist.size();jj++)
    {
      if (strcmp(m_vrgiffilelist[ii].remotefilename,m_vlfilelist[jj].remotefilename) == 0) 
      {
        if ( (strcmp(m_vrgiffilelist[ii].modtime,m_vlfilelist[jj].modtime) == 0) && (m_vrgiffilelist[ii].filesize==m_vlfilelist[jj].filesize) )
        {
	  m_vrgiffilelist[ii].ftpsts=2; 
	}
        break;
      }
    }
  }

  // logfile.Write("nlist check ok.\n");

  return TRUE;
}

// 从远程服务器get文件
BOOL ftpgetfiles()
{
  ProgramActive.WriteToFile();

  CTimer Timer;

  Timer.Beginning();

  logfile.Write("get %s ...",stfilelist.remotefilename);

  BOOL bIsLogFile=MatchFileName(stfilelist.localfilename,"*.LOG");

  MKDIR(stfilelist.localfilename,TRUE);

  // 获取文件
  if (ftp.get(stfilelist.remotefilename,stfilelist.localfilename,bIsLogFile) == FALSE)
  {
    logfile.WriteEx("FAILED.\n%s",ftp.response()); return FALSE;
  }

  logfile.WriteEx("ok(size=%d,time=%d).\n",stfilelist.filesize,Timer.Elapsed()); 

  // 判断是否需要删除远程服务器的文件，如果删除不成功，也不必返回失败。
  if (udeleteremote == 1)  
  {
    if (ftp.ftpdelete(stfilelist.remotefilename) == FALSE)
    {
      logfile.Write("ftp.ftpdelete(%s) failed.\n%s",stfilelist.remotefilename,ftp.response()); return TRUE;
    }
  }
  else
  {
    // 如果不需要删除，判断是否需要改名，如果改名不成功，也不必返回失败。
    if (strlen(strremotepathbak) != 0)
    {
      if (ftp.ftprename(stfilelist.remotefilename,stfilelist.remotefilenamebak) == FALSE)
      {
	logfile.Write("ftp.ftprename(%s,%s) failed.\n%s",stfilelist.remotefilename,stfilelist.remotefilenamebak,ftp.response()); return TRUE;
      }
    }
  }

  return TRUE;
}


