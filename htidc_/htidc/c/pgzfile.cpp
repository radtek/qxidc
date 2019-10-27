#include "_public.h"

CDir           Dir; 
CLogFile       Logfile;
CProgramActive ProgramActive;
CCmdStr        CmdStr;
CFile          File;

char strlogfile[301];
char strsrcfilepath[301];
char strstdfilepath[301];
char strmatchname[301];
char strstarthour[101];

void CallQuit(int sig);

int main( int argc,char *argv[])
{
  if(argc!=2)
  {
    printf("Using: /htidc/htidc/bin/RenameFile xmlbuffer\n");
    printf("Using: /htidc/htidc/bin/RenameFile \"<logfile>/log/gzqx/RenameFile.log</logfile><srcfilepath></srcfilepath><stdfilepath></stdfilepath><matchname>*.GIF</matchname><starthour></starthour>\"\n");

    printf("这是一个工具程序,用于改文件名\n");
    printf("有很多的非结构文件，由于文件名是固定，需要更改文件名之后再入库\n");
    printf("文件原名为csnl.html，改名为：csnl20151214.html\n");
    printf("这个工具程序只针对广州城市内涝和广州天气急件文件进行处理，因为从客户端上来的数据，文件名是固定的,\n");
    printf("但是他上传上来的数据有可能是昨天的，所以需要先进行格式转换，查看里面内容，然后再判断是什么时候的数据。\n");
    printf("只判断是不是今天的，如果不是今天的就不处理，也不删除\n");
    printf("starthour 是小时数列表，用逗号隔开，可以为空\n");
   
    return -1; 
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001];

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[1],4000);

  memset(strlogfile,0,sizeof(strlogfile));
  memset(strsrcfilepath,0,sizeof(strsrcfilepath));
  memset(strstdfilepath,0,sizeof(strstdfilepath));
  memset(strmatchname,0,sizeof(strmatchname));
  memset(strstarthour,0,sizeof(strstarthour));

  GetXMLBuffer(strXmlBuffer,"logfile",strlogfile,300);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,300);
  GetXMLBuffer(strXmlBuffer,"stdfilepath",strstdfilepath,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"starthour",strstarthour,100);

  // 打开日志文件 
  if (Logfile.Open(strlogfile,"a+") == FALSE) 
  {   
    printf("Logfile.Open(%s) failed.\n",argv[1]); return -1; 
   }

  if (strlen(strlogfile) == 0)     { Logfile.Write("logfile is null.\n"); return -1; }
  if (strlen(strsrcfilepath) == 0) { Logfile.Write("srcfilepath is null.\n"); return -1; }
  if (strlen(strstdfilepath) == 0) { Logfile.Write("stdfilepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0)   { Logfile.Write("matchname is null.\n"); return -1; }
 
  // 获取服务器的时间，判断是否该启用程序，如果时间中的小时不匹配，就退出
  if (strlen(strstarthour) != 0)
  {
     char Time[21];
     memset(Time,0,sizeof(Time));
     LocalTime(Time,"hh24mi");
     Time[2]=0;
     if (strstr(strstarthour,Time) == 0) return 0;
  }
  
  Logfile.SetAlarmOpt("RenameFile");

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&Logfile,"RenameFile",300);
  
  // 打开要处理的文件目录
  if (Dir.OpenDir(strsrcfilepath,TRUE) == FALSE)
  {
    Logfile.Write("Dir.OpenDir %s failed.\n",argv[2]); return -1;
  }

  char strcmd[1024];
  char strBuffer[8001];
  char strLocalTime[21];
  char strStdFileName[301];
  char mm[3],dd[3];
  char date[31];

  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(date,0,sizeof(date));

  LocalTime(strLocalTime,"yyyymmdd");
 
  strncpy(mm,strLocalTime+4,2);
  strncpy(dd,strLocalTime+6,2);
  snprintf(date,30,"%s月%s日",mm,dd); 
    
  while(Dir.ReadDir()==TRUE) 
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();
 
    if (MatchFileName(Dir.m_FileName,strmatchname) == FALSE) continue;	
    
    // 开始处理文件
    Logfile.Write("process file %s...",Dir.m_FileName);

    memset(strcmd,0,sizeof(strcmd));
    snprintf(strcmd,1000,"iconv -c -f utf-8 -t gb18030 %s -o /tmp/htidc/tmp/gztmpfile.txt",Dir.m_FullFileName);
    system(strcmd);

    // 打开转换文件
    if ((File.OpenForRead("/tmp/htidc/tmp/gztmpfile.txt","r")) == FALSE)
    {
       Logfile.Write("OpenForRead(/tmp/htidc/tmp/gztmpfile.txt) failed.\n"); return FALSE;
    }
    while(TRUE)
    {
      memset(strBuffer,0,sizeof(strBuffer));
      if (File.FFGETS(strBuffer,8000) == FALSE) break;

      // 如果没有找到当天的年月日就说明不是今天的文件
      if (strstr(strBuffer,date) == 0) continue;

      CmdStr.SplitToCmd(Dir.m_FileName,".");

      memset(strStdFileName,0,sizeof(strStdFileName));
      sprintf(strStdFileName,"%s/%s%s.%s",strstdfilepath,CmdStr.m_vCmdStr[0].c_str(),strLocalTime,CmdStr.m_vCmdStr[1].c_str());

      if (RENAME(Dir.m_FullFileName,strStdFileName) == FALSE) 
      {
         Logfile.Write("failed.RENAME %s to %s failed.\n",Dir.m_FullFileName,strStdFileName);
         return -1;
      }

      Logfile.Write("to %s...ok.\n",strStdFileName);
    }

    REMOVE("/tmp/htidc/tmp/gztmpfile.txt");
  }

  return 0;
}

  void CallQuit(int sig)
  {
  if (sig > 0) signal(sig,SIG_IGN);

  Logfile.Write("catching the signal(%d).\n",sig);

  Logfile.Write("RenameFile exit.\n");

  exit(0);
}

