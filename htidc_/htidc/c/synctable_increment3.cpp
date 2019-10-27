#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

BOOL bFirstStart=TRUE;

int lobfieldcount=0;

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
// �ж�Ŀ�ı��Ƿ���dstpkfieldname�У����û�У�����ʧ��
// �ж�dstpkfieldname���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName();

// �жϳ���������·�
void CheckMonth();

BOOL bContinue;
// ִ������ͬ��
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcpkfieldname[51];
//char strdstpkfieldname[51];
char strsrcfields[2001];
char strdsttname[51];
char strdstfields[2001];
int  timetvl;
char strand[1001];
char stralarmbz[21];
char strignoreerror[11];
int  maxcount;
char strmonth[3];
char strdblink[101];
char strtablename[51];

UINT uMaxKeyIDOld;  // Դ���ݿ��У���ͬ���ɹ��ļ�¼�����keyid
UINT uMinKeyID;     // ����ͬ���ļ�¼�У���С��keyid��
UINT uMaxKeyID;     // ����ͬ���ļ�¼�У�����keyid��

// ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
BOOL LoadSyncKeyID();
// ��uMaxKeyID��ֵ���µ�T_SYNCLOG����
BOOL UpdateSyncKeyID();  

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_increment3 xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_increment3 \"<logfilename>/log/szqx/synctable_increment2_LOCALOBTDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.98.24</connstr><srctname>T_LOCALOBTDATA@SZIDC</srctname><srcpkfieldname>keyid</srcpkfieldname><srcfields></srcfields><dsttname>T_LOCALOBTDATA</dsttname><dstfields></dstfields><timetvl>10</timetvl><and></and><ignoreerror>FALSE</ignoreerror><maxcount>300</maxcount><month>01</month><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("����һ�����߳���������Oracle���ݿ�֮��ͬ�����ݣ�ͬ���ķ�������������ͬ���޸ĺ�ɾ��������\n");
    printf("��������Ҫ��Դ���ݿ��д�������������־�����ǣ�Ҫ��Դ��������ֶ�һ��������������Ϊ�������ֶ�,Ŀ�ı����û������ֶΡ�\n");
    printf("synctable_increment3 ����Ϊ��Ŀ�ı�û��Դ����ͬ�������ֶζ����ӣ���Ȼ��Ҳ���ԣ��÷�����ͬsynctable_increment2ʹ����ͬ��\n");
    printf("<logfilename>/log/szqx/synctable_increment3_LOCALOBTDATA.log</logfilename> ���������е���־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> �������ݿ�����Ӳ�����\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> ���ݿ���ַ���������Ҫ��<connstr>"\
           "�������������ݿ���ͬ��\n");
    printf("<srctname>T_LOCALOBTDATA</srctname> ����Դ������\n");
    printf("<srcpkfieldname>keyid</srcpkfieldname> ����Դ��������ֶ�����\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> ����Դ����ֶ��б����������select��from֮�䣬"\
           "���ԣ�srcfields��������ʵ���ֶΣ�Ҳ�������κ�Oracle�ĺ��������㡣\n");
    printf("<dsttname>T_LOCALOBTDATA@SZIDC</dsttname> �������ݵ�Ŀ�ı���,�ڱ���������ݿ���·��������ָ��Զ�����ݿ⡣\n");
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
    printf("  3�����<timetvl>������30�룬������פ�ڴ棬���������30�루����30�룩��ִ����һ��ͬ�����˳���\n\n");
    printf("  4��<maxcount>�����ǳ���Ҫ���������Դ���зǷ����ݣ���<maxcount>Ϊ1ʱ��ʧ�ܵļ�¼�޷�ͬ��������\n");
    printf("     ��Ӱ�������ļ�¼�����<maxcount>����1������ʧ�ܼ�¼��ͬһ���ε�ͬ����ȫ��ʧ�ܡ�\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcpkfieldname,0,sizeof(strsrcpkfieldname));
  //memset(strdstpkfieldname,0,sizeof(strdstpkfieldname));
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
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcpkfieldname",strsrcpkfieldname,50);
  //GetXMLBuffer(argv[1],"dstpkfieldname",strdstpkfieldname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"timetvl",&timetvl);
  GetXMLBuffer(argv[1],"and",strand,1000);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);
  GetXMLBuffer(argv[1],"month",strmonth,2);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
 
  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  if (strlen(strsrcpkfieldname) == 0)    { printf("srcpkfieldname is null.\n"); return -1; }
  //if (strlen(strdstpkfieldname) == 0)    { printf("dstpkfieldname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)    { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)   { printf("dstfields is null.\n"); return -1; }
  if (timetvl<=0)   timetvl=30;  // ȱʡ30��
  if (timetvl<10)   timetvl=10;  // ��С����С��10
  // if (strlen(strand) == 0)       { printf("and is null.\n"); return -1; }
  // if (strlen(strignoreerror) == 0)       { printf("ignoreerror is null.\n"); return -1; }
  if ( (maxcount==0) || (maxcount>300) ) maxcount=300;
  // if (strlen(strmonth) == 0)       { printf("month is null.\n"); return -1; }
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");

  // �淶�������ֶ����Ĵ�Сд
  ToUpper(strsrctname);       ToUpper(strdsttname);
  ToLower(strsrcpkfieldname); 
  //ToLower(strdstpkfieldname); 
  ToLower(strsrcfields); ToLower(strdstfields);

  // ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�
  if ( (strlen(strsrcfields)==0) || (strlen(strdstfields)==0) )
  {
    memset(strsrcfields,0,sizeof(strsrcfields)); memset(strdstfields,0,sizeof(strdstfields));
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
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_increment3");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"synctable_increment3",1200);

  // ���Դ������"@"�ַ�����һ�������Դ���Ŀ�ı�Ĺ�ϵ
  if (strstr(strsrctname,"@") > 0) 
  { 
    logfile.Write("%s��ض���%s������ָ�������ݿ�ı��ر�������ָ��Զ�����ݿ�ı�\n",\
                   strsrctname,strconnstr);
    return -1;
  }

  // �������ݿ�
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // Ŀ�ı��������·���
  char *strpos=0;
  memset(strdblink,0,sizeof(strdblink));
  memset(strtablename,0,sizeof(strtablename));

  if ((strpos = strstr(strdsttname,"@")) == 0) return FALSE;

  strncpy(strtablename,strdsttname,strlen(strdsttname)-strlen(strpos));
  strncpy(strdblink,strpos,100);  

  // �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
  // �������������ж�
  // �ж�Ŀ�ı��Ƿ���dstpkfieldname�У����û�У�����ʧ��
  // �ж�dstpkfieldname���Ƿ񴴽���Ψһ���������û�У�����ʧ��
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ���������
  KillLocked(&conn,strdsttname);

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

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
// �������������ж�
// �ж�Ŀ�ı��Ƿ���dstpkfieldname�У����û�У�����ʧ��
// �ж�dstpkfieldname���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // ��ȡ�ñ�ȫ�����ֶ�
  TABFIELD.GetALLField(&conn,strdsttname,0,TRUE);

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

  lobfieldcount=0;
  // �ж�Ŀ�ı��Ƿ��ж������ֶΣ�����У����ֶεĸ��������lobfieldcount������
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare(\
     "select count(*) from USER_TAB_COLUMNS%s where table_name=upper('%s') and data_type in ('BLOB','CLOB')",strdblink,strtablename);
  //stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&lobfieldcount);
  stmt.execute();
  stmt.next();

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ���������
  KillLocked(&conn,strdsttname);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_increment3 exit.\n");

  exit(0);
}

// ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
BOOL LoadSyncKeyID()
{
  bContinue=FALSE;
  uMaxKeyIDOld=uMinKeyID=uMaxKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select keyid from T_SYNCLOG%s where tname=upper('%s')",strdblink,strtablename);
  //stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&uMaxKeyIDOld);

  if (stmt.execute() != 0)
  {
    // �����T_SYNCLOG�����ڣ��Ͳ�����ʧ�ܣ�������ᴴ����
    if (stmt.cda.rc!=942)
    {
      logfile.Write("select T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
    }

    // T_SYNCLOG�����ڣ��ʹ�����
    stmt.prepare("create table T_SYNCLOG%s(tname varchar2(30),keyid number(15),primary key(tname))",strdblink);
    stmt.execute();
  }
  else
  {
    if (stmt.next() != 0) 
    {
      stmt.prepare("insert into T_SYNCLOG%s(tname,keyid) values(upper('%s'),0)",strdblink,strtablename);
      //stmt.bindin(1,strdsttname,50);
      if (stmt.execute() != 0)
      {
        logfile.Write("insert T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
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
  stmt.prepare("select max(%s) from %s where %s<=:1+100000 %s",strsrcpkfieldname,strsrctname,strsrcpkfieldname,strand);
  stmt.bindin(1,&uMinKeyID);
  stmt.bindout(1,&uMaxKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrctname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  uMinKeyID=uMaxKeyIDOld+1;

  //logfile.Write("uMaxKeyIDOld=%lu\n",uMaxKeyIDOld);
  //logfile.Write("uMinKeyID=%lu\n",uMinKeyID);
  //logfile.Write("uMaxKeyID=%lu\n",uMaxKeyID);

  return TRUE;
}

BOOL UpdateSyncKeyID()
{
  // ������ͬ�����ݵ�λ��
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("update T_SYNCLOG%s set keyid=:1 where tname=upper('%s')",strdblink,strtablename);
  stmt.bindin(1,&uMaxKeyID);
  //stmt.bindin(2,strdsttname,50);
  if (stmt.execute() != 0)
  {
    logfile.Write("update T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  // ע�⣬�����ﲻҪ�ύ����T_SYNCLOG����Ҫ������ͬ����ͬһ������
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
  UINT ccount=0;
  UINT keyid,keyidn[MAXCOUNT];

  if (lobfieldcount == 0) uMaxCount=maxcount;
  if (lobfieldcount >  0) uMaxCount=1;

  sqlstatement selsrc,delstmt,insstmt;

  UINT synccount=0;

  selsrc.connect(&conn);
  selsrc.prepare("select %s from %s where %s>=:1 and %s<=:2",strsrcpkfieldname,strsrctname,strsrcpkfieldname,strsrcpkfieldname);
  selsrc.bindin(1,&uMinKeyID);
  selsrc.bindin(2,&uMaxKeyID);
  selsrc.bindout(1,&keyid);

  if (selsrc.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",selsrc.m_sql,selsrc.cda.message); return FALSE;
  }
  
  UINT ii=0;
  char strtemp[11],strBindStr[4096];

  memset(strBindStr,0,sizeof(strBindStr));

  for (ii=0; ii<uMaxCount; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%lu",ii+1);
    if (ii >0) sprintf(strtemp,",:%lu",ii+1);
    strcat(strBindStr,strtemp);
  }

  insstmt.connect(&conn);
  insstmt.prepare("insert into %s(%s) select %s from %s where %s in (%s) %s",strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname,strBindStr,strand);\

  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1,&keyidn[ii]);
  }

  while (TRUE)
  {
    if (selsrc.next() != 0) break;

    keyidn[ccount]=keyid;

    ccount++;

    // ÿuMaxCount����¼�Ͳ���һ��
    if (ccount == uMaxCount)
    {
      if (insstmt.execute() != 0)
      {
        logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

        if ( (insstmt.cda.rc != 1) && (insstmt.cda.rc != 1410) )
        {
          if (strcmp(strignoreerror,"TRUE") != 0) return FALSE;
        }
      }
      else
      {
        synccount=synccount+insstmt.cda.rpc;

        // ����ж������ֶΣ��͸�Ϊһ�����ύ
        if (lobfieldcount >  0) conn.commitwork();
      }

      ProgramActive.WriteToFile();

      memset(&keyidn,0,sizeof(keyidn));

      ccount=0;
    }
  }

  // ������ѭ�������ʱ���������uMaxCount���������ﴦ��
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("insert into %s(%s) select %s from %s where %s=:1 %s",strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname,strand);
    insstmt.bindin(1,&keyidn[ii]);

    if (insstmt.execute() != 0)
    {
      logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

      if ( (insstmt.cda.rc != 1) && (insstmt.cda.rc != 1410) )
      {
        if (strcmp(strignoreerror,"TRUE") != 0) return FALSE;
      }
    }
    else
    {
      synccount=synccount+insstmt.cda.rpc;

      // ����ж������ֶΣ��͸�Ϊһ�����ύ
      if (lobfieldcount >  0) conn.commitwork();
    }
  }

  logfile.WriteEx("ok,%lu,rows incrementd(%d).\n",synccount,Timer.Elapsed());

  if (UpdateSyncKeyID()==FALSE) return FALSE;

  if (synccount>10) bContinue=TRUE;

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


