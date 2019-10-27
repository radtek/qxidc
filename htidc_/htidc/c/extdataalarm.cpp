#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void EXIT(int sig);

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strtname[51];
char strddtfieldname[51];
char strobtfieldname[51];
char strdatfieldname[51];
char strminvalue[51];
char strmaxvalue[51];

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/extdataalarm xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 600 /htidc/htidc/bin/extdataalarm \"<logfilename>/log/gzqx/extdataalarm_LCOBTDATA_t.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>pwdidc/pwdidc@GZQX_10.153.130.61</connstr><tname>T_LCOBTDATA</tname><ddtfieldname>ddatetime</ddtfieldname><obtfieldname>obtid</obtfieldname><datfieldname>t</datfieldname><minvalue>0</minvalue><maxvalue>400</maxvalue>\"\n\n");

    printf("这是一个工具程序，用于发现观测数据中的异常数据，然后插入T_EXTDATAALARM表，同时把异常数据告警日志写入告警日志文件。利用数据中心的告警邮件通知功能发出告警邮件\n");
    printf("<logfilename>/log/sqxj/extdataalarm_LCOBTDATA_t.log</logfilename> 本程序运行产生的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> 目的数据库的连接参数。\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> 数据库的字符集，必须要与<connstr>参数描述的数据库相同。\n");
    printf("<tname>T_LCOBTDATA</tname> 存放观测数据的表名。\n");
    printf("<ddtfieldname>ddatetime</ddtfieldname> 数据时间字段名，一般是ddatetime。\n");
    printf("<obtfieldname>obtid</obtfieldname> 观测站点字段名，一般是obtid。\n");
    printf("<datfieldname>t</datfieldname> 观测数据字段名，这个就不一定要，要看数据结构。\n");
    printf("<minvalue>0</minvalue><maxvalue>400</maxvalue> 异常数据阀值，如果值小于minvalue或大于maxvalue，都将定义为异常数据。\n");

    printf("注意：本程序每10分钟启动一次，由procctl调度，在每小时的20-30分之间扫描上一小时01到00分的数据，生成的告警内容插入T_ALARMLOG和T_EXTDATAALARM表中。\n");

    return -1;
  }

  char strLocalTime[31]; 
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"mi");  
  if (atoi(strLocalTime) < 20 || atoi(strLocalTime) >30) return -1;

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strtname,0,sizeof(strtname));
  memset(strddtfieldname,0,sizeof(strddtfieldname));
  memset(strobtfieldname,0,sizeof(strobtfieldname));
  memset(strdatfieldname,0,sizeof(strdatfieldname));
  memset(strminvalue,0,sizeof(strminvalue));
  memset(strmaxvalue,0,sizeof(strmaxvalue));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"tname",strtname,50);
  GetXMLBuffer(argv[1],"ddtfieldname",strddtfieldname,50);
  GetXMLBuffer(argv[1],"obtfieldname",strobtfieldname,50);
  GetXMLBuffer(argv[1],"datfieldname",strdatfieldname,50);
  GetXMLBuffer(argv[1],"minvalue",strminvalue,50);
  GetXMLBuffer(argv[1],"maxvalue",strmaxvalue,50);

  if (strlen(strlogfilename) == 0)      { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)          { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)          { printf("charset is null.\n"); return -1; }
  if (strlen(strtname) == 0)            { printf("tname is null.\n"); return -1; }
  if (strlen(strddtfieldname) == 0)     { printf("ddtfieldname is null.\n"); return -1; }
  if (strlen(strobtfieldname) == 0)     { printf("obtfieldname is null.\n"); return -1; }
  if (strlen(strdatfieldname) == 0)     { printf("datfieldname is null.\n"); return -1; }
  if ( (strlen(strminvalue) == 0) && (strlen(strmaxvalue) == 0) ) { printf("minvalue and maxvalue is null.\n"); return -1; }

  // 规范表名和字段名的大小写
  ToUpper(strtname); ToLower(strddtfieldname); ToLower(strobtfieldname); ToLower(strdatfieldname);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
 
  // 打开日志文件
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  logfile.SetAlarmOpt("extdataalarm");

  // 注意，程序超时是120秒   xxxxxxxxxxxx，暂时用1200秒，方便调试
  ProgramActive.SetProgramInfo(&logfile,"extdataalarm",1200);

  // 连接数据库
  if (conn.connecttodb(strconnstr,TRUE) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  char obtid[31];
  char ddatetime[31];
  char value[31];
  char strddatetime01[21];   // 上一时次01分的时间，即上一时次第一条记录。
  char strddatetime00[21];   // 本小时00分的时间，即上一时次最后一条记录。

  memset(strddatetime01,0,sizeof(strddatetime01));
  memset(strddatetime00,0,sizeof(strddatetime00));

  LocalTime(strddatetime01,"yyyy-mm-dd hh24",0-60*60);
  strcat(strddatetime01,":01:00");

  LocalTime(strddatetime00,"yyyy-mm-dd hh24");
  strcat(strddatetime00,":00:00");

  // 如果有最大阀值参数
  if (strlen(strmaxvalue)!=0)
  {
    sqlstatement stmt;
    stmt.connect(&conn);

    //stmt.prepare("select %s,min(%s),max(%s) from %s where ddatetime>to_date('%s','yyyy-mm-dd hh24:mi:ss') and ddatetime<=to_date('%s','yyyy-mm-dd hh24:mi:ss') and %s>%s group by %s",strobtfieldname,strdatfieldname,strdatfieldname,strtname,strddatetime01,strddatetime00,strdatfieldname,strmaxvalue,strobtfieldname);

    stmt.prepare("select %s,to_char(%s,'yyyy-mm-dd hh24:mi:ss'),%s from %s where %s>to_date('%s','yyyy-mm-dd hh24:mi:ss') and %s<=to_date('%s','yyyy-mm-dd hh24:mi:ss') and %s>%s order by %s",strobtfieldname,strddtfieldname,strdatfieldname,strtname,strddtfieldname,strddatetime01,strddtfieldname,strddatetime00,strdatfieldname,strmaxvalue,strobtfieldname);

    stmt.bindout(1,obtid,30);
    stmt.bindout(2,ddatetime,30);
    stmt.bindout(3,value,30);

    sqlstatement ins;
    ins.connect(&conn);

    if (stmt.execute() != 0)
    {
      logfile.Write("select %s failed.%s\n",stmt.cda.message); EXIT(-1);
    }

    while(TRUE)
    { 
      memset(obtid,0,sizeof(obtid));
      memset(ddatetime,0,sizeof(ddatetime));
      memset(value,0,sizeof(value));

      if (stmt.next() != 0) break;
      LocalTime(logfile.m_stime);

      if (strlen(obtid) != 0) 
      {
        memset(logfile.m_message,0,sizeof(logfile.m_message));

        snprintf(logfile.m_message,300,"异常数据告警，表名：%s，站点：%s，时间：%s，字段：%s，值为：%s。",strtname,obtid,ddatetime,strdatfieldname,value);

	// 插入异常数据告警日志表T_EXTDATAALARM
	ins.prepare("insert into T_EXTDATAALARM(logid,tname,obtid,ddatetime,fieldname,value,crttime) values(SEQ_EXTDATAALARM.nextval,'%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s',sysdate)",strtname,obtid,ddatetime,strdatfieldname,value);

        if (ins.execute() != 0)
	{
	  logfile.Write("insert into T_EXTDATAALARM failed.%s\n",ins.cda.message); EXIT(-1);
	}

        // 把异常数据告警日志写入告警日志文件
        logfile.WriteAlarmFile();

        logfile.Write("异常数据告警，表名：%s，站点：%s，时间：%s，字段：%s，值为：%s。\n",strtname,obtid,ddatetime,strdatfieldname,value);
        sleep(1); // 必须休眠1秒钟以上，不然只有最后一条告警信息
      }
    } 
  }

  // 如果有最小阀值参数
  if (strlen(strminvalue)!=0)
  {
    sqlstatement stmt;
    stmt.connect(&conn);

    stmt.prepare("select %s,to_char(%s,'yyyy-mm-dd hh24:mi:ss'),%s from %s where %s>to_date('%s','yyyy-mm-dd hh24:mi:ss') and %s<=to_date('%s','yyyy-mm-dd hh24:mi:ss') and %s<%s order by %s",strobtfieldname,strddtfieldname,strdatfieldname,strtname,strddtfieldname,strddatetime01,strddtfieldname,strddatetime00,strdatfieldname,strminvalue,strobtfieldname);
    stmt.bindout(1,obtid,30);
    stmt.bindout(2,ddatetime,30);
    stmt.bindout(3,value,30);

    sqlstatement ins;
    ins.connect(&conn);

    if (stmt.execute() != 0)
    {
      logfile.Write("select %s failed.%s\n",stmt.cda.message); EXIT(-1);
    }

    while(TRUE)
    { 
      memset(obtid,0,sizeof(obtid));
      memset(ddatetime,0,sizeof(ddatetime));
      memset(value,0,sizeof(value));

      if (stmt.next() != 0) break;
      LocalTime(logfile.m_stime);

      if (strlen(obtid) != 0) 
      {
        memset(logfile.m_message,0,sizeof(logfile.m_message));

        snprintf(logfile.m_message,300,"异常数据告警，表名：%s，站点：%s，时间：%s，字段：%s，值为：%s。",strtname,obtid,ddatetime,strdatfieldname,value);

	// 插入异常数据告警日志表T_EXTDATAALARM
	ins.prepare("insert into T_EXTDATAALARM(logid,tname,obtid,ddatetime,fieldname,value,crttime) values(SEQ_EXTDATAALARM.nextval,'%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s',sysdate)",strtname,obtid,ddatetime,strdatfieldname,value);

        if (ins.execute() != 0)
 	{
	   logfile.Write("insert into T_EXTDATAALARM failed.%s\n",ins.cda.message); EXIT(-1);
	}

        // 把异常数据告警日志写入告警日志文件
        logfile.WriteAlarmFile();

        logfile.Write("异常数据告警，表名：%s，站点：%s，时间：%s，字段：%s，值为：%s。\n",strtname,obtid,ddatetime,strdatfieldname,value);
        sleep(1); // 必须休眠1秒钟以上，不然只有最后一条告警信息
      }
    } 
  } 

  return 0;
}

void EXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();
  
  logfile.Write("catching the signal(%d).\n",sig);
  
  logfile.Write("extdataalarm exit.\n");
  
  exit(0);
}
