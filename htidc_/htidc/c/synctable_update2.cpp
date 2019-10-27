#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

// �ж�Ŀ�ı��Ƿ���ڣ���������ڣ�����ʧ��
BOOL CheckDstTName();

// ִ������ͬ��
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[101];
char strsrcfields[2001];
char strdsttname[101];
char strdstfields[2001];
char strwhere[501];
char stralarmbz[21];

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_update2 xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/synctable_update2 \"<logfilename>/log/sqxj/synctable_update2_INDEXINFO.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_INDEXINFO@SZIDC</srctname><srcfields>cityid,cityname,pubdate,gm</srcfields><dsttname>T_INDEXINFO</dsttname><dstfields>cityid,cityname,pubdate,gm</dstfields><where>where pubdate>sysdate-1</where><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("����һ�����߳���������Oracle���ݿ�֮��ͬ�����ݣ�ͬ���ķ�����ȫ��ˢ�»�����ˢ�¡�\n");
    printf("synctable_update2����Ҫ��Ŀ�ı���������Ҳ����Ҫ����sync_rowid�У�ִ��һ���洢���̣�ɾ��-����Ŀ�ı�\n");
    printf("���ԣ�������������ͬ��������̫�������\n");
    printf("���У����Դ��û��delete��������synctable_update1��synctable_update2�����ԣ������delete��������ֻ���ñ������ˡ�\n");
    printf("<logfilename>/log/sqxj/synctable_update2_INDEXINFO.log</logfilename> ���������е���־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr> Ŀ�����ݿ�����Ӳ�����\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> ���ݿ���ַ���������Ҫ��<connstr>"\
           "�������������ݿ���ͬ��\n");
    printf("<srctname>T_INDEXINFO@SZIDC</srctname> ����Դ�������ڱ���������ݿ���·�������Ϳ���ָ��Զ�����ݿ⡣\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> ����Դ����ֶ��б����������select��from֮�䣬"\
           "���ԣ�srcfields��������ʵ���ֶΣ�Ҳ�������κ�Oracle�ĺ��������㡣\n");
    printf("<dsttname>T_INDEXINFO</dsttname> �������ݵ�Ŀ�ı�����\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> �������ݵ�Ŀ�ı���ֶ��б���"\
           "<srcfields>��ͬ������������ʵ���ڵ��ֶΡ�\n");
    printf("<where>where pubdate>sysdate-1</where> ͬ������������select����where���֡�\n");
    printf("<alarmbz>FALSE</alarmbz> ���������Ƿ񷢳��澯��ȱʡ��TRUE��\n\n");

    printf("ע�⣺\n");
    printf("  1��<srcfields>��<dstfields>����Ϊ�գ����Ϊ�գ�����Ŀ�ı�������ֵ��е��ֶ���䡣\n");
    printf("     ���<srcfields>��<dstfields>��ֻҪ��һ������Ϊ�գ�������Ϊ����������Ϊ�ա�\n");
    printf("  2��<where>����Ϊ�գ����Ϊ�գ�ÿ�ζ���ˢ�±���ȫ���ļ�¼��\n");
    printf("  3��<charset>����Ϊ�գ����Ϊ�գ������򽫲��������ַ���������������ô�ڲ���ϵͳ�����б�����\n");
    printf("     ������ȷ��NLS_LANG����������\n");
    printf("  4��<dsttname>������Ŀ�ı�������sync_rowid�ֶΣ���������Ϊrowid��\n");
    printf("     ���ң�sync_rowid�ֶα�����Ψһ������\n");
    printf("  5����������procctl���ȣ�����һ�μ�ִ��һ��ͬ����\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  memset(strwhere,0,sizeof(strwhere));
  memset(stralarmbz,0,sizeof(stralarmbz));

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,100);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,100);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"where",strwhere,500);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);

  if (strlen(strlogfilename) == 0)      { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)          { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)          { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)         { printf("srctname is null.\n"); return -1; }
  // if (strlen(strsrcfields) == 0)     { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)         { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)     { printf("dstfields is null.\n"); return -1; }
  // if (strlen(strwhere) == 0)         { printf("where is null.\n"); return -1; }
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");

  // �淶�������ֶ����Ĵ�Сд
  ToUpper(strsrctname);  ToUpper(strdsttname);
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
 

  // ����־�ļ�
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_update2");

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"synctable_update2",1200);

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
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ���������
  KillLocked(&conn,strdsttname);

  // ִ������ͬ��
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

  // ������sync_rowid�ֶ�
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

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // ɱ����ͬ��Ŀ�ı��йصĻỰ���������
  KillLocked(&conn,strdsttname);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_update2 exit.\n");

  exit(0);
}

// ִ������ͬ��
BOOL SyncTable()
{
  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);
 
  char strLocalTime[21];
  char strsysdate[101];
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strsysdate,0,sizeof(strsysdate));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  sprintf(strsysdate,"to_date('%s','yyyymmddhh24miss')",strLocalTime);
  
  UpdateStr(strwhere,"sysdate",strsysdate,TRUE);

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    BEGIN\
       delete from %s %s;\
       insert into %s(%s) select %s from %s %s;\
    END;",strdsttname,strwhere,strdsttname,strdstfields,strsrcfields,strsrctname,strwhere);
  if (stmt.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",stmt.m_sql,stmt.cda.message); return FALSE;
  }

  logfile.WriteEx("ok,table refreshed(%s)\n",strwhere);

  conn.commitwork();

  return TRUE;
}

