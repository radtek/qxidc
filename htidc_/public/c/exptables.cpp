#include "_public.h"
#include "_ooci.h"

// 主程序的参数
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

// 本程序的业务流程主函数
bool _exptables();

void EXIT(int sig);

vector<string> vfieldname; // 存放拆分fieldname后的容器
vector<int>    vfieldlen;  // 存放拆分fieldlen后的容器
int maxfieldlen;           // 存放fieldlen的最大值
void SplitFields();        // 拆分fieldname和fieldlen

// 显示程序的帮助
void _help(char *argv[]);

int maxkeyid;   // 已导出数据的最大的keyid

bool LoadMaxKeyid(); // 从系统参数T_SYSARG表中加载已导出数据的最大的keyid
bool UptMaxKeyid();  // 更新系统参数T_SYSARG表中已导出数据的最大的keyid
  
// 把xml解析到参数starg结构中
bool _xmltoarg(char *strxmlbuffer);

int main(int argc,char *argv[])
{
  if (argc!=3) { _help(argv); return -1; }

  // 关闭全部的信号和输入输出
  CloseIOAndSignal();

  // 处理程序退出的信号
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[1],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
  }

  // 把xml解析到参数starg结构中
  if (_xmltoarg(argv[2])==false) return -1;

  while (true)
  {
    // 连接数据库
    if (conn.connecttodb(starg.connstr,"Simplified Chinese_China.ZHS16GBK") != 0)
    {
      logfile.Write("connect database %s failed.\n",starg.connstr); sleep(starg.timetvl); continue;
    }

    // logfile.Write("export table %s.\n",starg.tname);

    // 导出数据的主函数
    if (_exptables() == false) logfile.Write("export tables failed.\n");

    conn.disconnect();   // 断开与数据库的连接

    sleep(starg.timetvl);
  }

  return 0;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d\n\n",sig);

  exit(0);
}

// 显示程序的帮助
void _help(char *argv[])
{
  printf("\n");
  printf("Using:/htidc/public/bin/exptables logfilename xmlbuffer\n\n");

  printf("Sample:/htidc/public/bin/exptables /log/shqx/exptables_surfdata_for_hb.log \"<connstr>shqx/pwdidc@snorcl11g_198</connstr><charset>Simplified Chinese_China.ZHS16GBK</charset><tname>T_SURFDATA</tname><cols>obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis</cols><fieldname>obtid,ddatetime,t,p,u,wd,wf,r,vis</fieldname><fieldlen>5,14,8,8,8,8,8,8,8</fieldlen><exptype>1</exptype><andstr> and obtid in ('59293','50745')</andstr><bname>SURFDATA_</bname><ename>_for_hb</ename><taskname>SURFDATA_FOR_HB</taskname><exppath>/data/shqx/exp/tohb</exppath><timetvl>30</timetvl>\"\n\n");
  printf("Sample:/htidc/public/bin/exptables /log/shqx/exptables_obtcode_for_hb.log \"<connstr>shqx/pwdidc@snorcl11g_198</connstr><charset>Simplified Chinese_China.ZHS16GBK</charset><tname>T_OBTCODE</tname><cols>obtid,obtname,provname,lat,lon,height</cols><fieldname>obtid,obtname,provname,lat,lon,height</fieldname><fieldlen>5,30,30,8,8,8</fieldlen><exptype>2</exptype><andstr> and rsts=1 and obtid in ('59293','50745')</andstr><bname>OBTCODE_</bname><ename>_for_hb</ename><exppath>/data/shqx/exp/tohb</exppath><timetvl>300</timetvl>\"\n\n");

  printf("本程序是数据中心的公共功能模块，从数据库的表中导出数据生成xml文件，用于数据交换。\n");
  printf("logfilename是本程序运行的日志文件。\n");
  printf("xmlbuffer为文件传输的参数，如下：\n");
  printf("数据库的连接参数 <connstr>shqx/pwdidc@snorcl11g_198</connstr>\n");
  printf("数据库的字符集 <charset>Simplified Chinese_China.ZHS16GBK</charset> 这个参数要与数据源数据库保持>一致，否则会出现中文乱码的情况。\n");
  printf("待导出数据的表名 <tname>T_SURFDATA</tname>\n");
  printf("需要导出字段的列表 <cols>obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis</cols> 可以采用函数。\n");
  printf("导出字段的别名列表 <fieldname>obtid,ddatetime,t,p,u,wd,wf,r,vis</fieldname> 必须与cols一一对应。\n");
  printf("导出字段的长度列表 <fieldlen>5,14,8,8,8,8,8,8,8</fieldlen> 必须与cols一一对应。\n");
  printf("导出数据的方式 <exptype>1</exptype> 1-增量导出；2-全量导出。\n");
  printf("导出数据的附加条件 <andstr> and obtid in ('59293','50745')</andstr> 注意，关键字and不能少。\n");
  printf("导出文件的命名的前部分 <bname>SURFDATA_</bname>\n");
  printf("导出文件的命名的后部分 <ename>_for_hb</ename>\n");
  printf("导出任务的命名 <taskname>SURFDATA_FOR_HB</taskname> 当exptype=1时该参数有效。\n");
  printf("导出文件存放的目录 <exppath>/data/shqx/exp/tohb</exppath>\n");
  printf("导出数据的时间间隔 <timetvl>30</timetvl> 单位：秒，建议大于10。\n");
  printf("以上参数，除了taskname和andstr，其它字段都不允许为空。\n\n\n");
}

