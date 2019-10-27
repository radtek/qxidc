#include "idcapp.h"

void CallQuit(int sig);

char strLogPath[201]; // 日志文件目录
char strTmpPath[201]; // 进程活动信息文件目录
char strIDCConnStr[201]; // 数据库连接参数
char strHostName[31]; 
char strAlarmValue[11];
double dusep=0.00;   // CPU的利用率

class CCPUSTAT
{
public:
  char   m_hostname[31];
  struct st_CPUSTAT
  {
    char cpu[11];
    UINT user;
    UINT system;
    UINT nice;
    char idle[16];
    UINT iowait;
    UINT irq;
    UINT softirq;
  };

  st_CPUSTAT m_stCPUSTAT;
  vector<struct st_CPUSTAT> m_vCPUSTAT;

  connection *m_conn;
  CLogFile   *m_logfile;

  CCPUSTAT();
 ~CCPUSTAT();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // 获取CPU报告
  BOOL GETCPUSTAT();

  // 把CPU报告更新到表中
  long UptCPUSTAT();

  // 分析CPU利用率是否达到m_AlarmValue，如果有，将生成告警
  UINT m_AlarmValue;

  char m_AlarmSTR[1024];
  void AlanyCPUSTAT();
};

connection     conn;
CLogFile       logfile;
CCPUSTAT       CPUSTAT;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./cpustat username/password@tnsname hostname alarmvalue\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/cpustat hssms/smspwd@EJETDB_221.179.6.136 61.235.114.95 50\n\n");
    printf("此程序用于把本服务器的CPU使用率信息报告给数据库T_CPUSTAT表，它运行在需要监控的服务器上。\n");
    printf("username/password@tnsname中指定了数据库表的数据库的连接参数。\n");
    printf("hostname是本服务器的主机名，为了方便识别，也可以用IP。\n");
    printf("alarmvalue是产生告警的阀值，当CPU使用率超过这个值时，产生告警。\n\n\n");
 
    return -1;
  }

  memset(strHostName,0,sizeof(strHostName));
  memset(strAlarmValue,0,sizeof(strAlarmValue));

  strncpy(strHostName,argv[2],20);
  strncpy(strAlarmValue,argv[3],10);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open("/tmp/htidc/log/cpustat.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/cpustat.log) failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("cpustat");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"cpustat",180);

  // 连接数据库，采用自动提交的方式，不需要事务控制。
  if (conn.connecttodb(argv[1],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[1]); return -1;
  }

  CPUSTAT.BindConnLog(&conn,&logfile);

  CPUSTAT.m_AlarmValue=atoi(strAlarmValue);

  strcpy(CPUSTAT.m_hostname,strHostName);

  // 获取CPU报告
  if (CPUSTAT.GETCPUSTAT() == FALSE)
  {
    logfile.Write("CPUSTAT.GETCPUSTAT failed.\n"); return -1;
  }

  // 把CPU报告更新到表中
  if (CPUSTAT.UptCPUSTAT() != 0)
  {
    logfile.Write("CPUSTAT.UptCPUSTAT failed.\n"); return -1;
  }

  // 分析是否有超过阀值的CPU空间利用率，如果有，将生成告警
  CPUSTAT.AlanyCPUSTAT();

  if ( strlen(CPUSTAT.m_AlarmSTR) > 0)
  {
    logfile.Write("服务器：%s，%s\n",CPUSTAT.m_hostname,CPUSTAT.m_AlarmSTR);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("cpustat exit.\n");

  exit(0);
}


CCPUSTAT::CCPUSTAT()
{
  memset(m_hostname,0,sizeof(m_hostname));
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));
  m_conn=0; m_logfile=0;
}

CCPUSTAT::~CCPUSTAT()
{
  m_vCPUSTAT.clear();
}

void CCPUSTAT::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn; m_logfile=in_logfile;
}

BOOL CCPUSTAT::GETCPUSTAT()
{
  FILE *fp=0;

  if ( (fp=FOPEN("/proc/stat","r")) == NULL )
  {
    m_logfile->Write("open /proc/stat failed.\n"); return FALSE;
  }

  m_vCPUSTAT.clear();

  char strLine[512];

  CCmdStr CmdStr;

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
    memset(&m_stCPUSTAT,0,sizeof(m_stCPUSTAT));

    if (FGETS(strLine,500,fp) == FALSE) break;

    // 删除字符串前后的空格
    Trim(strLine);

    // 把字符串中间的多个空格全部转换为一个空格
    UpdateStr(strLine,"  "," ");

    CmdStr.SplitToCmd(strLine," ");

    CmdStr.GetValue(0, m_stCPUSTAT.cpu,10);
    CmdStr.GetValue(1,&m_stCPUSTAT.user);
    CmdStr.GetValue(2,&m_stCPUSTAT.system);
    CmdStr.GetValue(3,&m_stCPUSTAT.nice);
    CmdStr.GetValue(4, m_stCPUSTAT.idle,15);
    CmdStr.GetValue(5,&m_stCPUSTAT.iowait);
    CmdStr.GetValue(6,&m_stCPUSTAT.irq);
    CmdStr.GetValue(7,&m_stCPUSTAT.softirq);

    // 把CPU的名称转换成小写
    ToLower(m_stCPUSTAT.cpu);

    if (strcmp(m_stCPUSTAT.cpu,"cpu") != 0) continue;

    m_vCPUSTAT.push_back(m_stCPUSTAT);
  }

  fclose(fp);

  return TRUE;
}

