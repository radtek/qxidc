#ifndef _IDCAPP_H
#define _IDCAPP_H

#include "_oracle.h"
#include "_public.h"

struct st_APPDTYPE
{
  long dtypeid;
  char dtypename[51];
  char pfilename[201];
  char addatetime[201];
  char tname[31];
  char dmintime[20];
  int  upttype;
  int  upttlimit;
};

// 表的全部字段和值的结构
struct st_TABCOLUMNS
{
  char  COLUMN_NAME[31];
  char  DATA_TYPE[107];
  int   DATA_LENGTH;
  char  COLUMN_VALUE[4001];
};

// 文件采集和分发表的数据结构
struct st_FileList
{
  char filename[301];
  int  taskid;
  char modtime[20];
  int  filesize;
  char ddatetime[20];
};

// 表的全部字段的结构
struct st_TABFIELD
{
  char  fieldname[31];
  char  datatype[107];
  int   datalen;
};

struct st_SYNCTABLE
{
  char rowid[31];
  long keyid;
  char tname[31];
  int  ctype;
  char sync_rowid[31];
  char crttime[20];
};

/*
// 自动站日数据
struct st_OBTDAYD
{
  char rowid[31];
  char ddatetime[20];
  char obtid[6];

  char r2008[9];
  char r0820[9];
  char r24h[9];

  char maxr10m[9];
  char maxr10mtime[20];
  char maxr01h[9];
  char maxr01htime[20];

  char maxt[9];
  char maxttime[20];
  char mint[9];
  char minttime[20];
  char avgt[9];

  char maxrh[9];
  char maxrhtime[20];
  char minrh[9];
  char minrhtime[20];
  char avgrh[9];

  char maxp0[9];
  char maxp0time[20];
  char minp0[9];
  char minp0time[20];
  char avgp0[9];

  char maxp[9];
  char maxptime[20];
  char minp[9];
  char minptime[20];
  char avgp[9];

  char wd3smaxdf[9];
  char wd3smaxdd[9];
  char wd3smaxtime[20];
  char avgwdidf[9];

  char wd2maxdf[9];
  char wd2maxdd[9];
  char wd2maxtime[20];
  char avgwd2df[9];

  char wd10maxdf[9];
  char wd10maxdd[9];
  char wd10maxtime[20];
  char avgwd10df[9];

  char maxv[9];
  char maxvtime[20];
  char minv[9];
  char minvtime[20];
  char minvtimeu[9];
  char avgv[9];

  char latestrdtime[20];
  char latestpdtime[20];

  UINT mdcount;
  UINT hdcount;

  UINT keyid;
};
*/

/*
class CDPALARMCFG
{
public:
  char alarmcode[31];
  char alarmname[51];
  long dalarmtvl;
  char aleasttime[20];
  long ifsendsms;
  char mobilenostr[501];
  UINT keyid;
  long rsts;
  char alarmtext[1401];

  long m_TimePassed;

  connection *m_conn;
  CLogFile   *m_logfile;

  CDPALARMCFG();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  void initdata();

  // 把告警日志写入日志表和短信发送表中
  long CrtAlarmLog(char *in_AlarmCode,char *fmt,...);

 ~CDPALARMCFG();
};
*/

// 检查表是否存在
BOOL CheckTExist(connection *in_conn,char *in_tablename);

// 检查序列是否存在
BOOL CheckSEQExist(connection *in_conn,char *in_seqname);

// 检查触发器是否存在
BOOL CheckTRExist(connection *in_conn,char *in_trname);

// 判断tname1表是否存在，如果不存在，就用tname2生成它
long CrtByTable(connection *conn,char *strtempletname,char *strtname);

class CSYNCTABLE
{
public:
  connection *m_conndst,*m_connsrc;
  CLogFile   *m_logfile;

  CSYNCTABLE();
  ~CSYNCTABLE();

  void BindConnLog(connection *in_conndst,connection *in_connsrc,CLogFile *in_logfile);

