#include "idcapp.h"
#include "_public.h"

void CallQuit(int sig);

char strIDCConnStr[201]; // 数据库连接参数
char strLogPath[201];    // 日志文件目录
char strTmpPath[201];    // 进程活动信息文件目录
char strIniFile[201]; 
//char strDType[11];
int  iDays;
char strObtID[11];

connection     conn;
CLogFile       logfile;
CIniFile       IniFile;
CProgramActive ProgramActive;
CAWSDAYD       AWSDAYD;

long _pawsdayd();

int main(int argc,char *argv[])
{
  if ( argc != 3 && argc != 4)
  {
    printf("\n");
    printf("Using:./pawsdayd inifile days [obtid]\n");

    printf("Example:/htidc/htidc/bin/procctl 1800 /htidc/htidc/bin/pawsdayd /htidc/sqxt/ini/sqxt.xml 1 G3501\n");
    printf("        /htidc/htidc/bin/procctl 7200 /htidc/htidc/bin/pawsdayd /htidc/sqxt/ini/sqxt.xml 1 \n\n");
 
    printf("此程序用于统计自动站的日数据，每次统计全部站点最近days天的记录。\n");
    printf("该程序统计的日数据用于气候统计，它不同于pcalobtdayd程序。\n");
    printf("目前只统计深圳自动站的数据，没有统计全省自动站的数据。\n");
  
    return -1;
  }

  memset(strIniFile,0,sizeof(strIniFile));
//  memset(strDType,0,sizeof(strDType));
  memset(strObtID,0,sizeof(strObtID));
  iDays=0;

  strcpy(strIniFile,argv[1]);
  iDays=atoi(argv[2]);
//  strncpy(strDType,argv[3],10);
  if (argc==4) strncpy(strObtID,argv[3],5);

  // 导入参数文件
  if (IniFile.LoadFile(strIniFile) == FALSE)
  {
    printf("IniFile.LoadFile(%s) failed.\n",strIniFile); return -1;
  }

  // 从参数文件中获取数据库连接参数
  memset(strIDCConnStr,0,sizeof(strIDCConnStr));
  if (IniFile.GetValue("appconnstr",strIDCConnStr) == FALSE)
  {
    printf("IniFile.GetValue field(appconnstr) failed.\n"); return -1;
  }

  // 从参数文件中获取日志文件存放目录参数
  memset(strLogPath,0,sizeof(strLogPath));
  if (IniFile.GetValue("logpath",strLogPath) == FALSE)
  {
    printf("IniFile.GetValue field(logpath) failed.\n"); return -1;
  }
  
  // 从参数文件中获取临时文件存放目录参数
  memset(strTmpPath,0,sizeof(strTmpPath));
  if (IniFile.GetValue("tmppath",strTmpPath) == FALSE)
  {
    printf("IniFile.GetValue field(tmppath) failed.\n"); return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,201,"%s/pawsdayd.log",strLogPath);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt( "%s", argv[0] );

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"pawsdayd",1200);

  //logfile.Write("pawsdayd begin.\n");

  AWSDAYD.BindConnLog(&conn,&logfile);

  // 连接数据库
  /*
   drop   sequence SEQ_LOCALAWSDAYD;
   create sequence SEQ_LOCALAWSDAYD increment by 1 minvalue 1 nocycle;
  */

  if (conn.connecttodb(strIDCConnStr,TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strIDCConnStr); CallQuit(-1);
  }

  // 指定数据表的种类
//  strncpy(AWSDAYD.m_dtype,strDType,5);

  _pawsdayd();

  logfile.WriteEx("\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("pawsdayd exit.\n");

  exit(0);
}

