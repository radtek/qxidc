#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

int lobfieldcount=0;

// �ж�Ŀ�ı��Ƿ���ڣ�����������ڣ�����ʧ��
// �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
// �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName();

BOOL bFirstStart=TRUE;

BOOL bContinue;
// ִ������ͬ��
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcfields[4001];
char strsrclogtname[51];
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

// �жϳ���������·�
void CheckMonth();

// ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
BOOL LoadSyncKeyID();
// ��uMaxKeyID��ֵ���µ�T_SYNCLOG����
BOOL UpdateSyncKeyID();  

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_increment xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_increment \"<logfilename>/log/sqxj/synctable_increment_ALLAWSDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_ALLAWSDATA@SZIDC</srctname><srcfields></srcfields><srclogtname>T_ALLAWSDATA@SZIDC</srclogtname><dsttname>T_ALLAWSDATA</dsttname><dstfields></dstfields><timetvl>10</timetvl><and>and ossmocode like '5%%'</and><ignoreerror>FALSE</ignoreerror><maxcount>300</maxcount><month>01</month><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("����һ�����߳���������Oracle���ݿ�֮��ͬ�����ݣ�ͬ���ķ�����������ͬ�����ӡ��޸ĺ�ɾ�����ֲ�����\n");
    printf("�������Դ���������ֶΣ��ɲ���synctable_increment1����������Ҫ��Դ���ݿ��д�������������־����\n");
    printf("<logfilename>/log/sqxj/synctable_increment_ALLAWSDATA.log</logfilename> ���������е���־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> Ŀ�����ݿ�����Ӳ�����\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> ���ݿ���ַ���������Ҫ��<connstr>"\
           "�������������ݿ���ͬ��\n");
    printf("<srctname>T_ALLAWSDATA@SZIDC</srctname> ����Դ�������ڱ���������ݿ���·�������Ϳ���ָ��Զ�����ݿ⡣\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> ����Դ�����ֶ��б������������select��from֮�䣬"\
           "���ԣ�srcfields��������ʵ���ֶΣ�Ҳ�������κ�Oracle�ĺ��������㡣\n");
    printf("<srclogtname>T_ALLAWSDATA@SZIDC</srclogtname> ����Դ������־�����ñ����������Դ��"\
           "��rowid��keyid����־����crtsynclogtable���򴴽����������Դ����keyid�ֶΣ��ò�������Ϊ�ա�\n");
    printf("<dsttname>T_ALLAWSDATA</dsttname> �������ݵ�Ŀ�ı�����\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> �������ݵ�Ŀ�ı����ֶ��б�����"\
           "<srcfields>��ͬ������������ʵ���ڵ��ֶΡ�\n");
    printf("<timetvl>10</timetvl> ִ��ͬ����ʱ��������λ���룬���Ϊ�գ�ȱʡ��30��\n");
    printf("<and>and ossmocode like '5%%'</and> ͬ������������select���where����Ĳ��֣�ע�⣬Ҫ��and ��ͷ��\n");
    printf("<ignoreerror>FALSE</ignoreerror> ���������ݴ���ʱ���Ƿ������һ�������ȡFALSE��"\
           "ֻ�е�����Դ����ĳЩ��¼�л���ʱ���Ų���TRUE��\n");
    printf("<maxcount>300</maxcount> ִ��һ��ͬ�������ļ�¼�����������300��\n");
    printf("<month>01</month> �����������·ݣ�����һ����ѡ������ֻ�����ڰ����ݴ��ܱ�ͬ���±��ĳ��ϡ�"\
           "���ָ���˸ò�������ô����ֻ��ָ�����·��������������·ݳ����������״̬��\n");
    printf("<alarmbz>FALSE</alarmbz> ���������Ƿ񷢳��澯��ȱʡ��TRUE��\n\n");

    printf("ע�⣺\n");
    printf("  1��<srcfields>��<dstfields>����Ϊ�գ����Ϊ�գ�����Ŀ�ı��������ֵ��е��ֶ���䡣\n");
    printf("     ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�\n");
    printf("  2��<charset>����Ϊ�գ����Ϊ�գ������򽫲��������ַ���������������ô�ڲ���ϵͳ�����б�����\n");
    printf("     ������ȷ��NLS_LANG����������\n");
    printf("  3��<dsttname>������Ŀ�ı��������sync_rowid�ֶΣ���������Ϊrowid��\n");
    printf("     ���ң�sync_rowid�ֶα�����Ψһ������\n");
    printf("  4�����<timetvl>������30�룬������פ�ڴ棬���������30�루����30�룩��ִ����һ��ͬ�����˳���\n\n");
    printf("  5��<maxcount>�����ǳ���Ҫ���������Դ���зǷ����ݣ���<maxcount>Ϊ1ʱ��ʧ�ܵļ�¼�޷�ͬ��������\n");
    printf("     ��Ӱ�������ļ�¼�����<maxcount>����1������ʧ�ܼ�¼��ͬһ���ε�ͬ����ȫ��ʧ�ܡ�\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strsrclogtname,0,sizeof(strsrclogtname));
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
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,4000);
  GetXMLBuffer(argv[1],"srclogtname",strsrclogtname,50);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,4000);
  GetXMLBuffer(argv[1],"timetvl",&timetvl);
  GetXMLBuffer(argv[1],"and",strand,500);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);
  GetXMLBuffer(argv[1],"month",strmonth,2);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
 
  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  // if (strlen(strsrclogtname) == 0)   { printf("srclogtname is null.\n"); return -1; }
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
  ToUpper(strsrctname);       ToUpper(strdsttname); ToUpper(strsrclogtname);       
  ToLower(strsrcfields);      ToLower(strdstfields);

  // �������Դ������־��Ϊ�գ���ʾԴ����keyid�ֶΣ����԰�Դ���������־����
  if (strlen(strsrclogtname) == 0) strcpy(strsrclogtname,strsrctname);

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

  // �򿪸澯
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_increment");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"synctable_increment",1200);

  // ���Ŀ�ı�����"@"�ַ�����һ�������Դ����Ŀ�ı��Ĺ�ϵ
  if (strstr(strdsttname,"@") > 0) 
  { 
    logfile.Write("%s���ض���%s������ָ�������ݿ�ı��ر���������ָ��Զ�����ݿ�ı���\n",\
                   strdsttname,strconnstr);
    return -1;
  }

  // �������ݿ�
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // �ж�Ŀ�ı��Ƿ���ڣ�����������ڣ�����ʧ��
  // �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
  // �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ����������
  KillLocked(&conn,strdsttname);

  // ִ������ͬ��
  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (SyncTable()==FALSE) CallQuit(-1);

    // ����������30�룬�ɴ��˳�ȥ���ˣ�������û��Ҫ�����ڴ��У��˷���Դ
    if (timetvl>=30) break;

    // �жϳ���������·�
    CheckMonth();
  }

  return 0;
}

