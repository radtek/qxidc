#include "_public.h"
#include "_oracle.h"
#include "wandlife.h"

void CallQuit(int sig);

connection conn;
CLogFile   logfile;
CZHCITYCODE  ZHCITYCODE;
CINDEXINFO INDEXINFO;

char strIndexPath[301];

// ��m_XMLBuffer�е�2bcf��2b31�������滻Ϊ��
BOOL UpdateXML2xxx(char *strXMLBuffer);

// ��ȡ�ó��е�����ָ���������µ����ݿ��T_INDEXINFO����
BOOL HttpDminData(struct st_ZHCITYCODE *stZHCITYCODE);

// ��html�������н���ʡ�������ͳ���
BOOL SplitFromBuffer_ProvAreaCity(char *strHtmlBuffer,char *IndexName,char *strProvAreaCity);

// ��html�������н�������ʱ��
BOOL SplitFromBuffer_PubDate(char *strHtmlBuffer,char *IndexName,char *strPubDate);

// ��html�������н������ճ�����
BOOL SplitFromBuffer_RCRL(char *strHtmlBuffer,char *IndexName,char *strRC,char *strRL);

// ��html�������н���������ָ��
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
 
  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹ�˽���
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); 
  signal(SIGTERM,CallQuit);   // ��ctl+c
  signal(SIGINT,CallQuit);    // kill �� killall 

  CProgramActive ProgramActive;

  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"hh24");
  if (strstr("08,09,11,12,18,19",strLocalTime) == 0) return 0;

  //�򿪸澯
  logfile.SetAlarmOpt("pindexfile");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"pindexfile",180);

  if (conn.connecttodb(argv[2])!=0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); return -1;
  }

  ZHCITYCODE.BindConnLog(&conn,&logfile);
  INDEXINFO.BindConnLog(&conn,&logfile);

  // ����ȫ���ĳ��в���
  if (ZHCITYCODE.LoadCityCode() == FALSE) return -1;

  // ���л�ȡ����
  for (UINT ii=0;ii<ZHCITYCODE.m_vZHCITYCODE.size();ii++)
  {
    if (ZHCITYCODE.m_vZHCITYCODE[ii].cityid[5]!=argv[4][0]) continue;

    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    memset(&ZHCITYCODE.m_stZHCITYCODE,0,sizeof(struct st_ZHCITYCODE));
    memcpy(&ZHCITYCODE.m_stZHCITYCODE,&ZHCITYCODE.m_vZHCITYCODE[ii],sizeof(struct st_ZHCITYCODE));

    // ��ȡ�ó��е�����ָ���������µ����ݿ��T_INDEXINFO����
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

// ��ȡ�ó��е�����ָ���������µ����ݿ��T_INDEXINFO����
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

  // ��html�������н���������ָ��
  memset(&INDEXINFO.m_stINDEXINFO,0,sizeof(struct st_INDEXINFO));

  strcpy(INDEXINFO.m_stINDEXINFO.cityid,stZHCITYCODE->cityid);

  logfile.WriteEx("���� %s\n",stZHCITYCODE->cityid);

  // ��m_XMLBuffer�е�2bcf��2b31�������滻Ϊ��
  UpdateXML2xxx(IniFile.m_XMLBuffer);

  // ��html�������н���ʡ�������ͳ���
  SplitFromBuffer_ProvAreaCity(IniFile.m_XMLBuffer,"urlSTATIONNAME",INDEXINFO.m_stINDEXINFO.urlstationname);
  SplitFromBuffer_ProvAreaCity(IniFile.m_XMLBuffer,"urlCITY",INDEXINFO.m_stINDEXINFO.urlcity);
  SplitFromBuffer_ProvAreaCity(IniFile.m_XMLBuffer,"urlPROVINCE",INDEXINFO.m_stINDEXINFO.urlprovince);

  // ��html�������н�������ʱ��
  if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"������ָ��",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
    if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"�ջ�ָ��",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
      if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
        if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
          if (SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"������ָ��",INDEXINFO.m_stINDEXINFO.pubdate) == FALSE)
            SplitFromBuffer_PubDate(IniFile.m_XMLBuffer,"������ָ��",INDEXINFO.m_stINDEXINFO.pubdate);

  logfile.WriteEx("����ʱ�� %s\n",INDEXINFO.m_stINDEXINFO.pubdate);

  // ��ȡ�ճ�����
  SplitFromBuffer_RCRL(IniFile.m_XMLBuffer,"�����ճ�����ʱ��",INDEXINFO.m_stINDEXINFO.jrrc,INDEXINFO.m_stINDEXINFO.jrrl);
  SplitFromBuffer_RCRL(IniFile.m_XMLBuffer,"�����ճ�����ʱ��",INDEXINFO.m_stINDEXINFO.mrrc,INDEXINFO.m_stINDEXINFO.mrrl);

  logfile.WriteEx("�����ճ����� %s %s \n�����ճ����� %s %s\n",\
                   INDEXINFO.m_stINDEXINFO.jrrc,INDEXINFO.m_stINDEXINFO.jrrl,\
                   INDEXINFO.m_stINDEXINFO.mrrc,INDEXINFO.m_stINDEXINFO.mrrl);

  // ��ȡ��������ָ��
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"Ϣ˹����������ָ��",INDEXINFO.m_stINDEXINFO.gm);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"������Ⱦ��ɢ����ָ��",INDEXINFO.m_stINDEXINFO.wrks);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.zs);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"������ָ��",INDEXINFO.m_stINDEXINFO.zwx);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.cy);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"���ʶ�ָ��",INDEXINFO.m_stINDEXINFO.ssd);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"��ױָ��",INDEXINFO.m_stINDEXINFO.hz);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.mf);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"ϴ��ָ��",INDEXINFO.m_stINDEXINFO.xc);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"·��ָ��",INDEXINFO.m_stINDEXINFO.lk);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"��ָͨ��",INDEXINFO.m_stINDEXINFO.jt);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.ly);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"�˶�ָ��",INDEXINFO.m_stINDEXINFO.yd);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.cl);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.dy);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"����ָ��",INDEXINFO.m_stINDEXINFO.hc);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"Լ��ָ��",INDEXINFO.m_stINDEXINFO.yh);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"���ָ��",INDEXINFO.m_stINDEXINFO.gj);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"��ɹָ��",INDEXINFO.m_stINDEXINFO.ls);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"��ɡָ��",INDEXINFO.m_stINDEXINFO.ys);
  SplitFromBuffer_ZS(IniFile.m_XMLBuffer,"��ðָ��",INDEXINFO.m_stINDEXINFO.ganmao);
  logfile.WriteEx("\n\n");

  INDEXINFO.UptIndexInfo();

  return TRUE;
}

