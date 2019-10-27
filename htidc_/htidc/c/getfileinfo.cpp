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

// 文件信息
struct st_fileinfo
{
  char ddatetime[31]; // 数据时间
  char filepath[301]; // 文件路径
  char filename[301]; // 文件名
  char modtime[31];   // 文件修改时间
  UINT filesize;      // 文件大小
  int  filests;       // 1-旧数据已入库,2-新数据未入库。
};

struct st_fileinfo stfileinfo;

// 本次服务器文件清单的容器
vector<struct st_fileinfo> v_newfileinfo;

// 上次服务器文件清单的容器
vector<struct st_fileinfo> v_oldfileinfo;

// 把新增文件信息生成XML文件
BOOL WriteToXML();

// 把已经入库过的文件信息写入List文件
BOOL WriteToListFile();

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./fileinfo xmlbuffer\n");

    printf("Example:/htidc/htidc/bin/procctl 120 /htidc/htidc/bin/getfileinfo \"<logfilename>/log/szqx/getfileinfo_radar_dpradar_gd.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.98.31</connstr><type>radar_dpradar_gd</type><timesql>select substr(:1,16,14) from dual</timesql><stdname>UNSTRUCT_FILESINFO</stdname><outputpath>/tmp/htidc/qxmonclient</outputpath><pathname>/szmbdata01/radar/dpradar/gd/2019/{YYYY}{MM}{DD}</pathname><andchild>FALSE</andchild><matchstr>*.*</matchstr><timetvl>-8</timetvl><mtime>24</mtime><listfilename>/tmp/htidc/list/getfileinfo_radar_dpradar_gd.xml</listfilename>\"\n\n");

    printf("用于(增量)收集指定目录下的文件信息，生成xml文件，然后入库。\n");
    printf("<logfilename>/tmp/htidc/log/fileinfo_ecdata</logfilename>是本程序的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.98.31</connstr> 数据库连接参数，用于执行下面的SQL语句。\n");
    printf("<type>radar_dpradar_gd 数据类型，采用:xx_xx_xx，广东省双偏振雷达基数据：radar_dpradar_gd 。\n");
    printf("<timesql>select substr(:1,16,14) from dual 取数据时间SQL语句，:1代表文件名，不能为空。\n");
    printf("<stdname>UNSTRUCT_FILESINFO</stdname> xml文件前缀\n");
    printf("<outputpath>/tmp/htidc/qxmonclient</outputpath>xml文件输出目录。\n");
    printf("<pathname>/ecdata</pathname>待收集文件存放的目录。\n");
    printf("<andchild>TRUE</andchild>是否扫描子目录。\n");
    printf("<matchstr>S1D*</matchstr>目录下文件的匹配规则。\n");
    printf("<mtime>24</mtime> 待传输文件时间的范围，单位：小时，如果文件时间在本参数之前，就不传输。\n");
    printf("<listfilename>getfileinfo_radar_dpradar_gd.xml 存放已收集过的文件列表的xml文件。\n");
    printf("xmlbuffer可以处理时间变量，<timetvl>-8</timetvl> 为时间变量的偏移时间。"\
           "目前可以处理以下时间变量：{YYYY}（4位的年）、{YYY}（后三位的年）、"\
           "{YY}（后两位的年）、{MM}（月月）、{DD}（日日）、{HH}（时时）、{MI}（分分）、{SS}（秒秒）。\n");
    printf("以上的参数只有timetvl、mtime为可选参数，其它的都必填。\n\n\n");

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

  // 处理xmlbuffer中的时间变量
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

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("getfileinfo");

  // 程序超时是3600秒
  ProgramActive.SetProgramInfo(&logfile,"getfileinfo",3600);

  // 连接数据源数据库
  if (conn.connecttodb(strconnstr) != 0)
  {
     logfile.Write("conn.connecttodb(%s) failed\n",strconnstr); return -1;
  }
  
  // 写入进程活动信息
  ProgramActive.WriteToFile();

  // 加载上次服务器文件信息清单到v_oldfileinfo
  char strLine[2048];

  v_oldfileinfo.clear();

  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // 从文件中获取一行
      if (FGETS(strLine,2000,listfp,"<endl/>") == FALSE) break;

      memset(&stfileinfo,0,sizeof(stfileinfo));

      GetXMLBuffer(strLine,"filename", stfileinfo.filename,300);
      GetXMLBuffer(strLine,"modtime",  stfileinfo.modtime,30);
      GetXMLBuffer(strLine,"filesize",&stfileinfo.filesize);

      v_oldfileinfo.push_back(stfileinfo);
    }
    fclose(listfp);
  }

  // 读取目录下的全部文件
  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));
  LocalTime(strmtime,"yyyymmddhh24miss",0-umtime*60*60);

  Dir.SetDateFMT("yyyymmddhh24miss");

  v_newfileinfo.clear();

  // 打开目录，读取文件，不排序
  BOOL bandchild=FALSE;

  if ( (strcmp(strandchild,"TRUE")==0) || (strcmp(strandchild,"true")==0) ) bandchild=TRUE;

  if (Dir.OpenDirNoSort(strpathname,bandchild) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",strpathname); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    if (MatchFileName(Dir.m_FileName,strmatchstr)==FALSE) continue;

    // 如果文件时间小于umtime，就丢弃这个文件
    if ( (umtime>0) && (strcmp(Dir.m_ModifyTime,strmtime)<0) ) continue;

    strncpy(stfileinfo.filepath,Dir.m_DirName,300);  // 文件路径
    strncpy(stfileinfo.filename,Dir.m_FileName,300); // 文件名
    strncpy(stfileinfo.modtime,Dir.m_ModifyTime,30); // 文件修改时间
    stfileinfo.filesize = Dir.m_FileSize ;           // 文件大小
    stfileinfo.filests = 2 ;                         // 默认为新数据
    
    v_newfileinfo.push_back(stfileinfo);
  }

  // 把本次文件和上次文件做对比，得出需要入库的文件。
  for (UINT ii=0;ii<v_newfileinfo.size();ii++)
  {
    if (v_newfileinfo.size() == 0) break;

    // 比较文件名、文件大小和文件时间，如果全部相同，就把filests设置为1，不用入库。
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

  // 把新增文件信息生成XML文件
  WriteToXML();

  // 把已经入库过的文件信息写入List文件
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
    // 只处理新增文件(filests=2)
    if (v_newfileinfo[kk].filests==2)
    {
      // 取数据时间
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

    // 修改标记为已入库
    v_newfileinfo[kk].filests = 1;
    }
  }

  File.Fprintf("</data>\n");

  File.CloseAndRename();

  // 如果一条都没有，那就删除list文件。
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

  // 把已经入库过的文件信息写入List文件
  WriteToListFile();

  logfile.Write("fileinfo exit.\n");

  exit(0);
}