// 把xml解析到参数starg结构中
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

  // 拆分fieldname和fieldlen
  SplitFields();

  // 判断fieldname和fieldlen中元素的个数一定要相同
  if (vfieldname.size() != vfieldlen.size() ) { logfile.Write("fieldname和fieldlen的元素个数不同。.\n"); return false; }

  return true;
}

// 本程序的业务流程主函数
bool _exptables()
{
  // 从系统参数T_SYSARG表中加载已导出数据的最大的keyid
  if (LoadMaxKeyid()==false) { logfile.Write("LoadMaxKeyid() failed.\n"); return false; }

  // 生成导出数据的SQL语句
  char strsql[4096];   
  char fieldvalue[vfieldname.size()][maxfieldlen+1]; // 输出变量定义为一个二维数组
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
  // 如果是增量导出，还要绑定keyid字段
  if (starg.exptype==1) stmt.bindout(vfieldname.size()+1,&maxkeyid);
  
  // 执行导出数据的SQL
  if (stmt.execute() != 0)
  {
    logfile.Write("select %s failed.\n%s\n%s\n",starg.tname,stmt.m_cda.message,stmt.m_sql); return false;
  }

  int  iFileSeq=1;   // 待生成文件的序号
  char strFileName[301],strLocalTime[21];
  CFile File;

  while (true)
  {
    memset(fieldvalue,0,sizeof(fieldvalue));
   
    if (stmt.next() !=0) break;

    // 把数据写入文件
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

    // 1000条记录写入一个文件完成
    if (stmt.m_cda.rpc%1000==0)
    {
      File.Fprintf("</data>\n");
      if (File.CloseAndRename()==false)
      {
        logfile.Write("File.CloseAndRename(%s) failed.\n",strFileName); return false;
      }

      // 更新系统参数T_SYSARG表中已导出数据的最大的keyid
      if (UptMaxKeyid()==false) { logfile.Write("UptMaxKeyid() failed.\n"); return false; }

      logfile.Write("create file %s ok.\n",strFileName);
    }
  }

  // 不够1000条的写入一个文件
  if (File.IsOpened()==true)
  {
    File.Fprintf("</data>\n");
    if (File.CloseAndRename()==false)
    {
      logfile.Write("File.CloseAndRename(%s) failed.\n",strFileName); return false;
    }

    // 更新系统参数T_SYSARG表中已导出数据的最大的keyid
    if (UptMaxKeyid()==false) { logfile.Write("UptMaxKeyid() failed.\n"); return false; }

    logfile.Write("create file %s ok.\n",strFileName);
  }

  if (stmt.m_cda.rpc>0) logfile.Write("本次导出了%d条记录。\n",stmt.m_cda.rpc);

  return true;
}

// 拆分fieldname和fieldlen
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
    if (ifieldlen>maxfieldlen) maxfieldlen=ifieldlen;   // 得到fieldlen的最大值
    vfieldlen.push_back(ifieldlen);
  }
}

// 从系统参数T_SYSARG表中加载已导出数据的最大的keyid
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

  // 如果记录不存在，插入一新记录。
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

// 更新系统参数T_SYSARG表中已导出数据的最大的keyid
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
