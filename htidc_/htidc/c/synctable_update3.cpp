#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

int lobfieldcount=0;

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
// �ж�Ŀ�ı��Ƿ���dstpkfieldname�У����û�У�����ʧ��
// �ж�dstpkfieldname���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName();

// ִ������ͬ��
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcpkfieldname[51];
char strdstpkfieldname[51];
char strsrcfields[2001];
char strdsttname[51];
char strdstfields[2001];
char strwhere[501];
char stralarmbz[21];
int  maxcount;

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_update3 xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/synctable_update3 \"<logfilename>/log/szqx/synctable_update3_INDEXINFO.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.98.24</connstr><srctname>T_INDEXINFO</srctname><srcpkfieldname>cityid</srcpkfieldname><dstpkfieldname>cityid</dstpkfieldname><srcfields>cityid,cityname,pubdate,gm</srcfields><dsttname>T_INDEXINFO@SZIDC</dsttname><dstfields>cityid,cityname,pubdate,gm</dstfields><where>where pubdate>sysdate-1</where><maxcount>300</maxcount><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("����һ�����߳���������Oracle���ݿ�֮��ͬ�����ݣ�ͬ���ķ�����ȫ��ˢ�»�����ˢ�¡�\n");
    printf("synctable_update3����Ϊ�˲�����Ŀ�����ݿ��д������ݿ���·���ӣ�����Ϊ�ڱ������ݿⴴ����·,�÷�����ͬsynctable_update1ʹ����ͬ����ͬ����constrΪ�������ݿ����Ӳ�����ֻ����dsttname�м�Ŀ�����ݿ���·��\n");
    printf("<logfilename>/log/szqx/synctable_update3_INDEXINFO.log</logfilename> ���������е���־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.98.24</connstr> �������ݿ�����Ӳ�����\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> ���ݿ���ַ���������Ҫ��<connstr>"\
           "�������������ݿ���ͬ��\n");
    printf("<srctname>T_INDEXINFO</srctname> ����Դ������\n");
    printf("<srcpkfieldname>cityid</srcpkfieldname> ����Դ��������ֶ�����ע�⣬���ֶ�ȡֵ�����������\n"\
           "���Դ���������ֶΣ���ô�����������ֶΣ����û�У�����rowid�����whereΪ�գ����ֶ������塣\n");
    printf("<dstpkfieldname>cityid</dstpkfieldname> ����Ŀ�ı�������ֶ�����ע�⣬���ֶ�ȡֵ�����������\n"\
           "���Ŀ�ı��������ֶΣ���ô�����������ֶΣ����û�У�����sync_rowid�����whereΪ�գ����ֶ������塣\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> ����Դ����ֶ��б����������select��from֮�䣬"\
           "���ԣ�srcfields��������ʵ���ֶΣ�Ҳ�������κ�Oracle�ĺ��������㡣\n");
    printf("<dsttname>T_INDEXINFO@SZIDC</dsttname> �������ݵ�Ŀ�ı���,�ڱ���������ݿ���·�������Ϳ���ָ��Զ�����ݿ⡣\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> �������ݵ�Ŀ�ı���ֶ��б���"\
           "<srcfields>��ͬ������������ʵ���ڵ��ֶΡ�\n");
    printf("<where>where pubdate>sysdate-1</where> ͬ������������select����where���֡�\n");
    printf("<maxcount>300</maxcount> ִ��һ��ͬ�������ļ�¼�����������300��\n");
    printf("<alarmbz>FALSE</alarmbz> ���������Ƿ񷢳��澯��ȱʡ��TRUE��\n\n");

    printf("ע�⣺\n");
    printf("  1��<srcfields>��<dstfields>����Ϊ�գ����Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣\n");
    printf("     ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�\n");
    printf("  2��<where>����Ϊ�գ����Ϊ�գ�ÿ�ζ���ˢ�±���ȫ���ļ�¼��\n");
    printf("  3��<charset>����Ϊ�գ����Ϊ�գ������򽫲��������ַ���������������ô�ڲ���ϵͳ�����б�����\n");
    printf("     ������ȷ��NLS_LANG����������\n");
    printf("  4������Դ���Ŀ�ı����Ҫ�����������û���������ɲ���rowidͬ����Ŀ�ı�������sync_rowid�ֶΣ���������Ϊrowid��\n");
    printf("     ���ң�sync_rowid�ֶα�����Ψһ������\n");
    printf("  5����������procctl���ȣ�����һ�μ�ִ��һ��ͬ����\n\n");
    printf("  6��<maxcount>�����ǳ���Ҫ���������Դ���зǷ����ݣ���<maxcount>Ϊ1ʱ��ʧ�ܵļ�¼�޷�ͬ��������\n");
    printf("     ��Ӱ�������ļ�¼��Ŀ�ı���ʧһ�����ݣ����<maxcount>����1������ʧ�ܼ�¼��ͬһ���ε�ͬ����ȫ��ʧ�ܣ�Ŀ�ı���ʧ�������ݡ�\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcpkfieldname,0,sizeof(strsrcpkfieldname));
  memset(strdstpkfieldname,0,sizeof(strdstpkfieldname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  memset(strwhere,0,sizeof(strwhere));
  memset(stralarmbz,0,sizeof(stralarmbz));
  maxcount=0;

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcpkfieldname",strsrcpkfieldname,50);
  GetXMLBuffer(argv[1],"dstpkfieldname",strdstpkfieldname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"where",strwhere,500);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  if (strlen(strwhere)!=0) { if (strlen(strsrcpkfieldname) == 0)    { printf("srcpkfieldname is null.\n"); return -1; } }
  if (strlen(strwhere)!=0) { if (strlen(strdstpkfieldname) == 0)    { printf("dstpkfieldname is null.\n"); return -1; } }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)    { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)   { printf("dstfields is null.\n"); return -1; }
  // if (strlen(strwhere) == 0)       { printf("where is null.\n"); return -1; }
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");
  if ( (maxcount==0) || (maxcount>300) ) maxcount=300;

  // �淶�������ֶ����Ĵ�Сд
  ToUpper(strsrctname);       ToUpper(strdsttname);
  ToLower(strsrcpkfieldname); ToLower(strdstpkfieldname); ToLower(strsrcfields); ToLower(strdstfields);

  // ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�
  if ( (strlen(strsrcfields)==0) || (strlen(strdstfields)==0) )
  {
    memset(strsrcfields,0,sizeof(strsrcfields)); memset(strdstfields,0,sizeof(strdstfields));
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);
 

  // ����־�ļ�
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_update");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"synctable_update3",1200);

  // ������ر�����"@"�ַ�����һ�������Դ���Ŀ�ı�Ĺ�ϵ
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

  // �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
  // �ж�Ŀ�ı��Ƿ���dstpkfieldname�У����û�У�����ʧ��
  // �ж�dstpkfieldname���Ƿ񴴽���Ψһ���������û�У�����ʧ��
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ���������
  KillLocked(&conn,strdsttname);

  // ִ������ͬ��
  if (SyncTable()==FALSE) CallQuit(-1);

  return 0;
}

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
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

  // Ŀ�ı��������·���
  char strdblink[101],*strpos=0,strtablename[51];
  memset(strdblink,0,sizeof(strdblink));
  memset(strtablename,0,sizeof(strtablename));

  if ((strpos = strstr(strdsttname,"@")) == 0) return FALSE;

  strncpy(strtablename,strdsttname,strlen(strdsttname)-strlen(strpos));
  strncpy(strdblink,strpos,100);

  // �����ȫ��ˢ�µ�ͬ����ʽ������ļ�鶼������
  if (strlen(strwhere)!=0) 
  {
    // �ж�Ŀ�ı��Ƿ���dstpkfieldname�У����û�У�����ʧ��
    if (strstr(TABFIELD.m_allfieldstr,strdstpkfieldname) == 0)
    {
      logfile.Write("%s��û��%s�У��봴������\n",strdsttname,strdstpkfieldname); return FALSE;
    }
  
    // �ж�dstpkfieldname���Ƿ񴴽���Ψһ���������û�У�����ʧ��
    char stroutput[51];
    memset(stroutput,0,sizeof(stroutput));
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare("select column_name from USER_IND_COLUMNS%s where table_name=upper('%s') and column_name=upper('%s')",strdblink,strtablename,strdstpkfieldname);

    //stmt.bindin(1,strdsttname,50);
    stmt.bindout(1,stroutput,50);
    stmt.execute();
    stmt.next();
  
    if (strlen(stroutput)==0)
    {
      logfile.Write("%s���%s��û�д���Ψһ�������봴������\n",strdsttname,strdstpkfieldname); return FALSE;
    }
  }

  UpdateStr(TABFIELD.m_allfieldstr,",sync_rowid","");
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rowid,","");

  // <srcfields>��<dstfields>Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣
  if ( (strlen(strsrcfields)==0) && (strlen(strdstfields)==0) )
  {
    strcpy(strsrcfields,TABFIELD.m_allfieldstr); strcpy(strdstfields,TABFIELD.m_allfieldstr);
  }

  // ����ǰ�rowidͬ������Ϊstrsrcfields����rowid�У�Ϊstrdstfields����sync_rowid��
  if (strcmp(strsrcpkfieldname,"rowid")==0) 
  {
    strcat(strsrcfields,",rowid"); strcat(strdstfields,",sync_rowid");
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

  logfile.Write("synctable_update3 exit.\n");

  exit(0);
}

#define MAXCOUNT 300

// ִ������ͬ��
BOOL SyncTable()
{
  CTimer Timer;
  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);

  // ���ͬ�������Ӿ�Ϊ�գ���һ����ͬ���ñ�ȫ���ļ�¼��
  // ���ֱ�ļ�¼һ�㲻��̫�࣬Ϊ�˱�֤����һ���Ժ�Ч�ʣ���һ��SQL���̸㶨����
  if (strlen(strwhere)==0)
  {
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare("\
      BEGIN\
         delete from %s;\
         insert into %s(%s) select %s from %s;\
      END;",strdsttname,strdsttname,strdstfields,strsrcfields,strsrctname);
    if (stmt.execute() != 0)
    {
      logfile.WriteEx("failed.\n%s\n%s\n",stmt.m_sql,stmt.cda.message); return FALSE;
    }

    logfile.WriteEx("ok,table refreshed(%d).\n",Timer.Elapsed());

    conn.commitwork();

    return TRUE;
  }

  // ͬ�������Ӿ䲻Ϊ�գ��Ͱ���������
  UINT uMaxCount=0;
  UINT ccount=0;
  char pkfieldvalue[51],pkfieldvaluen[MAXCOUNT][51];

  if (lobfieldcount == 0) uMaxCount=maxcount;
  if (lobfieldcount >  0) uMaxCount=1;

  sqlstatement selsrc,delstmt,insstmt;

  selsrc.connect(&conn);
  selsrc.prepare("select %s from %s %s",strsrcpkfieldname,strsrctname,strwhere);
  selsrc.bindout(1,pkfieldvalue,50);

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
  insstmt.prepare("\
    BEGIN\
      delete from %s where %s in (%s);\
      insert into %s(%s) select %s from %s where %s in (%s);\
    END;",strdsttname,strdstpkfieldname,strBindStr,strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname,strBindStr);\
  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1, pkfieldvaluen[ii],50);
  }

  while (TRUE)
  {
    memset(pkfieldvalue,0,sizeof(pkfieldvalue));

    if (selsrc.next() != 0) break;

    strncpy(pkfieldvaluen[ccount],pkfieldvalue,50);

    ccount++;

    // ÿuMaxCount����¼�Ͳ���һ��
    if (ccount == uMaxCount)
    {
      if (insstmt.execute() != 0)
      {
        logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

        if (insstmt.cda.rc != 1) return FALSE;
      }

      ProgramActive.WriteToFile();

      conn.commitwork();

      memset(pkfieldvaluen,0,sizeof(pkfieldvaluen));

      ccount=0;
    }
  }

  // ������ѭ�������ʱ���������uMaxCount���������ﴦ��
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      BEGIN\
        delete from %s where %s=:1;\
        insert into %s(%s) select %s from %s where %s=:2;\
      END;",strdsttname,strdstpkfieldname,strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname);
    insstmt.bindin(1, pkfieldvaluen[ii],50);
    insstmt.bindin(2, pkfieldvaluen[ii],50);
    if (insstmt.execute() != 0)
    {
      logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

      if (insstmt.cda.rc != 1) return FALSE;
    }

    conn.commitwork();
  }

  logfile.WriteEx("ok,%lu rows updated(%d).\n",selsrc.cda.rpc,Timer.Elapsed());

  return TRUE;
}

