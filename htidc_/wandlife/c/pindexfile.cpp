#include "_public.h"
#include "_oracle.h"
#include "wandlife.h"

void CallQuit(int sig);

connection conn;
CLogFile   logfile;
CZHCITYCODE  ZHCITYCODE;
CINDEXINFO INDEXINFO;

char strIndexPath[301];

// 把m_XMLBuffer中的2bcf、2b31等内容替换为空
BOOL UpdateXML2xxx(char *strXMLBuffer);

// 获取该城市的生活指数，并更新到数据库的T_INDEXINFO表中
BOOL HttpDminData(struct st_ZHCITYCODE *stZHCITYCODE);

// 从html的内容中解析省、地区和城镇
BOOL SplitFromBuffer_ProvAreaCity(char *strHtmlBuffer,char *IndexName,char *strProvAreaCity);

// 从html的内容中解析发布时间
BOOL SplitFromBuffer_PubDate(char *strHtmlBuffer,char *IndexName,char *strPubDate);

// 从html的内容中解析出日出日落
BOOL SplitFromBuffer_RCRL(char *strHtmlBuffer,char *IndexName,char *strRC,char *strRL);

// 从html的内容中解析出生活指数
BOOL SplitFromBuffer_ZS(char *strHtmlBuffer,char *IndexName,char *strOutPut);

int main(int argc,char *argv[])
{
  if (argc != 5)
  {
    printf("\n");
    printf("Using:/htidc/wandlife/bin/pindexfile logfile username/passwd@tnsname indexpath citytype\n");

    printf("Example:/htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_0.log wandlife/pwdidc /qxdata/wandlife/sdata/index 0\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_1.log wandlife/pwdidc /qxdata/wandlife/sdata/index 1\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_2.log wandlife/pwdidc /qxdata/wandlife/sdata/index 2\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 3\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 4\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 5\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 6\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 7\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 8\n");
    printf("        /htidc/htidc/bin/procctl 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 9\n");

    return -1;
  }

  memset(strIndexPath,0,sizeof(strIndexPath));
  strncpy(strIndexPath,argv[3],300);
 
  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止此进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); 
  signal(SIGTERM,CallQuit);   // 按ctl+c
  signal(SIGINT,CallQuit);    // kill 或 killall 

  CProgramActive ProgramActive;

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"hh24");
  if (strstr("08,09,11,12,18,19",strLocalTime) == 0) return 0;

  //打开告警
  logfile.SetAlarmOpt("pindexfile");

  // 注意，程序超时是180秒
  ProgramActive.SetProgramInfo(&logfile,"pindexfile",180);

  if (conn.connecttodb(argv[2])!=0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); return -1;
  }

  ZHCITYCODE.BindConnLog(&conn,&logfile);
  INDEXINFO.BindConnLog(&conn,&logfile);

  // 加载全部的城市参数
  if (ZHCITYCODE.LoadCityCode() == FALSE) return -1;

  // 逐行获取城市
  for (UINT ii=0;ii<ZHCITYCODE.m_vZHCITYCODE.size();ii++)
  {
    if (ZHCITYCODE.m_vZHCITYCODE[ii].cityid[5]!=argv[4][0]) continue;

    // 写入进程活动信息
    ProgramActive.WriteToFile();

    memset(&ZHCITYCODE.m_stZHCITYCODE,0,sizeof(struct st_ZHCITYCODE));
    memcpy(&ZHCITYCODE.m_stZHCITYCODE,&ZHCITYCODE.m_vZHCITYCODE[ii],sizeof(struct st_ZHCITYCODE));

    // 获取该城市的生活指数，并更新到数据库的T_INDEXINFO表中
    HttpDminData(&ZHCITYCODE.m_stZHCITYCODE);
  }
 
  
  return 0;
}

void CallQuit(int sig)
{
  signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
 
  logfile.Write("pindexfile exit.\n");
 
  exit(0);
}