// �ж�Ŀ�ı��Ƿ���ڣ�����������ڣ�����ʧ��
// �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
// �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // ��ȡ�ñ�ȫ�����ֶ�
  TABFIELD.GetALLField(&conn,strdsttname);

  if (TABFIELD.m_fieldcount==0)
  {
    logfile.Write("%s�������ڣ��봴������\n",strdsttname); return FALSE;
  }

  // �ɵ�ͬ��������sync_rownum�ֶΣ��ų���
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rownum,","");
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rownum","");

  // �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
  if (strstr(TABFIELD.m_allfieldstr,"sync_rowid") == 0)
  {
    logfile.Write("%s��û��sync_rowid�У��봴������\n",strdsttname); return FALSE;
  }

  // �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
  char stroutput[51];
  memset(stroutput,0,sizeof(stroutput));
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select column_name from USER_IND_COLUMNS where table_name=upper(:1) and column_name='SYNC_ROWID'");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,stroutput,50);
  stmt.execute();
  stmt.next();

  if (strlen(stroutput)==0)
  {
    logfile.Write("%s����sync_rowid��û�д���Ψһ�������봴������\n",strdsttname); return FALSE;
  }

  UpdateStr(TABFIELD.m_allfieldstr,",sync_rowid","");
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rowid,","");

  // <srcfields>��<dstfields>Ϊ�գ�����Ŀ�ı��������ֵ��е��ֶ���䡣
  if ( (strlen(strsrcfields)==0) && (strlen(strdstfields)==0) )
  {
    strcpy(strsrcfields,TABFIELD.m_allfieldstr); strcpy(strdstfields,TABFIELD.m_allfieldstr);
  }

  // Ϊstrsrcfields����rowid�У�Ϊstrdstfields����sync_rowid��
  strcat(strsrcfields,",rowid"); strcat(strdstfields,",sync_rowid");

  lobfieldcount=0;
  // �ж�Ŀ�ı��Ƿ��ж������ֶΣ�����У����ֶεĸ��������lobfieldcount������
  stmt.prepare(\
     "select count(*) from USER_TAB_COLUMNS where table_name=upper(:1) and data_type in ('BLOB','CLOB')");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&lobfieldcount);
  stmt.execute();
  stmt.next();

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ����������
  KillLocked(&conn,strdsttname);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_increment exit.\n");

  exit(0);
}