long _pawsdayd()
{
  char strMinTName[51],strHourTName[51],strRainTName[51];

  memset(strMinTName,0,sizeof(strMinTName));
  memset(strHourTName,0,sizeof(strHourTName));
  memset(strRainTName,0,sizeof(strRainTName));

  if (iDays <= 5)
  {
    sprintf(strMinTName ,"T_OBTMIND@SZQX_10_148_124_85");
  }
  else
  {
    sprintf(strMinTName ,"V_OBTMIND@SZQX_10_148_124_85");
  }

  sprintf(strHourTName,"T_OBTHOURD@SZQX_10_148_124_85");
  sprintf(strRainTName,"T_OBTRAIND@SZQX_10_148_124_85");

  struct st_OBTDAYD *pst=&AWSDAYD.m_stOBTDAYD;

  int okcount=0;

  while (TRUE)
  {
    // 获取一个需要统计的天的开始时间，结束时间等参数
    if (AWSDAYD.GetADayInfo(iDays--) == FALSE) break;

    logfile.Write("datetime=%s,begintime=%s,endtime=%s\n",AWSDAYD.m_ddatetime,AWSDAYD.m_begintime,AWSDAYD.m_endtime);

    // 从自动站的整点数据视图中获取这一天的全部的自动站代码
    if (AWSDAYD.GetAllObtID(strHourTName,strObtID) != 0) { logfile.Write("call AWSDAYD.GetAllObtID failed.\n"); return -1; }

    while (AWSDAYD.GetAllObtIDNext() == 0)
    {
      //logfile.Write("obtid=%s\n",AWSDAYD.m_obtid);

      if (okcount++ > 10) { okcount=0; ProgramActive.WriteToFile(); }

      memset(pst,0,sizeof(AWSDAYD.m_stOBTDAYD));
   
      strcpy(pst->ddatetime,AWSDAYD.m_ddatetime);
      strcpy(pst->obtid,AWSDAYD.m_obtid);

      // 计算20-08时雨量，08-20时雨量，日雨量
      AWSDAYD.CALDayR(strHourTName);

      // 计算最大十分钟滑动雨量和出现时间
      AWSDAYD.GetMathValue(strRainTName,"r10m","max",pst->maxr10m,"and substr(hdrbz,3,1)='1'");

      if (atoi(pst->maxr10m) == 0)
      {
        // 没有降雨就清空日表的最大十分钟滑动雨量和出现时间字段
        memset(pst->maxr10m,0,sizeof(pst->maxr10m)); 
        memset(pst->maxr10mtime,0,sizeof(pst->maxr10mtime));
      }
      else
      {
        AWSDAYD.GetRecordDate(strRainTName,"r10m",pst->maxr10m,pst->maxr10mtime,"and substr(hdrbz,3,1)='1'");
      }

      // 计算最大小时滑动雨量和出现时间
      AWSDAYD.GetMathValue(strRainTName,"r01h","max",pst->maxr01h,"and substr(hdrbz,5,1)='1'");
      if (atoi(pst->maxr01h) == 0)
      {
        // 没有降雨就清空日表的最大小时滑动雨量和出现时间字段
        memset(pst->maxr01h,0,sizeof(pst->maxr01h)); 
        memset(pst->maxr01htime,0,sizeof(pst->maxr01htime));
      }
      else
      {
        AWSDAYD.GetRecordDate(strRainTName,"r01h",pst->maxr01h,pst->maxr01htime,"and substr(hdrbz,5,1)='1'");
      }

      // 计算最高、最低空气温度和它的出现时间
      AWSDAYD.GetMathValue(strMinTName ,"t","max",pst->maxt);
      AWSDAYD.GetRecordDate(strMinTName,"t",pst->maxt,pst->maxttime);
      AWSDAYD.GetMathValue(strMinTName ,"t","min",pst->mint);
      AWSDAYD.GetRecordDate(strMinTName,"t",pst->mint,pst->minttime);

      // 计算最高、最低本站气压和它的出现时间
      AWSDAYD.GetMathValue(strMinTName ,"p","max",pst->maxp);
      AWSDAYD.GetRecordDate(strMinTName,"p",pst->maxp,pst->maxptime);
      AWSDAYD.GetMathValue(strMinTName ,"p","min",pst->minp);
      AWSDAYD.GetRecordDate(strMinTName,"p",pst->minp,pst->minptime);

      // 计算最高、最低海平面气压和它的出现时间
      AWSDAYD.GetMathValue(strMinTName ,"p0","max",pst->maxp0);
      AWSDAYD.GetRecordDate(strMinTName,"p0",pst->maxp0,pst->maxp0time);
      AWSDAYD.GetMathValue(strMinTName ,"p0","min",pst->minp0);
      AWSDAYD.GetRecordDate(strMinTName,"p0",pst->minp0,pst->minp0time);

      // 计算最高、最低相对湿度和它的出现时间
      AWSDAYD.GetMathValue(strMinTName ,"rh","max",pst->maxrh);
      AWSDAYD.GetRecordDate(strMinTName,"rh",pst->maxrh,pst->maxrhtime);
      AWSDAYD.GetMathValue(strMinTName ,"rh","min",pst->minrh);
      AWSDAYD.GetRecordDate(strMinTName,"rh",pst->minrh,pst->minrhtime);

      // 计算最大瞬时风速和对应的风向及出现时间
      AWSDAYD.GetMathValue(strMinTName,"wdidf","max",pst->wd3smaxdf);
      AWSDAYD.GetDDAndTime(strMinTName,"wdidf","wdidd",pst->wd3smaxdf,pst->wd3smaxdd,pst->wd3smaxtime);

      // 计算最大2分钟风速和对应的风向及出现时间
      AWSDAYD.GetMathValue(strMinTName,"wd2df","max",pst->wd2maxdf);
      AWSDAYD.GetDDAndTime(strMinTName,"wd2df","wd2dd",pst->wd2maxdf,pst->wd2maxdd,pst->wd2maxtime);

      // 计算最大10分钟风速和对应的风向及出现时间
      //AWSDAYD.GetMathValue(strHourTName,"wd10maxdf","max",pst->wd10maxdf,"and (rddatetime>=54 or rddatetime=0)");
      AWSDAYD.GetMathValue(strMinTName,"wd10df","max",pst->wd10maxdf);
      AWSDAYD.GetDDAndTime(strMinTName,"wd10df","wd10dd",pst->wd10maxdf,pst->wd10maxdd,pst->wd10maxtime);

      // 统计平均值项目
      // 如果一日内没有缺测，则日平均为24次的平均，
      // 如果有缺测但02、08、14、20次没有缺测，则日平均按四次观测的统计；
      // 如果以上两个条件都不满足，当缺测次数小于等于6次时按实有次数统计，大于6次时按缺测处理。
      AWSDAYD.GetAvgValue(strHourTName,"t",pst->avgt);
      AWSDAYD.GetAvgValue(strHourTName,"p0",pst->avgp0);
      AWSDAYD.GetAvgValue(strHourTName,"p",pst->avgp);
      AWSDAYD.GetAvgValue(strHourTName,"rh",pst->avgrh);
      AWSDAYD.GetAvgValue(strHourTName,"wdidf",pst->avgwdidf);
      AWSDAYD.GetAvgValue(strHourTName,"wd2df",pst->avgwd2df);
      AWSDAYD.GetAvgValue(strHourTName,"wd10df",pst->avgwd10df);

      // 计算最近数据的时间、分钟记录总数和整点记录的总数
      AWSDAYD.GetLTimeCount(strMinTName,strHourTName);
/*
      // 如果是深圳市自动站数据，还需要计算最小能见度及其出现时间和相对湿度
      if (strcmp(strDType,"LOCAL") == 0)
      {
        AWSDAYD.GetMathValue("T_LOCALOBTV@SZQX_10_153_98_5" ,"v","max",pst->maxv);
        AWSDAYD.GetMathValue("T_LOCALOBTV@SZQX_10_153_98_5" ,"v","min",pst->minv);

        if ( (atoi(pst->maxv) == 0) || (atoi(pst->minv) == 0) )
        {
          memset(pst->maxv,0,sizeof(pst->maxv));
          memset(pst->maxvtime,0,sizeof(pst->maxvtime));
          memset(pst->minv,0,sizeof(pst->minv));
          memset(pst->minvtime,0,sizeof(pst->minvtime));
          memset(pst->minvtimeu,0,sizeof(pst->minvtimeu));
          memset(pst->avgv,0,sizeof(pst->avgv));
        }
        else
        {
          AWSDAYD.GetRecordDate("T_LOCALOBTV@SZQX_10_153_98_5","v",pst->maxv,pst->maxvtime);
          AWSDAYD.GetRecordDate("T_LOCALOBTV@SZQX_10_153_98_5","v",pst->minv,pst->minvtime);
          AWSDAYD.GetMinVTimeU("V_LOCALOBTMIND@SZQX_10_153_98_5",pst->minvtime,pst->minvtimeu);
          AWSDAYD.GetAvgValue("T_LOCALOBTV@SZQX_10_153_98_5","v",pst->avgv);
        }
      }

*/
      // 把结果更新到日数据表中
      if (AWSDAYD.UPTDAYD() != 0) { logfile.Write("call AWSDAYD.UPTDAYD failed.\n"); return -1; }

      conn.commitwork();
    }

  }

  return 0;
}