// 获取该城市的生活指数，并更新到数据库的T_INDEXINFO表中
BOOL HttpDminData(struct st_ZHCITYCODE *stZHCITYCODE)
{
  char strHttpUrl[512];
  memset(strHttpUrl,0,sizeof(strHttpUrl));
  snprintf(strHttpUrl,500,"http://www.weather.com.cn/weather/101%s.shtml",stZHCITYCODE->cityid);

  char strCMD[1024];
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,1000,"/htidc/htidc/c/httpclient \"%s\" %s/%s.tmp %s/%s.shtml %s/%s.log",strHttpUrl,strIndexPath,stZHCITYCODE->cityid,strIndexPath,stZHCITYCODE->cityid,strIndexPath,stZHCITYCODE->cityid);
  system(strCMD);
  
  char strIndexFileName[1024];
  memset(strIndexFileName,0,sizeof(strIndexFileName));
  snprintf(strIndexFileName,300,"%s/%s.shtml",strIndexPath,stZHCITYCODE->cityid);

  CIniFile IniFile;

  if (IniFile.LoadFile(strIndexFileName) == FALSE) return FALSE;

  // 从html的内容中解析出生活指数
  memset(&INDEXINFO.m_stINDEXINFO,0,sizeof(struct st_INDEXINFO));

  strcpy(INDEXINFO.m_stINDEXINFO.cityid,stZHCITYCODE->cityid);

  logfile.WriteEx("城市 %s\n",stZHCITYCODE->cityid);

  // 把m_XMLBuffer中的2bcf、2b31等内容替换为空
  UpdateXML2xxx(IniFile.m_XMLBuffer);

  // 从html的内容中解析省、地区和城镇
  SplitFromBuffer_ProvAreaCity(IniFile.m_XMLBuffer,"urlSTATIONNAME",INDEXINFO.m_stINDEXINFO.urlstationname);
  SplitFromBuffer_ProvAreaCity(IniFile.m_XMLBuffer,"urlCITY",INDEXINFO.m_stINDEXINFO.urlcity);
  SplitFromBuffer_ProvAreaCity(IniFile.m_XMLBuffer,"urlPROVINCE",INDEXINFO.m_stINDEXINFO.urlprovince);

  // 从html的内容中解析发布时间
  if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"日生活指数",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
    if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"日活指数",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
      if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"日生指数",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
        if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"明生指数",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
          if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"明生活指数",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
            SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"今生活指数",INDEXINFO.m_stINDEXINFO.pubdate);

  logfile.WriteEx("发布时间 %s\n",INDEXINFO.m_stINDEXINFO.pubdate);

  // 获取日出日落
  SplitFromBuffer_RCRL(IniFile.m_XMLBuffer,"今日日出日落时间",INDEXINFO.m_stINDEXINFO.jrrc,INDEXINFO.m_stINDEXINFO.jrrl);
  SplitFromBuffer_RCRL(IniFile.m_XMLBuffer,"明日日出日落时间",INDEXINFO.m_stINDEXINFO.mrrc,INDEXINFO.m_stINDEXINFO.mrrl);

  logfile.WriteEx("今日日出日落 %s %s \n明日日出日落 %s %s\n",\
                   INDEXINFO.m_stINDEXINFO.jrrc,INDEXINFO.m_stINDEXINFO.jrrl,\
                   INDEXINFO.m_stINDEXINFO.mrrc,INDEXINFO.m_stINDEXINFO.mrrl);

  // 获取各种生活指数
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"息斯敏过敏气象指数",INDEXINFO.m_stINDEXINFO.gm);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"空气污染扩散条件指数",INDEXINFO.m_stINDEXINFO.wrks);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"中暑指数",INDEXINFO.m_stINDEXINFO.zs);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"紫外线指数",INDEXINFO.m_stINDEXINFO.zwx);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"穿衣指数",INDEXINFO.m_stINDEXINFO.cy);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"舒适度指数",INDEXINFO.m_stINDEXINFO.ssd);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"化妆指数",INDEXINFO.m_stINDEXINFO.hz);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"美发指数",INDEXINFO.m_stINDEXINFO.mf);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"洗车指数",INDEXINFO.m_stINDEXINFO.xc);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"路况指数",INDEXINFO.m_stINDEXINFO.lk);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"交通指数",INDEXINFO.m_stINDEXINFO.jt);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"旅游指数",INDEXINFO.m_stINDEXINFO.ly);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"运动指数",INDEXINFO.m_stINDEXINFO.yd);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"晨练指数",INDEXINFO.m_stINDEXINFO.cl);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"钓鱼指数",INDEXINFO.m_stINDEXINFO.dy);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"划船指数",INDEXINFO.m_stINDEXINFO.hc);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"约会指数",INDEXINFO.m_stINDEXINFO.yh);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"逛街指数",INDEXINFO.m_stINDEXINFO.gj);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"晾晒指数",INDEXINFO.m_stINDEXINFO.ls);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"雨伞指数",INDEXINFO.m_stINDEXINFO.ys);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"感冒指数",INDEXINFO.m_stINDEXINFO.ganmao);
  logfile.WriteEx("\n\n");

  INDEXINFO.UptIndexInfo();

  return TRUE;
}

