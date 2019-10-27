#include "_public.h"
#include "_ooci.h"

// ������Ĳ���
struct st_arg
{
  char connstr[101];
  char charset[51];
  char tname[51];
  char cols[1001];
  char fieldname[1001];
  char fieldlen[501];
  int  exptype;
  char andstr[501];
  char bname[51];
  char ename[51];
  char taskname[51];
  char exppath[301];
  int  timetvl;
} starg;

CLogFile logfile;

connection conn;

// �������ҵ������������
bool _exptables();

void EXIT(int sig);

vector<string> vfieldname; // ��Ų��fieldname�������
vector<int>    vfieldlen;  // ��Ų��fieldlen�������
int maxfieldlen;           // ���fieldlen�����ֵ
void SplitFields();        // ���fieldname��fieldlen

// ��ʾ����İ���
void _help(char *argv[]);

int maxkeyid;   // �ѵ������ݵ�����keyid

bool LoadMaxKeyid(); // ��ϵͳ����T_SYSARG���м����ѵ������ݵ�����keyid
bool UptMaxKeyid();  // ����ϵͳ����T_SYSARG�����ѵ������ݵ�����keyid
  
// ��xml����������starg�ṹ��
bool _xmltoarg(char *strxmlbuffer);

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // �ر�ȫ�����źź��������
  CloseIOAndSignal();

  // ��������˳����ź�
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("����־�ļ�ʧ�ܣ�%s����\n",argv[1]); return -1;
  }

  // ��xml����������starg�ṹ��
  if (_xmltoarg(argv[2])==false) return -1;

  while (true)
  {
    // �������ݿ�
    if (conn.connecttodb(starg.connstr,"Simplified Chinese_China.ZHS16GBK") != 0)
    {
      logfile.Write("connect database %s failed.\n",starg.connstr); sleep(starg.timetvl); continue;
    }

    // logfile.Write("export table %s.\n",starg.tname);

    // �������ݵ�������
    if (_exptables() == false) logfile.Write("export tables failed.\n");

    conn.disconnect();   // �Ͽ������ݿ������

    sleep(starg.timetvl);
  }

  return 0;
}

void EXIT(int sig)
{
  logfile.Write("�����˳���sig=%d\n\n",sig);

  exit(0);
}

// ��ʾ����İ���
void _help(char *argv[])
{
  printf("\n");
  printf("Using:/htidc/public/bin/exptables logfilename xmlbuffer\n\n");

  printf("Sample:/htidc/public/bin/exptables /log/shqx/exptables_surfdata_for_hb.log \"<connstr>shqx/pwdidc@snorcl11g_198</connstr><charset>Simplified Chinese_China.ZHS16GBK</charset><tname>T_SURFDATA</tname><cols>obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis</cols><fieldname>obtid,ddatetime,t,p,u,wd,wf,r,vis</fieldname><fieldlen>5,14,8,8,8,8,8,8,8</fieldlen><exptype>1</exptype><andstr> and obtid in ('59293','50745')</andstr><bname>SURFDATA_</bname><ename>_for_hb</ename><taskname>SURFDATA_FOR_HB</taskname><exppath>/data/shqx/exp/tohb</exppath><timetvl>30</timetvl>\"\n\n");
  printf("Sample:/htidc/public/bin/exptables /log/shqx/exptables_obtcode_for_hb.log \"<connstr>shqx/pwdidc@snorcl11g_198</connstr><charset>Simplified Chinese_China.ZHS16GBK</charset><tname>T_OBTCODE</tname><cols>obtid,obtname,provname,lat,lon,height</cols><fieldname>obtid,obtname,provname,lat,lon,height</fieldname><fieldlen>5,30,30,8,8,8</fieldlen><exptype>2</exptype><andstr> and rsts=1 and obtid in ('59293','50745')</andstr><bname>OBTCODE_</bname><ename>_for_hb</ename><exppath>/data/shqx/exp/tohb</exppath><timetvl>300</timetvl>\"\n\n");

  printf("���������������ĵĹ�������ģ�飬�����ݿ�ı��е�����������xml�ļ����������ݽ�����\n");
  printf("logfilename�Ǳ��������е���־�ļ���\n");
  printf("xmlbufferΪ�ļ�����Ĳ��������£�\n");
  printf("���ݿ�����Ӳ��� <connstr>shqx/pwdidc@snorcl11g_198</connstr>\n");
  printf("���ݿ���ַ��� <charset>Simplified Chinese_China.ZHS16GBK</charset> �������Ҫ������Դ���ݿⱣ��>һ�£�����������������������\n");
  printf("���������ݵı��� <tname>T_SURFDATA</tname>\n");
  printf("��Ҫ�����ֶε��б� <cols>obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis</cols> ���Բ��ú�����\n");
  printf("�����ֶεı����б� <fieldname>obtid,ddatetime,t,p,u,wd,wf,r,vis</fieldname> ������colsһһ��Ӧ��\n");
  printf("�����ֶεĳ����б� <fieldlen>5,14,8,8,8,8,8,8,8</fieldlen> ������colsһһ��Ӧ��\n");
  printf("�������ݵķ�ʽ <exptype>1</exptype> 1-����������2-ȫ��������\n");
  printf("�������ݵĸ������� <andstr> and obtid in ('59293','50745')</andstr> ע�⣬�ؼ���and�����١�\n");
  printf("�����ļ���������ǰ���� <bname>SURFDATA_</bname>\n");
  printf("�����ļ��������ĺ󲿷� <ename>_for_hb</ename>\n");
  printf("������������� <taskname>SURFDATA_FOR_HB</taskname> ��exptype=1ʱ�ò�����Ч��\n");
  printf("�����ļ���ŵ�Ŀ¼ <exppath>/data/shqx/exp/tohb</exppath>\n");
  printf("�������ݵ�ʱ���� <timetvl>30</timetvl> ��λ���룬�������10��\n");
  printf("���ϲ���������taskname��andstr�������ֶζ�������Ϊ�ա�\n\n\n");
}

