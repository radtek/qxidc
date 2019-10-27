#include "_public.h"

void CallQuit(int sig);

#define MAXFIELDCOUNT  200  // �ֶε������
#define MAXFIELDLEN   2000  // �ֶ�ֵ����󳤶�

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

    printf("���������������ĵĹ�������ģ�飬���ڴӱ��ļ�����Դ�ɼ����ݣ��������xml�ļ���/usr/bin/gzip����ѹ����\n");
    printf("logfilename�Ǳ��������е���־�ļ���\n");
    printf("xmlbufferΪ�����ھ�Ĳ��������£�\n");

    printf("����Դ�ļ����ļ��� <srcfilename>/tmp/swdata.csv</srcfilename> ע�⣬Ϊ�˷�ֹ�м�״̬���ļ�����������Դ�ļ�����Ҫ���ļ����ɵ�50���Ŵ���\n");
    printf("����Դ�ļ���¼���ֶ�֮��ķָ��� <splitstr>|</splitstr> �������ַ����ַ�����\n");
    printf("xml�ļ��ײ���SQL��䣬�������ڴ�������֮ǰ��ִ�����SQL <firstsql>truncate table T_OBTHOURD</firstsql>\n");
    printf("����Դ�ļ��ֶζ�Ӧ��xml�ֶ��� <fieldstr>ddatetime,obtid,wdidf,wdidd,wd2df,wd2dd,wd10df,wd10dd,wd3smaxdf,wd3smaxdd,wd3smaxtime,wd10maxdf,wd10maxdd,wd10maxtime,t,maxt,maxttime,mint,minttime,u,maxu,maxutime,minu,minutime,dp,p,maxp,maxptime,minp,minptime,p0,hourr,othfields,rddatetime,datatype,procsts,keyid</fieldstr>\n");
    printf("ÿ��xml�ֶεĳ��� <fieldlen>14,5,6,6,6,6,6,6,6,6,2,6,6,2,6,6,2,6,2,6,6,2,6,2,6,6,6,2,6,2,6,6,1000,2,1,1,1000,1,15</fieldlen>\n");
    printf("xml�ļ���ǰ׺ <bfilename>OBTHOURD</bfilename>\n");
    printf("xml�ļ��ĺ�׺ <efilename>HYSW</efilename>\n");
    printf("xml�ļ������Ŀ¼ <outpath>/tmp/htidc/ftpput</outpath>\n");
    printf("����������ʼʱ�� <starttime>02,10,20</starttime>��֧�ֶ��ʱ�䣬�м��ö��ŷָ�������02,10,20��ʾ��ÿ���02��10��20��������\n");
    printf("ע�⣬���starttimeΪ�գ���ôstarttime������ʧЧ��ֻҪ�����������ͻ�ִ�����ݲɼ���\n");
    printf("firstsql��outpathtmp��starttime����Ϊ�գ�����ֻ�ֶ�������Ϊ�գ������������Ϊ�գ������˳���\n\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  srcfp=xmlfp=0;

  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strXMLFileName,0,sizeof(strXMLFileName));

  char strXmlBuffer[4001];

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
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

  // firstsql��outpathtmp��starttime����Ϊ�գ�����ֻ�ֶ�������Ϊ�գ������������Ϊ�գ������˳�
  if (strlen(strsrcfilename) == 0) { logfile.Write("srcfilename is null.\n"); return -1; }
  if (strlen(strsplitstr) == 0) { logfile.Write("splitstr is null.\n"); return -1; }
  // if (strlen(strfirstsql) == 0) { logfile.Write("firstsql is null.\n"); return -1; }
  if (strlen(strfieldstr) == 0) { logfile.Write("fieldstr is null.\n"); return -1; }
  if (strlen(strfieldlen) == 0) { logfile.Write("fieldlen is null.\n"); return -1; }
  if (strlen(strbfilename) == 0) { logfile.Write("bfilename is null.\n"); return -1; }
  if (strlen(strefilename) == 0) { logfile.Write("efilename is null.\n"); return -1; }
  if (strlen(stroutpath) == 0) { logfile.Write("outpath is null.\n"); return -1; }
  // if (strlen(strstarttime) == 0) { logfile.Write("starttime is null.\n"); return -1; }

  // ��ȡ��������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬���˳�
  if (strlen(strstarttime) != 0)
  {
    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"hh24mi");
    strLocalTime[2]=0;
    if (strstr(strstarttime,strLocalTime) == 0) return 0;
  }

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"dminingtxt",1200);

  // ��xml���ֶ������ֶεĳ�����CCmdStr���ֿ�
  CCmdStr fieldstr,fieldlen;
  fieldstr.SplitToCmd(strfieldstr,",");
  fieldlen.SplitToCmd(strfieldlen,",");

  // ���xml���ֶ������ֶεĳ��ȵĸ�����ͬ��һ���ǲ�������ˣ������˳�
  if ( (fieldstr.CmdCount()==0) || (fieldstr.CmdCount() != fieldlen.CmdCount()) )
  {
    logfile.Write("fieldstr or fieldlen is invalid.\n"); return -1;
  }

  // xml���ֶ������ܳ���MAXFIELDCOUNT��
  if (fieldstr.CmdCount()>MAXFIELDCOUNT) 
  {
    logfile.Write("fields is to many,max is %d.\n",MAXFIELDCOUNT); return -1;
  }
  
  // ��xml�ֶ������ֶγ�����Ϣ��ֵ�������
  char strfieldname[MAXFIELDCOUNT][31];    // xml�ֶ�������
  int  ifieldlen[MAXFIELDCOUNT];   // xml�ֶγ�������
  char strfieldvalue[MAXFIELDLEN+1]; // ���ڴ���ֶ�ֵ
  UINT ii;

  memset(strfieldname,0,sizeof(strfieldname));
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  memset(&ifieldlen,0,sizeof(ifieldlen));

  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    fieldstr.GetValue(ii,strfieldname[ii],30);
    fieldlen.GetValue(ii,&ifieldlen[ii]);
  }

  // �ж�����Դ�ļ��Ƿ���ڣ���������ڣ�����
  if (access(strsrcfilename,R_OK) != 0) return 0;

  // �ж�����Դ�ļ�������ʱ�䣬�����50��֮�ڣ��Ͳ���������ļ�����Ϊ���ܿ������м�״̬���ļ�
  char strLocalTime[21],strFModTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strFModTime,0,sizeof(strFModTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss",0-50); // ��ǰʱ���50��֮ǰ��ʱ��
  // �ж��ļ���ʱ�䣬��modtime
  FileModTime(strsrcfilename,strFModTime);
  if (strcmp(strLocalTime,strFModTime) < 0) return 0;

  // ������Դ�ļ�
  if ( (srcfp=FOPEN(strsrcfilename,"r")) == 0 )
  {
    logfile.Write("FOPEN %s failed.\n",strsrcfilename); return -1;
  }
  
  // д����̻��Ϣ
  ProgramActive.WriteToFile();

  // �ɼ����ݵĿ�ʼʱ��ͽ���ʱ��
  char strBeginTime[21],strEndTime[21]; 

  memset(strBeginTime,0,sizeof(strBeginTime));
  memset(strEndTime,0,sizeof(strEndTime));

  // ��ȡ�ɼ����ݵĿ�ʼʱ��
  LocalTime(strBeginTime,"yyyymmddhh24miss");

  snprintf(strTmpFileName,300,"%s/%s_%s_%d.xml.tmp",stroutpath,strbfilename,strBeginTime,getpid());

  logfile.Write("begin process %s...\n",strsrcfilename);
  
  logfile.WriteEx("%s\n%s\n",strfieldstr,strfieldlen);

  char strLine[4096];
  CCmdStr CmdStr;

  UINT iinext=10000;
  
  UINT itotal,ivalid;
  
  itotal=ivalid=0;

  // ����SQL���ִ�к��ÿһ��
  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
    
    if (FGETS(strLine,4000,srcfp) == FALSE) break;
    
    itotal++;
    
    // ÿ��ȡ10000����¼д��һ�ν��̻��Ϣ
    if (iinext++ > 10000)
    {
      iinext=0; ProgramActive.WriteToFile();
    }
    
    CmdStr.SplitToCmd(strLine,strsplitstr);
    
    // ��Ч���н�����
    if (CmdStr.CmdCount() != fieldstr.CmdCount()) continue;
    
    ivalid++;

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
      memset(strfieldvalue,0,sizeof(strfieldvalue));
      CmdStr.GetValue(ii,strfieldvalue,ifieldlen[ii]);
      fprintf(xmlfp,"<%s>%s</%s>",strfieldname[ii],strfieldvalue,strfieldname[ii]);
    }

    fprintf(xmlfp,"<endl/>\n");
  }

  fclose(srcfp);

  logfile.Write("total=%ld,valid=%ld.\n",itotal,ivalid);

  // д����̻��Ϣ
  ProgramActive.WriteToFile();

  // �ر�xml�ļ���������Ϊ���յ�xml�ļ���
  if (xmlfp != 0)
  {
    fprintf(xmlfp,"</data>"); fclose(xmlfp); xmlfp=0;

    // ѹ��xml�ļ�
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

  // ��������쳣�˳����͹ر��ļ���������ɾ���м�״̬���ļ�
  if (srcfp!=0) { fclose(srcfp); }
  
  if (xmlfp!=0) { fclose(xmlfp); REMOVE(strTmpFileName); }

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("dminingtxt exit.\n");

  exit(0);
}

