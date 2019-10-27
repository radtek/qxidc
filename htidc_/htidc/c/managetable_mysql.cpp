#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

char strxmlbuffer[4001];
char strlogfilename[301];
char strconnstr[301];
char strLocalTime[21];
char strHours[301];

struct st_APPTABLE
{
  char tname[31]; 
  UINT hdataptype;
  UINT hdatapcfg;
};

struct st_APPTABLE stAPPTABLE;
vector<struct st_APPTABLE> vAPPTABLE;

// ���ز���
BOOL LoadForManager();

// �鵵
BOOL ArchTable(char *strtname,UINT days);

// ɾ��
BOOL DeleteTable(char *strtname,UINT days);

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/managetable_mysql xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/managetable_mysql \"<logfilename>/log/szqx/managetable_mysql.log</logfilename><connstr>10.151.235.98,szidc,pwdidc_2019,szqx,3306</connstr><hours>03,04,05</hours>\n");

    printf("������RDS���ݿ�鵵��ɾ����\n");
    printf("connstr �������ݿ�Ĳ�����\n");
    printf("logfilename �������������ɵ���־�ļ�����\n");
    printf("hours ����������Сʱ������Ӣ�Ķ��ŷָ���������賿������\n\n");

    return -1;
  }

  memset(strxmlbuffer,0,sizeof(strxmlbuffer));
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strHours,0,sizeof(strHours));

  strncpy(strxmlbuffer,argv[1],4000);

  GetXMLBuffer(strxmlbuffer,"logfilename",strlogfilename,300);
  GetXMLBuffer(strxmlbuffer,"connstr",strconnstr,300);
  GetXMLBuffer(strxmlbuffer,"hours",strHours,300);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0) { printf("connstr is null.\n"); return -1; }
  if (strlen(strHours) == 0) { printf("iflog is null.\n"); return -1; }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // �жϳ�������ʱ��
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"hh24");
  if (strstr(strHours,strLocalTime) == 0) return 0;

  //  ����־
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("managetable_mysql");

  // ע�⣬����ʱ��500��
  ProgramActive.SetProgramInfo(&logfile,"managetable_mysql",500);

  // ����Ӧ�����ݿ�
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strconnstr); CallQuit(-1);
  }

  // �����ַ���
  conn.character((char*)"gbk");

  logfile.Write("managetable_mysql beging.\n");

  // ��T_APPTABLE����������Ҫ����ı��������ݹ���
  if (LoadForManager() == FALSE)
  {
    logfile.Write("LoadForManager failed.\n"); CallQuit(-1);
  }

  for(UINT ii=0;ii<vAPPTABLE.size();ii++)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // �鵵
    if (vAPPTABLE[ii].hdataptype == 0)
    {
      logfile.Write("Begin ArchTable %s\n",vAPPTABLE[ii].tname);

      if (ArchTable(vAPPTABLE[ii].tname,vAPPTABLE[ii].hdatapcfg) == FALSE)
      {
        logfile.Write("ArchTable %s failed.\n",vAPPTABLE[ii].tname); conn.rollbackwork();
      }
    }

    // ɾ��
    if (vAPPTABLE[ii].hdataptype == 2)
    {
      logfile.Write("Begin DeleteTable %s\n",vAPPTABLE[ii].tname);

      if (DeleteTable(vAPPTABLE[ii].tname,vAPPTABLE[ii].hdatapcfg) == FALSE)
      {
        logfile.Write("DeleteTable %s failed.\n",vAPPTABLE[ii].tname); conn.rollbackwork();
      }
    }
  }

  logfile.Write("managetable_mysql end.\n");

  conn.disconnect(); 

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  conn.disconnect(); 

  logfile.Write("managetable_mysql exit.\n");

  exit(0);
}