// ��xml����������starg�ṹ��
bool _xmltoarg(char *strxmlbuffer)
{
  memset(&starg,0,sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer,"connstr",starg.connstr);
  if (strlen(starg.connstr)==0) { logfile.Write("connstr is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"charset",starg.charset);
  if (strlen(starg.charset)==0) { logfile.Write("charset is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"tname",starg.tname);
  if (strlen(starg.tname)==0) { logfile.Write("tname is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"cols",starg.cols);
  if (strlen(starg.cols)==0) { logfile.Write("cols is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"fieldname",starg.fieldname);
  if (strlen(starg.fieldname)==0) { logfile.Write("fieldname is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"fieldlen",starg.fieldlen);
  if (strlen(starg.fieldlen)==0) { logfile.Write("fieldlen is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"exptype",&starg.exptype);
  if ( (starg.exptype!=1) && (starg.exptype!=2) ) { logfile.Write("exptype is not in (1,2).\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"andstr",starg.andstr);
  if (strlen(starg.andstr)==0) { logfile.Write("andstr is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"bname",starg.bname);
  if (strlen(starg.bname)==0) { logfile.Write("bname is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"ename",starg.ename);
  if (strlen(starg.ename)==0) { logfile.Write("ename is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"taskname",starg.taskname);
  if ( (starg.exptype==1) && (strlen(starg.taskname)==0) ) { logfile.Write("taskname is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"exppath",starg.exppath);
  if (strlen(starg.exppath)==0) { logfile.Write("exppath is null.\n"); return false; }

  GetXMLBuffer(strxmlbuffer,"timetvl",&starg.timetvl);
  if (starg.timetvl==0) { logfile.Write("timetvl is null.\n"); return false; }

  // ���fieldname��fieldlen
  SplitFields();

  // �ж�fieldname��fieldlen��Ԫ�صĸ���һ��Ҫ��ͬ
  if (vfieldname.size() != vfieldlen.size() ) { logfile.Write("fieldname��fieldlen��Ԫ�ظ�����ͬ��.\n"); return false; }

  return true;
}

// �������ҵ������������
bool _exptables()
{
  // ��ϵͳ����T_SYSARG���м����ѵ������ݵ�����keyid
  if (LoadMaxKeyid()==false) { logfile.Write("LoadMaxKeyid() failed.\n"); return false; }

  // ���ɵ������ݵ�SQL���
  char strsql[4096];   
  char fieldvalue[vfieldname.size()][maxfieldlen+1]; // �����������Ϊһ����ά����
  memset(strsql,0,sizeof(strsql));
  if (starg.exptype==1)
    sprintf(strsql,"select %s,keyid from %s where 1=1 and keyid>%d %s order by keyid",starg.cols,starg.tname,maxkeyid,starg.andstr);
  else
    sprintf(strsql,"select %s from %s where 1=1 %s",starg.cols,starg.tname,starg.andstr);
  sqlstatement stmt(&conn);
  stmt.prepare(strsql);
  for (int ii=0;ii<vfieldname.size();ii++)
  {
    stmt.bindout(ii+1,fieldvalue[ii],vfieldlen[ii]);
  }
  // �����������������Ҫ��keyid�ֶ�
  if (starg.exptype==1) stmt.bindout(vfieldname.size()+1,&maxkeyid);
  
  // ִ�е������ݵ�SQL
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n%s\n",starg.tname,stmt.m_cda.message,stmt.m_sql); return false;
  }

  int  iFileSeq=1;   // �������ļ������
  char strFileName[301],strLocalTime[21];
  CFile File;

  while (true)
  {
    memset(fieldvalue,0,sizeof(fieldvalue));
   
    if (stmt.next() !=0) break;

    // ������д���ļ�
    if (File.IsOpened()==false)
    {
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyymmddhh24miss");
      memset(strFileName,0,sizeof(strFileName));
      sprintf(strFileName,"%s/%s%s%s_%d.xml",starg.exppath,starg.bname,strLocalTime,starg.ename,iFileSeq++);
      if (File.OpenForRename(strFileName,"w")==false)
      {
        logfile.Write("File.OpenForRename(%s) failed.\n",strFileName); return false;
      }
      File.Fprintf("<data>\n");
    }

    for (int ii=0;ii<vfieldname.size();ii++)
    {
      File.Fprintf("<%s>%s</%s>",vfieldname[ii].c_str(),fieldvalue[ii],vfieldname[ii].c_str());
    }
    File.Fprintf("<endl/>\n");

    // 1000����¼д��һ���ļ����
    if (stmt.m_cda.rpc%1000==0)
    {
      File.Fprintf("</data>\n");
      if (File.CloseAndRename()==false)
      {
        logfile.Write("File.CloseAndRename(%s) failed.\n",strFileName); return false;
      }

      // ����ϵͳ����T_SYSARG�����ѵ������ݵ�����keyid
      if (UptMaxKeyid()==false) { logfile.Write("UptMaxKeyid() failed.\n"); return false; }

      logfile.Write("create file %s ok.\n",strFileName);
    }
  }

  // ����1000����д��һ���ļ�
  if (File.IsOpened()==true)
  {
    File.Fprintf("</data>\n");
    if (File.CloseAndRename()==false)
    {
      logfile.Write("File.CloseAndRename(%s) failed.\n",strFileName); return false;
    }

    // ����ϵͳ����T_SYSARG�����ѵ������ݵ�����keyid
    if (UptMaxKeyid()==false) { logfile.Write("UptMaxKeyid() failed.\n"); return false; }

    logfile.Write("create file %s ok.\n",strFileName);
  }

  if (stmt.m_cda.rpc>0) logfile.Write("���ε�����%d����¼��\n",stmt.m_cda.rpc);

  return true;
}

// ���fieldname��fieldlen
void SplitFields()
{
  vfieldname.clear(); vfieldlen.clear(); maxfieldlen=0;
  
  CCmdStr CmdStr;
  CmdStr.SplitToCmd(starg.fieldname,",");
  vfieldname.swap(CmdStr.m_vCmdStr);

  int ifieldlen=0;
  CmdStr.SplitToCmd(starg.fieldlen,",");
  for (int ii=0;ii<CmdStr.CmdCount();ii++)
  {  
    CmdStr.GetValue(ii,&ifieldlen);
    if (ifieldlen>maxfieldlen) maxfieldlen=ifieldlen;   // �õ�fieldlen�����ֵ
    vfieldlen.push_back(ifieldlen);
  }
}

// ��ϵͳ����T_SYSARG���м����ѵ������ݵ�����keyid
bool LoadMaxKeyid()
{
  if (starg.exptype!=1) return true;

  sqlstatement stmt(&conn);
  stmt.prepare("select argvalue from T_SYSARG where argcode=:1");
  stmt.bindin(1,starg.taskname,50);
  stmt.bindout(1,&maxkeyid);
  if (stmt.execute() != 0)
  {
    logfile.Write("select T_SYSARG failed.\n%s\n%s\n",stmt.m_cda.message,stmt.m_sql); return false;
  }

  // �����¼�����ڣ�����һ�¼�¼��
  if (stmt.next() != 0)
  {
    stmt.prepare("insert into T_SYSARG(argcode,argname,argvalue) values(:1,:2,0)");
    stmt.bindin(1,starg.taskname,50);
    stmt.bindin(2,starg.taskname,50);
    stmt.execute();
    conn.commit();
  }

  // logfile.Write("maxkeyid=%d\n",maxkeyid);

  return true;
}

// ����ϵͳ����T_SYSARG�����ѵ������ݵ�����keyid
bool UptMaxKeyid()
{
  if (starg.exptype!=1) return true;

  sqlstatement stmt(&conn);
  stmt.prepare("update T_SYSARG set argvalue=:1 where argcode=:2");
  stmt.bindin(1,&maxkeyid);
  stmt.bindin(2,starg.taskname,50);
  if (stmt.execute() != 0)
  {
    logfile.Write("select T_SYSARG failed.\n%s\n%s\n",stmt.m_cda.message,stmt.m_sql); return false;
  }

  conn.commit();

  return true;
}