// ��html�������н���������ָ��
BOOL SplitFromBuffer_ZS(char *strHtmlBuffer,char *IndexName,char *strOutPut)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // ��ʼ�ַ���
  snprintf(strStartStr,300,"%s��" ,IndexName);
  // �����ַ���
  snprintf(strEndStr  ,300,"</dd>");

  // ��ȡָĳָ����Buffer
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
  
  // ��ȡָ���ļ���
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

  
  // ��ȡָ����˵��
  char strIdxMemo[1024];
  memset(strIdxMemo,0,sizeof(strIdxMemo));
  GetXMLBuffer(strBuffer,"blockquote",strIdxMemo,800);

  //UpdateStr(strIdxMemo,"�������ʡ�","���Ǻ����ʡ�");
  //UpdateStr(strIdxMemo,"���ɷ��ĳ���","���ɷ��������");
  
  logfile.WriteEx("%s\n%s\n%s\n",IndexName,strIdxLevel,strIdxMemo);

  snprintf(strOutPut,1000,"%s|%s",strIdxLevel,strIdxMemo);
  
  return TRUE;
}

// ��html�������н������ճ�����
BOOL SplitFromBuffer_RCRL(char *strHtmlBuffer,char *IndexName,char *strRC,char *strRL)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // ��ʼ�ַ���
  snprintf(strStartStr,300,"<dt><a>%s</a></dt>",IndexName);
  // �����ַ���
  snprintf(strEndStr  ,300,"</dd>");

  // ��ȡָĳָ����Buffer
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

  // ��ȡ�ճ�
  pos=strstr(strBuffer,"</strong>");
  if (pos==0) return FALSE;
  strncpy(strRC,pos-5,5);

  // ��ȡ����
  pos=strstr(pos+5,"</strong>");
  if (pos==0) return FALSE;
  strncpy(strRL,pos-5,5);

  return TRUE;
}

// ��html�������н�������ʱ��
BOOL SplitFromBuffer_PubDate(char *strHtmlBuffer,char *IndexName,char *strPubDate)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // ��ʼ�ַ���
  snprintf(strStartStr,300,"%s(",IndexName);
  // �����ַ���
  snprintf(strEndStr  ,300,"��");

  // ��ȡָĳָ����Buffer
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

// ��m_XMLBuffer�е�2bcf��2b31�������滻Ϊ��
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

// ��html�������н���ʡ�������ͳ���
BOOL SplitFromBuffer_ProvAreaCity(char *strHtmlBuffer,char *IndexName,char *strProvAreaCity)
{
  char strStartStr[51],strEndStr[51];
  memset(strStartStr,0,sizeof(strStartStr));
  memset(strEndStr,0,sizeof(strEndStr));

  // ��ʼ�ַ���
  snprintf(strStartStr,300,"%s=\"",IndexName);
  // �����ַ���
  snprintf(strEndStr  ,300,"\"");

  // ��ȡָĳָ����Buffer
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
