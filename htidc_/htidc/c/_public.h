#ifndef _public_H
#define _public_H

#include "_cmpublic.h"

// ��ȡ���������ļ����ݣ������ļ������Ǵ�ͳ��INI�ļ������ǲ���xml�ļ���ʽ��
class CIniFile
{
public:
  // ��Ų����ļ�ȫ�������ݣ���LoadFile���뵽�������С�
  char m_XMLBuffer[204801];

  CIniFile();

  // �Ѳ����ļ����������뵽m_XMLBuffer�����С�
  BOOL LoadFile(const char *in_FileName);
  
  // ��ȡ�����ļ��ֶε����ݡ�
  BOOL GetValue(const char *in_FieldName,char   *out_Value,int in_len=0);
  BOOL GetValue(const char *in_FieldName,int    *out_Value);
  BOOL GetValue(const char *in_FieldName,long   *out_Value);
  BOOL GetValue(const char *in_FieldName,UINT   *out_Value);
  BOOL GetValue(const char *in_FieldName,double *out_Value);
};

// ��ȡĳĿ¼�µ�ȫ�����ļ�
class CDir
{
public:
  char m_DirName[301];         // Ŀ¼��
  char m_FileName[301];        // �ļ���
  char m_UpperFileName[301];   // ��д���ļ���
  char m_FullFileName[301];    // �ļ�ȫ��������·��
  UINT m_FileSize;             // �ļ��Ĵ�С
  char m_ModifyTime[21];       // �ļ����һ�α��޸ĵ�ʱ��
  char m_CreateTime[21];       // �ļ����ɵ�ʱ��
  char m_AccessTime[21];       // �ļ����һ�α����ʵ�ʱ��
  UINT m_uPOS;                 // �Ѷ�ȡm_vFileName�б��λ��
  vector<string> m_vFileName;  //  ���OpenDir������ȡ�����ļ��б�
  BOOL m_bOnlyDir;             // TRUE-ֻ��ȡĿ¼��Ϣ��FALSE-��ȡĿ¼���ļ���Ϣ��ȱʡΪFALSE
  vector<string> m_vDirName;   // ��Ÿ���Ŀ¼����Ϣ
  UINT m_uMaxFileCount;        // ÿ��ɨ��Ŀ¼��������ļ�����ȱʡ�ǲ�����
  BOOL m_bAndTMPFiles;         // ɨ��Ŀ¼���ļ�ʱ���Ƿ����tmp�ļ���TRUE-������FALSE-��������ȱʡ��FALSE

  CDir();

  void initdata();

  // ��������ʱ��ĸ�ʽ��֧��"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"���ָ�ʽ��ȱʡ��ǰ��
  char m_DateFMT[31];
  void SetDateFMT(const char *strDateFMT);

  // ��Ŀ¼�������ļ�������
  BOOL OpenDir(const char *in_dirname,const BOOL bAndChild=FALSE);

  // ��Ŀ¼��������
  BOOL OpenDirNoSort(const char *in_dirname,const BOOL bAndChild=FALSE);

  BOOL _OpenDir(const char *in_dirname,const BOOL bAndChild=FALSE);

  // �����ȡĿ¼�е��ļ�
  BOOL ReadDir();

  ~CDir();
};

/* д��־�ļ��� */
class CLogFile 
{
public:
  FILE   *m_tracefp;           // ��־�ļ�ָ��
  char    m_filename[301];     // ��־�ļ�ȫ��
  char    m_openmode[11];      // ��־�ļ��Ĵ򿪷�ʽ
  char    m_stime[21];         // ��־�ļ�д��ʱ�ĵ�ǰʱ�����
  char    m_message[20480];    // ��д�����־����
  BOOL    m_bBackup;           // ��־�ļ�����100M���Ƿ��Զ�����
  BOOL    m_bEnBuffer;         // д����־ʱ���Ƿ����ò���ϵͳ�Ļ������
  va_list m_ap;

  CLogFile();

  // filename��־�ļ���
  // openmode���ļ��ķ�ʽ��������־�ļ���Ȩ��,ͬ���ļ�����(fopen)ʹ�÷���һ��
  // bBackup��TRUE-���ݣ�FALSE-�����ݣ��ڶ���̵ķ�������У����������й���һ����־�ļ���bBackup����ΪFALSE
  // bEnBuffer:TRUE-���û�������FALSE-�����û�������������û���������ôд����־�ļ��е����ݲ�������д���ļ���
  BOOL Open(const char *in_filename,const char *in_openmode,BOOL bBackup=TRUE,BOOL bEnBuffer=FALSE);

