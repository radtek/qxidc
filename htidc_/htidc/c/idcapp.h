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

// ���ȫ���ֶκ�ֵ�Ľṹ
struct st_TABCOLUMNS
{
  char  COLUMN_NAME[31];
  char  DATA_TYPE[107];
  int   DATA_LENGTH;
  char  COLUMN_VALUE[4001];
};

// �ļ��ɼ��ͷַ�������ݽṹ
struct st_FileList
{
  char filename[301];
  int  taskid;
  char modtime[20];
  int  filesize;
  char ddatetime[20];
};

// ���ȫ���ֶεĽṹ
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
// �Զ�վ������
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

  // �Ѹ澯��־д����־��Ͷ��ŷ��ͱ���
  long CrtAlarmLog(char *in_AlarmCode,char *fmt,...);

 ~CDPALARMCFG();
};
*/

// �����Ƿ����
BOOL CheckTExist(connection *in_conn,char *in_tablename);

// ��������Ƿ����
BOOL CheckSEQExist(connection *in_conn,char *in_seqname);

// ��鴥�����Ƿ����
BOOL CheckTRExist(connection *in_conn,char *in_trname);

// �ж�tname1���Ƿ���ڣ���������ڣ�����tname2������
long CrtByTable(connection *conn,char *strtempletname,char *strtname);

class CSYNCTABLE
{
public:
  connection *m_conndst,*m_connsrc;
  CLogFile   *m_logfile;

  CSYNCTABLE();
  ~CSYNCTABLE();

  void BindConnLog(connection *in_conndst,connection *in_connsrc,CLogFile *in_logfile);

  char m_tnamessrc[50][31];       // ����Դ��ͬ���ı���������
  char m_tnamesdst[50][31];    // Ŀ�Ŀ��ͬ���ı���������
  char m_columns[50][1024];    // ��ͬ���ı������е�����
  long LoadTInfo(char *strTNameStr);   // ������Դ���ݿ�������ֵ��������ͬ���ı�Ľṹ����Сд���

  char m_dblink[101];

  struct st_SYNCTABLE m_stSYNCTABLE;
  vector<struct st_SYNCTABLE> m_vSYNCTABLE; // ��ŵ�ͬ�������ݵ���־

  long LoadSyncLog(char *strSyncName);  // ������Դ���ݿ��T_SYNCLOG��������ȫ��δͬ������־

  long SyncDATA(CProgramActive *ProgramActive,char *strSyncName); // ����m_vSYNCTABLE�е���־ͬ��

  long SyncTableToIDC(char *strSyncName);  // ������Դ��ļ�¼�������������ĵı�

  long ClearSyncLog(char *strSyncName);  // ������Դ���ݿ��T_SYNCLOG����ɾ��һ����ͬ���ļ�¼����־
};

class CTABFIELD
{
public:
  CTABFIELD();
 ~CTABFIELD();

  UINT m_pkcount;    // �����ֶεĸ���
  UINT m_fieldcount; // ȫ���ֶεĸ���

  char m_pkfieldstr[301];    // ȫ���������ֶΣ����ַ�����ţ��м��ð�ǵĶ��ŷָ�
  char m_allfieldstr[3001];  // ȫ�����ֶΣ����ַ�����ţ��м��ð�ǵĶ��ŷָ�
  char strerrmesg[3001];  // ������Ϣ

  struct st_TABFIELD m_stTABFIELD;

  vector<struct st_TABFIELD> m_vPKFIELD;   // ��������ֶ���Ϣ������
  vector<struct st_TABFIELD> m_vALLFIELD;  // ���ȫ���ֶ���Ϣ������

  void initdata();

  // ��ȡָ����������ֶ���Ϣ��TRUE-��ȡ�ɹ���FALSE-��ȡʧ��
  BOOL GetPKField(connection *conn,char *tablename);

  // ��ȡָ�����ȫ���ֶ���Ϣ��TRUE-��ȡ�ɹ���FALSE-��ȡʧ��
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

