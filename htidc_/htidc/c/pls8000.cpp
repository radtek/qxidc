#include "_public.h"
#include "_oracle.h"
#include "_pgdb.h"

void CallQuit(int sig);

pgconnection   pgconnstr;
connection     oracleconnstr;
CLogFile       logfile;
CProgramActive ProgramActive;

char strXmlBuffer[4001];
char strpgconnstr[101];
char stroracleconnstr[101];
long totalcount=0;

struct st_LS8000
{
  char ddatetime[31];
  char nano[31];
  char latitude[31];
  char longitude[31];
  char multi[31];
  char signal[31];
  char flags[31];
  char bitflags[31];
  char chi_square[31];
  char ell_semimajor_axis[31];
  char ell_semiminor_axis[31];
  char ell_angle[31];
  char freedom[31];
  char num_dfrs[31];
  char secidx[31];
  char risetime[31];
  char maxraterise[31];
  char extended[31];
  char flash[31];
  char cgflash[31];
};

struct st_LS8000 stLS8000;
vector<struct st_LS8000> vLS8000;

// 从postgresql数据库源表采集数据,放到vLS8000。
BOOL GetData();

// 读取vLS8000，插入oracle数据库的t_ls8000表
BOOL InsData();

char strincfilename[201];
char incfieldvalue_old[51];
char incfieldvalue_new[51];

// 读取starttime的值存放的文件，结果保存在incfieldvalue_old变量中。
BOOL ReadIncFile();

// 把最新的starttime字段的值写入文件中
BOOL WriteIncFile();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/pls8000 logfilename xmlbuffer\n\n");
    printf("Sample:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/pls8000 /tmp/htidc/log/pls8000_.log \"<oracleconnstr>szidc/pwdidc@SZQX_10.153.98.31</oracleconnstr><pgconnstr>host=10.153.96.176 user=distadmin password=1996apa dbname=falls port=5432</pgconnstr><incfilename>/htidc/htidc/c/ls8000starttimefiles.txt</incfilename>\"\n");

    printf("本程序用于处理闪电定位数据，从postgresql数据库源表采集数据，然后插入oracle数据库的t_ls8000表。\n");
    printf("logfilename是本程序运行的日志文件。\n");
    printf("xmlbuffer的参数如下：\n");

    printf("oracle数据库的连接参数 <oracleconnstr>sqxt/pwdidc@SZQX_10.148.124.85</oracleconnstr>\n");
    printf("postgresql数据库的连接参数 <pgconnstr>host=10.151.64.158 user=staa password=123456abcdef dbname=meteo port=5432</pgconnstr>\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("pls8000");

  memset(strpgconnstr,0,sizeof(strpgconnstr));
  memset(stroracleconnstr,0,sizeof(stroracleconnstr));  
  memset(strincfilename,0,sizeof(strincfilename));

  GetXMLBuffer(strXmlBuffer,"oracleconnstr",stroracleconnstr,100);
  GetXMLBuffer(strXmlBuffer,"pgconnstr",strpgconnstr,100);
  GetXMLBuffer(strXmlBuffer,"incfilename",strincfilename,200);

  if (strlen(stroracleconnstr) == 0) { logfile.Write("oracleconnstr is null.\n"); return -1; }
  if (strlen(strpgconnstr) == 0)     { logfile.Write("pgconnstr is null.\n"); return -1;     }
  if (strlen(strincfilename) == 0)   { logfile.Write("incfilename is null.\n"); return -1;   }


  logfile.Write("pls8000 beginning.\n");

  // 注意，程序超时是3600秒
  ProgramActive.SetProgramInfo(&logfile,"pls8000",3600);

  // 连接postgresql数据源数据库
  if (pgconnstr.connecttodb(strpgconnstr) != 0)
  {
    logfile.Write("pgconnstr.connecttodb(%s) failed\n",strpgconnstr); CallQuit(-1);
  }

  // 连接oracle数据源数据库
  if (oracleconnstr.connecttodb(stroracleconnstr,TRUE) != 0)
  {
    logfile.Write("oracleconnstr.connecttodb(%s) failed\n",stroracleconnstr); CallQuit(-1);
  }

  // 写入进程活动信息
  ProgramActive.WriteToFile();

  while(TRUE)
  {
    if (ReadIncFile() == FALSE) { logfile.Write("ReadIncFile failed.\n"); }

    vLS8000.clear();

    if (GetData() == FALSE) { logfile.Write("GetData() failed.\n"); CallQuit(-1); }
  
    if (InsData() == FALSE) { logfile.Write("InsData() failed.\n"); CallQuit(-1); }

    if ( totalcount != 0 )  { logfile.Write("insert T_LS8000 %ld rows.\n",totalcount); totalcount=0; }

    logfile.Write("incfieldvalue_new=%s \n",incfieldvalue_new);

    if (WriteIncFile() == FALSE) { logfile.Write("WriteIncFile failed.\n"); }
    
    sleep(1);
  }
  
  return 0;
}