BOOL LoadForManager()
{
  sqlstatement loadstmt;
  loadstmt.connect(&conn);
  loadstmt.prepare("select upper(tname),hdataptype,hdatapcfg from T_APPTABLE where appid=1 and hdataptype in (0,2) order by tname desc");
  loadstmt.bindout(1, stAPPTABLE.tname,30);
  loadstmt.bindout(2,&stAPPTABLE.hdataptype);
  loadstmt.bindout(3,&stAPPTABLE.hdatapcfg);

  if (loadstmt.execute() != 0)
  {
    logfile.Write("LoadForManager failed.\n%s\n",loadstmt.cda.message);
    return FALSE;
  }

  while (TRUE)
  {
    memset(&stAPPTABLE,0,sizeof(stAPPTABLE));

    if (loadstmt.next() != 0) break;

    vAPPTABLE.push_back(stAPPTABLE);
  }

  return TRUE;
}

BOOL ArchTable(char *strtname,UINT days)
{
  char strArhTName[31];
  char strmintime[31];
  char strmaxtime[31];
  char strbegintime[31];
  char strendtime[31];
  char strddatetime[31];
  char strkeyid[31];

  sqlstatement stmtsel,stmtins,stmtdel;
  stmtsel.connect(&conn);
  stmtins.connect(&conn);
  stmtdel.connect(&conn);
  
  // ��ȡ��Сʱ������ʱ��
  memset(strmintime,0,sizeof(strmintime));
  memset(strmaxtime,0,sizeof(strmaxtime));
  stmtsel.prepare("select date_format(min(ddatetime),'%%Y%%m%%d'),date_format(date_sub(now(),interval %ld day),'%%Y%%m%%d') from %s",days,strtname);
  stmtsel.bindout(1,strmintime,30);
  stmtsel.bindout(2,strmaxtime,30);

  if (stmtsel.execute() != 0)
  {
    logfile.Write("get mintime failed.\n%s\n",stmtsel.cda.message);
    return FALSE;
  }

  //stmtsel.next();

  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  }

  if (strlen(strmintime) == 0) return TRUE;
  if (strlen(strmaxtime) == 0) return TRUE;

  strcat(strmintime,"000000");
  strcat(strmaxtime,"000000");

  // ����Сʱ�俪ʼ�����չ鵵��
  for (UINT ii=0;ii<999999;ii++)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ���ݹ鵵��ʼʱ��
    memset(strbegintime,0,sizeof(strbegintime));
    AddTime(strmintime,strbegintime,ii*60*60*24,"yyyymmddhh24miss");

    // ���ݹ鵵����ʱ��
    memset(strendtime,0,sizeof(strendtime));
    AddTime(strmintime,strendtime,(ii+1)*60*60*24,"yyyymmddhh24miss");

   logfile.Write("archive %s",strbegintime);

    // �ɵ�ǰ������ɹ鵵����
    memset(strArhTName,0,sizeof(strArhTName));
    strcpy(strArhTName,strtname);
    strcat(strArhTName,"_ARH_"); 
    strncat(strArhTName,strbegintime,4);

    stmtsel.prepare("select ddatetime,keyid from %s where ddatetime>=date_format('%s','%%Y%%m%%d%%H%%i%%s') \
                       and ddatetime<date_format('%s','%%Y%%m%%d%%H%%i%%s')",strtname,strbegintime,strendtime);

    stmtsel.bindout(1,strddatetime,30);
    stmtsel.bindout(2,strkeyid,30);

    if (stmtsel.execute() != 0)
    {
      logfile.Write("get mintime failed.\n%s\n",stmtsel.cda.message);
      return FALSE;
    }

    while (TRUE)
    {
      memset(strddatetime,0,sizeof(strddatetime));
      memset(strkeyid,0,sizeof(strkeyid));

      if (stmtsel.next() != 0) break;

      logfile.Write("ddatetime = %s,keyid = %s\n",strddatetime,strkeyid);
    }

    stmtins.prepare("insert into %s select * from %s where ddatetime>=date_format('%s','%%Y%%m%%d%%H%%i%%s') \
                       and ddatetime<date_format('%s','%%Y%%m%%d%%H%%i%%s')",strArhTName,strtname,strbegintime,strendtime);

    stmtdel.prepare("delete from %s where ddatetime>=date_format('%s','%%Y%%m%%d%%H%%i%%s') \
                       and ddatetime<date_format('%s','%%Y%%m%%d%%H%%i%%s')",strtname,strbegintime,strendtime);

    if (stmtins.execute() != 0)
    {
      if (stmtins.cda.rc != 1)
      {
        logfile.WriteEx("BackupToATable insert %s failed.\n%s\n",strArhTName,stmtins.cda.message);
        return FALSE;
      }
    }

    if (stmtdel.execute() != 0)
    {
      logfile.WriteEx("BackupToATable delete %s failed.\n%s\n",strtname,stmtdel.cda.message);
      return FALSE;
    }

    // �����ɾ������Ŀһ�²ſ��ԡ�
    // ���˳����´���ִ�оͺ��ˡ�
    if (stmtins.cda.rpc != stmtdel.cda.rpc) 
    {
      conn.rollbackwork();
    }
    else
    {
      conn.commitwork();
      logfile.WriteEx(" %ld rows to %s ok.\n",stmtins.cda.rpc,strArhTName);
    }

    if (strcmp(strbegintime,strmaxtime) == 0) break;
  }

  return TRUE;
}

