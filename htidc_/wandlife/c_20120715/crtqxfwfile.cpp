#include "_public.h"
#include "_oracle.h"
#include "wandlife.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

CQXFWFile QXFWFile;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf( "Usage: /htidc/qxfw/bin/crtqxfwfile logfile username/password@tnsname outpath\n" );
    printf( "Example: /htidc/htidc/bin/procctl 300 /htidc/qxfw/bin/crtqxfwfile /log/qxfw/crtqxfwfile.log qxfw/pwdidc /qxfw\n" );

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  // CloseIOAndSignal(); // 不能调用该函数，否则ZIPFiles函数调用不能成功
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("crtqxfwfile");

  logfile.Write("crtqxfwfile beginning.\n");

  // 在连数据库之前，超时设置为180秒
  ProgramActive.SetProgramInfo(&logfile,"crtqxfwfile",180);

  // 连接数据库
  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"crtqxfwfile",1200);

  QXFWFile.BindConnLog(&conn,&logfile);

  // 加载全部的站点参数
  if (QXFWFile.LoadObtCode() == FALSE) CallQuit(-1);

  // 获取重要提示的位置
  if (QXFWFile.LoadImpPos() == FALSE) CallQuit(-1);

  while (TRUE)
  {
    // 从m_vOBTCODE容器中获取一个站点，存放m_stOBTCODE结构中
    if (QXFWFile.FetchObtCode() == FALSE) break;

    // logfile.Write("obtid=%s\n",QXFWFile.m_stOBTCODE.obtid);

    // 打开数据文件，用于写入数据
    if (QXFWFile.OpenDFile(argv[3]) == FALSE) CallQuit(-1);

    // 获取当前时间点站点的实况数据，写入数据文件
    if (QXFWFile.WriteNowData() == FALSE) CallQuit(-1);

    // 获取从今日01时到现在00时全部整点时间的数据
    if (QXFWFile.WriteTodayData() == FALSE) CallQuit(-1);

    // 获取天气预报信息
    if (QXFWFile.WriteYBData() == FALSE) CallQuit(-1);

    char strDFileName[301];
    memset(strDFileName,0,sizeof(strDFileName));
    strcpy(strDFileName,QXFWFile.m_dfile.m_fullfilename);

    // 关闭已打开的数据文件
    QXFWFile.CloseDFile();

    ZIPFiles(strDFileName,FALSE);

    // 获取该城镇的重要提示信息
    if (QXFWFile.WriteImpMessInfo(argv[3]) == FALSE) CallQuit(-1);
  }

  // 更新已获取重要提示的位置
  if (QXFWFile.UptImpPos() == FALSE) CallQuit(-1);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("crtqxfwfile exit.\n");

  exit(0);
}