// 把CPU报告更新到表中
long CCPUSTAT::UptCPUSTAT()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("update T_CPUSTAT set user1=user2,system1=system2,nice1=nice2,idle1=idle2,iowait1=iowait2,irq1=irq2,softirq1=softirq2 where hostname=:1");
  stmt.bindin(1,m_hostname,30);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptCPUSTAT update1 T_CPUSTAT failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }

  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(m_conn,strLocalTime,"yyyymmddhh24miss");

  sqlstatement stmtins;
  stmtins.connect(m_conn);
  stmtins.prepare("insert into T_CPUSTAT(hostname,cpu,user2,system2,nice2,idle2,iowait2,irq2,softirq2,alarmvalue,ddatetime) values(:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,to_date(:11,'yyyymmddhh24miss'))");
  stmtins.bindin( 1, m_hostname,30);
  stmtins.bindin( 2, m_stCPUSTAT.cpu,10);
  stmtins.bindin( 3,&m_stCPUSTAT.user);
  stmtins.bindin( 4,&m_stCPUSTAT.system);
  stmtins.bindin( 5,&m_stCPUSTAT.nice);
  stmtins.bindin( 6, m_stCPUSTAT.idle,15);
  stmtins.bindin( 7,&m_stCPUSTAT.iowait);
  stmtins.bindin( 8,&m_stCPUSTAT.irq);
  stmtins.bindin( 9,&m_stCPUSTAT.softirq);
  stmtins.bindin(10,&m_AlarmValue);
  stmtins.bindin(11, strLocalTime,14);

  sqlstatement stmtupt;
  stmtupt.connect(m_conn);
  stmtupt.prepare("update T_CPUSTAT set user2=:1,system2=:2,nice2=:3,idle2=:4,iowait2=:5,irq2=:6,softirq2=:7,alarmvalue=:8,ddatetime=to_date(:9,'yyyymmddhh24miss'),usep=:10 where hostname=:11 and cpu=:12");
  stmtupt.bindin( 1,&m_stCPUSTAT.user);
  stmtupt.bindin( 2,&m_stCPUSTAT.system);
  stmtupt.bindin( 3,&m_stCPUSTAT.nice);
  stmtupt.bindin( 4, m_stCPUSTAT.idle,15);
  stmtupt.bindin( 5,&m_stCPUSTAT.iowait);
  stmtupt.bindin( 6,&m_stCPUSTAT.irq);
  stmtupt.bindin( 7,&m_stCPUSTAT.softirq);
  stmtupt.bindin( 8,&m_AlarmValue);
  stmtupt.bindin( 9, strLocalTime,14);
  stmtupt.bindin(10,&dusep);
  stmtupt.bindin(11, m_hostname,30);
  stmtupt.bindin(12, m_stCPUSTAT.cpu,10);

  for (UINT ii=0;ii<m_vCPUSTAT.size();ii++)
  {
    memcpy(&m_stCPUSTAT,&m_vCPUSTAT[ii],sizeof(m_stCPUSTAT));

    if (stmtins.execute() != 0)
    {
      if (stmtins.cda.rc == 1)
      {
        if (stmtupt.execute() != 0)
        {
          m_logfile->Write("UptCPUSTAT update2 T_CPUSTAT failed.\n%s\n",stmtupt.cda.message);
          return stmtupt.cda.rc;
        }
      }
      else
      {
        m_logfile->Write("UptCPUSTAT insert T_CPUSTAT failed.\n%s\n",stmtins.cda.message);
        return stmtins.cda.rc;
      }
    }
  }

  return 0;
}

// 分析CPU利用率是否达到m_AlarmValue，如果有，将生成告警
void CCPUSTAT::AlanyCPUSTAT()
{
  memset(m_AlarmSTR,0,sizeof(m_AlarmSTR));


  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select 100.00-sum(idle2-idle1)*100/sum(user2+system2+nice2+idle2+iowait2+irq2+softirq2-user1-system1-nice1-idle1-iowait1-irq1-softirq1) from T_CPUSTAT where hostname=:1 and cpu='cpu'");
  stmt.bindin(1, m_hostname,30);
  stmt.bindout(1,&dusep);

  stmt.execute();

  stmt.next();

  if ( dusep >= m_AlarmValue )
  {
    snprintf(m_AlarmSTR,1024,"failed.CPU使用率%.2f,达到了阀值%lu。",dusep,m_AlarmValue);
  }
  else
  {
    m_logfile->Write("CPU使用率%.2f,阀值%lu。\n",dusep,m_AlarmValue);
  }

  // 更新CPU的使用率字段
  stmt.prepare("update T_CPUSTAT set usep=:1 where hostname=:2 and cpu='cpu'");
  stmt.bindin(1,&dusep);
  stmt.bindin(2, m_hostname,30);
  stmt.execute();
}