// 从html的内容中解析出生活指数
BOOL SplitFromBuffer_ZS(char *strHtmlBuffer,char *IndexName,char *strOutPut)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // 起始字符串
  snprintf(strStartStr,300,"%s：" ,IndexName);
  // 结束字符串
  snprintf(strEndStr  ,300,"</dd>");

  // 获取指某指数的Buffer
  char *pos,*start,*end;
  pos=start=end=0;
  start=strstr(strHtmlBuffer,strStartStr);
  if (start==0) return FALSE;
  end=strstr(start+strlen(strStartStr),strEndStr);
  if (end==0) return FALSE;

  if ((end-start) > 500) return FALSE;

  char strBuffer[512];
  memset(strBuffer,0,sizeof(strBuffer));
  strncpy(strBuffer,start,end-start);
  //UpdateStr(strBuffer,"xxxx","");
  
  // 获取指数的级别
  pos=start=end=0;
  start=strstr(strBuffer,">");
  if (start==0) return FALSE;
  end=strstr(start,"<");
  if (end==0) return FALSE;
  if ((end-start) > 50) return FALSE;

  char strIdxLevel[51];
  memset(strIdxLevel,0,sizeof(strIdxLevel));
  strncpy(strIdxLevel,start+1,end-start-1);
  //UpdateStr(strIdxLevel,"xxxx","");

  
  // 获取指数的说明
  char strIdxMemo[1024];
  memset(strIdxMemo,0,sizeof(strIdxMemo));
  GetXMLBuffer(strBuffer,"blockquote",strIdxMemo,800);

  //UpdateStr(strIdxMemo,"不很舒适。","不是很舒适。");
  //UpdateStr(strIdxMemo,"，可放心出，","，可放心外出，");
  
  logfile.WriteEx("%s\n%s\n%s\n",IndexName,strIdxLevel,strIdxMemo);

  snprintf(strOutPut,1000,"%s|%s",strIdxLevel,strIdxMemo);
  
  return TRUE;
}

// 从html的内容中解析出日出日落
BOOL SplitFromBuffer_RCRL(char *strHtmlBuffer,char *IndexName,char *strRC,char *strRL)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // 起始字符串
  snprintf(strStartStr,300,"<dt><a>%s</a></dt>",IndexName);
  // 结束字符串
  snprintf(strEndStr  ,300,"</dd>");

  // 获取指某指数的Buffer
  char *pos,*start,*end;
  pos=start=end=0;
  start=strstr(strHtmlBuffer,strStartStr);
  if (start==0) return FALSE;
  end=strstr(start+strlen(strStartStr),strEndStr);
  if (end==0) return FALSE;

  if ((end-start) > 500) return FALSE;

  char strBuffer[512];
  memset(strBuffer,0,sizeof(strBuffer));
  strncpy(strBuffer,start,end-start);
  //UpdateStr(strBuffer,"xxxx","");

  // 获取日出
  pos=strstr(strBuffer,"</strong>");
  if (pos==0) return FALSE;
  strncpy(strRC,pos-5,5);

  // 获取日落
  pos=strstr(pos+5,"</strong>");
  if (pos==0) return FALSE;
  strncpy(strRL,pos-5,5);

  return TRUE;
}

// 从html的内容中解析发布时间
BOOL SplitFromBuffer_PubDate(char *strHtmlBuffer,char *IndexName,char *strPubDate)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // 起始字符串
  snprintf(strStartStr,300,"%s(",IndexName);
  // 结束字符串
  snprintf(strEndStr  ,300,"发");

  // 获取指某指数的Buffer
  char *pos,*start,*end;
  pos=start=end=0;
  start=strstr(strHtmlBuffer,strStartStr);
  if (start==0) return FALSE;
  /*
  end=strstr(start+strlen(strStartStr),strEndStr);
  if (end==0) return FALSE;

  if ((end-start) > 500) return FALSE;

  char strBuffer[512];
  memset(strBuffer,0,sizeof(strBuffer));
  strncpy(strBuffer,start+strlen(strStartStr),end-start-strlen(strStartStr));
  //UpdateStr(strBuffer,"xxxx","");
  */
  pos=strstr(start+strlen(strStartStr),"20");
  if (pos==0) return FALSE;
  char strBuffer[512];
  memset(strBuffer,0,sizeof(strBuffer));
  strncpy(strBuffer,pos,16);

  snprintf(strPubDate,20,"%s:00",strBuffer);

  return TRUE;
}