  char m_validdays[20];  // �ɼ��ļ���������Чʱ�䣬��ʽyyyymmddhh24miss�������ж��Ƿ�Ҫ�ɼ����ļ�

  struct st_FileList m_stLocalFile;

  connection *m_conn;
  CLogFile   *m_logfile;

  CFILEFTPTASK();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  void initdata();

  // ��FILEFTPTASK������ȡִ��ʱ�䣨SyncTime��С�ڵ��ڵ�ǰʱ���ȫ����¼��
  sqlstatement selstmt;
  long LoadFtpTask(char *strRemoteIP);
  long LoadFtpTaskNext();

  // ÿ��ִ���괫������֮ǰ����T_FILEFTPTASK���synctimeΪ10����֮��
  long UptSyncTime10M();

  // ÿ��ִ���괫������֮ǰ����T_FILEFTPTASK���synctimeΪ��һ���ڵ�ʱ���
  long UptSyncTime();

  // ���������״̬
  long UptFtpSTS();


  // ��ȡ�����Ҫִ�е������¼��ʱ��͵�ǰʱ����������
  int  m_sleeptvl;
  long SelLeastTimeTvl(char *strRemoteIP);

  // ��ȡȫ����Ҫ���ڹ����е�FTP�����¼
  sqlstatement selfrstmt;
  long LoadFaultRecord();
  long LoadFaultRecordNext();

  // ���¹��ϵ�ʱ��
  long UptFaultTime();

  // ���ѷ��͵��ļ�����Ϣ�����ļ��ַ���־��
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

  // ���ѻ�ȡ���ļ��嵥�л�ȡ�ļ��Ĵ�С���޸�����
  sqlstatement selliststmt;
  long FindFList();
  long FindFListNext();

  // ��ͬ���ɹ����ļ���Ϣ����FILEFTPLIST����
  sqlstatement insliststmt;
  long InsertList();

 ~CFILEFTPLIST();
};

// �������������
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

  // ��ȫ���������ļ�������������
  long LoadFileCFG();

  // �������л�ȡĳ�������ļ��Ĳ���
  BOOL GETFILECFG();

  struct st_TABCOLUMNS m_stTABCOLUMNS;
  vector<struct st_TABCOLUMNS> m_vTABCOLUMNS;

  // ��ȡ���ȫ��������Ϣ�������ֶΣ������m_vTABCOLUMNS������
  char m_KEYFIELDSTR[1024];
  long GETTABCOLUMNS();

  // �Ƿ���ڴ���ʱ���ֶ�
  BOOL m_bexistcrttimefield;

  // ���ɲ�ѯ�����ºͲ�����SQL���
  BOOL m_bnokeyidfield;
  char m_rowid[31],m_SelectSQL[1024],m_UpdateSQL[10240],m_InsertSQL[10240];
  CCmdStr m_CmdKeyFields;
  void CrtSQL();

  // ��ÿ�е����ݽ�����ֶ���
  void UNPackBuffer(char *strBuffer,int iFileVer);

  // ׼��SQL���
  long PreSQL();
  sqlstatement stmtselecttable,stmtupdatetable,stmtinserttable;

  long BindParams();

  // ��ѯ���ݱ��еļ�¼
  long InsertTable(char *strBuffer,char *striflog);

  // DDateTime�ֶ��������е�λ��
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
  char tname[31];       // ��Ӣ����
  char tcname[51];      // ��������
  UINT hdataptype;
  UINT hdatapcfg;
  char addterm[301];
  UINT syncinsert;
  UINT syncupdate;
  UINT syncdelete;
  UINT totalcount;      // �ܼ�¼��
  char counttime[20];   // ͳ��ʱ�䣬���һ��ִ�м�¼��ͳ�Ƶ�ʱ�䡣
  char backuptime[20];  // ����ʱ�䣬���һ��ִ�б��ݵ�ʱ�䡣
  UINT tablests;        // ��״̬��default 2��1-�����ã�2-�ѽ��ã�3-��ɾ����
  UINT tabletype;

  /*
  UINT ifcheck;
  UINT checktvl;
  char srcchecksql[301];
  char dstchecksql[301];
  UINT checkok;
  */
};

