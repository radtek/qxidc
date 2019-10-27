#include "_public.h"
#include "_oracle.h"

void CallQuit(int sig);

#define MAXFIELDCOUNT  200  // �ֶε������
#define MAXFIELDLEN   2000  // �ֶ�ֵ����󳤶�

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

// ��ȡ�����ɼ���־�ֶε�ֵ��ŵ��ļ������������incfieldvalue_old�����С�
BOOL ReadIncFile();

// �������ֶε�ֵд���ļ���
BOOL WriteIncFile();

// �ж��Ƿ���Ҫ����xmlbuffer�е�ʱ�����
//void MatchXMLBuffer();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/dminingoracle logfilename xmlbuffer\n\n");
    printf("Sample:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/dminingoracle /tmp/htidc/log/dminingoracle_OBTHOURD_HYCZ.log \"<charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>sqxt/pwdidc@SZQX_10.148.124.85</connstr><firstsql>truncate table T_OBTHOURD</firstsql><selectsql>select to_char(ddatetime,'yyyymmddhh24miss'),obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,temp,maxtemp,maxttime,mintemp,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid from T_OBTHOURD where ddatetime<to_date('2010-12-01 00:00:00','yyyy-mm-dd hh24:mi:ss')</selectsql><fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,temp,maxtemp,maxttime,mintemp,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr><fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen><bfilename>T_OBTHOURD</bfilename><efilename>HYCZ</efilename><outpathtmp>/tmp/htidc/tmp</outpathtmp><outpath>/tmp/htidc/ftpput</outpath><starttime>01,12</starttime><endsql></endsql><timetvl>-8</timetvl><incfield>keyid</incfield><incfilename>/tmp/htidc/list/dminingoracle_OBTHOURD_HYCZ.list</incfilename>\"\n\n");

    printf("���������������ĵĹ�������ģ�飬���ڴ�oracle���ݿ�Դ��ɼ����ݣ��������xml�ļ���/usr/bin/gzip����ѹ����\n");
    printf("logfilename�Ǳ��������е���־�ļ���\n");
    printf("xmlbufferΪ�����ھ�Ĳ��������£�\n");

    printf("���ݿ���ַ��� <charset>Simplified Chinese_China.ZHS16GBK</charset> �������Ҫ������Դ���ݿⱣ��һ�£�����������������������\n");
    printf("���ݿ�����Ӳ��� <connstr>sqxt/pwdidc@SZQX_10.148.124.85</connstr>\n");
    printf("xml�ļ��ײ���SQL��䣬�������ڴ�������֮ǰ��ִ�����SQL <firstsql>truncate table T_OBTHOURD</firstsql>\n");
    printf("������Դ���ݿ���ȡ���ݵ�SQL <selectsql>select to_char(ddatetime,'yyyymmddhh24miss'),obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid from T_OBTHOURD where ddatetime<to_date('2010-12-01 00:00:00','yyyy-mm-dd hh24:mi:ss')</selectsql>\n");
    printf("��ȡ���������ݶ�Ӧ��xml�ֶ��� <fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr>\n");
    printf("ÿ��xml�ֶεĳ��� <fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen> ���fieldlen�ֶ�Ϊ�գ��Ͳ���MAXFIELDLEN�ĳ��ȡ�\n");
    printf("xml�ļ���ǰ׺ <bfilename>OBTHOURD</bfilename>\n");
    printf("xml�ļ��ĺ�׺ <efilename>HYCZ</efilename>\n");
    printf("xml�ļ����������ʱĿ¼ <outpathtmp>/tmp/htidc/tmp</outpathtmp> ���outpathtmpΪ�գ�ȱʡ��outpath��䡣\n");
    printf("xml�ļ��������Ŀ¼ <outpath>/tmp/htidc/ftpput</outpath> ע�⣬outpath��outpathtmpһ��Ҫ��ͬһ���ļ������������ļ��ƶ�ʱ��ʧ�ܡ�\n");
    printf("����������ʼʱ�� <starttime>02,10,20</starttime>��֧�ֶ��ʱ�䣬�м��ö��ŷָ�������02,10,20��ʾ��ÿ���02��10��20��������\n");
    printf("ע�⣬���starttimeΪ�գ���ôstarttime������ʧЧ��ֻҪ�����������ͻ�ִ�����ݲɼ���Ϊ�˼�������Դ"\
           "��ѹ���������ݿ�ɼ����ݵ�ʱ��һ���ڶԷ����ݿ����е�ʱ��ʱ���У�"\
           "���Ǵ��ļ��вɼ����ݲ��������������\n");
    printf("���ݲɼ���ɺ�ִ�е�SQL�ű� <endsql>delete from T_OBTHOURD</endsql>��ÿ�βɼ������ݺ���Դ���ݿ���ִ�е�SQL�ű����ò���Ҫ���á�\n");
    printf("xmlbuffer���Դ���ʱ�������<timetvl>-8</timetvl> Ϊʱ�������ƫ��ʱ�䡣"\
           "Ŀǰ���Դ�������ʱ�������{YYYY}��4λ���꣩��{YYY}������λ���꣩��"\
           "{YY}������λ���꣩��{MM}�����£���{DD}�����գ���{HH}��ʱʱ����{MI}���ַ֣���{SS}�����룩��\n");
    printf("�����ɼ��ֶα�־ <incfield>keyid</incfield> ��������fieldstr�е��ֶ���������ֻ����������\n");
    printf("�����ɼ���־�ֶε�ֵ��ŵ��ļ� <incfilename>/tmp/htidc/list/dminingoracle_OBTHOURD_HYCZ.list</incfilename> ÿ�βɼ������ݺ�incfield�����ֵ������ڴ��ļ��С�\n");
    printf("firstsql��fieldlen��outpathtmp��starttime��endsql��timetvl��incfield��incfilename����Ϊ�գ�����ֻ�ֶζ�������Ϊ�գ������������Ϊ�գ�����ᱨ���˳���\n\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  xmlfp=0;

  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strXMLFileName,0,sizeof(strXMLFileName));

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],5000);

  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
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

  // ����xmlbuffer�е�ʱ�����
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

  // firstsql��fieldlen��outpathtmp��starttime��endsql��timetvl��incfield��incfilename����Ϊ�գ���
  // ��ֻ�ֶζ�������Ϊ�գ������������Ϊ�գ�����ᱨ���˳���
  if (strlen(strcharset) == 0) { logfile.Write("charset is null.\n"); return -1; }
  if (strlen(strconnstr) == 0) { logfile.Write("connstr is null.\n"); return -1; }
  if (strlen(strselectsql) == 0) { logfile.Write("selectsql is null.\n"); return -1; }
  if (strlen(strfieldstr) == 0) { logfile.Write("fieldstr is null.\n"); return -1; }
  if (strlen(strbfilename) == 0) { logfile.Write("bfilename is null.\n"); return -1; }
  if (strlen(strefilename) == 0) { logfile.Write("efilename is null.\n"); return -1; }
  if (strlen(stroutpathtmp) == 0) { strcpy(stroutpathtmp,stroutpath); }
  if (strlen(stroutpath) == 0) { logfile.Write("outpath is null.\n"); return -1; }

  // ��ȡ��������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬���˳�
  if (strlen(strstarttime) != 0)
  {
    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"hh24mi");
    strLocalTime[2]=0;
    if (strstr(strstarttime,strLocalTime) == 0) return 0;
  }

  // ��ȡ�����ɼ���־�ֶε�ֵ��ŵ��ļ������������incfieldvalue_old�����С�
  if (ReadIncFile() == FALSE) return -1;

  // 20190808 ����������Σ���Ȼ�޷�ʵ�������ɼ����ݡ�
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

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"dminingoracle",1200);

  // ��xml���ֶ������ֶεĳ�����CCmdStr���ֿ�
  CCmdStr fieldstr,fieldlen;
  fieldstr.SplitToCmd(strfieldstr,",");
  fieldlen.SplitToCmd(strfieldlen,",");

  if (strlen(strfieldlen) != 0)
  {
    // ���xml���ֶ������ֶεĳ��ȵĸ�����ͬ��һ���ǲ�������ˣ������˳�
    if ( (fieldstr.CmdCount()==0) || (fieldstr.CmdCount() != fieldlen.CmdCount()) )
    {
      logfile.Write("fieldstr(%d) or fieldlen(%d) is invalid.\n",fieldstr.CmdCount(),fieldlen.CmdCount()); return -1;
    }
  }

  // xml���ֶ������ܳ���MAXFIELDCOUNT��
  if (fieldstr.CmdCount()>MAXFIELDCOUNT) 
  {
    logfile.Write("fields is to many,max is %d.\n",MAXFIELDCOUNT); return -1;
  }
  
  // ��xml�ֶ������ֶγ�����Ϣ��ֵ�������
  char strfieldname[MAXFIELDCOUNT][31];    // xml�ֶ�������
  int  ifieldlen[MAXFIELDCOUNT];   // xml�ֶγ�������
  char strfieldvalue[MAXFIELDCOUNT][MAXFIELDLEN+1]; // ���ڴ���ֶ�ֵ������
  UINT ii;

  memset(strfieldname,0,sizeof(strfieldname));
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  memset(&ifieldlen,0,sizeof(ifieldlen));

  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    fieldstr.GetValue(ii,strfieldname[ii],30);
    if (strlen(strfieldlen) != 0) fieldlen.GetValue(ii,&ifieldlen[ii]);
    if (strlen(strfieldlen) == 0) ifieldlen[ii]=MAXFIELDLEN;

    // ���������ֶε�λ��
    if ( (strlen(strincfield)>0) && (strcmp(strfieldname[ii],strincfield)==0) ) incfieldpos=ii;
  }

  if ( (strlen(strincfield)>0) && (incfieldpos==-1) ) { logfile.Write("check incfield(%s) failed.not in fieldstr.\n",strincfield); return -1;}

  setenv("NLS_LANG",strcharset,1);

  // ��������Դ���ݿ�
  if (connsrc.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connsrc.connecttodb(%s) failed\n",strconnstr); CallQuit(-1);
  }

  // д����̻��Ϣ
  ProgramActive.WriteToFile();

  // �ɼ����ݵĿ�ʼʱ��ͽ���ʱ��
  char strBeginTime[21],strEndTime[21]; 

  memset(strBeginTime,0,sizeof(strBeginTime));
  memset(strEndTime,0,sizeof(strEndTime));

  // ��ȡ�ɼ����ݵĿ�ʼʱ��
  LocalTime(strBeginTime,"yyyymmddhh24miss");

  snprintf(strTmpFileName,300,"%s/%s_%s_%d.xml.tmp",stroutpathtmp,strbfilename,strBeginTime,getpid());

  // ׼����ȡ���ݵ�SQL��䣬���������
  sqlstatement stmt;
  stmt.connect(&connsrc);
  stmt.prepare(strselectsql);
  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    stmt.bindout(ii+1,strfieldvalue[ii],ifieldlen[ii]);
  }

  logfile.Write("begin exec sql.\n");

  logfile.WriteEx("%s\n%s\n%s\n",strselectsql,strfieldstr,strfieldlen);

  // ִ�вɼ����ݵ�SQL���
  if (stmt.execute() != 0)
  {
    logfile.Write("%s\nexec sql failed.\n%s\n",strselectsql,stmt.cda.message); return stmt.cda.rc;
  }

  logfile.Write("exec sql(%s) ok.\n",stmt.m_sql);

  UINT iinext=10000;

  // ����SQL���ִ�к��ÿһ��
  while (TRUE)
  {
    memset(strfieldvalue,0,sizeof(strfieldvalue));

    if (stmt.next() != 0) break;

    // ÿ��ȡ10000����¼д��һ�ν��̻��Ϣ
    if (iinext++ > 10000)
    {
      iinext=0; ProgramActive.WriteToFile();
    }

    if (strlen(strincfield)>0)
    {
      // �������¼�������ֶε�ֵС����һ�βɼ������ֵ���Ͷ�����
      if (atol(strfieldvalue[incfieldpos]) < incfieldvalue_old) continue;

      // ��¼�����ֶ��µ����ֵ
      if (atol(strfieldvalue[incfieldpos])>incfieldvalue_new) incfieldvalue_new=atol(strfieldvalue[incfieldpos]);
    }

    // ��xml�ļ�
    if (xmlfp == 0)
    {
      if ( (xmlfp=FOPEN(strTmpFileName,"w+")) == 0 )
      {
        logfile.Write("FOPEN %s failed.\n",strTmpFileName); return -1;
      }

      if (strlen(strfirstsql) != 0) fprintf(xmlfp,"<sql>\n%s\n</sql><endl/>\n",strfirstsql);

      fprintf(xmlfp,"<data>\n");
    }

    // ��ÿ��xml�ֶ�����ֵд��xml�ļ�
    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      Trim(strfieldvalue[ii]); // ɾ���ֶ�ֵ���ߵĿո�
      fprintf(xmlfp,"<%s>%s</%s>",strfieldname[ii],strfieldvalue[ii],strfieldname[ii]);
    }

    fprintf(xmlfp,"<endl/>\n");

    totalcount++;
  }

  logfile.Write("rows %ld.\n",totalcount);

  // д����̻��Ϣ
  ProgramActive.WriteToFile();

  // �ر�xml�ļ���ѹ�������Ϊ���յ�xml�ļ���
  if (xmlfp != 0)
  {
    fprintf(xmlfp,"</data>"); fclose(xmlfp); xmlfp=0;

    // ѹ��xml�ļ�
    if (strcmp(strcompress,"TRUE")==0)
    {
      char strCmd[4001];
      memset(strCmd,0,sizeof(strCmd));
      snprintf(strCmd,4000,"/usr/bin/gzip -c %s > %s.tmp 2>/dev/null",strTmpFileName,strTmpFileName);
      system(strCmd);

      // ɾ��ѹ��ǰ��xml�ļ�
      REMOVE(strTmpFileName);

      strncat(strTmpFileName,".tmp",4);

      // ��ȡ�ɼ����ݵĽ���ʱ��
      LocalTime(strEndTime,"yyyymmddhh24miss");

      snprintf(strXMLFileName,300,"%s/%s_%s_%s_%s.xml.gz",stroutpath,strbfilename,strBeginTime,strEndTime,strefilename);

      // ���ļ�����Ϊ��ʽ��ѹ���ļ�
      if (RENAME(strTmpFileName,strXMLFileName) == FALSE)
      {
        logfile.Write("RENAME %s to %s failed.\n",strTmpFileName,strXMLFileName); return -1;
      }
    }
    else
    {
      // ��ȡ�ɼ����ݵĽ���ʱ��
      LocalTime(strEndTime,"yyyymmddhh24miss");

      snprintf(strXMLFileName,300,"%s/%s_%s_%s_%s.xml",stroutpath,strbfilename,strBeginTime,strEndTime,strefilename);

      // ���ļ�����Ϊ��ʽ��xml�ļ�
      if (RENAME(strTmpFileName,strXMLFileName) == FALSE)
      {
        logfile.Write("RENAME %s to %s failed.\n",strTmpFileName,strXMLFileName); return -1;
      }
    }

    logfile.Write("/usr/bin/gzip %s ok.\n",strXMLFileName);
  }

  if ( strlen(strendsql) != 0 )
  {
    // �ɼ���ɺ�ִ��endsql�ű���
    stmt.prepare(strendsql);
    if (stmt.execute() != 0)
    {
      logfile.Write("exec %s failed.\n%s\n",strendsql,stmt.cda.message); return -1;
    }
  
    logfile.Write("exec %s ok.\n",strendsql);
  
    connsrc.commitwork();
  }

  logfile.WriteEx("\n\n");

  // �������ֶε�ֵд���ļ���
  WriteIncFile();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // ��������쳣�˳����͹ر��ļ���������ɾ���м�״̬���ļ�
  if (xmlfp!=0) { fclose(xmlfp); REMOVE(strTmpFileName); }

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("dminingoracle exit.\n");

  exit(0);
}

/*
// �ж��Ƿ���Ҫ����xmlbuffer�е�ʱ�����
// ���Դ�������ʱ�������YYYY��4λ���꣩��YYY������λ���꣩��
// YY������λ���꣩��MM�����£���DD�����գ���HH��ʱʱ����MI���ַ֣���SS�����룩��ע�⣬������Ҫ���ô�д
// �����ĸ�ʽΪ{������}
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

// ��ȡ�����ɼ���־�ֶε�ֵ��ŵ��ļ������������incfieldvalue_old�����С�
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

// �������ֶε�ֵд���ļ���
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
