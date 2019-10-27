#include "_public.h"

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if ( (argc != 3) && (argc != 4) )
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/deletefiles pathname dayout [matchstr]\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /tmp/htidc 5\n\n");
    printf("        /htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /tmp/htidc 5 *.GIF\n\n");

    printf("这是一个工具程序，用于清理pathname目录dayout天之前的全部文件，并且会处理pathname的各级子目录。\n");
    printf("如果文件的修改时间在dayout之前就会被清理，dayout可以是小数。\n");
    printf("matchstr是一个可选参数，指定被删除的文件名的匹配方式。\n");
    printf("本程序不写日志文件，也不会在屏幕上输出任何信息。\n");
    printf("本程序可以手工运行，也可以由procctl调度。\n\n\n");
 
    return -1;
  }

  char   strPathName[201];
  double dDayOut=0;

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  dDayOut=atof(argv[2]);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  //CloseIOAndSignal(); 
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strTimeOut[21];

  LocalTime(strTimeOut,"yyyy-mm-dd hh24:mi:ss",0-(int)(dDayOut*24*60*60));

  CDir Dir;

  Dir.m_uMaxFileCount=1000000;  // 一次只扫描1000000个文件

  Dir.m_bAndTMPFiles=TRUE;    // 包括*.tmp文件
 
  // 打开目录，读取文件，包括它的子目录
  if (Dir.OpenDirNoSort(strPathName,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",strPathName); exit(-1);
  }
  char strLocalTime[21];

  while (Dir.ReadDir() == TRUE)
  {
    if (strcmp(Dir.m_ModifyTime,strTimeOut) > 0) continue;

    if (argc == 4) if (MatchFileName(Dir.m_FileName,argv[3])==FALSE) continue;
    
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss");

    printf("%s delete %s ok.\n",strLocalTime,Dir.m_FullFileName);

    REMOVE(Dir.m_FullFileName);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  exit(0);
}

