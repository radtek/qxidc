#include "qxidc.h"

CIDCCFG IDCCFG;

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CTIMECHECK     TIMECHECK;

char strTableName[51];

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\nUsing:/htidc/pyqx/c/timecheck inifile tname\n");
    printf("Example:/htidc/pyqx/c/timecheck /htidc/pyqx/ini/pyqx.xml T_OBTMIND\n");
    printf("        /htidc/htidc/bin/procctl 10 /htidc/pyqx/c/timecheck /htidc/pyqx/ini/pyqx.xml T_OBTMIND\n\n\n");

    printf("该程序用于对数据表做时间一致性质量控制。\n\n\n");

    return -1;
  }

  // 从参数文件中加载全部的参数
  if (IDCCFG.LoadIniFile(argv[1]) == FALSE)
  {
    printf("IDCCFG.LoadIniFile(%s) failed.\n",argv[1]); return -1;
  }

  strcpy(strTableName,argv[2]);

  memset(strTableName,0,sizeof(strTableName));

  strcpy(strTableName,argv[2]);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; 
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,201,"%s/timecheck_%s.log",IDCCFG.m_logpath,strTableName);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }

  // 注意，程序超时是1800秒
  ProgramActive.SetProgramInfo(&logfile,"timecheck",1800);

  // 连接数据库，此数据库连接用于正常的数据处理
  if (conn.connecttodb(IDCCFG.m_idcconnstr,TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",IDCCFG.m_idcconnstr); CallQuit(-1);
  }

  logfile.Write("timecheck(%s) beging.\n",strTableName);

  TIMECHECK.BindConnLog(&conn,&logfile);

  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    if (TIMECHECK.CheckTable(strTableName) != 0)
    {
      logfile.Write("TIMECHECK","服务程序：timecheck，调用CheckTable(%s)失败!",strTableName); CallQuit(-1);
    }

    conn.commitwork();

    sleep(10);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("timecheck exit.\n");

  exit(0);
}

