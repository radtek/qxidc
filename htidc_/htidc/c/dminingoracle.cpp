#include "_public.h"
#include "_oracle.h"

void CallQuit(int sig);

#define MAXFIELDCOUNT  200  // 字段的最大数
#define MAXFIELDLEN   2000  // 字段值的最大长度

connection     connsrc;
CLogFile       logfile;
CProgramActive ProgramActive;

FILE *xmlfp;

char strTmpFileName[301];
char strXMLFileName[301];

char strXmlBuffer[5001];
char strcharset[51];
char strconnstr[101];
char strfirstsql[1001];
char strselectsql[4001];
char strfieldstr[2001];
char strfieldlen[1001];
char strbfilename[31];
char strefilename[31];
char stroutpathtmp[201];
char stroutpath[201];
char strstarttime[101];
char strendsql[1001];
int  itimetvl;
char strcompress[21];

char strincfield[31];
char strincfilename[201];
int  incfieldpos=-1;
long incfieldvalue_old=0;
long incfieldvalue_new=0;
long totalcount=0;

// 读取增量采集标志字段的值存放的文件，结果保存在incfieldvalue_old变量中。
BOOL ReadIncFile();

// 把增量字段的值写入文件中
BOOL WriteIncFile();

// 判断是否需要处理xmlbuffer中的时间变量
//void MatchXMLBuffer();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/dminingoracle logfilename xmlbuffer\n\n");
    printf("Sample:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/dminingoracle /tmp/htidc/log/dminingoracle_OBTHOURD_HYCZ.log \"<charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>sqxt/pwdidc@SZQX_10.148.124.85</connstr><firstsql>truncate table T_OBTHOURD</firstsql><selectsql>select to_char(ddatetime,'yyyymmddhh24miss'),obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,temp,maxtemp,maxttime,mintemp,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid from T_OBTHOURD where ddatetime<to_date('2010-12-01 00:00:00','yyyy-mm-dd hh24:mi:ss')</selectsql><fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,temp,maxtemp,maxttime,mintemp,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr><fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen><bfilename>T_OBTHOURD</bfilename><efilename>HYCZ</efilename><outpathtmp>/tmp/htidc/tmp</outpathtmp><outpath>/tmp/htidc/ftpput</outpath><starttime>01,12</starttime><endsql></endsql><timetvl>-8</timetvl><incfield>keyid</incfield><incfilename>/tmp/htidc/list/dminingoracle_OBTHOURD_HYCZ.list</incfilename>\"\n\n");

    printf("本程序是数据中心的公共功能模块，用于从oracle数据库源表采集数据，结果生成xml文件用/usr/bin/gzip命令压缩。\n");
    printf("logfilename是本程序运行的日志文件。\n");
    printf("xmlbuffer为数据挖掘的参数，如下：\n");

    printf("数据库的字符集 <charset>Simplified Chinese_China.ZHS16GBK</charset> 这个参数要与数据源数据库保持一致，否则会出现中文乱码的情况。\n");
    printf("数据库的连接参数 <connstr>sqxt/pwdidc@SZQX_10.148.124.85</connstr>\n");
    printf("xml文件首部的SQL语句，入库程序在处理数据之前先执行这个SQL <firstsql>truncate table T_OBTHOURD</firstsql>\n");
    printf("从数据源数据库提取数据的SQL <selectsql>select to_char(ddatetime,'yyyymmddhh24miss'),obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid from T_OBTHOURD where ddatetime<to_date('2010-12-01 00:00:00','yyyy-mm-dd hh24:mi:ss')</selectsql>\n");
    printf("提取后数据内容对应的xml字段名 <fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr>\n");
    printf("每个xml字段的长度 <fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen> 如果fieldlen字段为空，就采用MAXFIELDLEN的长度。\n");
    printf("xml文件的前缀 <bfilename>OBTHOURD</bfilename>\n");
    printf("xml文件的后缀 <efilename>HYCZ</efilename>\n");
    printf("xml文件的输出的临时目录 <outpathtmp>/tmp/htidc/tmp</outpathtmp> 如果outpathtmp为空，缺省用outpath填充。\n");
    printf("xml文件的输出的目录 <outpath>/tmp/htidc/ftpput</outpath> 注意，outpath和outpathtmp一定要在同一个文件分区，否则文件移动时会失败。\n");
    printf("程序运行起始时间 <starttime>02,10,20</starttime>，支持多个时间，中间用逗号分隔开，如02,10,20表示在每天的02、10和20点启动。\n");
    printf("注意，如果starttime为空，那么starttime参数将失效，只要本程序启动就会执行数据采集，为了减少数据源"\
           "的压力，从数据库采集数据的时候，一般在对方数据库最闲的时候时进行，"\
           "但是从文件中采集数据不存在这种情况。\n");
    printf("数据采集完成后执行的SQL脚本 <endsql>delete from T_OBTHOURD</endsql>，每次采集完数据后，在源数据库中执行的SQL脚本，该参数要慎用。\n");
    printf("xmlbuffer可以处理时间变量，<timetvl>-8</timetvl> 为时间变量的偏移时间。"\
           "目前可以处理以下时间变量：{YYYY}（4位的年）、{YYY}（后三位的年）、"\
           "{YY}（后两位的年）、{MM}（月月）、{DD}（日日）、{HH}（时时）、{MI}（分分）、{SS}（秒秒）。\n");
    printf("增量采集字段标志 <incfield>keyid</incfield> 它必须是fieldstr中的字段名，并且只能是整数。\n");
    printf("增量采集标志字段的值存放的文件 <incfilename>/tmp/htidc/list/dminingoracle_OBTHOURD_HYCZ.list</incfilename> 每次采集完数据后，incfield的最大值将存放在此文件中。\n");
    printf("firstsql、fieldlen、outpathtmp、starttime、endsql、timetvl、incfield、incfilename可以为空，其它只字段都不允许为空，如果其它参数为空，程序会报错退出。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  xmlfp=0;

  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strXMLFileName,0,sizeof(strXMLFileName));

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],5000);

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("dminingoracle");

  memset(strcharset,0,sizeof(strcharset));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strfirstsql,0,sizeof(strfirstsql));
  memset(strselectsql,0,sizeof(strselectsql));
  memset(strfieldstr,0,sizeof(strfieldstr));
  memset(strfieldlen,0,sizeof(strfieldlen));
  memset(strbfilename,0,sizeof(strbfilename));
  memset(strefilename,0,sizeof(strefilename));
  memset(stroutpathtmp,0,sizeof(stroutpathtmp));
  memset(stroutpath,0,sizeof(stroutpath));
  memset(strstarttime,0,sizeof(strstarttime));
  memset(strendsql,0,sizeof(strendsql));
  itimetvl=0;
  memset(strincfield,0,sizeof(strincfield));
  memset(strincfilename,0,sizeof(strincfilename));
  memset(strcompress,0,sizeof(strcompress));

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);

  // 处理xmlbuffer中的时间变量
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"charset",strcharset,50);
  GetXMLBuffer(strXmlBuffer,"connstr",strconnstr,100);
  GetXMLBuffer(strXmlBuffer,"firstsql",strfirstsql,1000);
  GetXMLBuffer(strXmlBuffer,"selectsql",strselectsql,4000);
  GetXMLBuffer(strXmlBuffer,"fieldstr",strfieldstr,2000);
  GetXMLBuffer(strXmlBuffer,"fieldlen",strfieldlen,1000);
  GetXMLBuffer(strXmlBuffer,"bfilename",strbfilename,30);
  GetXMLBuffer(strXmlBuffer,"efilename",strefilename,30);
  GetXMLBuffer(strXmlBuffer,"outpathtmp",stroutpathtmp,200);
  GetXMLBuffer(strXmlBuffer,"outpath",stroutpath,200);
  GetXMLBuffer(strXmlBuffer,"starttime",strstarttime,100);
  GetXMLBuffer(strXmlBuffer,"endsql",strendsql,1000);
  GetXMLBuffer(strXmlBuffer,"incfield",strincfield,30);
  GetXMLBuffer(strXmlBuffer,"incfilename",strincfilename,200);
  if (GetXMLBuffer(strXmlBuffer,"compress",strcompress,20)==FALSE) strcpy(strcompress,"TRUE");

  // firstsql、fieldlen、outpathtmp、starttime、endsql、timetvl、incfield、incfilename可以为空，其
  // 它只字段都不允许为空，如果其它参数为空，程序会报错退出。
  if (strlen(strcharset) == 0) { logfile.Write("charset is null.\n"); return -1; }
  if (strlen(strconnstr) == 0) { logfile.Write("connstr is null.\n"); return -1; }
  if (strlen(strselectsql) == 0) { logfile.Write("selectsql is null.\n"); return -1; }
  if (strlen(strfieldstr) == 0) { logfile.Write("fieldstr is null.\n"); return -1; }
  if (strlen(strbfilename) == 0) { logfile.Write("bfilename is null.\n"); return -1; }
  if (strlen(strefilename) == 0) { logfile.Write("efilename is null.\n"); return -1; }
  if (strlen(stroutpathtmp) == 0) { strcpy(stroutpathtmp,stroutpath); }
  if (strlen(stroutpath) == 0) { logfile.Write("outpath is null.\n"); return -1; }

  // 获取服务器的时间，判断是否该启用程序，如果时间中的小时不匹配，就退出
  if (strlen(strstarttime) != 0)
  {
    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"hh24mi");
    strLocalTime[2]=0;
    if (strstr(strstarttime,strLocalTime) == 0) return 0;
  }

  // 读取增量采集标志字段的值存放的文件，结果保存在incfieldvalue_old变量中。
  if (ReadIncFile() == FALSE) return -1;

  // 20190808 加了以下这段，不然无法实现增量采集数据。
  char strincresql[201];
  memset(strincresql,0,sizeof(strincresql));
  
  if (strlen(strincfield) > 0) 
  {
    snprintf(strincresql,200," %s > %ld",strincfield,incfieldvalue_old);

    if (strstr(strselectsql,"where") == 0)
    {
      strcat(strselectsql," where");
    }
    else
    {
      strcat(strselectsql," and");
    }
    strcat(strselectsql,strincresql);
  }

  logfile.Write("dminingoracle beginning.\n");

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"dminingoracle",1200);

  // 把xml的字段名和字段的长度用CCmdStr类拆分开
  CCmdStr fieldstr,fieldlen;
  fieldstr.SplitToCmd(strfieldstr,",");
  fieldlen.SplitToCmd(strfieldlen,",");

  if (strlen(strfieldlen) != 0)
  {
    // 如果xml的字段名和字段的长度的个数不同，一定是参数设错了，程序退出
    if ( (fieldstr.CmdCount()==0) || (fieldstr.CmdCount() != fieldlen.CmdCount()) )
    {
      logfile.Write("fieldstr(%d) or fieldlen(%d) is invalid.\n",fieldstr.CmdCount(),fieldlen.CmdCount()); return -1;
    }
  }

  // xml的字段数不能超过MAXFIELDCOUNT宏
  if (fieldstr.CmdCount()>MAXFIELDCOUNT) 
  {
    logfile.Write("fields is to many,max is %d.\n",MAXFIELDCOUNT); return -1;
  }
  
  // 把xml字段名和字段长度信息拆分到数据中
  char strfieldname[MAXFIELDCOUNT][31];    // xml字段名数组
  int  ifieldlen[MAXFIELDCOUNT];   // xml字段长度数组
  char strfieldvalue[MAXFIELDCOUNT][MAXFIELDLEN+1]; // 用于存放字段值的数组
  UINT ii;

  memset(strfieldname,0,sizeof(strfieldname));
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  memset(&ifieldlen,0,sizeof(ifieldlen));

  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    fieldstr.GetValue(ii,strfieldname[ii],30);
    if (strlen(strfieldlen) != 0) fieldlen.GetValue(ii,&ifieldlen[ii]);
    if (strlen(strfieldlen) == 0) ifieldlen[ii]=MAXFIELDLEN;

    // 记下增量字段的位置
    if ( (strlen(strincfield)>0) && (strcmp(strfieldname[ii],strincfield)==0) ) incfieldpos=ii;
  }

  if ( (strlen(strincfield)>0) && (incfieldpos==-1) ) { logfile.Write("check incfield(%s) failed.not in fieldstr.\n",strincfield); return -1;}

  setenv("NLS_LANG",strcharset,1);

  // 连接数据源数据库
  if (connsrc.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connsrc.connecttodb(%s) failed\n",strconnstr); CallQuit(-1);
  }

  // 写入进程活动信息
  ProgramActive.WriteToFile();

  // 采集数据的开始时间和结束时间
  char strBeginTime[21],strEndTime[21]; 

  memset(strBeginTime,0,sizeof(strBeginTime));
  memset(strEndTime,0,sizeof(strEndTime));

  // 获取采集数据的开始时间
  LocalTime(strBeginTime,"yyyymmddhh24miss");

  snprintf(strTmpFileName,300,"%s/%s_%s_%d.xml.tmp",stroutpathtmp,strbfilename,strBeginTime,getpid());

  // 准备获取数据的SQL语句，绑定输出变量
  sqlstatement stmt;
  stmt.connect(&connsrc);
  stmt.prepare(strselectsql);
  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    stmt.bindout(ii+1,strfieldvalue[ii],ifieldlen[ii]);
  }

  logfile.Write("begin exec sql.\n");

  logfile.WriteEx("%s\n%s\n%s\n",strselectsql,strfieldstr,strfieldlen);

  // 执行采集数据的SQL语句
  if (stmt.execute() != 0)
  {
    logfile.Write("%s\nexec sql failed.\n%s\n",strselectsql,stmt.cda.message); return stmt.cda.rc;
  }

  logfile.Write("exec sql(%s) ok.\n",stmt.m_sql);

  UINT iinext=10000;

  // 处理SQL语句执行后的每一行
  while (TRUE)
  {
    memset(strfieldvalue,0,sizeof(strfieldvalue));

    if (stmt.next() != 0) break;

    // 每获取10000条记录写入一次进程活动信息
    if (iinext++ > 10000)
    {
      iinext=0; ProgramActive.WriteToFile();
    }

    if (strlen(strincfield)>0)
    {
      // 如果本记录的增量字段的值小于上一次采集的最大值，就丢弃它
      if (atol(strfieldvalue[incfieldpos]) < incfieldvalue_old) continue;

      // 记录增量字段新的最大值
      if (atol(strfieldvalue[incfieldpos])>incfieldvalue_new) incfieldvalue_new=atol(strfieldvalue[incfieldpos]);
    }

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
      Trim(strfieldvalue[ii]); // 删除字段值两边的空格。
      fprintf(xmlfp,"<%s>%s</%s>",strfieldname[ii],strfieldvalue[ii],strfieldname[ii]);
    }

    fprintf(xmlfp,"<endl/>\n");

    totalcount++;
  }

  logfile.Write("rows %ld.\n",totalcount);

  // 写入进程活动信息
  ProgramActive.WriteToFile();

  // 关闭xml文件，压缩后改名为最终的xml文件名
  if (xmlfp != 0)
  {
    fprintf(xmlfp,"</data>"); fclose(xmlfp); xmlfp=0;

    // 压缩xml文件
    if (strcmp(strcompress,"TRUE")==0)
    {
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
    }
    else
    {
      // 获取采集数据的结束时间
      LocalTime(strEndTime,"yyyymmddhh24miss");

      snprintf(strXMLFileName,300,"%s/%s_%s_%s_%s.xml",stroutpath,strbfilename,strBeginTime,strEndTime,strefilename);

      // 把文件改名为正式的xml文件
      if (RENAME(strTmpFileName,strXMLFileName) == FALSE)
      {
        logfile.Write("RENAME %s to %s failed.\n",strTmpFileName,strXMLFileName); return -1;
      }
    }

    logfile.Write("/usr/bin/gzip %s ok.\n",strXMLFileName);
  }

  if ( strlen(strendsql) != 0 )
  {
    // 采集完成后，执行endsql脚本。
    stmt.prepare(strendsql);
    if (stmt.execute() != 0)
    {
      logfile.Write("exec %s failed.\n%s\n",strendsql,stmt.cda.message); return -1;
    }
  
    logfile.Write("exec %s ok.\n",strendsql);
  
    connsrc.commitwork();
  }

  logfile.WriteEx("\n\n");

  // 把增量字段的值写入文件中
  WriteIncFile();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // 如果程序异常退出，就关闭文件描述符，删除中间状态的文件
  if (xmlfp!=0) { fclose(xmlfp); REMOVE(strTmpFileName); }

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("dminingoracle exit.\n");

  exit(0);
}

