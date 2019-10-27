#include "_public.h"
#include "_oracle.h"

void EXIT(int sig);

CLogFile       logfile;
connection     conn;
CProgramActive ProgramActive;
FILE           *listfp;
CDir           Dir;

char strstdname[31]; 
char strlogfilename[301]; 
char stroutputpath[301]; 
char strpathname[301];
char strandchild[10];
char strmatchstr[101];
char strtype[301];
char strtimesql[301];
char strconnstr[101];
char strlistfilename[301]; 
int  itimetvl;
int  umtime;

// �ļ���Ϣ
struct st_fileinfo
{
  char ddatetime[31]; // ����ʱ��
  char filepath[301]; // �ļ�·��
  char filename[301]; // �ļ���
  char modtime[31];   // �ļ��޸�ʱ��
  UINT filesize;      // �ļ���С
  int  filests;       // 1-�����������,2-������δ��⡣
};

struct st_fileinfo stfileinfo;

// ���η������ļ��嵥������
vector<struct st_fileinfo> v_newfileinfo;

// �ϴη������ļ��嵥������
vector<struct st_fileinfo> v_oldfileinfo;

// �������ļ���Ϣ����XML�ļ�
BOOL WriteToXML();

// ���Ѿ��������ļ���Ϣд��List�ļ�
BOOL WriteToListFile();

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./fileinfo xmlbuffer\n");

    printf("Example:/htidc/htidc/bin/procctl 120 /htidc/htidc/bin/getfileinfo \"<logfilename>/log/szqx/getfileinfo_radar_dpradar_gd.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.98.31</connstr><type>radar_dpradar_gd</type><timesql>select substr(:1,16,14) from dual</timesql><stdname>UNSTRUCT_FILESINFO</stdname><outputpath>/tmp/htidc/qxmonclient</outputpath><pathname>/szmbdata01/radar/dpradar/gd/2019/{YYYY}{MM}{DD}</pathname><andchild>FALSE</andchild><matchstr>*.*</matchstr><timetvl>-8</timetvl><mtime>24</mtime><listfilename>/tmp/htidc/list/getfileinfo_radar_dpradar_gd.xml</listfilename>\"\n\n");

    printf("����(����)�ռ�ָ��Ŀ¼�µ��ļ���Ϣ������xml�ļ���Ȼ����⡣\n");
    printf("<logfilename>/tmp/htidc/log/fileinfo_ecdata</logfilename>�Ǳ��������־�ļ�����\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.98.31</connstr> ���ݿ����Ӳ���������ִ�������SQL��䡣\n");
    printf("<type>radar_dpradar_gd �������ͣ�����:xx_xx_xx���㶫ʡ˫ƫ���״�����ݣ�radar_dpradar_gd ��\n");
    printf("<timesql>select substr(:1,16,14) from dual ȡ����ʱ��SQL��䣬:1�����ļ���������Ϊ�ա�\n");
    printf("<stdname>UNSTRUCT_FILESINFO</stdname> xml�ļ�ǰ׺\n");
    printf("<outputpath>/tmp/htidc/qxmonclient</outputpath>xml�ļ����Ŀ¼��\n");
    printf("<pathname>/ecdata</pathname>���ռ��ļ���ŵ�Ŀ¼��\n");
    printf("<andchild>TRUE</andchild>�Ƿ�ɨ����Ŀ¼��\n");
    printf("<matchstr>S1D*</matchstr>Ŀ¼���ļ���ƥ�����\n");
    printf("<mtime>24</mtime> �������ļ�ʱ��ķ�Χ����λ��Сʱ������ļ�ʱ���ڱ�����֮ǰ���Ͳ����䡣\n");
    printf("<listfilename>getfileinfo_radar_dpradar_gd.xml ������ռ������ļ��б��xml�ļ���\n");
    printf("xmlbuffer���Դ���ʱ�������<timetvl>-8</timetvl> Ϊʱ�������ƫ��ʱ�䡣"\
           "Ŀǰ���Դ�������ʱ�������{YYYY}��4λ���꣩��{YYY}������λ���꣩��"\
           "{YY}������λ���꣩��{MM}�����£���{DD}�����գ���{HH}��ʱʱ����{MI}���ַ֣���{SS}�����룩��\n");
    printf("���ϵĲ���ֻ��timetvl��mtimeΪ��ѡ�����������Ķ����\n\n\n");

    return -1;
  }

  char strXmlBuffer[4001];
  memset(strXmlBuffer,0,sizeof(strXmlBuffer));
  strncpy(strXmlBuffer,argv[1],4000);

  memset(strstdname,0,sizeof(strstdname));
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(stroutputpath,0,sizeof(stroutputpath));
  memset(strpathname,0,sizeof(strpathname));
  memset(strandchild,0,sizeof(strandchild));
  memset(strmatchstr,0,sizeof(strmatchstr));
  memset(strtype,0,sizeof(strtype));
  memset(strtimesql,0,sizeof(strtimesql));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strlistfilename,0,sizeof(strlistfilename));
  itimetvl=0;
  umtime=0;

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);

  // ����xmlbuffer�е�ʱ�����
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"stdname",strstdname,30);
  GetXMLBuffer(strXmlBuffer,"logfilename",strlogfilename,300);
  GetXMLBuffer(strXmlBuffer,"outputpath",stroutputpath,300);
  GetXMLBuffer(strXmlBuffer,"pathname",strpathname,300);
  GetXMLBuffer(strXmlBuffer,"matchstr",strmatchstr,100);
  GetXMLBuffer(strXmlBuffer,"andchild",strandchild,10);
  GetXMLBuffer(strXmlBuffer,"mtime",&umtime);
  GetXMLBuffer(strXmlBuffer,"type",strtype,300);
  GetXMLBuffer(strXmlBuffer,"timesql",strtimesql,300);
  GetXMLBuffer(strXmlBuffer,"connstr",strconnstr,100);
  GetXMLBuffer(strXmlBuffer,"listfilename",strlistfilename,300);


  if (strlen(strstdname) == 0) { logfile.Write("stdname is null.\n"); return -1; }
  if (strlen(strlogfilename) == 0) { logfile.Write("logfilename is null.\n"); return -1; }
  if (strlen(stroutputpath) == 0) { logfile.Write("outputpath is null.\n"); return -1; }
  if (strlen(strpathname) == 0) { logfile.Write("pathname is null.\n"); return -1; }
  if (strlen(strandchild) == 0) { logfile.Write("andchild is null.\n"); return -1; }
  if (strlen(strmatchstr) == 0) { logfile.Write("matchstr is null.\n"); return -1; }
  if (strlen(strtimesql) == 0)  { logfile.Write("timesql is null.\n"); return -1; }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("getfileinfo");

  // ����ʱ��3600��
  ProgramActive.SetProgramInfo(&logfile,"getfileinfo",3600);

  // ��������Դ���ݿ�
  if (conn.connecttodb(strconnstr) != 0)
  {
     logfile.Write("conn.connecttodb(%s) failed\n",strconnstr); return -1;
  }
  
  // д����̻��Ϣ
  ProgramActive.WriteToFile();

  // �����ϴη������ļ���Ϣ�嵥��v_oldfileinfo
  char strLine[2048];

  v_oldfileinfo.clear();

  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // ���ļ��л�ȡһ��
      if (FGETS(strLine,2000,listfp,"<endl/>") == FALSE) break;

      memset(&stfileinfo,0,sizeof(stfileinfo));

      GetXMLBuffer(strLine,"filename", stfileinfo.filename,300);
      GetXMLBuffer(strLine,"modtime",  stfileinfo.modtime,30);
      GetXMLBuffer(strLine,"filesize",&stfileinfo.filesize);

      v_oldfileinfo.push_back(stfileinfo);
    }
    fclose(listfp);
  }

  // ��ȡĿ¼�µ�ȫ���ļ�
  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));
  LocalTime(strmtime,"yyyymmddhh24miss",0-umtime*60*60);

  Dir.SetDateFMT("yyyymmddhh24miss");

  v_newfileinfo.clear();

  // ��Ŀ¼����ȡ�ļ���������
  BOOL bandchild=FALSE;

  if ( (strcmp(strandchild,"TRUE")==0) || (strcmp(strandchild,"true")==0) ) bandchild=TRUE;

  if (Dir.OpenDirNoSort(strpathname,bandchild) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",strpathname); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    if (MatchFileName(Dir.m_FileName,strmatchstr)==FALSE) continue;

    // ����ļ�ʱ��С��umtime���Ͷ�������ļ�
    if ( (umtime>0) && (strcmp(Dir.m_ModifyTime,strmtime)<0) ) continue;

    strncpy(stfileinfo.filepath,Dir.m_DirName,300);  // �ļ�·��
    strncpy(stfileinfo.filename,Dir.m_FileName,300); // �ļ���
    strncpy(stfileinfo.modtime,Dir.m_ModifyTime,30); // �ļ��޸�ʱ��
    stfileinfo.filesize = Dir.m_FileSize ;           // �ļ���С
    stfileinfo.filests = 2 ;                         // Ĭ��Ϊ������
    
    v_newfileinfo.push_back(stfileinfo);
  }

  // �ѱ����ļ����ϴ��ļ����Աȣ��ó���Ҫ�����ļ���
  for (UINT ii=0;ii<v_newfileinfo.size();ii++)
  {
    if (v_newfileinfo.size() == 0) break;

    // �Ƚ��ļ������ļ���С���ļ�ʱ�䣬���ȫ����ͬ���Ͱ�filests����Ϊ1��������⡣
    for (UINT jj=0;jj<v_oldfileinfo.size();jj++)
    {
      if (strcmp(v_newfileinfo[ii].filename,v_oldfileinfo[jj].filename) == 0)
      {
        if ( (strcmp(v_newfileinfo[ii].modtime,v_oldfileinfo[jj].modtime) == 0) &&
             (v_newfileinfo[ii].filesize==v_oldfileinfo[jj].filesize) )
        {
          v_newfileinfo[ii].filests=1;
        }

        break;
      }
    }
  }

  // �������ļ���Ϣ����XML�ļ�
  WriteToXML();

  // ���Ѿ��������ļ���Ϣд��List�ļ�
  WriteToListFile();

  exit(0);
}

