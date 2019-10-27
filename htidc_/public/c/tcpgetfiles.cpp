/*
 这是一个通用的功能模块，采用TCP协议获取文件的客户端。
*/

#include "_public.h"

struct st_arg
{
  char ip[31];              // 服务器端的IP地址。
  int  port;                // 服务器端的端口。
  int  ptype;               // 文件获取成功后文件的处理方式：1-保留文件；2-删除文件；3-移动到备份目录。
  char clientpath[301];     // 本地文件存放的根目录。
  char srvpath[301];        // 服务端文件存放的根目录。
  char srvpathbak[301];     // 文件成功获取后，服务端文件备份的根目录，当ptype==3时有效。
  bool andchild;            // 是否获取srvpath目录下各级子目录的文件，true-是；false-否。
  char matchname[301];      // 待获取文件名的匹配方式，如"*.TXT,*.XML"，注意用大写。
  char okfilename[301];     // 已获取成功文件名清单。
  int  timetvl;             // 扫描本地目录文件的时间间隔，单位：秒。
} starg;

char strRecvBuffer[TCPBUFLEN+10]; // 接收报文的缓冲区
char strSendBuffer[TCPBUFLEN+10]; // 发送报文的缓冲区

vector<struct st_fileinfo> vlistfile,vlistfile1;
vector<struct st_fileinfo> vokfilename,vokfilename1;

// 把服务端srvpath目录下的文件加载到vlistfile容器中
bool LoadListFile();

// 把okfilename文件内容加载到vokfilename容器中
bool LoadOKFileName();

// 把vlistfile容器中的文件与vokfilename容器中文件对比，得到两个容器
// 一、在vlistfile中存在，并已经采集成功的文件vokfilename1
// 二、在vlistfile中存在，新文件或需要重新采集的文件vlistfile1
bool CompVector();

// 把vokfilename1容器中的内容先写入okfilename文件中，覆盖之前的旧okfilename文件
bool WriteToOKFileName();

// 如果ptype==1，把采集成功的文件记录追加到okfilename文件中
bool AppendToOKFileName(struct st_fileinfo *stfileinfo);

CTcpClient TcpClient;

CLogFile logfile;

// 本程序的业务流程主函数
bool _tcpgetfiles();

void EXIT(int sig);

// 显示程序的帮助
void _help(char *argv[]);
  
// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

// 登录服务器
bool ClientLogin(const char *argv);

// 向服务端发送心跳报文
bool ActiveTest();

// 实现文件获取的功能
bool _tcpgetfiles();

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // 关闭全部的信号和输入输出
  CloseIOAndSignal();

  // 处理程序退出的信号
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
  }

  // 把xml解析到参数starg结构中
  if (_xmltoarg(argv[2])==false) return -1;

  while (true)
  {
    // 向服务器发起连接并登录
    ClientLogin(argv[2]);

    // 实现文件获取的功能
    _tcpgetfiles();

    if (vlistfile.size()==0)
    {
      // 向服务端发送心跳报文
      ActiveTest(); 
    
      sleep(starg.timetvl);
    }
  }

  return 0;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d\n\n",sig);

  TcpClient.Close();

  exit(0);
}

// 显示程序的帮助
void _help(char *argv[])
{
  printf("\n");
  printf("Using:/htidc/public/bin/tcpgetfiles logfilename xmlbuffer\n\n");

  printf("Sample:/htidc/public/bin/tcpgetfiles /log/shqx/tcpgetfiles_surfdata.log \"<ip>172.16.0.15</ip><port>5010</port><ptype>1</ptype><clientpath>/data/shqx/sdata/surfdata</clientpath><srvpath>/data/shqx/tcp/surfdata</srvpath><srvpathbak>/data/shqx/tcp/surfdatabak</srvpathbak><andchild>true</andchild><matchname>SURF_*.TXT,*.DAT</matchname><okfilename>/data/shqx/tcplist/tcpgetfiles_surfdata.xml</okfilename><timetvl>10</timetvl>\"\n\n\n");

  printf("这是一个通用的功能模块，采用TCP协议获取文件的客户端。\n");
  printf("logfilename   本程序运行的日志文件。\n");
  printf("xmlbuffer     本程序运行的参数，如下：\n");
  printf("ip            服务器端的IP地址。\n");
  printf("port          服务器端的端口。\n");
  printf("clientpath    客户端文件存放的根目录。\n");
  printf("srvpath       服务端文件存放的根目录。\n");
  printf("ptype         文件获取成功后服务端文件的处理方式：1-保留文件；2-删除文件；3-移动到备份目录。\n");
  printf("srvpathbak    文件成功获取后，服务端文件备份的根目录，当ptype==3时有效，缺省为空。\n");
  printf("andchild      是否获取srvpath目录下各级子目录的文件，true-是；false-否，缺省为false。\n");
  printf("matchname     待获取文件名的匹配方式，如\"*.TXT,*.XML\"，注意用大写。\n");
  printf("okfilename    已获取成功文件名清单，缺省为空。\n");
  printf("timetvl       扫描本地目录文件的时间间隔，单位：秒，取值在1-50之间。\n\n\n");
}