  // �����־�ļ�����100M���ͱ�����
  BOOL BackupLogFile();

  // д��־�ļ�,���Ǹ��ɱ�����ķ���,ͬprintf����
  BOOL Write(const char *fmt,...);
  BOOL WriteEx(const char *fmt,...);

  // �ر���־�ļ�,����ļ�̫��(����100M),���ԭ��־�ļ�����
  void Close();

  // ���ø澯����
  BOOL m_balarmbz;
  char m_programname[101];
  void SetAlarmOpt(const char *fmt,...);

  // ��m_messageд��澯��־�ļ���
  BOOL WriteAlarmFile();

  ~CLogFile();
};

// ����ַ�������,
// �ַ����ĸ�ʽΪ:����1+�ָ��ַ���+����2+�ָ��ַ���+����3
// ��:num~!~name~!~address,�ָ���Ϊ"~!~"
class CCmdStr 
{
public:
  vector<string> m_vCmdStr;  // ��Ų�ֺ���ֶ����ݡ�

  CCmdStr();

  // ����ַ�����������
  void SplitToCmd(const string in_string,const char *in_sep,const BOOL bdeletespace=TRUE);

  UINT CmdCount();

  // ��ȡ�ֶε�ֵ��ȡÿ���ֶε�ֵinum��0��ʼ
  BOOL GetValue(const int inum,char   *in_return);
  BOOL GetValue(const int inum,char   *in_return,const int in_len);
  BOOL GetValue(const int inum,int    *in_return);
  BOOL GetValue(const int inum,long   *in_return);
  BOOL GetValue(const int inum,UINT   *in_return);
  BOOL GetValue(const int inum,float  *in_return);
  BOOL GetValue(const int inum,double *in_return);

  ~CCmdStr();
};

// ��ʱ��
class CTimer
{
public:
  // ����Beginning����ʱ��m_BeginningPOS���ڼ�¼���ʱ���
  time_t  m_BeginningPOS;

  CTimer();

  // ��ʼ��ʱ
  void Beginning();

  // ��������ȥ��ʱ�䣬��λ���롣
  long Elapsed();
};


class CProgramActive
{
public:
  CLogFile *m_logfile;
  CTimer    m_Timer;                 // һ��ʱ�������

  int    m_PID;                      // ���̵�ID
  char   m_ProgramName[51];          // ������
  time_t m_MaxTimeOut;               // ÿ�λ֮����������ʱ�䳤������Ϊ��λ
  time_t m_LatestActiveTime;        // ���һ�λʱ�䣬��������ʾ
  char   m_FileName[301];
  time_t m_Elapsed;                  // ���һ�λʱ���뵱ǰʱ���ʱ���
  time_t m_NowTime;

  CProgramActive();

  void initdata();

  // ���ý��̲���
  void SetProgramInfo(const CLogFile *in_logfile,const char *in_ProgramName,const int in_MaxTimeOut);

  // �����һ�εĽ��̻��Ϣд���ļ�
  BOOL WriteToFile(const BOOL bIsFirstTime=FALSE);

  // ���ļ��ж�ȡ���̻��Ϣ
  BOOL ReadFromFile(const char *in_FileName);

  ~CProgramActive();
};


// ����XMLBuffer����
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,char   *out_Value,const int in_StrLen=0);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,int    *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned int *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,long   *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned long *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,float  *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,double *out_Value);


/* ɾ���ַ������ָ�����ַ� */
void DeleteLChar(char *in_string,const char in_char);
/* ɾ����ߵĿո� */
void LTrim(char *in_string);
/* ɾ���ַ����ұ�ָ�����ַ� */
void DeleteRChar(char *in_string,const char in_char);
/* ɾ���ұߵĿո� */
void RTrim(char *in_string);
/* ɾ���ַ�������ָ�����ַ� */
void DeleteChar(char *in_string,const char in_char);
/* ɾ�����ߵĿո� */
void Trim(char *in_string);
// ɾ���ַ����м���ַ�
void DeleteMChar(char *in_string,const char in_char);

// ɾ���ַ����м���ַ���
void DeleteMStr(char *in_string,const char *in_str);
void SDeleteMStr(string &in_string,const string in_str);

