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

    printf("����һ�����߳������ڷ��ֹ۲������е��쳣���ݣ�Ȼ�����T_EXTDATAALARM��ͬʱ���쳣���ݸ澯��־д��澯��־�ļ��������������ĵĸ澯�ʼ�֪ͨ���ܷ����澯�ʼ�\n");
    printf("<logfilename>/log/sqxj/extdataalarm_LCOBTDATA_t.log</logfilename> ���������в�������־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> Ŀ�����ݿ�����Ӳ�����\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> ���ݿ���ַ���������Ҫ��<connstr>�������������ݿ���ͬ��\n");
    printf("<tname>T_LCOBTDATA</tname> ��Ź۲����ݵı�����\n");
    printf("<ddtfieldname>ddatetime</ddtfieldname> ����ʱ���ֶ�����һ����ddatetime��\n");
    printf("<obtfieldname>obtid</obtfieldname> �۲�վ���ֶ�����һ����obtid��\n");
    printf("<datfieldname>t</datfieldname> �۲������ֶ���������Ͳ�һ��Ҫ��Ҫ�����ݽṹ��\n");
    printf("<minvalue>0</minvalue><maxvalue>400</maxvalue> �쳣���ݷ�ֵ�����ֵС��minvalue�����maxvalue����������Ϊ�쳣���ݡ�\n");

    printf("ע�⣺������ÿ10��������һ�Σ���procctl���ȣ���ÿСʱ��20-30��֮��ɨ����һСʱ01��00�ֵ����ݣ����ɵĸ澯���ݲ���T_ALARMLOG��T_EXTDATAALARM���С�\n");

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

  // �淶�������ֶ����Ĵ�Сд
  ToUpper(strtname); ToLower(strddtfieldname); ToLower(strobtfieldname); ToLower(strdatfieldname);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
 
  // ����־�ļ�
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  logfile.SetAlarmOpt("extdataalarm");

  // ע�⣬����ʱ��120��   xxxxxxxxxxxx����ʱ��1200�룬�������
  ProgramActive.SetProgramInfo(&logfile,"extdataalarm",1200);

  // �������ݿ�
  if (conn.connecttodb(strconnstr,TRUE) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  char obtid[31];
  char ddatetime[31];
  char value[31];
  char strddatetime01[21];   // ��һʱ��01�ֵ�ʱ�䣬����һʱ�ε�һ����¼��
  char strddatetime00[21];   // ��Сʱ00�ֵ�ʱ�䣬����һʱ�����һ����¼��

  memset(strddatetime01,0,sizeof(strddatetime01));
  memset(strddatetime00,0,sizeof(strddatetime00));

  LocalTime(strddatetime01,"yyyy-mm-dd hh24",0-60*60);
  strcat(strddatetime01,":01:00");

  LocalTime(strddatetime00,"yyyy-mm-dd hh24");
  strcat(strddatetime00,":00:00");

  // ��������ֵ����
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

        snprintf(logfile.m_message,300,"�쳣���ݸ澯��������%s��վ�㣺%s��ʱ�䣺%s���ֶΣ�%s��ֵΪ��%s��",strtname,obtid,ddatetime,strdatfieldname,value);

	// �����쳣���ݸ澯��־��T_EXTDATAALARM
	ins.prepare("insert into T_EXTDATAALARM(logid,tname,obtid,ddatetime,fieldname,value,crttime) values(SEQ_EXTDATAALARM.nextval,'%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s',sysdate)",strtname,obtid,ddatetime,strdatfieldname,value);

        if (ins.execute() != 0)
	{
	  logfile.Write("insert into T_EXTDATAALARM failed.%s\n",ins.cda.message); EXIT(-1);
	}

        // ���쳣���ݸ澯��־д��澯��־�ļ�
        logfile.WriteAlarmFile();

        logfile.Write("�쳣���ݸ澯��������%s��վ�㣺%s��ʱ�䣺%s���ֶΣ�%s��ֵΪ��%s��\n",strtname,obtid,ddatetime,strdatfieldname,value);
        sleep(1); // ��������1�������ϣ���Ȼֻ�����һ���澯��Ϣ
      }
    } 
  }

  // �������С��ֵ����
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

        snprintf(logfile.m_message,300,"�쳣���ݸ澯��������%s��վ�㣺%s��ʱ�䣺%s���ֶΣ�%s��ֵΪ��%s��",strtname,obtid,ddatetime,strdatfieldname,value);

	// �����쳣���ݸ澯��־��T_EXTDATAALARM
	ins.prepare("insert into T_EXTDATAALARM(logid,tname,obtid,ddatetime,fieldname,value,crttime) values(SEQ_EXTDATAALARM.nextval,'%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s',sysdate)",strtname,obtid,ddatetime,strdatfieldname,value);

        if (ins.execute() != 0)
 	{
	   logfile.Write("insert into T_EXTDATAALARM failed.%s\n",ins.cda.message); EXIT(-1);
	}

        // ���쳣���ݸ澯��־д��澯��־�ļ�
        logfile.WriteAlarmFile();

        logfile.Write("�쳣���ݸ澯��������%s��վ�㣺%s��ʱ�䣺%s���ֶΣ�%s��ֵΪ��%s��\n",strtname,obtid,ddatetime,strdatfieldname,value);
        sleep(1); // ��������1�������ϣ���Ȼֻ�����һ���澯��Ϣ
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