// ��ȡuMaxKeyIDOld��uMinKeyID��uMaxKeyID��ֵ
BOOL LoadSyncKeyID()
{
  bContinue=FALSE;
  uMaxKeyIDOld=uMinKeyID=uMaxKeyID=0;

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("select keyid from T_SYNCLOG where tname=upper(:1)");
  stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&uMaxKeyIDOld);
  if (stmt.execute() != 0)
  {
    // �����T_SYNCLOG�������ڣ��Ͳ�������ʧ�ܣ�������ᴴ����
    if (stmt.cda.rc!=942)
    {
      logfile.Write("select T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
    }

    // T_SYNCLOG�������ڣ��ʹ�����
    stmt.prepare("create table T_SYNCLOG(tname varchar2(30),keyid number(15),primary key(tname))");
    stmt.execute();
  }
  else
  {
    if (stmt.next() != 0) 
    {
      stmt.prepare("insert into T_SYNCLOG(tname,keyid) values(upper(:1),0)");
      stmt.bindin(1,strdsttname,50);
      if (stmt.execute() != 0)
      {
        logfile.Write("insert T_SYNCLOG failed.\n%s\n",stmt.cda.message); return FALSE;
      }

      conn.commitwork();
    }
  }

  // ������Դ������־���л�ȡ����uMaxKeyIDOld����Сkeyid
  stmt.prepare("select min(keyid) from %s where keyid>:1",strsrclogtname);
  stmt.bindin(1,&uMaxKeyIDOld);
  stmt.bindout(1,&uMinKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrclogtname,stmt.cda.message); return FALSE;
  }

  stmt.next();

  // ���uMinKeyID==0�����ʾû����Ҫͬ���ļ�¼��
  if (uMinKeyID==0) return TRUE;

  // ������Դ������־���л�ȡ����uMinKeyID+100000�����keyid
  stmt.prepare("select max(keyid) from %s where keyid<=:1+100000",strsrclogtname);
  stmt.bindin(1,&uMinKeyID);
  stmt.bindout(1,&uMaxKeyID);
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n",strsrclogtname,stmt.cda.message); return FALSE;
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
  stmt.prepare("update T_SYNCLOG set keyid=:1 where tname=upper(:2)");
  stmt.bindin(1,&uMaxKeyID);
  stmt.bindin(2,strdsttname,50);
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

  // �������Դ���ݿ��ͬһ�ű��Ĳ����ǲ��в��������п������keyid���ҵ������
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

  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);

  UINT uMaxCount=0;
  UINT ccount=0;
  char rowid[31],rowidn[MAXCOUNT][31];

  if (lobfieldcount == 0) uMaxCount=maxcount;
  if (lobfieldcount >  0) uMaxCount=1;

  UINT keyidtmp=0;
  sqlstatement selsrc,delstmt,insstmt;

  UINT synccount=0;

  selsrc.connect(&conn);
  if (strcmp(strsrctname,strsrclogtname)==0)
    selsrc.prepare("select rowid,keyid from %s where keyid>=:1 and keyid<=:2",strsrclogtname);
  else
    selsrc.prepare("select sync_rowid,keyid from %s where keyid>=:1 and keyid<=:2",strsrclogtname);
  selsrc.bindin(1,&uMinKeyID);
  selsrc.bindin(2,&uMaxKeyID);
  selsrc.bindout(1,rowid,30);
  selsrc.bindout(2,&keyidtmp);

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
  /*
  insstmt.prepare("\
      insert into %s(%s) select %s from %s where rowid in (%s) %s",\
          strdsttname,strdstfields,strsrcfields,strsrctname,strBindStr,strand);
  */
  insstmt.prepare("\
    BEGIN\
      delete from %s where sync_rowid in (%s);\
      insert into %s(%s) select %s from %s where rowid in (%s) %s;\
    END;",strdsttname,strBindStr,strdsttname,strdstfields,strsrcfields,strsrctname,strBindStr,strand);\
  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1, rowidn[ii],30);
  }

  while (TRUE)
  {
    memset(rowid,0,sizeof(rowid));

    if (selsrc.next() != 0) break;

    strncpy(rowidn[ccount],rowid,30);

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
      }

      ProgramActive.WriteToFile();

      memset(rowidn,0,sizeof(rowidn));

      ccount=0;
    }
  }

  // ������ѭ��������ʱ���������uMaxCount���������ﴦ��
  for (ii=0; ii<ccount; ii++)
  {
    /*
    insstmt.prepare("\
        insert into %s(%s) select %s from %s where rowid=:1 %s",strdsttname,strdstfields,strsrcfields,strsrctname,strand);
    insstmt.bindin(1, rowidn[ii],30);
    */ 
    insstmt.prepare("\
      BEGIN\
        delete from %s where sync_rowid=:1;\
        insert into %s(%s) select %s from %s where rowid=:2 %s;\
      END;",strdsttname,strdsttname,strdstfields,strsrcfields,strsrctname,strand);
    insstmt.bindin(1, rowidn[ii],30);
    insstmt.bindin(2, rowidn[ii],30);
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
    }
  }

  logfile.WriteEx("ok,%lu,rows incrementd.\n",synccount);

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