// ���ַ����е�ĳ�ַ�������һ���ַ������棬bLoop�Ƿ�ѭ��ִ���滻
void UpdateStr(char *in_string,const char *in_str1,const char *in_str2,BOOL bLoop=TRUE);
// ���ַ����е�ĳ�ַ�������һ���ַ������棬bLoop�Ƿ�ѭ��ִ���滻
void UpdateStr(string &in_string,const char *in_str1,const char *in_str2,BOOL bLoop=TRUE);

// ��Сдת���ɴ�д����������в�����ĸ���ַ��Ͳ��ı�
void ToUpper(char *str);
// ��Сдת���ɴ�д����������в�����ĸ���ַ��Ͳ��ı�
void ToUpper(string &str);

// �Ѵ�дת����Сд����������в�����ĸ���ַ��Ͳ��ı�
void ToLower(char *str);
void ToLower(string &str);

// ��һ���ַ�������ȡ���֣�bWithSign==TRUE��ʾ�������ţ�bWithDOT==TRUE��ʾ����Բ��
void PickNumber(const char *strSrc,char *strDst,const BOOL bWithSign,const BOOL bWithDOT);

/*
  ȡ����ϵͳ��ʱ��,interval��ƫ�Ƴ���,��λ����,���صĸ�ʽ��fmt����
  fmtĿǰ��ȡֵ���£������Ҫ���������ӣ�
  yyyy-mm-dd hh24:mi:ss���˸�ʽ��ȱʡ��ʽ
  yyyymmddhh24miss
  yyyy-mm-dd
  yyyymmdd
  hh24:mi:ss
  hh24miss
  hh24:mi
  hh24mi
  hh24
  mi
*/
void LocalTime(char *stime,const char *in_fmt=0,const int interval=0);

// ��time_t���͵�����ʱ(�ַ���)ת��Ϊ���ݿ��׼ʱ yyyy-mm-dd hh24:mi:ss
void STDTime(char *strtime, char *buff);
void STDTime(char *strtime, long buff);

// ��yyyy-mm-dd hh24:mi:ssת��Ϊ��������ʾ��UTCʱ�䣬�����ڴ���Ĺ����н��������ڴ��еĸ�ʽ
long UTCTime(const char *stime);

/*
  in_stime�Ǵ����ʱ�䣬�����ʽ������һ��Ҫ����yyyymmddhh24miss���Ƿ��зָ���û�й�ϵ��
  ��yyyy-mm-dd hh24:mi:ssƫ��interval��
  �����ĸ�ʽ��fmt������fmtĿǰ��ȡֵ���£������Ҫ���������ӣ�
  yyyy-mm-dd hh24:mi:ss���˸�ʽ��ȱʡ��ʽ��
  yyyymmddhh24miss
  yyyymmddhh24miss
  yyyy-mm-dd
  yyyymmdd
  hh24:mi:ss
  hh24miss
  hh24:mi
  hh24mi
  ����ֵ��0-�ɹ���-1-ʧ�ܡ�
*/
int AddTime(const char *in_stime,char *out_stime,const int interval=0,const char *in_fmt=0);

// �ڵ��µ������ϼ���һ����
void SubtractAMonth(const char *strLocalYM,char *strPreYM);

// �ر�ȫ�����źź��������
void CloseIOAndSignal();

// �ж��ļ��Ƿ���strBZ����
BOOL CheckFileSTS(const char *strFileName,const char *strBZ);

// ��ȡ�ļ���ÿ�У�����ÿһ�еĻس����з�ȥ�����������ж�ÿ�е������Ƿ���strEndStr������
// �Ƿ���ÿ�е���󲹳�strPadStr�ַ�
BOOL FGETS(char *strBuffer,const int ReadSize,const FILE *fp,const char *strEndStr=0,const char *strPadStr=0);

// �ж��ļ��Ĵ�С�������ֽ���
long FileSize(const char *in_FullFileName);

// �ж��ļ���ʱ�䣬��modtime
void FileModTime(const char *in_FullFileName,char *out_ModTime);

// �ж��ļ����Ƿ��MatchFileNameƥ�䣬�����ƥ�䣬����ʧ��
// in_MatchStr�е���ĸ���ô�д�������Ҫƥ��yyyymmdd-xx.yyhh24mi��ʱ����ʾҪ����Сд��
BOOL MatchFileName(const string in_FileName,const string in_MatchStr,BOOL bPMatch=TRUE);

