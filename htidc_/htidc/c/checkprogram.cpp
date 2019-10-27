#include "_public.h"

CLogFile       logfile;
CDir           ProcDir;
CProgramActive ProgramActive;

// 在/tmp目录下创建htidc和各级子目录
void mkdirhtidcdirs();

// 程序运行需要一个参数，即接口子系统的根目录
int main(int argc,char *argv[])
{
  if (argc != 1)
  {
    printf("\n");
    printf("Using:./htidc/htidc/bin/checkprogram\n"); 

    printf("Example:/htidc/htidc/bin/procctl 60 /htidc/htidc/bin/checkprogram\n\n");

    printf("此程序用于检查后台应用程序是否超时，如果已超时，就杀掉它。\n");
    printf("此程序运行在任何有后台程序运行的服务器上，由procctl调度，运行日志文件名为/tmp/htidc/log/checkprogram.log。\n");
    printf("在数据中心，被监控的后台程序生成的active文件存放在/tmp/htidc/proc目录中，以_P_A_打头。\n\n");

    printf("注意，不是任何程序都可以被checkprogram监控，被监控的后程序程序必须按要求写active文件，\n");
    printf("生成active文件的方法请与吴从周沟通。\n\n\n");
 
    return 0;
  }

  // 关闭全部的信号和输入输出，本程序只能用killall -9 checkprogram中止。
  CloseIOAndSignal(); 

  // 在/tmp目录下创建htidc和各级子目录
  mkdirhtidcdirs();

  if (logfile.Open("/tmp/htidc/log/checkprogram.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/checkprogram.log) failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("checkprogram");

  ProgramActive.m_logfile=&logfile;

  char strPIDNode[201];

  if (ProcDir.OpenDirNoSort("/tmp/htidc/proc") == FALSE)
  {
    logfile.Write("Dir.Open(/tmp/htidc/proc) FAILED.\n");  return -1;
  }

  while (ProcDir.ReadDir() == TRUE)
  {
    // 如果不是进程活动信息文件，就删除这个文件
    if (MatchFileName(ProcDir.m_FileName,"_P_A_*") == FALSE) 
    {
      logfile.Write("invalied file %s.\n",ProcDir.m_FullFileName);
      REMOVE(ProcDir.m_FullFileName); 
      continue;
    }

    // 读取进程文件的信息
    if (ProgramActive.ReadFromFile(ProcDir.m_FullFileName) == FALSE) 
    {
      logfile.Write("ProgramActive.ReadFromFile(%s) FAILED.\n",ProcDir.m_FileName); 
      REMOVE(ProcDir.m_FullFileName); 
      continue;
    }

    // 组成进程在/proc目录下的临时文件节点名，用于判断进程是否还存在。
    memset(strPIDNode,0,sizeof(strPIDNode));
    snprintf(strPIDNode,200,"/proc/%d",ProgramActive.m_PID);

    // 如果进程已不存在，直接删除这个文件。
    if (access(strPIDNode,R_OK) != 0) { REMOVE(ProcDir.m_FullFileName); continue; }

    // 已经超时
    if (ProgramActive.m_Elapsed >= ProgramActive.m_MaxTimeOut)
    {
      logfile.Write("pid=%d,cmd=%s,%d>%d\n",ProgramActive.m_PID,ProgramActive.m_ProgramName,\
                     ProgramActive.m_Elapsed,ProgramActive.m_MaxTimeOut);

      // 向程序发送退出信号
      kill(ProgramActive.m_PID,SIGINT);

      // 等待5秒，5秒后再强行杀死
      sleep(5);
    
      kill(ProgramActive.m_PID,SIGKILL); 

      // 删除进程活动信息文件。
      REMOVE(ProcDir.m_FullFileName);
    }
  }

  return 0;
}


// 在/tmp目录下创建htidc和各级子目录
void mkdirhtidcdirs()
{
  // 如果/tmp/htidc以级各子目录不存在，就创建它

  if (access("/tmp/htidc",F_OK) != 0)           mkdir("/tmp/htidc",00777);
  if (access("/tmp/htidc/proc",F_OK) != 0)      mkdir("/tmp/htidc/proc",00777);
  if (access("/tmp/htidc/log",F_OK) != 0)       mkdir("/tmp/htidc/log",00777);
  if (access("/tmp/htidc/bak",F_OK) != 0)       mkdir("/tmp/htidc/bak",00777);
  if (access("/tmp/htidc/tmp",F_OK) != 0)       mkdir("/tmp/htidc/tmp",00777);
  if (access("/tmp/htidc/list",F_OK) != 0)      mkdir("/tmp/htidc/list",00777);
  if (access("/tmp/htidc/ftpput",F_OK) != 0)    mkdir("/tmp/htidc/ftpput",00777);
  if (access("/tmp/htidc/ftpputbak",F_OK) != 0) mkdir("/tmp/htidc/ftpputbak",00777);
  if (access("/tmp/htidc/ftpget",F_OK) != 0)    mkdir("/tmp/htidc/ftpget",00777);
  if (access("/tmp/htidc/alarmxml",F_OK) != 0)  mkdir("/tmp/htidc/alarmxml",00777);

  // 一定要改为777权限，因为这些目录下写文件的程序不一定是root用户启动的
  // 如果其它用户对它没有写入权限，就会导致程序失败
  system("/bin/chmod -R 777 /tmp/htidc");

  return;
}
