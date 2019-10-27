#include "idcapp.h"
#include "mysql.h"

MYSQL      *mysqlconn = NULL;
connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

BOOL bFirstStart=TRUE;

// ��ȡԴ��������ֶ�
// ��dstfieldsΪ�գ�����Դ��������ֵ��е��ֶ���䡣
BOOL CheckDstTName();

// �жϳ���������·�
void CheckMonth();

// ִ������ͬ��
BOOL SyncTable();

char strlogfilename[301];
char strmysqlconnstr[201];
char stroracleconnstr[201];
char strcharset[51];
char strsrctname[51];
char strsrcpkfieldname[51];
char strdstpkfieldname[51];
char strsrcfields[4001];
char strdsttname[51];
char strdstfields[4001];
int  timetvl;
char strand[501];
char stralarmbz[21];
char strignoreerror[11];
int  maxcount;
char strmonth[3];

UINT uMaxKeyIDOld;  // Դ���ݿ��У���ͬ���ɹ��ļ�¼�����keyid
UINT uMinKeyID;     // ����ͬ���ļ�¼�У���С��keyid��
UINT uMaxKeyID;     // ����ͬ���ļ�¼�У�����keyid��

// ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
BOOL LoadSyncKeyID();

// ��uMaxKeyID��ֵ���µ�T_SYNCORACLETOMYSQL����
BOOL UpdateSyncKeyID();  

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/syncoracletomysql xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/syncoracletomysql \"<logfilename>/log/szqx/syncoracletomysql_LOCALOBTDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><oracleconnstr>szidc/pwdidc@SZQX_10.153.98.31</oracleconnstr><srctname>T_LOCALOBTDATA</srctname><srcpkfieldname>keyid</srcpkfieldname><dstpkfieldname>keyid</dstpkfieldname><srcfields></srcfields><mysqlconnstr>10.153.98.22,3306,admin,ssqx323,gbk,mas</mysqlconnstr><dsttname>T_LOCALOBTDATA</dsttname><dstfields></dstfields><timetvl>10</timetvl><and></and><ignoreerror>FALSE</ignoreerror><maxcount>300</maxcount><month>01</month><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("����һ�����߳������ڴ�Oracle���ݿ�ͬ����MYSQL���ݿ⣬ͬ���ķ�������������ͬ���޸ĺ�ɾ��������\n");
    printf("��������ԴORACLE���ݿ��д�����־��T�T_SYNCORACLETOMYS,��Ҫ���������ֶ�һ��������������Ϊ�������ֶΡ�\n");
    printf("<logfilename>/log/szqx/syncoracletomysql_LOCALOBTDATA.log</logfilename> ���������е���־�ļ�����\n");
    printf("<oracleconnstr>szidc/pwdidc@SZQX_10.153.97.251</oracleconnstr> Դoracle���ݿ�����Ӳ�����\n");
    printf("<mysqlconnstr>172.22.11.235,3306,gps,gps,gbk,gps_db</mysqlconnstr> Ŀ��mysql���ݿ�����Ӳ���\
             �ֶηֱ�Ϊ��IP��ַ,�˿�,�û�����,�ַ���,�򿪵����ݿ⡣\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> Դoracle���ݿ���ַ�����\n");
    printf("<srctname>T_LOCALOBTDATA</srctname> ����Դ������\n");
    printf("<srcpkfieldname>keyid</srcpkfieldname> ����Դ��������ֶ�����\n");
    printf("<dstpkfieldname>keyid</dstpkfieldname> ����Ŀ�ı�������ֶ�����\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> ����Դ����ֶ��б����������select��from֮�䣬"\
           "���ԣ�srcfields��������ʵ���ֶΣ�Ҳ�������κ�Oracle�ĺ��������㡣\n");
    printf("<dsttname>T_LOCALOBTDATA</dsttname> �������ݵ�Ŀ�ı�����\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> �������ݵ�Ŀ�ı���ֶ��б���"\
           "<srcfields>��ͬ������������ʵ���ڵ��ֶΡ�\n");
    printf("<timetvl>10</timetvl> ִ��ͬ����ʱ��������λ���룬���Ϊ�գ�ȱʡ��30��\n");
    printf("<and>and ossmocode like '5%%'</and> ͬ������������select���where����Ĳ��֣�ע�⣬Ҫ��and ��ͷ��\n");
    printf("<ignoreerror>FALSE</ignoreerror> ���������ݴ���ʱ���Ƿ������һ�������ȡFALSE��"\
           "ֻ�е�����Դ���ĳЩ��¼�л���ʱ���Ų���TRUE��\n");
    printf("<maxcount>300</maxcount> ִ��һ��ͬ�������ļ�¼�����������300��\n\n");
    printf("<month>01</month> �����������·ݣ�����һ����ѡ������ֻ�����ڰ����ݴ��ܱ�ͬ���±�ĳ��ϡ�"\
           "���ָ���˸ò�������ô����ֻ��ָ�����·��������������·ݳ����������״̬��\n");
    printf("<alarmbz>FALSE</alarmbz> ���������Ƿ񷢳��澯��ȱʡ��TRUE��\n\n");

    printf("ע�⣺\n");
    printf("  1��<srcfields>��<dstfields>����Ϊ�գ����Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣\n");
    printf("     ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�\n");
    printf("  2��<charset>����Ϊ�գ����Ϊ�գ������򽫲��������ַ���������������ô�ڲ���ϵͳ�����б�����\n");
    printf("     ������ȷ��NLS_LANG����������\n");
    printf("  3�����<timetvl>������30�룬������פ�ڴ棬���������30�루����30�룩��ִ����һ��ͬ�����˳���\n");
    printf("  4��<maxcount>�����ǳ���Ҫ���������Դ���зǷ����ݣ���<maxcount>Ϊ1ʱ��ʧ�ܵļ�¼�޷�ͬ��������\n");
    printf("     ��Ӱ�������ļ�¼�����<maxcount>����1������ʧ�ܼ�¼��ͬһ���ε�ͬ����ȫ��ʧ�ܡ�\n");
    printf("  5����֧��ͬ�����ֶΡ�\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(stroracleconnstr,0,sizeof(stroracleconnstr));
  memset(strmysqlconnstr,0,sizeof(strmysqlconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcpkfieldname,0,sizeof(strsrcpkfieldname));
  memset(strdstpkfieldname,0,sizeof(strdstpkfieldname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  timetvl=0;
  memset(strand,0,sizeof(strand));
  memset(strignoreerror,0,sizeof(strignoreerror));
  maxcount=0;
  memset(strmonth,0,sizeof(strmonth));
  memset(stralarmbz,0,sizeof(stralarmbz));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"oracleconnstr",stroracleconnstr,200);
  GetXMLBuffer(argv[1],"mysqlconnstr",strmysqlconnstr,200);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcpkfieldname",strsrcpkfieldname,50);
  GetXMLBuffer(argv[1],"dstpkfieldname",strdstpkfieldname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,4000);
  GetXMLBuffer(argv[1],"timetvl",&timetvl);
  GetXMLBuffer(argv[1],"and",strand,500);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);
  GetXMLBuffer(argv[1],"month",strmonth,2);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
 
  if (strlen(strlogfilename) == 0)       { printf("logfilename is null.\n"); return -1;    }
  if (strlen(stroracleconnstr) == 0)     { printf("oracleconnstr is null.\n"); return -1;  }
  if (strlen(strcharset) == 0)           { printf("charset is null.\n"); return -1;        }
  if (strlen(strsrctname) == 0)          { printf("srctname is null.\n"); return -1;       }
  if (strlen(strsrcpkfieldname) == 0)    { printf("srcpkfieldname is null.\n"); return -1; }
  if (strlen(strdstpkfieldname) == 0)    { printf("dstpkfieldname is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)          { printf("dsttname is null.\n"); return -1;       }
  if (timetvl<=0)                        timetvl=30;  // ȱʡ30��
  if (timetvl<10)                        timetvl=10;  // ��С����С��10
  if ( (maxcount==0) || (maxcount>300) ) maxcount=300;
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");

  // �淶�������ֶ����Ĵ�Сд
  ToUpper(strsrctname);       ToUpper(strdsttname);
  ToLower(strsrcpkfieldname); ToLower(strdstpkfieldname); 
  ToLower(strsrcfields);      ToLower(strdstfields);

  // ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�
  if ( (strlen(strsrcfields)==0) || (strlen(strdstfields)==0) )
  {
    memset(strsrcfields,0,sizeof(strsrcfields)); 
    memset(strdstfields,0,sizeof(strdstfields));
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // �жϳ���������·�
  CheckMonth();

  // ����־�ļ�
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("syncoracletomysql");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"syncoracletomysql",1200);

  // �������ݿ���ַ���
  if (strlen(strcharset) != 0) setenv("NLS_LANG",strcharset,1);

  // ��õ�ַ�˿�,�û���,����,�򿪵����ݿ�
  char strdburl[51],strport[8],strdbuser[31],strpassword[31],strcharset[51],strdbname[31];
  int  port;
  memset(strdburl,0,sizeof(strdburl));
  memset(strport,0,sizeof(strport));
  memset(strdbuser,0,sizeof(strdbuser));
  memset(strpassword,0,sizeof(strpassword));
  memset(strcharset,0,sizeof(strcharset));
  memset(strdbname,0,sizeof(strdbname));

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strmysqlconnstr,",");;
  if (CmdStr.CmdCount() != 6)
  {
    logfile.Write("mysqlconnstr(%s) is invalied.\n",strmysqlconnstr); return -1;
  }

  CmdStr.GetValue(0,strdburl,30);
  CmdStr.GetValue(1,strport,8);
  CmdStr.GetValue(2,strdbuser,30);
  CmdStr.GetValue(3,strpassword,30);
  CmdStr.GetValue(4,strcharset,30);
  CmdStr.GetValue(5,strdbname,30);
  port = atoi(strport);

  // ����MYSQL����Դ���ݿ�
  if ( (mysqlconn = mysql_init(NULL)) == NULL )
  {
    logfile.Write( "Init mysql failed\n" ); return -1;
  }

  if ( mysql_real_connect(mysqlconn,strdburl,strdbuser,strpassword,strdbname,port, NULL, 0 ) == NULL )
  {
    logfile.Write( "Connect to db(%s) failed.\n", strmysqlconnstr ); mysql_close(mysqlconn); return -1;
  }

  if ( mysql_set_character_set(mysqlconn, strcharset) != 0 )
  {
    logfile.Write("set character_set(%s) failed:%s\n",strcharset,mysql_error(mysqlconn));
    mysql_close(mysqlconn); return -1;
  }

  // ����oracle���ݿ�
  if (conn.connecttodb(stroracleconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",stroracleconnstr); return -1;
  }

  // ��ȡԴ��������ֶ�
  // ��dstfieldsΪ�գ�����Դ��������ֵ��е��ֶ���䡣
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // ִ������ͬ��
  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (SyncTable()==FALSE) CallQuit(-1);

    // ����������30�룬�ɴ��˳�ȥ���ˣ�������û��Ҫ�����ڴ��У��˷���Դ
    if (timetvl>=30) break;
  }

  return 0;
}

// ��ȡԴ��������ֶ�
// ��dstfieldsΪ�գ�����Դ��������ֵ��е��ֶ���䡣
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // ��ȡ�ñ�ȫ�����ֶ�
  TABFIELD.GetALLField(&conn,strdsttname);

  if (TABFIELD.m_fieldcount==0)
  {
    logfile.Write("%s�����ڣ��봴������\n",strdsttname); return FALSE;
  }

  // �ɵ�ͬ��������sync_rownum�ֶΣ��ų���
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rownum,","");
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rownum","");

  // <srcfields>��<dstfields>Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣
  if ( (strlen(strsrcfields)==0) && (strlen(strdstfields)==0) )
  {
    strcpy(strsrcfields,TABFIELD.m_allfieldstr); strcpy(strdstfields,TABFIELD.m_allfieldstr);
  }

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("syncoracletomysql exit.\n");

  exit(0);
}

// ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
BOOL LoadSyncKeyID()
{
  uMaxKeyIDOld=uMinKeyID=uMaxKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select keyid from T_SYNCORACLETOMYSQL where tname=upper(:1)");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&uMaxKeyIDOld);
  if (stmt.execute() != 0)
  {
    // �����T_SYNCORACLETOMYSQL�����ڣ��Ͳ�����ʧ�ܣ�������ᴴ����
    if (stmt.cda.rc!=942)
    {
      logfile.Write("select T_SYNCORACLETOMYSQL failed.\n%s\n",stmt.cda.message); return FALSE;
    }

    // T_SYNCORACLETOMYSQL�����ڣ��ʹ�����
    stmt.prepare("create table T_SYNCORACLETOMYSQL(tname varchar2(30),keyid number(15),primary key(tname))");
    stmt.execute();
  }
  else
  {
    if (stmt.next() != 0) 
    {
      stmt.prepare("insert into T_SYNCORACLETOMYSQL(tname,keyid) values(upper(:1),0)");
      stmt.bindin(1,strdsttname,50);
      if (stmt.execute() != 0)
      {
        logfile.Write("insert T_SYNCORACLETOMYSQL failed.\n%s\n",stmt.cda.message); return FALSE;
      }

      conn.commitwork();
    }
  }

  // ������Դ�����־���л�ȡ����uMaxKeyIDOld����Сkeyid
  stmt.prepare("select min(%s) from %s where %s>:1",strsrcpkfieldname,strsrctname,strsrcpkfieldname);
  stmt.bindin(1,&uMaxKeyIDOld);
  stmt.bindout(1,&uMinKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrctname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  // ���uMinKeyID==0�����ʾû����Ҫͬ���ļ�¼��
  if (uMinKeyID==0) return TRUE;

  // ������Դ�����־���л�ȡ����uMinKeyID+100000�����keyid
  stmt.prepare("select max(%s) from %s where %s<=:1+100000",strsrcpkfieldname,strsrctname,strsrcpkfieldname);
  stmt.bindin(1,&uMinKeyID);
  stmt.bindout(1,&uMaxKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrctname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  uMinKeyID=uMaxKeyIDOld+1;

  return TRUE;
}

BOOL UpdateSyncKeyID()
{
  // ������ͬ�����ݵ�λ��
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("update T_SYNCORACLETOMYSQL set keyid=:1 where tname=upper(:2)");
  stmt.bindin(1,&uMaxKeyID);
  stmt.bindin(2,strdsttname,50);
  if (stmt.execute() != 0)
  {
    logfile.Write("update T_SYNCORACLETOMYSQL failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  // ע�⣬�����ﲻҪ�ύ����T_SYNCORACLETOMYSQL����Ҫ������ͬ����ͬһ������
  return TRUE;
}

#define MAXCOUNT 300

// ִ������ͬ��
BOOL SyncTable()
{
  // ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
  if (LoadSyncKeyID() == FALSE) return FALSE;

  // ���û����Ҫͬ���ļ�¼����ֱ�ӷ���
  if (uMinKeyID==0) return TRUE;

  // �������Դ���ݿ��ͬһ�ű�Ĳ����ǲ��в��������п������keyid���ҵ������
  // ������sleep���ȴ����������ύ��ɡ�
  // ���ǣ�sleep(timetvl)����һ������ĵȴ�ȫ��������ɣ�ȴ���Ծ����ܱ������ֻ���
  if (bFirstStart==TRUE)
  {
    bFirstStart=FALSE; sleep(20);
  }
  else
  {
    if (timetvl<=30) sleep(timetvl);
  }

  CTimer Timer;

  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);

  UINT uMaxCount=0;
  UINT keyid;
  UINT synccount=0;

  uMaxCount=maxcount;

  sqlstatement selsrc,insstmt;

  selsrc.connect(&conn);
  selsrc.prepare("select %s from %s where %s>=:1 and %s<=:2 %s",strsrcpkfieldname,strsrctname,strsrcpkfieldname,strsrcpkfieldname,strand);
  selsrc.bindin(1,&uMinKeyID);
  selsrc.bindin(2,&uMaxKeyID);
  selsrc.bindout(1,&keyid);

  if (selsrc.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",selsrc.m_sql,selsrc.cda.message); return FALSE;
  }

  // ���ֶ�����CCmdStr���ֿ�
  CCmdStr fieldstr;
  fieldstr.SplitToCmd(strsrcfields,",");

  // ���ڴ���ֶ�ֵ������,���500���ֶΣ�ÿ���ֶεĳ������4000��
  char strfieldvalue[500][4001]; 
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  UINT ii=0;

  // ׼����ȡ���ݵ�SQL��䣬���������
  char strselectsql[4001],strtemp[4001];
  char strBindStr[409600],strinssql[4096];

  // ����keyid��һ��һ����ȡ���ݣ���һ��һ�����뵽mysql��
  while (TRUE)
  {
    // ��ȡkeyid
    if (selsrc.next() != 0) break;

    memset(strselectsql,0,sizeof(strselectsql));
    snprintf(strselectsql,4000,"select %s from %s where %s=%ld",strsrcfields,strsrctname,strsrcpkfieldname,keyid);

    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare(strselectsql);

    // ����Դ�ж��ٸ��ֶΣ���Ҫ�󶨶��ٸ������
    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      stmt.bindout(ii+1,strfieldvalue[ii],4000);
    }
    
    // ִ�в�ѯ����ݵ�SQL���
    if (stmt.execute() != 0)
    {
      logfile.Write("%s\nexec sql failed.\n%s\n",strselectsql,stmt.cda.message); return stmt.cda.rc;
    }

    stmt.next();

    memset(strBindStr,0,sizeof(strBindStr));

    // ƴ������ֵ��
    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      memset(strtemp,0,sizeof(strtemp));
      if (ii==0) snprintf(strtemp,4000,"'%s'",strfieldvalue[ii]);
      if (ii >0) snprintf(strtemp,4000,",'%s'",strfieldvalue[ii]);
      strcat(strBindStr,strtemp);
    }

    // ִ�����ݲ��롣
    memset(strinssql,0,sizeof(strinssql));
    snprintf(strinssql,4000,"insert into  %s(%s) values(%s)",strdsttname,strdstfields,strBindStr);

    if (mysql_query(mysqlconn,strinssql) != 0)
    {
      logfile.Write("Insert into %s failed: %s\n",strdsttname,mysql_error(mysqlconn));

      // �����������ͻ�������ѭ����
      if ( strstr(mysql_error(mysqlconn),"PRIMARY") != 0) continue;

      else
        return FALSE;
    }

    synccount++;
  }

  mysql_commit(mysqlconn);

  logfile.WriteEx("ok,%lu rows incrementd(%d).\n",synccount,Timer.Elapsed());

  if (UpdateSyncKeyID()==FALSE) return FALSE;

  conn.commitwork();

  return TRUE;
}


// �жϳ���������·�
void CheckMonth()
{
  if (strlen(strmonth)==0) return;

  char strLocalTime[21];

  // �ж��Ƿ�ƥ�䵱ǰʱ������
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd");
  if (strncmp(strLocalTime+4,strmonth,2)==0) return;
  
  // �ж��Ƿ�ƥ�䵱ǰʱ����һ�����
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd",0-1*24*60*60);
  if (strncmp(strLocalTime+4,strmonth,2)==0) return;
  
  // ����һСʱ���˳�
  sleep(1*60*60);
  
  exit(0);
}