  char m_tnamessrc[50][31];       // 数据源待同步的表名的数组
  char m_tnamesdst[50][31];    // 目的库待同步的表名的数组
  char m_columns[50][1024];    // 待同步的表名的列的数组
  long LoadTInfo(char *strTNameStr);   // 从数据源数据库的数据字典中载入待同步的表的结构，用小写存放

  char m_dblink[101];

  struct st_SYNCTABLE m_stSYNCTABLE;
  vector<struct st_SYNCTABLE> m_vSYNCTABLE; // 存放等同步的数据的日志

  long LoadSyncLog(char *strSyncName);  // 从数据源数据库的T_SYNCLOG表中载入全部未同步的日志

  long SyncDATA(CProgramActive *ProgramActive,char *strSyncName); // 根据m_vSYNCTABLE中的日志同步

  long SyncTableToIDC(char *strSyncName);  // 把数据源表的记录更新新数据中心的表

  long ClearSyncLog(char *strSyncName);  // 从数据源数据库的T_SYNCLOG表中删除一条已同步的记录的日志
};

class CTABFIELD
{
public:
  CTABFIELD();
 ~CTABFIELD();

  UINT m_pkcount;    // 主键字段的个数
  UINT m_fieldcount; // 全部字段的个数

  char m_pkfieldstr[301];    // 全部的主键字段，以字符串存放，中间用半角的逗号分隔
  char m_allfieldstr[3001];  // 全部的字段，以字符串存放，中间用半角的逗号分隔
  char strerrmesg[3001];  // 错误信息

  struct st_TABFIELD m_stTABFIELD;

  vector<struct st_TABFIELD> m_vPKFIELD;   // 存放主键字段信息的容器
  vector<struct st_TABFIELD> m_vALLFIELD;  // 存放全部字段信息的容器

  void initdata();

  // 获取指定表的主键字段信息，TRUE-获取成功，FALSE-获取失败
  BOOL GetPKField(connection *conn,char *tablename);

  // 获取指定表的全部字段信息，TRUE-获取成功，FALSE-获取失败
  BOOL GetALLField(connection *conn,char *tablename,const char *exceptfields=0,BOOL ISDBLINK=FALSE);
};


class CFILEFTPTASK
{
public:
  UINT taskid;
  char taskname[101];
  UINT ftptype;
  char remoteip[51];
  UINT port;
  char username[51];
  char password[51];
  UINT ftpcmode;
  char localpath[301];
  UINT dstpathmatchbz;
  char dstpath[301];
  char matchstr[1025];
  UINT checkend;
  UINT cmodtime;
  UINT cfilesize;
  double validdays;
  char filesizestr[51];
  UINT deletebz;
  UINT renamebz;
  char renamepath[301];
  UINT synctvl;
  char synctime[20];
  UINT alarmbz;
  char mobilenostr[501];
  UINT ftpsts;
  char faulttime[20];
  UINT rsts;

  char m_validdays[20];  // 采集文件的最早有效时间，格式yyyymmddhh24miss，用于判断是否要采集该文件

  struct st_FileList m_stLocalFile;

  connection *m_conn;
  CLogFile   *m_logfile;

  CFILEFTPTASK();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  void initdata();

  // 从FILEFTPTASK表中提取执行时间（SyncTime）小于等于当前时间的全部记录。
  sqlstatement selstmt;
  long LoadFtpTask(char *strRemoteIP);
  long LoadFtpTaskNext();

  // 每次执行完传输任务之前更新T_FILEFTPTASK表的synctime为10分钟之后
  long UptSyncTime10M();

  // 每次执行完传输任务之前更新T_FILEFTPTASK表的synctime为下一周期的时间点
  long UptSyncTime();

  // 更新任务的状态
  long UptFtpSTS();


  // 获取最近需要执行的任务记录的时间和当前时间间隔的秒数
  int  m_sleeptvl;
  long SelLeastTimeTvl(char *strRemoteIP);

  // 获取全部需要正在故障中的FTP任务记录
  sqlstatement selfrstmt;
  long LoadFaultRecord();
  long LoadFaultRecordNext();

  // 更新故障的时间
  long UptFaultTime();