// 从postgresql数据库源表采集数据,放到vLS8000。
BOOL GetData()
{
  // 准备获取数据的SQL语句，绑定输出变量
  pgsqlstatement pgstmt;
  pgstmt.connect(&pgconnstr);
  pgstmt.prepare("select to_char(time+ interval '8 hours','yyyy-mm-dd hh24:mi:ss'),nano,latitude,longitude,round(signal*0.185),\
                 flags,flags::bit(9),chi_square,ell_semimajor_axis,ell_semiminor_axis,ell_angle,freedom,multi,num_dfrs,secidx,\
                 \"RiseTime\",\"MaxRateRise\",\"Extended\",(case when flash = 'f' then 0 else 1 end) as flash \
                 from realtime.dffsyb where time>=to_date('%s','YYYY-MM-DD HH24:MI:SS') ORDER BY \"time\"",incfieldvalue_old);
  pgstmt.bindout(1,stLS8000.ddatetime,30);
  pgstmt.bindout(2,stLS8000.nano,30);
  pgstmt.bindout(3,stLS8000.latitude,30);
  pgstmt.bindout(4,stLS8000.longitude,30);
  pgstmt.bindout(5,stLS8000.signal,30);
  pgstmt.bindout(6,stLS8000.flags,30);
  pgstmt.bindout(7,stLS8000.bitflags,30);
  pgstmt.bindout(8,stLS8000.chi_square,30);
  pgstmt.bindout(9,stLS8000.ell_semimajor_axis,30);
  pgstmt.bindout(10,stLS8000.ell_semiminor_axis,30);               
  pgstmt.bindout(11,stLS8000.ell_angle,30);
  pgstmt.bindout(12,stLS8000.freedom,30);
  pgstmt.bindout(13,stLS8000.multi,30);
  pgstmt.bindout(14,stLS8000.num_dfrs,30);
  pgstmt.bindout(15,stLS8000.secidx,30);
  pgstmt.bindout(16,stLS8000.risetime,30);
  pgstmt.bindout(17,stLS8000.maxraterise,30);
  pgstmt.bindout(18,stLS8000.extended,30);
  pgstmt.bindout(19,stLS8000.flash,30);
  
  // 执行采集数据的SQL语句
  if (pgstmt.execute() != 0)
  {
    logfile.Write("exec pgsql failed.\n%s\n",pgstmt.cda.message); return FALSE;
  }

  logfile.Write("exec sql ok.\n");

  // 处理SQL语句执行后的每一行
  while (TRUE)
  {
    memset(&stLS8000,0,sizeof(struct st_LS8000));
	
    if (pgstmt.next() != 0) break;

    // 将flags转换成二进制数据，从低位向高位数，第7-9位如果都是0，输出0，否则输出1。
    if ( strncmp(stLS8000.bitflags,"000",3) == 0) 
      strcpy(stLS8000.cgflash,"0");
    else 
      strcpy(stLS8000.cgflash,"1");
	
    vLS8000.push_back(stLS8000);
 
    memset(incfieldvalue_new,0,sizeof(incfieldvalue_new));
    strcpy(incfieldvalue_new,stLS8000.ddatetime);
    AddTime(incfieldvalue_new,incfieldvalue_new,0-8*60*60,"yyyy-mm-dd hh24:mi:ss");

  }
  
  return TRUE;
}