// ���ڴ�ű�ά���ı�����ݼ�¼
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

  // ��TAB�����ֵ��д��ڣ�  ��T_ALLTABLE���в����ڵļ�¼����T_ALLTABLE��
  // ��TAB�����ֵ��в����ڣ���T_ALLTABLE���д��ڵļ�¼ɾ����
  // ��USER_SEQUENCES�����ֵ��д��ڣ�  ��T_SEQANDTABLE���в����ڵļ�¼����T_SEQANDTABLE
  // ��USER_SEQUENCES�����ֵ��в����ڣ���T_SEQANDTABLE���д��ڵļ�¼ɾ����
  long UpdateALLTABLEAndSEQ();

  UINT m_appid;

  sqlstatement loadstmt;
  long LoadForCount();    // ��T_APPTABLE��������ĳ��Ⱥ��������ȫ����¼
  long LoadForCrtView();  // ��T_APPTABLE��������ĳ��Ⱥ������ȫ���ĵ�ǰ�����ڴ�����ͼ
  long LoadForManager();
  long LoadAllRecordNext();

  /*
  char m_dblinkname[50];
  long LoadForCheck(int in_appid);    // ��T_APPTABLE����������Ҫ�˶����ݵĲ�����
  long CheckTable();      // �˶Ա�ļ�¼��
  long UptCheckTime();    // ����T_ALLTABLE��ĺ˶Խ���ͺ˶�ʱ��
  */

  // ͳ�Ʊ�ļ�¼��
  long CountTable(char *in_DBLinkName);

  // ����T_ALLTABLE��ļ�¼������ͳ��ʱ���ֶ�
  long UptCountTime(char *in_DBLinkName);

  // ��ȡȫ����Ҫ���ݵļ�¼��������m_vALLTABLE��
  long LoadEXPRecord();

  // ж����
  BOOL ExpTable(char *strTmpPath,char *strStdPath,char *strConnStr,char *strTName);

  // ����T_ALLTABLE��ı���ʱ���ֶ�
  long UptBackupTime(char *strTName);

  struct st_ROWIDYEAR m_stROWIDYEAR;
  vector<struct st_ROWIDYEAR> m_vROWIDYEAR;

  long LoadALLROWID();

  // ��ȡ���ȫ��������Ϣ��������m_COLUMNSSTR��
  char m_COLUMNSSTR[1024];

  // ���ݵ���ʷ��
  long BackupToHTable();

  // ֻɾ��������
  long DeleteTable();

  // ���ݵ��鵵��
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

  // ��T_BDSYNCCFG����������Ҫִ������ͬ���ı�
  long LoadBSyncTable(int in_appid);

  // ������ͬ�����ִ��ʱ��Ϊ��ǰʱ��
  long UptBSyncTable(int in_appid,char *in_tname);

  // ������ͬ�������ͬ����¼�ı�־
  long UptBSyncTable(int in_appid,char *in_tname,UINT in_orgkeyid);
};

// �õ�ǰ�����������ϵ�ǰ����ʷ��͹鵵�����ͼ
long CrtView(connection *conn,char *strTName);

// �ж�tname1���Ƿ���ڣ���������ڣ�����tname2������
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

  // ɾ����������֮ǰ�ļ�¼
  long DelAlarmLog();
};

class CIDCCFG
{
public:
  char m_idcconnstr[101];          // ��������Ӧ�����ݿ�����Ӵ�

  char m_idcusername[101];         // �����������ݿ���û���

  char m_logpath[301];             // ��־�ļ�Ŀ¼

  char m_listpath[301];            // ��Ŷ������ĵ��ļ������б��ļ���Ŀ¼

  char m_tmppath[301];             // �����ʱ�ļ���Ŀ¼