BOOL DeleteTable(char *strtname,UINT days)
{
  char strmintime[31];
  char strmaxtime[31];
  char strbegintime[31];
  char strendtime[31];
  char strddatetime[31];
  char strkeyid[31];

  sqlstatement stmtsel,stmtdel;
  stmtsel.connect(&conn);
  stmtdel.connect(&conn);
  
  // ��ȡ��Сʱ������ʱ��
  memset(strmintime,0,sizeof(strmintime));
  memset(strmaxtime,0,sizeof(strmaxtime));
  stmtsel.prepare("select date_format(min(ddatetime),'%%Y%%m%%d'),date_format(date_sub(now(),interval %ld day),'%%Y%%m%%d') from %s",days,strtname);
  stmtsel.bindout(1,strmintime,30);
  stmtsel.bindout(2,strmaxtime,30);

  if (stmtsel.execute() != 0)
  {
    logfile.Write("get mintime failed.\n%s\n",stmtsel.cda.message);
    return FALSE;
  }

  //stmtsel.next();

  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  }

  if (strlen(strmintime) == 0) return TRUE;
  if (strlen(strmaxtime) == 0) return TRUE;

  strcat(strmintime,"000000");
  strcat(strmaxtime,"000000");

  // ����Сʱ�俪ʼ������ɾ��
  for (UINT ii=0;ii<999999;ii++)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ������ʼʱ��
    memset(strbegintime,0,sizeof(strbegintime));
    AddTime(strmintime,strbegintime,ii*60*60*24,"yyyymmddhh24miss");

    // ���ݽ���ʱ��
    memset(strendtime,0,sizeof(strendtime));
    AddTime(strmintime,strendtime,(ii+1)*60*60*24,"yyyymmddhh24miss");

    logfile.Write("delete %s",strbegintime);

    stmtsel.prepare("select ddatetime,keyid from %s where ddatetime>=date_format('%s','%%Y%%m%%d%%H%%i%%s') \
                       and ddatetime<date_format('%s','%%Y%%m%%d%%H%%i%%s')",strtname,strbegintime,strendtime);

    stmtsel.bindout(1,strddatetime,30);
    stmtsel.bindout(2,strkeyid,30);

    if (stmtsel.execute() != 0)
    {
      logfile.Write("get mintime failed.\n%s\n",stmtsel.cda.message);
      return FALSE;
    }

    while (TRUE)
    {
      memset(strddatetime,0,sizeof(strddatetime));
      memset(strkeyid,0,sizeof(strkeyid));

      if (stmtsel.next() != 0) break;

      logfile.Write("ddatetime = %s,keyid = %s\n",strddatetime,strkeyid);
    }

    stmtdel.prepare("delete from %s where ddatetime>=date_format('%s','%%Y%%m%%d%%H%%i%%s') \
                       and ddatetime<date_format('%s','%%Y%%m%%d%%H%%i%%s')",strtname,strbegintime,strendtime);

    if (stmtdel.execute() != 0)
    {
      logfile.WriteEx("DeleteTable %s failed.\n%s\n",strtname,stmtdel.cda.message);
      return FALSE;
    }
    else
    {
      conn.commitwork();
      logfile.WriteEx(" %ld rows ok.\n",stmtdel.cda.rpc);
    }

    if (strcmp(strbegintime,strmaxtime) == 0) break;
  }

  return TRUE;
}
