#include "idcapp.h"

void CallQuit(int sig);
BOOL SplitBuffer(char *strline);

CCmdStr cmdfieldstr;           // 所有字段和需要字段拆分和存放
vector<char *> vstrline;       // 存放一行数据

CDir           Dir;
CLogFile       logfile;
CProgramActive ProgramActive;

CFile filecsv,fileout;         //  csv输入文件，out输出文件 
char strXmlBuffer[4001];
char strsrcfilepath[301];
char strfieldstr[2001];
char strstdname[31];
char strstdpath[201];
char strTmpFileName[301];
char strdiscard[201];
char strandchild[11];
BOOL bandchild=FALSE;
char strmatchname[301];
UINT Seq;

BOOL _pcsvfiles();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/pcsvfiles logfilename xmlbuffer\n");
    printf("Sample:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/pcsvfiles /tmp/htidc/log/pcsvfiles.log \"<srcfilepath>/qxdata/szqx/sdata/tmp</srcfilepath><matchname>*.CSV</matchname><andchild>FALSE</andchild><fieldstr>obtid,lon,lat,height,,,city,,town</fieldstr><stdname>OBTHOURD</stdname><stdpath>/tmp/htidc/tmp</stdpath><discard>XK_WSH,XK_XMMC</discard>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于从csv文件中采集数据，结果生成xml文件存放在stdpath中。\n");
    printf("logfilename是本程序运行的日志文件。\n");
    printf("xmlbuffer为数据挖掘的参数，如下：\n");
    printf("提取后数据内容对应的xml字段名 <fieldstr>obtid,lon,lat,height,,,city,,town</fieldstr>\n");
    printf("源文件要是有标题行，要去掉标题行，源文件有多少个字段，全部字段都要写，不要的字段就留空，用逗号分离\n");
    printf("xml文件的前缀 <stdname>OBTHOURD</stdname>\n");
    printf("xml文件的输出的目录 <outpath>/tmp/htidc/ftpput</outpath>。\n");
    printf("discard 如果内容包含了这个的，就丢弃本条数据，一般用于数据内容包含了标题行，那就要去掉标题行\n");
    printf("本程序所有参数都不能为空，如果参数为空，程序会报错退出。注意:csv文件中字段值不得包括\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("pcsvfiles");

  memset(strsrcfilepath,0,sizeof(strsrcfilepath));
  memset(strfieldstr,0,sizeof(strfieldstr));
  memset(strstdname,0,sizeof(strstdname));
  memset(strstdpath,0,sizeof(strstdpath));
  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strdiscard,0,sizeof(strdiscard));
  memset(strmatchname,0,sizeof(strmatchname));
  memset(strandchild,0,sizeof(strandchild));

  GetXMLBuffer(strXmlBuffer,"fieldstr",strfieldstr,2000);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,300);
  GetXMLBuffer(strXmlBuffer,"stdname",strstdname,30);
  GetXMLBuffer(strXmlBuffer,"stdpath",strstdpath,200);
  GetXMLBuffer(strXmlBuffer,"discard",strdiscard,200);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"andchild",strandchild,10);

  // 如果其它参数为空，程序会报错退出。
  if (strlen(strsrcfilepath) == 0) { logfile.Write("srcfilepath is null.\n"); return -1; }
  if (strlen(strfieldstr) == 0)    { logfile.Write("fieldstr is null.\n"); return -1;    }
  if (strlen(strstdname) == 0)     { logfile.Write("stdname is null.\n"); return -1;     }
  if (strlen(strstdpath) == 0)     { logfile.Write("stdpath is null.\n"); return -1;     }
  if (strlen(strmatchname) == 0)   { logfile.Write("strmatchname is null.\n"); return -1;}
  if (strlen(strandchild) == 0)    { logfile.Write("andchild is null.\n"); return -1;    }

  if ( (strcmp(strandchild,"TRUE") == 0) || (strcmp(strandchild,"true") == 0) ) bandchild=TRUE;

  logfile.Write("pcsvfiles beginning.\n");

  // 注意，程序超时是600秒
  ProgramActive.SetProgramInfo(&logfile,"pcsvfiles",600);

  // 字段名要小写，不然入库有问题
  // 把xml的字段名用CCmdStr类拆分开并存放在m_vCmdStr
  ToLower(strfieldstr);
  cmdfieldstr.SplitToCmd(strfieldstr,",");

  if (cmdfieldstr.CmdCount() == 0) { logfile.Write("cmdallfieldstr or cmdfieldstr is invalid.\n"); return -1; }

  // 写入进程活动信息
  ProgramActive.WriteToFile();

  // 打开待发送文件的目录
  if (Dir.OpenDirNoSort(strsrcfilepath,bandchild) == FALSE)
  {
    logfile.Write("Dir.OpenDirNoSort(%s) failed.\n",strsrcfilepath); 
    return -1;
  } 

  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 读取一个文件
    if (Dir.ReadDir() == FALSE) break;

    // 如果文件名不匹配或后缀为TMP，就不传输，跳过它
    if ( (MatchFileName(Dir.m_FileName,strmatchname)==FALSE) ||
         (MatchFileName(Dir.m_FileName,"*.TMP,*.SWP")== TRUE) ) continue;

    logfile.Write("Process file %s...",Dir.m_FileName);

    if (_pcsvfiles() == FALSE) continue;

    logfile.Write("ok.\n");

    REMOVE(Dir.m_FullFileName);
 
  }

  return 0;

}