// �����ַ�����ƥ�������յ�����������Դ����ʽΪdd-xx.yy��ƥ�䣬xx.yy�ĵ�λ����
// ע���ʽ��xxһ��Ҫ��2λ���ȣ�yyһ��Ҫ��λ���ȣ�����Ĳ�0��
void ProcMatchDTime(char *in_MatchStr);

// �ж������Ƿ�ȫ��������
BOOL IsDigit(const char *strBuffer);

// �ж������Ƿ�ȫ���Ǵ�д��ĸ
BOOL IsUpper(const char *strBuffer);

// �ж������Ƿ�ȫ����ASCII�ַ�
BOOL IsASCII(const char *strBuffer);

// �ж������Ƿ�ȫ�������ֻ�ո�
BOOL IsDigitOrSpace(const char *strLine);

// ͳ���ַ����ֵĸ�����һ�����֣���Ӣ�Ļ����ֶ���һ���֡�
int SumWord(const char *in_Content);

class CSplitSMS
{
public:
  vector<string> m_vContent;

  // ���һ�����ŵ�����
  UINT m_EndCharCount;

  CSplitSMS();

  void initdata();

  void SplitToCmd(const char *in_Content,const int in_smslen);
};

// �Ѹ������ľ�γ��ת��Ϊ�ȷ��룬�ֱ�����strd,strf,strm��
void doubletodfm(const double dd,char *strd,char *strf,char *strm);

// ���ַ�����ʽ�ľ�γ�ȣ���111��23'45"��1112345��ת��Ϊ�ȣ���������double��char��
void dfmtodouble(const char *strdfm,double *dd);
void dfmtochar(const char *strdfm,char *strdd);

// ����x1,y1��x2,y2֮��ľ��룬x1,y1,x2,y2�ĵ�λ���Ƕ�
float x1y1tox2y2(const float x1,const float x2,const float y1,const float y2);

// �Ѷ��ŵ���Чʱ��Ͷ�ʱʱ��ת��
// valid_time��at_time�ĸ�ʽ��yyyy-mm-dd hh24:mi:ss
// strvalidtime��strattime�ĸ�ʽ��yymmddhhmisst32+
void ConvertTime(char *valid_time,char *strvalidtime,char *at_time,char *strattime);

#define TCPBUFLEN 8192  // TCP���ĵ���󳤶ȣ���ͨѶ�ײ����װ�����ƣ���������9999

// ����TCPͨѶ�Ŀͻ���
class CTcpClient
{
public:
  int  m_sockfd;
  char m_IP[17];
  int  m_Port;
  BOOL m_State;
  char m_ConnStr[200];
  BOOL m_bTimeOut;
  long m_BufLen;       // ���յ��ı��ĵİ���С
  CLogFile *m_logfile;

  CTcpClient();
  void SetConnectOpt(char *in_ConnStr);  // ���÷���˵Ĳ�������ʽΪ��IP,Port
  BOOL ConnectToServer();                // ���ӷ�����
  
  BOOL Read(char *strRead); // û�г�ʱ���ƵĶ�ȡ����
  BOOL Read(char *strRead,long iSecond); // �г�ʱ���ƵĶ�ȡ����
  BOOL Write(char *strWrite); // �����ַ���������Ϊ�ַ����ĳ���
  BOOL Write(char *strWrite,long buflen); // ���ʹ�����Ҫָ������
  
  void Close();
  ~CTcpClient();
};

// ����TCPͨѶ�ķ�����
class CTcpServer
{
public:
  int m_socklen;              // struct sockaddr_in�Ĵ�С
  struct sockaddr_in m_servaddr,m_clientaddr;
  int  m_listenfd;            // ���ڼ�����������
  int  m_connfd;              // ���ڿͻ������ӵ�������
  BOOL m_bTimeOut;
  long m_BufLen;             // ���յ��ı��ĵİ���С
  CLogFile *m_logfile;

  CTcpServer();
  BOOL InitServer(int port); // ��ʼ��TcpServer
  BOOL Accept();             // ���ܿͻ��˵�����
  char *GetIP();             // ��ȡ�Է���IP
  
  BOOL Read(char *strRead);               // û�г�ʱ����
  BOOL Read(char *strRead,long iSecond);  // �г�ʱ����
  BOOL Write(char *strWrite);             // �����ַ���������Ϊ�ַ����ĳ���
  BOOL Write(char *strWrite,long buflen); // ���ʹ�����Ҫָ������
  
  void CloseListen();              // �ر�TCP����
  void CloseClient();              // �ر�TCP����

  ~CTcpServer();
};