// 把m_XMLBuffer中的2bcf、2b31等内容替换为空
BOOL UpdateXML2xxx(char *strXMLBuffer)
{
  //UpdateStr(strXMLBuffer,"&nbsp;"," ");
  //UpdateStr(strXMLBuffer,"2d4a","");
  //UpdateStr(strXMLBuffer,"2d97","");

  for (UINT ii=0;ii<strlen(strXMLBuffer);ii++)
  {
    if ( (strXMLBuffer[ii]=='1') || (strXMLBuffer[ii]=='2') )
    {
      if ( (isalpha(strXMLBuffer[ii+1])==0) && (isdigit(strXMLBuffer[ii+1])==0) ) continue;
      if ( (isalpha(strXMLBuffer[ii+2])==0) && (isdigit(strXMLBuffer[ii+2])==0) ) continue;
      if ( (isalpha(strXMLBuffer[ii+3])==0) && (isdigit(strXMLBuffer[ii+3])==0) ) continue;

      if ((isalpha(strXMLBuffer[ii+1])!=0)||(isalpha(strXMLBuffer[ii+2])!=0)||(isalpha(strXMLBuffer[ii+3])!=0))
      {
        strXMLBuffer[ii]='x'; strXMLBuffer[ii+1]='x'; strXMLBuffer[ii+2]='x'; strXMLBuffer[ii+3]='x';
      }

      if ( (strXMLBuffer[ii+1]=='7') && (isdigit(strXMLBuffer[ii+2])!=0) && (isdigit(strXMLBuffer[ii+3])!=0) )
      {
        strXMLBuffer[ii]='x'; strXMLBuffer[ii+1]='x'; strXMLBuffer[ii+2]='x'; strXMLBuffer[ii+3]='x';
      }

      if ( (strXMLBuffer[ii+1]=='5') && (isdigit(strXMLBuffer[ii+2])!=0) && (isdigit(strXMLBuffer[ii+3])!=0) )
      {
        strXMLBuffer[ii]='x'; strXMLBuffer[ii+1]='x'; strXMLBuffer[ii+2]='x'; strXMLBuffer[ii+3]='x';
      }
    }
  }

  UpdateStr(strXMLBuffer,"xxxx","");

  return TRUE;
}

// 从html的内容中解析省、地区和城镇
BOOL SplitFromBuffer_ProvAreaCity(char *strHtmlBuffer,char *IndexName,char *strProvAreaCity)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // 起始字符串
  snprintf(strStartStr,300,"%s=\"",IndexName);
  // 结束字符串
  snprintf(strEndStr  ,300,"\"");

  // 获取指某指数的Buffer
  char *pos,*start,*end;
  pos=start=end=0;
  start=strstr(strHtmlBuffer,strStartStr);
  if (start==0) return FALSE;
  end=strstr(start+strlen(strStartStr),strEndStr);
  if (end==0) return FALSE;

  if ((end-start) > 500) return FALSE;

  char strBuffer[512];
  memset(strBuffer,0,sizeof(strBuffer));
  strncpy(strBuffer,start+strlen(strStartStr),end-start-strlen(strStartStr));
  strncpy(strProvAreaCity,strBuffer,20);

  return TRUE;
}



/*
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_0.log wandlife/pwdidc /qxdata/wandlife/sdata/index 0 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_1.log wandlife/pwdidc /qxdata/wandlife/sdata/index 1 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_2.log wandlife/pwdidc /qxdata/wandlife/sdata/index 2 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 3 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_4.log wandlife/pwdidc /qxdata/wandlife/sdata/index 4 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_5.log wandlife/pwdidc /qxdata/wandlife/sdata/index 5 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_6.log wandlife/pwdidc /qxdata/wandlife/sdata/index 6 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_7.log wandlife/pwdidc /qxdata/wandlife/sdata/index 7 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_8.log wandlife/pwdidc /qxdata/wandlife/sdata/index 8 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_9.log wandlife/pwdidc /qxdata/wandlife/sdata/index 9 &

select zwx,count(*) from T_INDEXINFO group by zwx order by zwx;
select ssd,count(*) from T_INDEXINFO group by ssd order by ssd;
select cy,count(*) from T_INDEXINFO group by cy order by cy;
select ly,count(*) from T_INDEXINFO group by ly order by ly;
select yd,count(*) from T_INDEXINFO group by yd order by yd;
select ls,count(*) from T_INDEXINFO group by ls order by ls;
select ys,count(*) from T_INDEXINFO group by ys order by ys;
select xc,count(*) from T_INDEXINFO group by xc order by xc;
*/
