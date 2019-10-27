#include "qxidc.h"

connection conn;
CLogFile   logfile;

char strConnStr[101];
char strTableName[51];

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if (argc != 3) 
  {
    printf("\nUsing:./qcrevert username/password@tnsname tablename\n\n"); 
    printf("        ./qcrevert pyidc/pwdidc T_OBTMIND\n\n"); 

    printf("这是一个工具程序，用于还原质控，恢复原始数据。\n");
    printf("注意，质控还原时，手工修改后的数据字段值不会被还原。\n");
    printf("质控还原时，质控状态被还原成质控初始状态，不一定是0。\n");
    printf("质控表的数据还原一定要用此程序，禁止手工修改表的质控字段。\n\n\n");

    return -1;
  }

  memset(strConnStr,0,sizeof(strConnStr));
  memset(strTableName,0,sizeof(strTableName));

  strcpy(strConnStr,argv[1]);
  strcpy(strTableName,argv[2]);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  logfile.Open("/tmp/qcrevert.log","a+");

  // 连接数据库
  if (conn.connecttodb(strConnStr,TRUE) != 0)
  {
    printf("conn.connecttodb(%s) failed\n",strConnStr); exit(-1);
  }

  CDATAQC DATAQC;

  DATAQC.BindConnLog(&conn,&logfile);

  DATAQC.RevertTable(strTableName);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("qcrevert exit.\n");

  exit(0);
}

