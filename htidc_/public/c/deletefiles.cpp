#include "_public.h"

void EXIT(int sig);

// 显示程序的帮助
void _help(char *argv[]);

int main(int argc,char *argv[])
{
  if ( (argc != 3) && (argc != 4) ) { _help(argv); return -1; }

  // 关闭全部的信号和输入输出
  CloseIOAndSignal();

  // 处理程序退出的信号
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  char   strPathName[201];
  double dDayOut=0;

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  dDayOut=atof(argv[2]);

  char strTimeOut[21];

  LocalTime(strTimeOut,"yyyy-mm-dd hh24:mi:ss",0-(int)(dDayOut*24*60*60));

  CDir Dir;

  char strMatch[50]; memset(strMatch,0,sizeof(strMatch));
  if (argc==3) strcpy(strMatch,"*");
  else strcpy(strMatch,argv[3]);

  // 打开目录，读取文件，包括它的子目录
  if (Dir.OpenDir(strPathName,strMatch,10000,true,false) == false)
  {
    printf("Dir.OpenDir(%s) failed.\n",strPathName); return -1;
  }

  char strLocalTime[21];

  while (Dir.ReadDir() == true)
  {
    if (strcmp(Dir.m_ModifyTime,strTimeOut) > 0) continue;

    printf("delete %s ok.\n",Dir.m_FullFileName);

    REMOVE(Dir.m_FullFileName);
  }

  return 0;
}

void EXIT(int sig)
{
  printf("程序退出，sig=%d\n\n",sig);

  exit(0);
}


// 显示程序的帮助
void _help(char *argv[])
{
  printf("\n");
  printf("Using:/htidc/public/bin/deletefiles pathname dayout [matchstr]\n");
  printf("Sample:/htidc/public/bin/deletefiles /data/shqx/ftp/surfdata 0.1 \"*.TXT,*.CSV\"\n\n");

  printf("本程序是数据中心的公共功能模块，用于删除指定目录下的历史文件。\n");
  printf("pathname 待清理的目录名，包括这个目录下的各级子目录。\n");
  printf("dayout   文件保留天数，单位是天，可以用小数。\n");
  printf("matchstr 待清理文件名的匹配规则，这是一个可选参数，可以匹配多种类型的文件，中间用逗号分隔，最好用双引号包含起来。\n\n");
}