  // 把已发送的文件的信息插入文件分发日志表
  long InsertPutList();

 ~CFILEFTPTASK();
};

class CFILEFTPLIST
{
public:
  UINT m_taskid;

  struct st_FileList m_stRemoteFile;
  vector<struct st_FileList> m_vRemoteFile;

  connection *m_conn;
  CLogFile   *m_logfile;

  CFILEFTPLIST();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  void initdata();

  // 从已获取的文件清单中获取文件的大小和修改日期
  sqlstatement selliststmt;
  long FindFList();
  long FindFListNext();

  // 把同步成功的文件信息插入FILEFTPLIST表中
  sqlstatement insliststmt;
  long InsertList();

 ~CFILEFTPLIST();
};

// 气象数据入库类
class CQXDATA
{
public:
  struct st_APPDTYPE m_stAPPDTYPE;
  vector<struct st_APPDTYPE> m_vAPPDTYPE;

  long m_dtypeid;
  char m_dtypename[51];
  char m_pfilename[201];
  char m_addatetime[201];
  char m_tname[31];
  char m_dmintime[20];
  int  m_upttype;
  int  m_upttlimit;

  char m_filename[301];

  UINT m_TotalCount,m_UptCount,m_InsCount,m_DiscardCount;

  connection *m_conn;
  CLogFile   *m_logfile;

  CQXDATA();
 ~CQXDATA();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  void initdata();

  // 把全部的气象文件参数载入容器
  long LoadFileCFG();

  // 从容器中获取某种类型文件的参数
  BOOL GETFILECFG();

  struct st_TABCOLUMNS m_stTABCOLUMNS;
  vector<struct st_TABCOLUMNS> m_vTABCOLUMNS;

  // 获取表的全部的列信息和主键字段，存放在m_vTABCOLUMNS容器中
  char m_KEYFIELDSTR[1024];
  long GETTABCOLUMNS();

  // 是否存在创建时间字段
  BOOL m_bexistcrttimefield;

  // 生成查询、更新和插入表的SQL语句
  BOOL m_bnokeyidfield;
  char m_rowid[31],m_SelectSQL[1024],m_UpdateSQL[10240],m_InsertSQL[10240];
  CCmdStr m_CmdKeyFields;
  void CrtSQL();

  // 把每行的内容解包到字段中
  void UNPackBuffer(char *strBuffer,int iFileVer);

  // 准备SQL语句
  long PreSQL();
  sqlstatement stmtselecttable,stmtupdatetable,stmtinserttable;

  long BindParams();

  // 查询数据表中的记录
  long InsertTable(char *strBuffer,char *striflog);

  // DDateTime字段在容器中的位置
  int  m_ddtfieldpos;
};

struct st_ALARMLOG
{
  long logid;
  char progname[101];
  int  alarmtimes;
  char begintime[21];
  char leasttime[21];
  char alarmtext[1001];
  int  readbz;
  int  noticebz;
};

struct st_ALLTABLE
{
  UINT appid;
  char tname[31];       // 表英文名
  char tcname[51];      // 表中文名
  UINT hdataptype;
  UINT hdatapcfg;
  char addterm[301];
  UINT syncinsert;
  UINT syncupdate;
  UINT syncdelete;
  UINT totalcount;      // 总记录数
  char counttime[20];   // 统计时间，最近一次执行记录数统计的时间。
  char backuptime[20];  // 备份时间，最近一次执行备份的时间。
  UINT tablests;        // 表单状态，default 2，1-已启用；2-已禁用；3-已删除。
  UINT tabletype;

  /*
  UINT ifcheck;
  UINT checktvl;
  char srcchecksql[301];
  char dstchecksql[301];
  UINT checkok;
  */
};

// 用于存放被维护的表的数据记录
struct st_ROWIDYEAR
{
  char rowid[31];
  char yearstr[5];
};

class CALLTABLE
{
public:
  connection *m_connidc,*m_conndst;
  CLogFile   *m_logfile;

  CProgramActive *m_ProgramActive;

  struct st_ALLTABLE m_stALLTABLE;
  vector<struct st_ALLTABLE> m_vALLTABLE;

