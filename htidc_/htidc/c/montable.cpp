#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

CDMONCFG DMONCFG;

int _montable();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./montable logfilename connstr\n");

    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/montable /log/ssqx/montable.log qxidc/pwdidc@EJETDB_221.179.6.136\n\n");
 
    printf("数据中心的数据量告警程序，程序读取数据监控参数表（T_DMONCFG）和数据量日志表（T_DMONITEM）表中。\n");
    printf("程序每50秒启动一次，由procctl调度。\n\n");
 
    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("montable");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"montable",300);

  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  DMONCFG.BindConnLog(&conn,&logfile);

  // 从T_DMONCFG表中加载需要全部的记录，存放在m_vDMONCFG中
  if (DMONCFG.LoadDMONCFG("where rsts=1") == FALSE) CallQuit(-1);

  // 读取m_vDMONCFG中全部的参数，分析告警
  if (DMONCFG.MONTable() == FALSE) CallQuit(-1);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("montable exit.\n");

  exit(0);
}