BOOL TcpRead(int fd,char *strRead,long *buflen,int iSecond=0);

BOOL TcpWrite(int fd,char *strWrite,long buflen=0,int iSecond=0);

BOOL Readn(int fd,char *vptr,size_t n);
BOOL Writen(int fd,const char *vptr,size_t n);

// �ļ�������
class CFile
{
public:
  char  m_fullfilename[301];  // ȫ·�����ļ���
  char  m_filename[301];      // ������·�����ļ���
  FILE *m_fp;                 // �ļ�ָ��
  BOOL  m_bEnBuffer;          // �Ƿ����û������ı�־��TRUE-���ã�FALSE-�����á�

  // ���³�Ա����ֻ�����ڴ��ļ�����д�����������
  char  m_tmppathname[301];   // �������ļ���ŵ���ʱĿ¼
  char  m_pathname[301];      // �������ļ���ŵ���ʽĿ¼
  char  m_tmpfullfilename[301];   // �������ļ����ļ�����������·����

  int   m_filetype;               // �ļ��򿪵ķ�ʽ��1-�ļ���ֻ����2-�ļ��򿪿�д��3-�ļ���Ϊ����

  CFile();

 ~CFile();  // ���������е���Fclose()����

  void initdata();

  // �����ļ��Ƿ�򿪣�TRUE��ʾ�Ѿ��򿪣�FALSE��ʾû�д�
  BOOL IsOpened();

  // ��һ���ļ����ڶ���fullfilenameΪȫ·�����ļ�����
  // openmodeΪ���ļ��ķ�ʽ��ֻ����"r"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
  BOOL OpenForRead(const char *fullfilename, const char *openmode);

  // ����fread���ļ��ж�ȡ����
  size_t Fread(void *ptr, size_t size, size_t nitems=1);

  // ��ȡ�ļ���ÿ�У�����ÿһ�еĻس����з�ȥ�����������ж�ÿ�е������Ƿ���strEndStr������
  // �Ƿ���ÿ�е���󲹳�strPadStr�ַ�
  BOOL FFGETS(char *strBuffer,const int ReadSize,const char *strEndStr=NULL,const char *strPadStr=NULL);

  // �ر��Ѵ򿪵�ֻ���ļ�����ɾ������
  BOOL CloseAndRemove();

  // ��һ���ļ�����д��fullfilenameΪ������·�����ļ���
  // openmodeΪ���ļ��ķ�ʽ��ͬfopenʹ�÷���һ�£�������"r"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
  // bEnBuffer:TRUE-���û�������FALSE-�����û�����
  BOOL OpenForWrite(const char *fullfilename, const char *openmode, BOOL bEnBuffer=FALSE);

  // ��һ���ļ�����д��tmppathname�ļ���ŵ���ʱĿ¼��pathname�ļ���ŵ���ʽĿ¼��filenameΪ������·�����ļ���
  // openmodeΪ���ļ��ķ�ʽ��ͬfopenʹ�÷���һ�£�������"r"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
  // bEnBuffer:TRUE-���û�������FALSE-�����û�����
  // ���������Ϊ�˼��ݾɵĳ���
  BOOL OpenForRename(const char *tmppathname,const char *pathname,const char *filename, const char *openmode, BOOL bEnBuffer=FALSE);
  // �Ժ�������º���
  BOOL OpenForRename(const char *fullfilename, const char *openmode, BOOL bEnBuffer=FALSE);

  // ���Ѵ򿪵��ļ�д�����ݡ�
  void Fprintf(const char *fmt, ... );

  // ����fwrite���ļ���д���ݣ���֪����ô��ģ��ú��������⣬��ʱ������
  // size_t Fwrite(const void *ptr, size_t size, size_t nitems=1);

  // �ر��ļ�ָ�룬������ʱ�ļ�����Ϊ��ʽ�ļ������������ʧ�ܣ�������FALSE���ɹ�����TRUE
  BOOL CloseAndRename();

  // ���m_filetype==1��m_filetype==2���͹ر�����
  // ���m_filetype==3���ر��ļ�ָ�룬��ɾ������ʱ�ļ���
  void Fclose();
};

// ɾ���ļ������ɾ��ʧ�ܣ��᳢��in_times��
BOOL REMOVE(const char *in_filename,const int in_times=3);

// ��in_srcfilename����Ϊin_dstfilename���������ʧ�ܣ��᳢��in_times��
BOOL RENAME(const char *in_srcfilename,const char *in_dstfilename,const int in_times=3);