BOOL _pcsvfiles()
{
  char strBeginTime[21]; 
  memset(strBeginTime,0,sizeof(strBeginTime));
  LocalTime(strBeginTime,"yyyymmddhh24miss");

  snprintf(strTmpFileName,300,"%s/%s_%s_%d_%lu.xml",strstdpath,strstdname,strBeginTime,getpid(),Seq);

  if (filecsv.OpenForRead(Dir.m_FullFileName,"r") == FALSE) { logfile.Write("Open %s failed.\n",Dir.m_FullFileName); return FALSE; }

  if (fileout.OpenForRename(strTmpFileName,"a+") == FALSE)  { logfile.Write("fileout.OpenForRename is failed.\n"); return FALSE; }

  char strtmpline[8092];
  memset(strtmpline,0,sizeof(strtmpline));

  int totalcount=0;

  fileout.Fprintf("<data>\n");

  while (TRUE)
  {
    memset(strtmpline,0,sizeof(strtmpline));

    if ( filecsv.FFGETS(strtmpline,8000) == FALSE ) break;   

    if (strstr(strtmpline,strdiscard) != 0) continue;

    if ( SplitBuffer(strtmpline) == FALSE ) continue; 

    for(UINT i=0;i<cmdfieldstr.CmdCount();i++)
    { 
      if (strlen(cmdfieldstr.m_vCmdStr[i].c_str()) == 0 || strlen(vstrline[i]) == 0) continue;
      fileout.Fprintf("<%s>%s</%s>",cmdfieldstr.m_vCmdStr[i].c_str(),vstrline[i],cmdfieldstr.m_vCmdStr[i].c_str());
    }

    fileout.Fprintf("<endl/>\n");

    totalcount++;
  }

  logfile.Write("create %s(%lu) ok.\n",fileout.m_filename,totalcount);

  fileout.Fprintf("</data>\n");

  fileout.CloseAndRename();

  filecsv.Fclose();
 
  return TRUE;

}

BOOL SplitBuffer(char *strline)
{
  if ( strlen(strline) == 0 ) return FALSE;

  vstrline.clear();
   
  CCmdStr cmdstr;
  cmdstr.SplitToCmd(strline,",");
  for(UINT i=0; i<cmdfieldstr.CmdCount();i++)
  {
    vstrline.push_back((char *)cmdstr.m_vCmdStr[i].c_str());
  }
   
  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // 如果程序异常退出，删除中间状态的文件
  REMOVE(strTmpFileName);

  filecsv.Fclose();  

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("pcsvfiles exit.\n");

  exit(0);
}