  char m_sdatastdpath[301];        // ��ά������ݱ�׼XML�ļ���ŵ�Ŀ¼
  char m_sdatastdbakpath[301];     // ��ά������ݱ�׼XML�ļ����ݵ�Ŀ¼
  char m_sdatastderrpath[301];     // ��ά������ݱ�׼XML�ļ������ļ��ı��ݵ�Ŀ¼

  char m_wfilestdpath[301];        // �������ĵ��ļ����ݱ�׼�����ļ���ŵ�Ŀ¼
  char m_wfilestdbakpath[301];     // �������ĵ��ļ����ݱ�׼�����ļ����ݵ�Ŀ¼
  char m_wfilestderrpath[301];     // �������ĵ��ļ����ݱ�׼�����ļ����ݵ�Ŀ¼

  CIDCCFG();

  void initdata();

  // �������ϵͳ����
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

  // ����һ��ʱ�����ȱ���timespan���ӵ���ǰʱ��
  // unitΪʱ�����ȣ�1-����ӣ�2-�����ӣ�3-ʮ���ӣ�4-Сʱ��5-�졣
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

  // ��T_DMONCFG���м�����Ҫͳ�Ƶļ�¼�������m_vDMONCFG��
  BOOL LoadDMONCFG(const char *strWhere);

  // ��ȡm_vDMONCFG��ȫ���Ĳ�����ͳ��������
  BOOL CountTable();

  // ��ȡm_vDMONCFG��ȫ���Ĳ����������澯
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

// �����ļ������
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

  // ��ȫ���������ļ�������������
  long LoadFileCFG();


  // �������л�ȡĳ�������ļ��Ĳ���
  BOOL GETFILECFG();

  // �������������ļ�������ѯ�����Ƿ��Ѵ��ڸ��ļ�
  long m_timeexist;
  sqlstatement stmtfexist;
  long FindFExist();
  long FindFExistNext();

  BOOL m_IsInsertFile;

  // ���ļ����
  long InsertFileToDBEx();

  // �����������ļ�
  long UpdateFileToDBEx();
};

// ��ȡ������������ֵ
long FetchSequence(connection *conn,char *SequenceName,UINT &uValue);

// �ж�����ʱ���Ƿ�Ϸ������Ƿ���dmintime�յ���ǰʱ��֮���24��Сʱ
BOOL CheckDDateTime(char *in_DDateTime,char *in_DMinTime);

// ɱ����ͬ��Ŀ�ı��йصĻỰ���������
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

  // ����tasktype��������л�ȡ�������ϸ��Ϣ����������m_stEXPDTASK�С�
  sqlstatement selstmt;
  long GetTaskByID(UINT in_tasktype);
  long GetTaskByIDNext();

  // ��������ĵ���ʱ��
  long UptExpTime();
};

// ���ݲɼ���־
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

// ���ݴ�����־
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

// ���������־
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

// ���̻��־
struct st_PROACTLOG
{
  char indexid[501];
  char serverip[501];
  char programname[501];
  char pid[501];
  char maxtimeout[501];
  char latestactivetime[501];
};

// ���μӹ���ͳ�����ݣ���־
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

  CTimer    m_Timer; // ��ʱ��

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

  // �������ݲɼ���־XML�ļ�
  BOOL WriteToCollectLog();
 
  // �������ݴ�����־XML�ļ�
  BOOL WriteToProcessLog();

  // �������������־XML�ļ�
  BOOL WriteToDbLog();

  // ���ɽ��̻��־XML�ļ�
  BOOL bIsFirstTime; // ����ǵ�һ�Σ��Ǿ����������ļ�������Ҫ10���дһ�Ρ�
  BOOL WriteToProActLog(const char *in_Indexid,const char *in_ProgramName,const int in_MaxTimeOut);

  // ���ɶ��μӹ���ͳ�����ݣ���־XML�ļ�
  BOOL WriteToCalcLog();

  // �������ݴ�����־XML�ļ�
  BOOL WriteToTransferLog();

};

#endif
