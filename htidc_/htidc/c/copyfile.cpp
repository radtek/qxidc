#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;
CIniFile       IniFile;
CProgramActive ProgramActive;
CDir           SrcDir;

char strlogfilename[501];
char strsrcfilepath[501];
char strdstfilepath[501];
char strsecond[501];
char strmatchfile[501];
char strifdeletesrcfile[501];
int  itimetvl;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\nExample:/htidc/htidc/bin/procctl 20 /htidc/htidc/bin/copyfile \"<logfilename>/log/szqx/copyfile.log</logfilename><srcfilepath>/tmp/11</srcfilepath><dstfilepath>/tmp/22</dstfilepath><second>5</second><matchfile>*.A</matchfile><ifdeletesrcfile>TRUE</ifdeletesrcfile><timetvl>-8</timetvl>\"\n\n");

    printf("此程序用于把本地/挂载目录中的文件复制到目的目录，复制完之后可以选择是否删除源文件。\n");
    printf("logfilename: 日志文件名。\n");
    printf("srcfilepath: 源文件目录。\n");
    printf("dstfilepath: 目的存放目录。\n");
    printf("seconde: 单位:秒，如果文件的时间在当前时间的前N秒之内，就暂时不处理,用于保证文件的完整性。\n");
    printf("matchfile: 文件名匹配方式，采用大写。\n");
    printf("ifdeletesrcfile: 复制完之后是否删除源文件。\n");
    printf("xmlbuffer可以处理时间变量，{簕YYYY}（4位的年）、{YYY}（后三位的年）、"\
           "{YY}（后两位的年）、{MM}（月月）、{DD}（日日）、{HH}（时时）、{MI}（分分）、{SS}（秒秒）。\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strsrcfilepath,0,sizeof(strsrcfilepath));
  memset(strdstfilepath,0,sizeof(strdstfilepath));
  memset(strsecond,0,sizeof(strsecond));
  memset(strmatchfile,0,sizeof(strmatchfile));
  memset(strifdeletesrcfile,0,sizeof(strifdeletesrcfile));
  itimetvl=0;

  char strXmlBuffer[4001];
  memset(strXmlBuffer,0,sizeof(strXmlBuffer));
  strncpy(strXmlBuffer,argv[1],4000);

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);

  // 处理xmlbuffer中的时间变量
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"logfilename",strlogfilename,500);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,500);
  GetXMLBuffer(strXmlBuffer,"dstfilepath",strdstfilepath,500);
  GetXMLBuffer(strXmlBuffer,"second",strsecond,500);
  GetXMLBuffer(strXmlBuffer,"matchfile",strmatchfile,500);
  GetXMLBuffer(strXmlBuffer,"ifdeletesrcfile",strifdeletesrcfile,500);
  
  if (strlen(strlogfilename) == 0)     { printf("logfilename is null.\n"); return -1;     }
  if (strlen(strsrcfilepath) == 0)     { printf("srcfilepath is null.\n"); return -1;     }
  if (strlen(strdstfilepath) == 0)     { printf("dstfilepath is null.\n"); return -1;     }
  if (strlen(strsecond) == 0)          { printf("second is null.\n"); return -1;          }
  if (strlen(strmatchfile) == 0)       { printf("matchfile is null.\n"); return -1;       }
  if (strlen(strifdeletesrcfile) == 0) { printf("ifdeletesrcfile is null.\n"); return -1; }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&logfile,"copyfile",300);

  char strLocalTime[21];
  char strTempFileName[501];
  char strStdFileName[501];

  if (SrcDir.OpenDir(strsrcfilepath) == FALSE)
  {
    logfile.Write("SrcDir.OpenDir %s failed.\n",strsrcfilepath); return -1;
  }

  while (SrcDir.ReadDir() == TRUE)
  {
    ProgramActive.WriteToFile();

    // 如果文件的时间在当前时间的前N秒之内，就暂时不处理。
    LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-atoi(strsecond));

    if ( (strcmp(SrcDir.m_ModifyTime,strLocalTime)>0) ) continue;

    if ( (MatchFileName(SrcDir.m_FileName,strmatchfile)==FALSE) ) continue;

    logfile.Write("copy file %s",SrcDir.m_FullFileName);

    memset(strTempFileName,0,sizeof(strTempFileName));
    memset(strStdFileName,0,sizeof(strStdFileName));
    
    snprintf(strTempFileName,500,"%s/%s.tmp",strdstfilepath,SrcDir.m_FileName);
    snprintf(strStdFileName,500,"%s/%s",strdstfilepath,SrcDir.m_FileName);
    
    COPY(SrcDir.m_FullFileName,strTempFileName);
    RENAME(strTempFileName,strStdFileName);
   
    if (strcmp(strifdeletesrcfile,"TRUE") == 0) REMOVE(SrcDir.m_FullFileName);

    logfile.WriteEx(" ok.\n");
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("copyfile exit.\n");

  exit(0);
}
