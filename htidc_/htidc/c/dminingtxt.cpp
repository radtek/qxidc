#include "_public.h"

void CallQuit(int sig);

#define MAXFIELDCOUNT  200  // 字段的最大数
#define MAXFIELDLEN   2000  // 字段值的最大长度

CLogFile       logfile;
CProgramActive ProgramActive;

FILE *srcfp,*xmlfp;

char strTmpFileName[301];
char strXMLFileName[301];

char strsrcfilename[301];
char strsplitstr[11];
char strfirstsql[1001];
char strfieldstr[1001];
char strfieldlen[1001];
char strbfilename[31];
char strefilename[31];
char stroutpath[201];
char strstarttime[101];

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/dminingtxt logfilename xmlbuffer\n\n");
    printf("Sample:/htidc/htidc/bin/procctl 800 /htidc/htidc/bin/dminingtxt /tmp/htidc/log/dminingtxt_OBTHOURD_HYSW.log \"<srcfilename>/tmp/swdata.csv</srcfilename><splitstr>|</splitstr><firstsql>truncate table T_OBTHOURD</firstsql><fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr><fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen><bfilename>T_OBTHOURD</bfilename><efilename>HYSW</efilename><outpathtmp>/tmp/htidc/tmp</outpathtmp><starttime>00,01,02,03,04,05,06,07,08,09,10,11,12,13,14,15,16,17,18,19,20,21,22,23</starttime>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于从本文件数据源采集数据，结果生成xml文件用/usr/bin/gzip命令压缩。\n");
    printf("logfilename是本程序运行的日志文件。\n");
    printf("xmlbuffer为数据挖掘的参数，如下：\n");

    printf("数据源文件的文件名 <srcfilename>/tmp/swdata.csv</srcfilename> 注意，为了防止中间状态的文件被处理，数据源文件至少要在文件生成的50秒后才处理。\n");
    printf("数据源文件记录的字段之间的分隔符 <splitstr>|</splitstr> 可以是字符或字符串。\n");
    printf("xml文件首部的SQL语句，入库程序在处理数据之前先执行这个SQL <firstsql>truncate table T_OBTHOURD</firstsql>\n");
    printf("数据源文件字段对应的xml字段名 <fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr>\n");
    printf("每个xml字段的长度 <fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen>\n");
    printf("xml文件的前缀 <bfilename>OBTHOURD</bfilename>\n");
    printf("xml文件的后缀 <efilename>HYSW</efilename>\n");
    printf("xml文件的输出目录 <outpath>/tmp/htidc/ftpput</outpath>\n");
    printf("程序运行起始时间 <starttime>02,10,20</starttime>，支持多个时间，中间用逗号分隔开，如02,10,20表示在每天的02、10和20点启动。\n");
    printf("注意，如果starttime为空，那么starttime参数将失效，只要本程序启动就会执行数据采集。\n");
    printf("firstsql、outpathtmp和starttime可以为空，其它只字都不允许为空，如果其它参数为空，程序退出。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  srcfp=xmlfp=0;

  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strXMLFileName,0,sizeof(strXMLFileName));

  char strXmlBuffer[4001];

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("dminingtxt");

  logfile.Write("dminingtxt beginning.\n");

  memset(strsrcfilename,0,sizeof(strsrcfilename));
  memset(strsplitstr,0,sizeof(strsplitstr));
  memset(strfirstsql,0,sizeof(strfirstsql));
  memset(strfieldstr,0,sizeof(strfieldstr));
  memset(strfieldlen,0,sizeof(strfieldlen));
  memset(strbfilename,0,sizeof(strbfilename));
  memset(strefilename,0,sizeof(strefilename));
  memset(stroutpath,0,sizeof(stroutpath));
  memset(strstarttime,0,sizeof(strstarttime));

  GetXMLBuffer(strXmlBuffer,"srcfilename",strsrcfilename,300);
  GetXMLBuffer(strXmlBuffer,"splitstr",strsplitstr,10);
  GetXMLBuffer(strXmlBuffer,"firstsql",strfirstsql,2000);
  GetXMLBuffer(strXmlBuffer,"fieldstr",strfieldstr,1000);
  GetXMLBuffer(strXmlBuffer,"fieldlen",strfieldlen,1000);
  GetXMLBuffer(strXmlBuffer,"bfilename",strbfilename,30);
  GetXMLBuffer(strXmlBuffer,"efilename",strefilename,30);
  GetXMLBuffer(strXmlBuffer,"outpath",stroutpath,200);
  GetXMLBuffer(strXmlBuffer,"starttime",strstarttime,100);

  // firstsql、outpathtmp和starttime可以为空，其它只字都不允许为空，如果其它参数为空，程序退出
  if (strlen(strsrcfilename) == 0) { logfile.Write("srcfilename is null.\n"); return -1; }
  if (strlen(strsplitstr) == 0) { logfile.Write("splitstr is null.\n"); return -1; }
  // if (strlen(strfirstsql) == 0) { logfile.Write("firstsql is null.\n"); return -1; }
  if (strlen(strfieldstr) == 0) { logfile.Write("fieldstr is null.\n"); return -1; }
  if (strlen(strfieldlen) == 0) { logfile.Write("fieldlen is null.\n"); return -1; }
  if (strlen(strbfilename) == 0) { logfile.Write("bfilename is null.\n"); return -1; }
  if (strlen(strefilename) == 0) { logfile.Write("efilename is null.\n"); return -1; }
  if (strlen(stroutpath) == 0) { logfile.Write("outpath is null.\n"); return -1; }
  // if (strlen(strstarttime) == 0) { logfile.Write("starttime is null.\n"); return -1; }

  // 获取服务器的时间，判断是否该启用程序，如果时间中的小时不匹配，就退出
  if (strlen(strstarttime) != 0)
  {
    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"hh24mi");
    strLocalTime[2]=0;
    if (strstr(strstarttime,strLocalTime) == 0) return 0;
  }

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"dminingtxt",1200);

  // 把xml的字段名和字段的长度用CCmdStr类拆分开
  CCmdStr fieldstr,fieldlen;
  fieldstr.SplitToCmd(strfieldstr,",");
  fieldlen.SplitToCmd(strfieldlen,",");

  // 如果xml的字段名和字段的长度的个数不同，一定是参数设错了，程序退出
  if ( (fieldstr.CmdCount()==0) || (fieldstr.CmdCount() != fieldlen.CmdCount()) )
  {
    logfile.Write("fieldstr or fieldlen is invalid.\n"); return -1;
  }

  // xml的字段数不能超过MAXFIELDCOUNT宏
  if (fieldstr.CmdCount()>MAXFIELDCOUNT) 
  {
    logfile.Write("fields is to many,max is %d.\n",MAXFIELDCOUNT); return -1;
  }
  
  // 把xml字段名和字段长度信息拆分到数据中
  char strfieldname[MAXFIELDCOUNT][31];    // xml字段名数组
  int  ifieldlen[MAXFIELDCOUNT];   // xml字段长度数组
  char strfieldvalue[MAXFIELDLEN+1]; // 用于存放字段值
  UINT ii;

  memset(strfieldname,0,sizeof(strfieldname));
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  memset(&ifieldlen,0,sizeof(ifieldlen));

  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    fieldstr.GetValue(ii,strfieldname[ii],30);
    fieldlen.GetValue(ii,&ifieldlen[ii]);
  }

  // 判断数据源文件是否存在，如果不存在，返回
  if (access(strsrcfilename,R_OK) != 0) return 0;

  // 判断数据源文件的生成时间，如果在50秒之内，就不处理这个文件，因为它很可能是中间状态的文件
  char strLocalTime[21],strFModTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strFModTime,0,sizeof(strFModTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss",0-50); // 当前时间点50秒之前的时间
  // 判断文件的时间，即modtime
  FileModTime(strsrcfilename,strFModTime);
  if (strcmp(strLocalTime,strFModTime) < 0) return 0;

  // 打开数据源文件
  if ( (srcfp=FOPEN(strsrcfilename,"r")) == 0 )
  {
    logfile.Write("FOPEN %s failed.\n",strsrcfilename); return -1;
  }
  
  // 写入进程活动信息
  ProgramActive.WriteToFile();

  // 采集数据的开始时间和结束时间
  char strBeginTime[21],strEndTime[21]; 

  memset(strBeginTime,0,sizeof(strBeginTime));
  memset(strEndTime,0,sizeof(strEndTime));

  // 获取采集数据的开始时间
  LocalTime(strBeginTime,"yyyymmddhh24miss");

  snprintf(strTmpFileName,300,"%s/%s_%s_%d.xml.tmp",stroutpath,strbfilename,strBeginTime,getpid());

  logfile.Write("begin process %s...\n",strsrcfilename);
  
  logfile.WriteEx("%s\n%s\n",strfieldstr,strfieldlen);

  char strLine[4096];
  CCmdStr CmdStr;

  UINT iinext=10000;
  
  UINT itotal,ivalid;
  
  itotal=ivalid=0;

  // 处理SQL语句执行后的每一行
  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
    
    if (FGETS(strLine,4000,srcfp) == FALSE) break;
    
    itotal++;
    
    // 每获取10000条记录写入一次进程活动信息
    if (iinext++ > 10000)
    {
      iinext=0; ProgramActive.WriteToFile();
    }
    
    CmdStr.SplitToCmd(strLine,strsplitstr);
    
    // 无效的行将丢弃
    if (CmdStr.CmdCount() != fieldstr.CmdCount()) continue;
    
    ivalid++;

    // 打开xml文件
    if (xmlfp == 0)
    {
      if ( (xmlfp=FOPEN(strTmpFileName,"w+")) == 0 )
      {
        logfile.Write("FOPEN %s failed.\n",strTmpFileName); return -1;
      }

      if (strlen(strfirstsql) != 0) fprintf(xmlfp,"<sql>\n%s\n</sql><endl/>\n",strfirstsql);

      fprintf(xmlfp,"<data>\n");
    }

    // 把每个xml字段名和值写入xml文件
    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      memset(strfieldvalue,0,sizeof(strfieldvalue));
      CmdStr.GetValue(ii,strfieldvalue,ifieldlen[ii]);
      fprintf(xmlfp,"<%s>%s</%s>",strfieldname[ii],strfieldvalue,strfieldname[ii]);
    }

    fprintf(xmlfp,"<endl/>\n");
  }

  fclose(srcfp);

  logfile.Write("total=%ld,valid=%ld.\n",itotal,ivalid);

  // 写入进程活动信息
  ProgramActive.WriteToFile();

  // 关闭xml文件，并改名为最终的xml文件名
  if (xmlfp != 0)
  {
    fprintf(xmlfp,"</data>"); fclose(xmlfp); xmlfp=0;

    // 压缩xml文件
    char strCmd[4001];
    memset(strCmd,0,sizeof(strCmd));
    snprintf(strCmd,4000,"/usr/bin/gzip -c %s > %s.tmp 2>/dev/null",strTmpFileName,strTmpFileName);
    system(strCmd);

    // 删除压缩前的xml文件
    REMOVE(strTmpFileName);

    strncat(strTmpFileName,".tmp",4);

    // 获取采集数据的结束时间
    LocalTime(strEndTime,"yyyymmddhh24miss");

    snprintf(strXMLFileName,300,"%s/%s_%s_%s_%s.xml.gz",stroutpath,strbfilename,strBeginTime,strEndTime,strefilename);

    // 把文件改名为正式的压缩文件
    if (RENAME(strTmpFileName,strXMLFileName) == FALSE)
    {
      logfile.Write("RENAME %s to %s failed.\n",strTmpFileName,strXMLFileName); return -1;
    }

    logfile.Write("/usr/bin/gzip %s ok.\n",strXMLFileName);

    if (REMOVE(strsrcfilename) == FALSE)
    {
      logfile.Write("REMOVE %s failed.\n",strsrcfilename);
    }

    logfile.Write("REMOVE %s ok.\n",strsrcfilename);
  }

  logfile.WriteEx("\n\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // 如果程序异常退出，就关闭文件描述符，删除中间状态的文件
  if (srcfp!=0) { fclose(srcfp); }
  
  if (xmlfp!=0) { fclose(xmlfp); REMOVE(strTmpFileName); }

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("dminingtxt exit.\n");

  exit(0);
}

