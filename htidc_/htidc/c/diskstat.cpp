#include "idcapp.h"

void CallQuit(int sig);

char strLogPath[201]; // 日志文件目录
char strTmpPath[201]; // 进程活动信息文件目录
char strALMConnStr[201]; // 日志数据库连接参数
char strHostName[31]; 
char strAlarmValue[11];
char strExpPart[201];

class CDISKSTAT
{
public:
  char   m_hostname[31];
  struct st_DISKSTAT
  {
    char filesystem[101];
    char total[31];
    char used[31];
    char available[31];
    char usep[31];
    char mount[31];
  };

  st_DISKSTAT m_stDISKSTAT;
  vector<struct st_DISKSTAT> m_vDISKSTAT;

  connection *m_conn;
  CLogFile   *m_logfile;

  CDISKSTAT();
 ~CDISKSTAT();


  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // 获取磁盘报告
  BOOL GETDISKSTAT(char *in_ExpPart);

  // 把磁盘报告更新到表中
  long UptDISKSTAT();

  // 分析磁盘空间利用率是否达到m_AlarmValue，如果有，将生成告警
  UINT m_AlarmValue;
  char m_AlarmSTR[1024];
  void AlanyDiskSTAT();
};


connection     connalarm;
CLogFile       logfile;
CDISKSTAT      DISKSTAT;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if ( (argc != 4) && (argc != 5) )
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/diststat inifile hostname alarmvalue [exppart]\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/diskstat hssms/smspwd@EJETDB_221.179.6.136 61.235.114.95 80\n");


    printf("此程序用于把本服务器的磁盘使用率信息报告给数据库T_DISKSTAT表，它运行在需要监控的服务器上。\n");
    printf("username/password@tnsname中指定了数据库表的数据库的连接参数。\n");
    printf("hostname是本服务器的主机名，为了方便识别，也可以用IP。\n");
    printf("alarmvalue是产生告警的阀值，当磁盘使用率超过这个值时，产生告警。\n");
    printf("exppart是可选参数，指定不需要告警的例外分区，用逗号分隔，监控程序对该分区完全不理会。\n\n\n");

    return -1;
  }

  memset(strHostName,0,sizeof(strHostName));
  memset(strAlarmValue,0,sizeof(strAlarmValue));
  memset(strExpPart,0,sizeof(strExpPart));

  strncpy(strHostName,argv[2],30);
  strncpy(strAlarmValue,argv[3],10);

  if (argc == 5) strncpy(strExpPart,argv[4],200);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open("/tmp/htidc/log/diskstat.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/diskstat.log) failed.\n"); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("diskstat");

  ProgramActive.SetProgramInfo(&logfile,"diskstat",500);

  // 连接日志数据库
  if (connalarm.connecttodb(argv[1],TRUE) != 0)
  {
    logfile.Write("connalarm.connecttodb(%s) failed\n",argv[1]); return -1;
  }

  // 注意，程序超时之前是500秒，现在改为180秒，因为上面连数据库失败时可能要很长时间，如果把时间设为180秒，
  // 可能导致告警无法捕获。
  ProgramActive.SetProgramInfo(&logfile,"diskstat",180);

  DISKSTAT.BindConnLog(&connalarm,&logfile);

  DISKSTAT.m_AlarmValue=atoi(strAlarmValue);

  strcpy(DISKSTAT.m_hostname,strHostName);

  // 获取磁盘报告
  if (DISKSTAT.GETDISKSTAT(strExpPart) == FALSE)
  {
    logfile.Write("DISKSTAT.GETDISKSTAT failed.\n"); return -1;
  }

  // 把磁盘报告更新到表中
  if (DISKSTAT.UptDISKSTAT() != 0)
  {
    logfile.Write("DISKSTAT.UptDISKSTAT failed.\n"); return -1;
  }

  // 分析是否有超过阀值的磁盘空间利用率，如果有，将生成告警
  DISKSTAT.AlanyDiskSTAT();

  if ( strlen(DISKSTAT.m_AlarmSTR) > 0)
  {
    logfile.Write("%s，%s\n",DISKSTAT.m_hostname,DISKSTAT.m_AlarmSTR);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("diskstat exit.\n");

  exit(0);
}


CDISKSTAT::CDISKSTAT()
{
  m_conn=0; m_logfile=0;
  memset(m_hostname,0,sizeof(m_hostname));
  m_AlarmValue=0;
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));
}

CDISKSTAT::~CDISKSTAT()
{
  m_vDISKSTAT.clear();
}

void CDISKSTAT::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn = in_conn; m_logfile = in_logfile;
}