/*
// 判断是否需要处理xmlbuffer中的时间变量
// 可以处理以下时间变量：YYYY（4位的年）、YYY（后三位的年）、
// YY（后两位的年）、MM（月月）、DD（日日）、HH（时时）、MI（分分）、SS（秒秒），注意，变量都要采用大写
// 变量的格式为{变量名}
void MatchXMLBuffer()
{
  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss",0+itimetvl*60*60);

  char YYYY[5],YYY[4],YY[3],MM[3],DD[3],HH[3],MI[3],SS[3];

  memset(YYYY,0,sizeof(YYYY));
  memset(YYY,0,sizeof(YYY));
  memset(YY,0,sizeof(YY));
  memset(MM,0,sizeof(MM));
  memset(DD,0,sizeof(DD));
  memset(HH,0,sizeof(HH));
  memset(MI,0,sizeof(MI));
  memset(SS,0,sizeof(SS));

  strncpy(YYYY,strLocalTime,4);
  strncpy(YYY,strLocalTime+1,3);
  strncpy(YY,strLocalTime+2,2);
  strncpy(MM,strLocalTime+4,2);
  strncpy(DD,strLocalTime+6,2);
  strncpy(HH,strLocalTime+8,2);
  strncpy(MI,strLocalTime+10,2);
  strncpy(SS,strLocalTime+12,2);

  UpdateStr(strXmlBuffer,"{YYYY}",YYYY);
  UpdateStr(strXmlBuffer,"{YYY}",YYY);
  UpdateStr(strXmlBuffer,"{YY}",YY);
  UpdateStr(strXmlBuffer,"{MM}",MM);
  UpdateStr(strXmlBuffer,"{DD}",DD);
  UpdateStr(strXmlBuffer,"{HH}",HH);
  UpdateStr(strXmlBuffer,"{MI}",MI);
  UpdateStr(strXmlBuffer,"{SS}",SS);

  logfile.Write("xmlbuffer=%s\n",strXmlBuffer);
}
*/