  CALLTABLE();
 ~CALLTABLE();

  void BindConnLog(connection *in_connidc,connection *in_conndst,CLogFile *in_logfile,CProgramActive *in_ProgramActive);

  // 把TAB数据字典中存在，  但T_ALLTABLE表中不存在的记录插入T_ALLTABLE中
  // 把TAB数据字典中不存在，但T_ALLTABLE表中存在的记录删除掉
  // 把USER_SEQUENCES数据字典中存在，  但T_SEQANDTABLE表中不存在的记录插入T_SEQANDTABLE
  // 把USER_SEQUENCES数据字典中不存在，但T_SEQANDTABLE表中存在的记录删除掉
  long UpdateALLTABLEAndSEQ();

  UINT m_appid;

  sqlstatement loadstmt;
  long LoadForCount();    // 从T_APPTABLE表中载入某集群服务器的全部记录
  long LoadForCrtView();  // 从T_APPTABLE表中载入某集群服务器全部的当前表，用于创建视图
  long LoadForManager();
  long LoadAllRecordNext();

  /*
  char m_dblinkname[50];
  long LoadForCheck(int in_appid);    // 从T_APPTABLE表中载入需要核对数据的参数。
  long CheckTable();      // 核对表的记录数
  long UptCheckTime();    // 更新T_ALLTABLE表的核对结果和核对时间
  */

  // 统计表的记录数
  long CountTable(char *in_DBLinkName);

  // 更新T_ALLTABLE表的记录总数和统计时间字段
  long UptCountTime(char *in_DBLinkName);

  // 获取全部需要备份的记录，并放入m_vALLTABLE中
  long LoadEXPRecord();

  // 卸出表
  BOOL ExpTable(char *strTmpPath,char *strStdPath,char *strConnStr,char *strTName);

  // 更新T_ALLTABLE表的备份时间字段
  long UptBackupTime(char *strTName);

  struct st_ROWIDYEAR m_stROWIDYEAR;
  vector<struct st_ROWIDYEAR> m_vROWIDYEAR;

  long LoadALLROWID();

  // 获取表的全部的列信息，存入在m_COLUMNSSTR中
  char m_COLUMNSSTR[1024];

  // 备份到历史表
  long BackupToHTable();

  // 只删除不备份
  long DeleteTable();

  // 备份到归档表
  long BackupToATable();
};

struct st_BDSYNCCFG
{
  char tname[31];
  UINT bsynctype;
  UINT orgkeyid;
  char bsyncterm[1001];
  int  lobfieldcount;
};


class CBDSYNCCFG
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_BDSYNCCFG m_stBDSYNCCFG;
  vector<struct st_BDSYNCCFG> m_vBDSYNCCFG;

  CBDSYNCCFG();
 ~CBDSYNCCFG();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // 从T_BDSYNCCFG表中载入需要执行批量同步的表
  long LoadBSyncTable(int in_appid);

  // 更新已同步表的执行时间为当前时间
  long UptBSyncTable(int in_appid,char *in_tname);

  // 更新已同步表的已同步记录的标志
  long UptBSyncTable(int in_appid,char *in_tname,UINT in_orgkeyid);
};

// 用当前表名创建联合当前表、历史表和归档表的视图
long CrtView(connection *conn,char *strTName);

// 判断tname1表是否存在，如果不存在，就用tname2生成它
long CrtByTable(connection *conn,char *strtempletname,char *strtname);

class CALARMLOG
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_ALARMLOG m_stALARMLOG;
  vector<struct st_ALARMLOG> m_vALARMLOG;

  CALARMLOG();
 ~CALARMLOG();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  BOOL ReadXMLFile(char *in_FileName);

  long UptAlarmLog();

  // 删除表中三天之前的记录
  long DelAlarmLog();
};

class CIDCCFG
{
public:
  char m_idcconnstr[101];          // 数据中心应用数据库的连接串

  char m_idcusername[101];         // 数据中心数据库的用户名

  char m_logpath[301];             // 日志文件目录