// ���ļ�������ļ���Ŀ¼�����ڣ��ʹ�����Ŀ¼�����ǣ�����򿪷�ʽΪ"r"��"rb"���Ͳ��ᴴ��Ŀ¼��
FILE *FOPEN(const char *filename,const char *mode);

int OPEN(const char *filename, int flags);
int OPEN(const char *filename, int flags, mode_t mode);

// ��ĳ�ļ���Ŀ¼��ȫ·���е�Ŀ¼����Ŀ¼���Լ���Ŀ¼�µĸ�����Ŀ¼
BOOL MKDIR(const char *filename,BOOL bisfilename=TRUE);

// ��ĳһ���ļ����Ƶ���һ���ļ�
BOOL COPY(const char *srcfilename,const char *dstfilename);

// �����ļ����޸�ʱ�����ԣ�mtimeָ����ʱ�䣬��ʽ���ޣ�ֻҪ������yyyy,mm,dd,hh24,mi,ss���ɡ�
int UTime(const char *filename,const char *mtime);

// �Ѷȶȶȷַ������ʽ���ַ���ת��Ϊ��λΪ�ȵ��ַ�����
// strAzimuth��ʾ���Ȼ�γ�ȣ����ȵĳ��ȱ���7��γ�ȵĳ��ȱ�����6λ��
// strNumber���8λ�ĸ���������ȷ��С�����5λ��
BOOL AzimuthToNumber(char *strAzimuth,char *strNumber);

// lon_rd�״�ľ��ȣ�lat_rd�״��γ�ȣ�h_rd�״�ĸ߶�
// ָ����㾭��lon_p��γ��lat_p���߶�h_p
// angle���״�վ��ָ��������ӵ��ĵļнǣ�б��range������elev����λ��azim
int get_sea(double lon_rd, double lat_rd, double h_rd ,
            double lon_p , double lat_p , double h_p  ,
            double *angle, double *range, double *elev, double *azim);


// lon_rd�״�ľ��ȣ�lat_rd�״��γ�ȣ�h_rd�״�ĸ߶ȣ���λ���ף�
// Ŀ�ĵ��rangб�࣬elev���ǣ�azim��λ��
// Ŀ�ĵ��lon_p���ȣ�lat_pγ�ȣ�h_p�߶ȣ���λ���ף�
int get_llh(double lon_rd, double lat_rd, double h_rd,
            double rang  , double elev  , double azim,
            double *lon_p, double *lat_p, double *h_p);

// �ж��Ƿ���Ҫ����buffer�е�ʱ�����
// ���Դ�������ʱ�������{yyyy}��4λ���꣩��{yyy}������λ���꣩��
// {yy}������λ���꣩��{mm}�����£���{dd}�����գ���{hh}��ʱʱ����{mi}���ַ֣���{ss}�����룩
void MatchBuffer(char *buffer,int timetvl=0);

// ѹ���ļ�Ϊzip��ʽ
void ZIPFiles(char *strFileName,BOOL bKeep=FALSE);

// ���ַ���ת��Ϊdouble
double ATOF(const char *nptr);

// �Ѷ����Ƶ��ַ���ת��Ϊʮ����
int bintodec(char *strbin,int length);

// ����־�ļ��ж�ȡ�Ѵ����¼��λ��
long ReadLogFile(const char *strFileName);

// �ѱ����Ѵ�����ļ���λ��д����־�ļ�
BOOL WriteToLogFile(FILE *fpSend,const char *strFileName);

// �ѱ����Ѵ�����ɵĵ���־�ļ�ɾ��
BOOL RemoveLogFile(char *strFileName);

// ���ļ��м���һ����¼������ļ�û���¼�¼��strBuffer��Ϊ��
BOOL LoadOneLine(const char *strFileName,char *strBuffer);

// �ѷ���ת��Ϊ�ȼ�
// in_wf-���٣���λ��0.1m/s��out_wdj-���ٵȼ���2-С��������3-������4-�ļ�...12-ʮ������13-����12��
BOOL wftowdj(char *in_wf,char *out_wdj);

// ת��,UTF-8 ת GB18030
// dest:Ŀ�ĸ�ʽ, src:ԭ��ʽ, input:�����ַ���, output:����ַ���
int conv_charset(const char *dest, const char *src, char *input, size_t inlen, char *output, size_t outlen);

// ���ô˺�������Ҫ���÷�������hostname����hosts�ļ������
BOOL getLocalIP(char *localip);

#endif
