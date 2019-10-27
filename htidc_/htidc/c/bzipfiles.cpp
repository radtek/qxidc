#include "_public.h"

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  char   strPathName[201];
  double dDayOut=0;

  if (argc != 3 && argc != 4)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/bzipfiles pathname dayout [matchname]\n\n");

    printf("Example:/htidc/htidc/bin/bzipfiles /htidc/data 0.01 *20190412*.bz2\n\n");

    printf("这是一个工具程序，用于压缩pathname目录dayout天之前的全部文件，并且会处理pathname的各级子目录。\n");
    printf("如果文件的修改时间在dayout之前就会被压缩，dayout可以是小数。\n");
    printf("本程序不写日志文件，也不会在控制输出任何信息。\n");
    printf("本程序可以手工运行，也可以由procctl调度。\n");
    printf("本程序调用/usr/bin/bunzip2命令压缩文件。\n\n\n");
 
    return -1;
  }

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  dDayOut=atof(argv[2]);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); 
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strTimeOut[21];

  LocalTime(strTimeOut,"yyyy-mm-dd hh24:mi:ss",0-(int)(dDayOut*24*60*60));

  CDir Dir;
 
  // 打开目录，读取文件，包括它的子目录
  if (Dir.OpenDir(strPathName,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",strPathName); exit(-1);
  }

  char strCmd[1024];

  while (Dir.ReadDir() == TRUE)
  {
    if ( (strcmp(Dir.m_ModifyTime,strTimeOut) > 0) && (MatchFileName(Dir.m_FileName,"*.bz2") == TRUE) )
    {
      if (argc == 4)
      {
        if(MatchFileName(Dir.m_FileName,argv[3]) == TRUE)
        {
          memset(strCmd,0,sizeof(strCmd));
          sprintf(strCmd,"/usr/bin/bunzip2 -f %s 1>/dev/null 2>/dev/null",Dir.m_FullFileName);
          system(strCmd);
        }
      }
    }
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  exit(0);
}