BOOL WriteToXML()
{
  char strXMLFileName[301],strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  memset(strXMLFileName,0,sizeof(strXMLFileName));
  snprintf(strXMLFileName,300,"%s/%s_%s_%d.xml",stroutputpath,strstdname,strLocalTime,getpid());

  CFile File;
  
  if (File.OpenForRename(strXMLFileName,"w+")==FALSE)
  {
    logfile.Write("File.OpenForRename(%s) failed.\n",strXMLFileName); return FALSE;
  }
  
  File.Fprintf("<data>\n");

  sqlstatement stmt;
  stmt.connect(&conn);

  UINT rows = 0;

  for (UINT kk=0;kk<v_newfileinfo.size();kk++)
  {
    // ֻ���������ļ�(filests=2)
    if (v_newfileinfo[kk].filests==2)
    {
      // ȡ����ʱ��
      stmt.prepare(strtimesql);
      stmt.bindin(1,v_newfileinfo[kk].filename,300);
      stmt.bindout(1,v_newfileinfo[kk].ddatetime,30);
      if (stmt.execute() != 0)
      {
        logfile.Write("exec %s failed.%s\n",stmt.m_sql,stmt.cda.message); continue;
      }
      stmt.next();

      File.Fprintf(\
                     "<ddatetime>%s</ddatetime>"
                     "<typeid>%s</typeid>"
                     "<filepath>%s</filepath>"
                     "<filename>%s</filename>"
                     "<filesize>%ld</filesize><endl/>\n",
                      v_newfileinfo[kk].ddatetime,\
                      strtype,\
                      v_newfileinfo[kk].filepath,\
                      v_newfileinfo[kk].filename,\
                      v_newfileinfo[kk].filesize); 
    rows ++;

    // �޸ı��Ϊ�����
    v_newfileinfo[kk].filests = 1;
    }
  }

  File.Fprintf("</data>\n");

  File.CloseAndRename();

  // ���һ����û�У��Ǿ�ɾ��list�ļ���
  if (rows == 0) REMOVE(strXMLFileName);
  else
    logfile.Write("create %s(%ld rows) ok.\n",strXMLFileName,rows);

  return TRUE;
}

BOOL WriteToListFile()
{
  if ( (listfp=FOPEN(strlistfilename,"w")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strlistfilename); return FALSE;
  }

  fprintf(listfp,"<data>\n");

  for (UINT nn=0;nn<v_newfileinfo.size();nn++)
  {
    if (v_newfileinfo[nn].filests==1)
    {
      fprintf(listfp,"<filename>%s</filename><modtime>%s</modtime><filesize>%ld</filesize><endl/>\n",v_newfileinfo[nn].filename,v_newfileinfo[nn].modtime,v_newfileinfo[nn].filesize);
    }
  }

  fprintf(listfp,"</data>\n");

  return TRUE;
}

void EXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  // ���Ѿ��������ļ���Ϣд��List�ļ�
  WriteToListFile();

  logfile.Write("fileinfo exit.\n");

  exit(0);
}