// 读取增量采集标志字段的值存放的文件，结果保存在incfieldvalue_old变量中。
BOOL ReadIncFile()
{
  if (strlen(strincfield)==0) return TRUE;

  incfieldvalue_old=0;

  char strincfieldvalue[51];
  memset(strincfieldvalue,0,sizeof(strincfieldvalue));

  CFile File;

  if (File.OpenForRead(strincfilename,"r") == FALSE) return TRUE;

  File.FFGETS(strincfieldvalue,30);

  incfieldvalue_old=atol(strincfieldvalue);

  return TRUE;
}

// 把增量字段的值写入文件中
BOOL WriteIncFile()
{
  if (incfieldvalue_new == 0) return TRUE;

  CFile File;

  if (File.OpenForWrite(strincfilename,"w+") == FALSE) 
  {
    logfile.Write("File.OpenForWrite(%s) failed.\n",strincfilename); return FALSE;
  }

  File.Fprintf("%ld",incfieldvalue_new);

  File.Fclose();

  return TRUE;
}

/*
/htidc/htidc/bin/dminingoracle /tmp/htidc/log/dminingoracle_SEND_LIST_HIS.log "<charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>hzzwt/esaserver</connstr><firstsql>truncate table SEND_LIST_HIS</firstsql><selectsql>select clientserialno,to_char(crttime,'yyyy-mm-dd hh24:mi:ss'),sendno from SEND_LIST_HIS where crttime>sysdate-5</selectsql><fieldstr>clientserialno,crttime,sendno</fieldstr><fieldlen> </fieldlen><bfilename>SEND_LIST_HIS</bfilename><efilename>HZZWT</efilename><outpathtmp>/tmp/htidc/tmp</outpathtmp><outpath>/tmp/htidc/ftpput</outpath><starttime></starttime><endsql></endsql><incfield>sendno</incfield><incfilename>/tmp/htidc/list/dminingoracle_SEND_LIST_HIS.list</incfilename>"
*/
