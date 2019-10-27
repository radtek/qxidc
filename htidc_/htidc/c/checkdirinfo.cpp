#include "_public.h"

// 程序异常中断处理函数
void EXIT(int sig);

CLogFile       logfile;                // 日志文件类
CProgramActive ProgramActive;          // 后台进程监控类
CFile          File;
CDir           Dir;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/checkdirinfo checkdirinfo.xml\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/checkdirinfo /htidc/hssms/ini/checkdirinfo.xml\n\n");
    printf("程序读取checkdirinfo.xml参数文件，分析待监控目录下的文件数是否超出阀值，如果超出，就发出告警。\n");
    printf("本程序5分钟运行一次，由调度程序控制，日志文件名为/tmp/htidc/log/checkdirinfo.log。\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开程序运行日志
  if (logfile.Open("/tmp/htidc/log/checkdirinfo.log","a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n","/tmp/htidc/log/checkdirinfo.log"); return -1;
  }

  logfile.SetAlarmOpt("checkdirinfo");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"checkdirinfo",180);

  logfile.Write("开始分析.\n");

  if (File.OpenForRead(argv[1],"r") == FALSE)
  {
    logfile.Write("File.OpenForRead(%s) failed.\n",argv[2]); EXIT(-1);
  }

  char strBuffer[1024];
  char pathname[301],andchild[11];
  long filecount;

  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));
    memset(pathname,0,sizeof(pathname));
    memset(andchild,0,sizeof(andchild));
    filecount=0;

    if (File.FFGETS(strBuffer,1000) == FALSE) break;

    if (strstr(strBuffer,"#") != 0) continue;

    GetXMLBuffer(strBuffer,"pathname",pathname,300);
    GetXMLBuffer(strBuffer,"andchild",andchild,5);
    GetXMLBuffer(strBuffer,"filecount",&filecount);

    if ( (strlen(pathname)==0) || (strlen(andchild)==0) || (filecount<=0) ) continue;

    BOOL bAndChild=FALSE;
    if ( (strcmp(andchild,"true")==0) || (strcmp(andchild,"TRUE")==0) ) bAndChild=TRUE;

    if (Dir.OpenDir(pathname,bAndChild) == FALSE)
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",pathname); continue;
    }

    logfile.Write("pathname=%s,andchild=%s,filecount=%ld,fileexist=%lu.\n",pathname,andchild,filecount,Dir.m_vFileName.size());

    if (Dir.m_vFileName.size() >= (UINT)filecount)
    {
      logfile.Write("目录%s（是否包括子目录：%s）中的文件数为%lu，超过了监控的阀值%ld。failed.\n",pathname,andchild,Dir.m_vFileName.size(),filecount);
    }
  }

  logfile.Write("分析完成.\n\n");

  exit(0);
}

void EXIT(int sig)
{
  if (sig > 0) 
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  logfile.Write("checkdirinfo EXIT.\n");

  exit(0);
}