  char m_listpath[301];            // 存放二进制文档文件导出列表文件的目录

  char m_tmppath[301];             // 存放临时文件的目录

  char m_sdatastdpath[301];        // 二维表格数据标准XML文件存放的目录
  char m_sdatastdbakpath[301];     // 二维表格数据标准XML文件备份的目录
  char m_sdatastderrpath[301];     // 二维表格数据标准XML文件错误文件的备份的目录

  char m_wfilestdpath[301];        // 二进制文档文件数据标准命名文件存放的目录
  char m_wfilestdbakpath[301];     // 二进制文档文件数据标准命名文件备份的目录
  char m_wfilestderrpath[301];     // 二进制文档文件数据标准命名文件备份的目录

  CIDCCFG();

  void initdata();

  // 载入短信系统参数
  BOOL LoadIniFile(char *in_inifile);
};

struct st_timespan
{
  char begintime[21];
  char endtime[21];
};

class CTimeSpan
{
public:
  struct st_timespan m_sttimespan;
  vector<struct st_timespan> m_vtimespan;

  // 分配一个时间粒度表，从timespan分钟到当前时间
  // unit为时间粒度，1-五分钟；2-六分钟；3-十分钟；4-小时；5-天。
  void defvector(int unit,int spanunit);
};

struct st_DMONCFG
{
  long taskid;
  char taskname[101];
  char tname[51];
  char strwhere[501];
  int  analytype;
  int  analyunit;
  int  spanunit;
  int  exectvl;
  char exectime[21];
  int  alarmbz;
  int  dcount;
  int  delaytime;
  int  alarmtvl;
  char alarmtime[21];
  int  alarmsts;
  char alarminfo[501];
  int  alarmtimes;
  int  alarmedtimes;
  int  alarmedtvl;
};

class CDMONCFG
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_DMONCFG m_stDMONCFG;
  vector<struct st_DMONCFG> m_vDMONCFG;

  CTimeSpan TimeSpan;

  CDMONCFG();
 ~CDMONCFG();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // 从T_DMONCFG表中加载需要统计的记录，存放在m_vDMONCFG中
  BOOL LoadDMONCFG(const char *strWhere);

  // 读取m_vDMONCFG中全部的参数，统计数据量
  BOOL CountTable();

  // 读取m_vDMONCFG中全部的参数，分析告警
  BOOL MONTable();
};


struct st_FILELIST
{
  char ftypename[51];
  char pfilename[201];
  char dmintime[20];
  char addatetime[201];
  int  timeoffset;
  long dtypeid;
  int  upttype;
  int  upttlimit;
  char tname[31];
};

// 气象文件入库类
class CFILELIST
{
public:
  struct st_FILELIST m_stFILELIST;
  vector<struct st_FILELIST> m_vFILELIST;

  char m_filename[301];
  char m_ddatetime[20];

  char m_ftypename[51];   //
  char m_pfilename[201];
  char m_addatetime[201];
  char m_dmintime[20];
  long m_dtypeid;
  int  m_upttype;
  int  m_upttlimit;
  char m_tname[31];

  char m_fullfilename[301];
  int  m_filesize;
  UINT m_keyid;

  connection *m_conn;
  CLogFile   *m_logfile;

  CFILELIST();
 ~CFILELIST();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // 把全部的气象文件参数载入容器
  long LoadFileCFG();


  // 从容器中获取某种类型文件的参数
  BOOL GETFILECFG();

  // 根据数据类别和文件名，查询表中是否已存在该文件
  long m_timeexist;
  sqlstatement stmtfexist;
  long FindFExist();
  long FindFExistNext();

  BOOL m_IsInsertFile;

  // 把文件入库
  long InsertFileToDBEx();

  // 更新已入库的文件
  long UpdateFileToDBEx();
};

// 获取序列生成器的值
long FetchSequence(connection *conn,char *SequenceName,UINT &uValue);

// 判断数据时间是否合法，即是否在dmintime日到当前时间之后的24个小时
BOOL CheckDDateTime(char *in_DDateTime,char *in_DMinTime);

