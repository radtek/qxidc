#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;

CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 6)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/backupapp hostname pathandfiles backuppath timeout starttime\n");  
 
    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/backupapp 59.33.252.244 \"/etc/rc.d/rc.local /etc/profile.d/sysenv.sh $ORACLE_HOME/network/admin /www/Tomcat/webapps /home/oracle/esanew\" /tmp/htidc/bak 10 02\n\n");

    printf("此程序用于备份某服务器上全部的应用程序、启动脚本、环境配置脚本。\n");
    printf("hostname指需要备份的服务器的名称，一般用系统名或IP，备份结果的文件为hostname_yyyymmdd.tgz。\n");
    printf("pathandfiles指需要备份的文件和目录的列表，需要包括以下几类：\n");
    printf("/etc/rc.d/rc.local /etc/profile.d/sysenv.sh oracle的tns配置文件 tomcat的webapps目录 其它应用程序。\n");
    printf("backuppath指备份后的*.tgz文件存放的目录。\n");
    printf("timeout指执行备份指令的超时时间，单位：分钟。\n");
    printf("starttime指程序每天启动的时间，只支持一个时间，取值从00,01,02,03,04......22,23。\n");
    printf("注意，本程序一定要3600秒启动一次，调用操作系统的tar命令，程序的日志文件为/tmp/htidc/log/backupapp.log。\n\n\n");
 
    return -1;
  }

  char strHostName[201];      // 
  char strPathAndFiles[1024];     // 临时存放用户名
  char strBackupPath[301];    // 备份后的dmp文件和log文件存放的目录
  char strLocalTime[21];      // 系统时间
  char strTgzFileName[201];      // 存放备份文件的文件名
  char strCmdTar[2048];          // tar压缩语句
  int  iTimeOut=0;            // 超时时间秒

  memset(strHostName,0,sizeof(strHostName));
  memset(strPathAndFiles,0,sizeof(strPathAndFiles));
  memset(strBackupPath,0,sizeof(strBackupPath));
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strTgzFileName,0,sizeof(strTgzFileName));
  memset(strCmdTar,0,sizeof(strCmdTar));

  strcpy(strHostName,argv[1]);
  strcpy(strPathAndFiles,argv[2]);
  strcpy(strBackupPath,argv[3]);

  // 把超时的时间单位转为秒
  iTimeOut=atoi(argv[4])*60;

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);  

  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  if (strncmp(strLocalTime+8,argv[5],2) != 0) return 0;

  logfile.Open("/tmp/htidc/log/backupapp.log","a+");

  logfile.SetAlarmOpt("backupapp");

  // 获取系统时间
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd");

  // 拼凑tar执行语句
  snprintf(strCmdTar,2000,"tar zcvf %s/%s_%s.tgz %s 1>/dev/null 2>/dev/null",strBackupPath,strHostName,strLocalTime,strPathAndFiles);

  // 注意，程序超时是iTimeOut秒
  ProgramActive.SetProgramInfo(&logfile,"backupapp",iTimeOut);

  // 开始备份
  logfile.Write("%s ...",strCmdTar);

  // 执行备份tgz语句
  system(strCmdTar);

  // 完成导出压缩
  logfile.WriteEx("ok\n"); 

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
 
  logfile.Write("backupapp exit.\n");
 
  exit(0);
}