BOOL InsData()
{
  if (vLS8000.size() == 0) return TRUE;

  sqlstatement stmtins;
  stmtins.connect(&oracleconnstr);
  stmtins.prepare("insert into T_LS8000(ddatetime,nano,latitude,longitude,multi,signal,flags,chi_square,ell_semimajor_axis,ell_semiminor_axis,ell_angle,freedom,num_dfrs,secidx,risetime,maxraterise,extended,flash,cgflash,keyid,crttime) values(to_date(:1,'yyyy-mm-dd hh24:mi:ss'),:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:15,:16,:17,:18,:19,SEQ_LS8000.nextval,sysdate)");

  stmtins.bindin(1,stLS8000.ddatetime,30);
  stmtins.bindin(2,stLS8000.nano,30);
  stmtins.bindin(3,stLS8000.latitude,30);
  stmtins.bindin(4,stLS8000.longitude,30);
  stmtins.bindin(5,stLS8000.multi,30);
  stmtins.bindin(6,stLS8000.signal,30);
  stmtins.bindin(7,stLS8000.flags,30);
  stmtins.bindin(8,stLS8000.chi_square,30);
  stmtins.bindin(9,stLS8000.ell_semimajor_axis,30);
  stmtins.bindin(10,stLS8000.ell_semiminor_axis,30);
  stmtins.bindin(11,stLS8000.ell_angle,30);
  stmtins.bindin(12,stLS8000.freedom,30);
  stmtins.bindin(13,stLS8000.num_dfrs,30);
  stmtins.bindin(14,stLS8000.secidx,30);
  stmtins.bindin(15,stLS8000.risetime,30);
  stmtins.bindin(16,stLS8000.maxraterise,30);
  stmtins.bindin(17,stLS8000.extended,30);
  stmtins.bindin(18,stLS8000.flash,30);
  stmtins.bindin(19,stLS8000.cgflash,30);

  for (UINT ii=0;ii<vLS8000.size();ii++)
  {
    memset(&stLS8000,0,sizeof(struct st_LS8000));
    memcpy(&stLS8000,&vLS8000[ii],sizeof(struct st_LS8000));

    if (stmtins.execute() != 0)
    {
      if (stmtins.cda.rc != 1)
      {
        logfile.Write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",stLS8000.ddatetime,stLS8000.nano,stLS8000.latitude,stLS8000.longitude,stLS8000.multi,stLS8000.signal,stLS8000.flags,stLS8000.chi_square,stLS8000.ell_semimajor_axis,stLS8000.ell_semiminor_axis,stLS8000.ell_angle,stLS8000.freedom,stLS8000.num_dfrs,stLS8000.secidx,stLS8000.risetime,stLS8000.maxraterise,stLS8000.extended,stLS8000.flash,stLS8000.cgflash);

        logfile.Write("insert T_LS8000 failed.%s\n",stmtins.cda.message); continue;
      }
    }
    else

      totalcount++;
  }

  oracleconnstr.commitwork(); 

  return TRUE;

}

 void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("pls8000 exit.\n");

  exit(0);
}

BOOL ReadIncFile()
{
  memset(incfieldvalue_old,0,sizeof(incfieldvalue_old));

  CFile File;

  if (File.OpenForRead(strincfilename,"r") == FALSE) return FALSE;

  File.FFGETS(incfieldvalue_old,50);

  Trim(incfieldvalue_old);

  // 如果是空值，那就取系统时间-12小时。
  if (strlen(incfieldvalue_old) == 0)
  {
    memset(incfieldvalue_old,0,sizeof(incfieldvalue_old));
    LocalTime(incfieldvalue_old,"yyyy-mm-dd hh24:mi:ss",0-12*60*60);  
  }

  logfile.Write("incfieldvalue_old=%s \n",incfieldvalue_old);

  return TRUE;
}

BOOL WriteIncFile()
{
  CFile File;

  if (File.OpenForWrite(strincfilename,"w+") == FALSE)
  {
    logfile.Write("File.OpenForWrite(%s) failed.\n",strincfilename); return FALSE;
  }

  Trim(incfieldvalue_new);

  File.Fprintf("%s",incfieldvalue_new);

  File.Fclose();

  return TRUE;
}