// 杀死与同步目的表有关的会话，清理出锁
BOOL KillLocked(connection *in_conn,char *in_tname);

struct st_EXPDTASK
{
  UINT taskid;
  char taskname[101];
  char tnsname[101];
  char selectsql[4001];
  char fieldstr[4001];
  char fieldlen[4001];
  UINT exptype;
  UINT position;
  char firstsql[4001];
  char bfilename[31];
  char efilename[31];
  char outpath[201];
  UINT rsts;
};

class CEXPDTASK
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_EXPDTASK m_stEXPDTASK;

  CEXPDTASK();
 ~CEXPDTASK();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // 根据tasktype从任务表中获取任务的详细信息，结果存放在m_stEXPDTASK中。
  sqlstatement selstmt;
  long GetTaskByID(UINT in_tasktype);
  long GetTaskByIDNext();

  // 更新任务的导出时间
  long UptExpTime();
};

// 数据采集日志
struct st_COLLECTLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char colltype[501];
  char colltime[501];
  char remoteip[501];
  char filename[501];
  char filesize[501];
  char filetime[501];
};

// 数据处理日志
struct st_PROCESSLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char ddatetime[501];
  char srcfilename[501];
  char srcfiletime[501];
  char srcfilesize[501];
  char stdfilename[501];
  char stdfiletime[501];
  char stdfilesize[501];
  char count[501];
};

// 数据入库日志
struct st_TODBLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char ddatetime[501];
  char filetime[501];
  char filename[501];
  char filesize[501];
  char tname[501];
  char total[501];
  char insrows[501];
  char uptrows[501];
  char disrows[501];
};

// 进程活动日志
struct st_PROACTLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char pid[501];
  char maxtimeout[501];
  char latestactivetime[501];
};

// 二次加工（统计数据）日志
struct st_CALCLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char ddatetime[501];
  char srctname[501];
  char stdtname[501];
  char insrows[501];
  char uptrows[501];
};

struct st_TRANSFERLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char transfertype[501];
  char transfertime[501];
  char remoteip[501];
  char localfilename[501];
  char remotefilename[501];
  char filesize[501];
  char filetime[501];
};

class CRealMon
{
public:
  char strCollFileName[501];
  char strProcFileName[501];
  char strTodbFileName[501];
  char strActiFileName[501];
  char strCalcFileName[501];
  char strTranFileName[501];
  char strLocalTime[31];
  int  uFileSeq;

  CTimer    m_Timer; // 计时器

  CFile CollFile,ProcFile,TodbFile,ActiFile,CalcFile,TranFile;

  struct st_COLLECTLOG  m_stCOLLECTLOG;
  struct st_PROCESSLOG  m_stPROCESSLOG;
  struct st_TODBLOG     m_stTODBLOG;
  struct st_PROACTLOG   m_stPROACTLOG;
  struct st_CALCLOG     m_stCALCLOG;
  struct st_TRANSFERLOG m_stTRANSFERLOG;

  vector<struct st_COLLECTLOG>  m_vCOLLECTLOG;
  vector<struct st_PROCESSLOG>  m_vPROCESSLOG;
  vector<struct st_TODBLOG   >  m_vTODBLOG   ;
  vector<struct st_CALCLOG   >  m_vCALCLOG   ;
  vector<struct st_TRANSFERLOG> m_vTRANSFERLOG;

  CRealMon();
 ~CRealMon();

  // 生成数据采集日志XML文件
  BOOL WriteToCollectLog();
 
  // 生成数据处理日志XML文件
  BOOL WriteToProcessLog();

  // 生成数据入库日志XML文件
  BOOL WriteToDbLog();

  // 生成进程活动日志XML文件
  BOOL bIsFirstTime; // 如果是第一次，那就生成心跳文件，否则要10秒才写一次。
  BOOL WriteToProActLog(const char *in_Indexid,const char *in_ProgramName,const int in_MaxTimeOut);

  // 生成二次加工（统计数据）日志XML文件
  BOOL WriteToCalcLog();

  // 生成数据传输日志XML文件
  BOOL WriteToTransferLog();

};

#endif