// 把xml解析到参数starg结构中
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


// 登录服务器
bool ClientLogin(const char *argv)
{
  if (TcpClient.m_sockfd>0) return true;

  int ii=0;

  while (true)
  {
    if (ii++>0) sleep(20);    // 第一次进入循环不休眠

    // 向服务器发起连接
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

// 向服务端发送心跳报文
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

// 实现文件获取的功能
bool _tcpgetfiles()
{
  // 把服务端srvpath目录下的文件加载到vlistfile容器中
  if (LoadListFile()==false)
  {
    logfile.Write("LoadListFile() failed.\n"); TcpClient.Close(); return false;
  }

  if (starg.ptype==1)
  {
    // 加载okfilename文件中的内容到容器vokfilename中
    LoadOKFileName();

    // 把vlistfile容器中的文件与vokfilename容器中文件对比，得到两个容器
    // 一、在vlistfile中存在，并已经采集成功的文件vokfilename1
    // 二、在vlistfile中存在，新文件或需要重新采集的文件vlistfile1
    CompVector();

    // 把vokfilename1容器中的内容先写入okfilename文件中，覆盖之前的旧okfilename文件
    WriteToOKFileName();
   
    // 把vlistfile1容器中的内容复制到vlistfile容器中
    vlistfile.clear(); vlistfile.swap(vlistfile1);
  }

  // 从服务端逐个获取新文件或已改动过的文件
  for (int ii=0;ii<vlistfile.size();ii++)
  {
    // 向服务端发送将获取（下载）的文件名信息
    memset(strSendBuffer,0,sizeof(strSendBuffer));
    sprintf(strSendBuffer,"<filename>%s</filename><filesize>%d</filesize><mtime>%s</mtime>",vlistfile[ii].filename,vlistfile[ii].filesize,vlistfile[ii].mtime);
    // logfile.Write("3 strSendBuffer=%s\n",strSendBuffer);     // xxxxxx  
    if (TcpClient.Write(strSendBuffer) == false)
    {
      logfile.Write("3 TcpClient.Write() failed.\n"); TcpClient.Close(); return false;
    }

    // 此报文有些多余，但是为了兼容SendFile和RecvFile函数，但是对性能不会有影响。
    if (TcpClient.Read(strRecvBuffer) == false)
    {
      logfile.Write("3 TcpClient.Read() failed.\n"); TcpClient.Close(); return false;
    }
    // logfile.Write("3 strRecvBuffer=%s\n",strRecvBuffer);     // xxxxxx  
    
    // 把文件名中的clientpath替换成srvpath，要小心第三个参数
    struct st_fileinfo stfileinfo;
    memset(&stfileinfo,0,sizeof(struct st_fileinfo));
    strcpy(stfileinfo.filename,vlistfile[ii].filename);
    strcpy(stfileinfo.mtime,vlistfile[ii].mtime);
    stfileinfo.filesize=vlistfile[ii].filesize;
    UpdateStr(stfileinfo.filename,starg.srvpath,starg.clientpath);

    logfile.Write("get %s ...",stfileinfo.filename);

    // 接收文件的内容
    if (RecvFile(&logfile,TcpClient.m_sockfd,&stfileinfo)== false)
    {
      logfile.Write("RecvFile() failed.\n"); TcpClient.Close(); return false;
    }

    logfile.WriteEx("ok.\n");

    // 如果ptype==1，把采集成功的文件记录追加到okfilename文件中
    if (starg.ptype==1) AppendToOKFileName(&vlistfile[ii]);
  }

  return true;
}

// 把服务端srvpath目录下的文件加载到vlistfile容器中
bool LoadListFile()
{
  // 1、客户向服务端发送一个请求报文；
  // 2、服务端Dir扫描srvpath下面文件，得到一个文件清单；
  // 3、服务端向客户端发送一个文件总数的报文；
  // 4、把文件信息一条一条的传给客户端；
  // 5、客户端接收到的记录直接存放在vlistfile容器中。

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

// 把okfilename文件内容加载到vokfilename容器中
bool LoadOKFileName()
{
  vokfilename.clear();

  CFile File;

  // 注意：如果程序是第一次采集，okfilename是不存在的，并不是错误，所以也返回true。
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

// 把vlistfile容器中的文件与vokfilename容器中文件对比，得到两个容器
// 一、在vlistfile中存在，并已经采集成功的文件vokfilename1
// 二、在vlistfile中存在，新文件或需要重新采集的文件vlistfile1
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

// 把vokfilename1容器中的内容先写入okfilename文件中，覆盖之前的旧okfilename文件
bool WriteToOKFileName()
{
  CFile File;

  // 注意，打开文件不要采用缓冲机制
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

// 如果ptype==1，把采集成功的文件记录追加到okfilename文件中
bool AppendToOKFileName(struct st_fileinfo *stfileinfo)
{
  CFile File;

  // 注意，打开文件不要采用缓冲机制
  if (File.Open(starg.okfilename,"a",false) == false)
  {
    logfile.Write("File.Open(%s) failed.\n",starg.okfilename); return false;
  }

  File.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n",stfileinfo->filename,stfileinfo->mtime);

  return true;
}