// 获取磁盘报告
BOOL CDISKSTAT::GETDISKSTAT(char *in_ExpPart)
{
  ToUpper(in_ExpPart);

  FILE *fp=0;

  if ( (fp=popen("df -k --block-size=1M","r")) == NULL )
  {
    m_logfile->Write("exec /bin/df -k failed.\n"); return FALSE;
  }

  m_vDISKSTAT.clear();

  CCmdStr CmdStr;

  char strLine[512],strLine1[512];

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    memset(&m_stDISKSTAT,0,sizeof(m_stDISKSTAT));

    if (FGETS(strLine,500,fp) == FALSE) break;

    // 如果没有找到“%”，就再读取一行
    if (strstr(strLine,"%") == 0)
    {
      memset(strLine1,0,sizeof(strLine1));
      if (FGETS(strLine1,500,fp) == FALSE) break;
      strcat(strLine," ");
      strcat(strLine,strLine1);
    }

    // 如果找不到“/”，就越过该行，它很可能是第一行的标题
    if (strstr(strLine,"/dev") == 0) continue;

    // 在有些服务器上,内存也是作为文件系统mount上去的,以下两行代码删除内存文件系统
    if (strstr(strLine,"none")     != 0) continue;
    if (strstr(strLine,"/boot")    != 0) continue;
    if (strstr(strLine,"/dev/shm") != 0) continue;

    // 删除字符串前后的空格
    Trim(strLine);
    // 把字符串中间的多个空格全部转换为一个空格
    UpdateStr(strLine,"  "," ");


    CmdStr.SplitToCmd(strLine," ");

    CmdStr.GetValue(0,m_stDISKSTAT.filesystem,100);
    CmdStr.GetValue(1,m_stDISKSTAT.total,30);
    CmdStr.GetValue(2,m_stDISKSTAT.used,30);
    CmdStr.GetValue(3,m_stDISKSTAT.available,30);
    CmdStr.GetValue(4,m_stDISKSTAT.usep,30);
    CmdStr.GetValue(5,m_stDISKSTAT.mount,30);

    if (MatchFileName(m_stDISKSTAT.mount,in_ExpPart)==TRUE) continue;

    snprintf(m_stDISKSTAT.usep,30,"%d",atoi(m_stDISKSTAT.usep));

    if (strncmp(m_stDISKSTAT.filesystem,"/dev",4) != 0) continue;

    m_vDISKSTAT.push_back(m_stDISKSTAT);
  }

  fclose(fp);

  return TRUE;
}

// 把磁盘报告更新到表中
long CDISKSTAT::UptDISKSTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("delete T_DISKSTAT where hostname=:1");
  stmt.bindin(1,m_hostname,30);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptDISKSTAT delete T_DISKSTAT failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(m_conn,strLocalTime,"yyyymmddhh24miss");

  stmt.prepare("insert into T_DISKSTAT(hostname,filesystem,total,used,available,usep,mount,alarmvalue,ddatetime) values(:1,:2,:3,:4,:5,:6,:7,:8,to_date(:9,'yyyymmddhh24miss'))");
  stmt.bindin(1, m_hostname,30);
  stmt.bindin(2, m_stDISKSTAT.filesystem,100);
  stmt.bindin(3, m_stDISKSTAT.total,30);
  stmt.bindin(4, m_stDISKSTAT.used,30);
  stmt.bindin(5, m_stDISKSTAT.available,30);
  stmt.bindin(6, m_stDISKSTAT.usep,30);
  stmt.bindin(7, m_stDISKSTAT.mount,30);
  stmt.bindin(8,&m_AlarmValue);
  stmt.bindin(9, strLocalTime,14);


  for (UINT ii=0;ii<m_vDISKSTAT.size();ii++)
  {
    memcpy(&m_stDISKSTAT,&m_vDISKSTAT[ii],sizeof(m_stDISKSTAT));

    if (stmt.execute() != 0)
    {
      m_logfile->Write("UptDISKSTAT insert T_DISKSTAT failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
    }
  }

  return 0;
}

// 分析磁盘空间利用率是否达到m_AlarmValue，如果有，将生成告警
void CDISKSTAT::AlanyDiskSTAT()
{
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));

  char strTemp[256];

  for (UINT ii=0;ii<m_vDISKSTAT.size();ii++)
  {
    m_logfile->Write("文件系统%s，共%sM，已使用%sM，约占%s%%。\n",m_vDISKSTAT[ii].mount,\
                      m_vDISKSTAT[ii].total,m_vDISKSTAT[ii].used,m_vDISKSTAT[ii].usep);


    if ( (UINT)atoi(m_vDISKSTAT[ii].usep) >= m_AlarmValue)
    {
      memset(strTemp,0,sizeof(strTemp));
      snprintf(strTemp,200,"failed.文件系统%s，共%sM，已使用%sM，约占%s%%；",m_vDISKSTAT[ii].mount,\
              m_vDISKSTAT[ii].total,m_vDISKSTAT[ii].used,m_vDISKSTAT[ii].usep);
      strcat(m_AlarmSTR,strTemp);
    }
  }

  // 把最后的全角逗号替换成全角的句号
  if (strlen(m_AlarmSTR) > 0) strcpy(m_AlarmSTR+strlen(m_AlarmSTR)-2,"。");
}

