#include "idcapp.h"

void CallQuit(int sig);

char strLogPath[201]; // 日志文件目录
char strTmpPath[201]; // 进程活动信息文件目录
char strIniFile[201];
char strIDCConnStr[201]; // 数据库连接参数

CALARMLOG ALARMLOG;

CDir Dir;
connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if ( (argc != 2) && (argc != 3) )
  {
    printf("\n");
    printf("Using:./alarmtotab username/password@tnsname [NLS_LANG]\n\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/alarmtotab hssms/smspwd@EJETDB_192.168.1.10\n\n");
    printf("此程序用于把存放在/tmp/htidc/alarmxml目录中后台服务的告警日志文件更新到T_ALARMLOG表中。\n\n");
 
    return -1;
  }

  memset(strIniFile,0,sizeof(strIniFile));

  if (argc == 3)  setenv("NLS_LANG",argv[2],1);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open("/tmp/htidc/log/alarmtotab.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/alarmtotab.log) failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("alarmtotab");

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&logfile,"alarmtotab",300);

  // 连接数据库，采用自动提交的方式，不需要事务控制。
  if (conn.connecttodb(argv[1],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[1]); return -1;
  }

  ALARMLOG.BindConnLog(&conn,&logfile);

  // 打开标准格式文件目录
  if (Dir.OpenDir("/tmp/htidc/alarmxml",TRUE) == FALSE)
  {
    logfile.Write("Dir.OpenDir /tmp/htidc/alarmxml failed.\n"); CallQuit(-1);
  }

  while (TRUE)
  {
    if (Dir.ReadDir() == FALSE) break;

    // 如果文件名不是以.XML结束，就不处理该文件
    if (MatchFileName(Dir.m_FileName,"*.XML") == FALSE)
    {
      REMOVE(Dir.m_FullFileName); continue;
    }

    logfile.Write("%s\n",Dir.m_FileName);

    ALARMLOG.ReadXMLFile(Dir.m_FullFileName);

    // 以下日志不能再用logfile.Write去写日，否则会造成failed内容循环
    fprintf(logfile.m_tracefp,"leasttime=%s\n",ALARMLOG.m_stALARMLOG.leasttime);
    fprintf(logfile.m_tracefp,"progname=%s\n",ALARMLOG.m_stALARMLOG.progname);
    fprintf(logfile.m_tracefp,"alarmtext=%s\n\n",ALARMLOG.m_stALARMLOG.alarmtext);
    fflush(logfile.m_tracefp);

    // 不管日志处理是否成功，都不判断UptAlarmLog的结果，否则会造成本程序的循环告警。
    ALARMLOG.UptAlarmLog();

    REMOVE(Dir.m_FullFileName);

    conn.commitwork();
  }

  // 删除表中三天之前的记录
  ALARMLOG.DelAlarmLog();

  conn.commitwork();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("alarmtotab exit.\n");

  exit(0);
}


