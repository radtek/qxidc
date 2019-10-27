#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

int lobfieldcount=0;

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
// �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
// �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName();

// ִ������ͬ��
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcfields[2001];
char strdsttname[51];
char strdstfields[2001];
char strand[501];
char strignoreerror[11];

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_append xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_append \"<logfilename>/log/sqxj/synctable_append_ALLAWSDATA.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_ALLAWSDATA@SZIDC</srctname><srcfields></srcfields><dsttname>T_ALLAWSDATA</dsttname><dstfields></dstfields><and>and ossmocode like '5%%'</and><ignoreerror>FALSE</ignoreerror>\"\n\n");

    printf("����һ�����߳���������Oracle���ݿ�֮��ͬ�����ݣ���Ϊsynctable_append����Ĳ��䣬��Ŀ�����ݿ����Դ���ݿ������ݲ�һ��ʱ�����б�������԰�Դ���ݿ���е����ݲ���Ŀ�����ݱ��С�\n\n");
    printf("<logfilename>/log/sqxj/synctable_append_ALLAWSDATA.log</logfilename> ���������е���־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> Ŀ�����ݿ�����Ӳ�����\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> ���ݿ���ַ���������Ҫ��<connstr>"\
           "�������������ݿ���ͬ��\n");
    printf("<srctname>T_ALLAWSDATA@SZIDC</srctname> ����Դ�������ڱ���������ݿ���·�������Ϳ���ָ��Զ�����ݿ⡣\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> ����Դ����ֶ��б����������select��from֮�䣬"\
           "���ԣ�srcfields��������ʵ���ֶΣ�Ҳ�������κ�Oracle�ĺ��������㡣\n");
    printf("<dsttname>T_ALLAWSDATA</dsttname> �������ݵ�Ŀ�ı�����\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> �������ݵ�Ŀ�ı���ֶ��б���"\
           "<srcfields>��ͬ������������ʵ���ڵ��ֶΡ�\n");
    printf("<and>and ossmocode like '5%%'</and> ͬ������������select���where����Ĳ��֣�ע�⣬Ҫ��and ��ͷ��\n");
    printf("<ignoreerror>FALSE</ignoreerror> ���������ݴ���ʱ���Ƿ������һ�������ȡFALSE��"\
           "ֻ�е�����Դ���ĳЩ��¼�л���ʱ���Ų���TRUE��\n\n");

    printf("ע�⣺\n");
    printf("  1��<srcfields>��<dstfields>����Ϊ�գ����Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣\n");
    printf("     ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�\n");
    printf("  2��<charset>����Ϊ�գ����Ϊ�գ������򽫲��������ַ���������������ô�ڲ���ϵͳ�����б�����\n");
    printf("     ������ȷ��NLS_LANG����������\n");
    printf("  3��<dsttname>������Ŀ�ı�������sync_rowid�ֶΣ���������Ϊrowid��\n");
    printf("     ���ң�sync_rowid�ֶα�����Ψһ������\n");
    printf("  4�������������synctable_increment����Ĳ���������־�ļ���һ����ͬ��\n\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  memset(strand,0,sizeof(strand));
  memset(strignoreerror,0,sizeof(strignoreerror));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"and",strand,500);
  GetXMLBuffer(argv[1],"ignoreerror",strignoreerror,10);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)    { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)   { printf("dstfields is null.\n"); return -1; }
  // if (strlen(strand) == 0)       { printf("and is null.\n"); return -1; }
  // if (strlen(strignoreerror) == 0)       { printf("ignoreerror is null.\n"); return -1; }

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
  logfile.SetAlarmOpt("synctable_append");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"synctable_append",1200);

  // ���Ŀ�ı�����"@"�ַ�����һ�������Դ���Ŀ�ı�Ĺ�ϵ
  if (strstr(strdsttname,"@") > 0) 
  { 
    logfile.Write("%s��ض���%s������ָ�������ݿ�ı��ر�������ָ��Զ�����ݿ�ı�\n",\
                   strdsttname,strconnstr);
    return -1;
  }

  // �������ݿ�
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
  // �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
  // �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
  if (CheckDstTName()==FALSE) CallQuit(-1);

  ProgramActive.WriteToFile();

  if (SyncTable()==FALSE) CallQuit(-1);

  return 0;
}

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
// �ж�Ŀ�ı��Ƿ���sync_rowid�У����û�У�����ʧ��
// �ж�sync_rowid���Ƿ񴴽���Ψһ���������û�У�����ʧ��
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // ��ȡ�ñ�ȫ�����ֶ�
  TABFIELD.GetALLField(&conn,strdsttname);

  if (TABFIELD.m_fieldcount==0)
  {
    logfile.Write("%s�����ڣ��봴������\n",strdsttname); return FALSE;
  }

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
    logfile.Write("%s���sync_rowid��û�д���Ψһ�������봴������\n",strdsttname); return FALSE;
  }

  UpdateStr(TABFIELD.m_allfieldstr,",sync_rowid","");
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rowid,","");

  // �ɵ�ͬ��������sync_rownum�ֶΣ��ų���
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rownum,","");
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rownum","");

  // <srcfields>��<dstfields>Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣
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

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_append exit.\n");

  exit(0);
}

// ִ������ͬ��
BOOL SyncTable()
{
  char rowid[31];

  sqlstatement selstmt,insstmt;

  selstmt.connect(&conn);
  selstmt.prepare("select rowid from %s",strsrctname);
  selstmt.bindout(1,rowid,30);

  if (selstmt.execute() != 0)
  {
    logfile.Write("%s\n%s failed.\n",selstmt.m_sql,selstmt.cda.message); return FALSE;
  }
  
  insstmt.connect(&conn);
  insstmt.prepare("\
    BEGIN\
      delete from %s where sync_rowid=:1;\
      insert into %s(%s) select %s from %s where rowid=:2 %s;\
    END;",strdsttname,strdsttname,strdstfields,strsrcfields,strsrctname,strand);
  insstmt.bindin(1,rowid,30);
  insstmt.bindin(2,rowid,30);

  while (TRUE)
  {
    memset(rowid,0,sizeof(rowid));

    if ( (selstmt.cda.rpc>1) && (fmod(selstmt.cda.rpc,5000) < 1) )
    {
      logfile.Write("%d rows append.\n",selstmt.cda.rpc);
      ProgramActive.WriteToFile();
      conn.commitwork();
    }

    if (selstmt.next() != 0) break;

    if (insstmt.execute() != 0)
    {
      logfile.Write("\n%s\n%s failed.\n",insstmt.m_sql,insstmt.cda.message); 

      if ( (insstmt.cda.rc != 1) && (insstmt.cda.rc != 1410) )
      {
        if (strcmp(strignoreerror,"TRUE") != 0) return FALSE;
      }
    }
  }

  conn.commitwork();

  logfile.Write("%d rows append.\n",selstmt.cda.rpc);

  return TRUE;
}

