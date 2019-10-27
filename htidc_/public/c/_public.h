/*
   �˳���������ʾ���Զ��庯�������ڳ����ⲿ��ͷ�ļ���
   ���ߣ�������   ���ڣ�20190910
*/
#ifndef _PUBLIC_H
#define _PUBLIC_H 1

#include "_cmpublic.h"

// ȫ·���ļ�������С��ʱ��Ľṹ��
struct st_fileinfo
{
  char filename[301];
  int  filesize;
  char mtime[21];
};

// ��ȡĳĿ¼�µ�ȫ�����ļ�
class CDir
{
public:
  char m_DirName[301];         // Ŀ¼��
  char m_FileName[301];        // �ļ�����������Ŀ¼��
  char m_FullFileName[301];    // �ļ�ȫ��������Ŀ¼��
  int  m_FileSize;             // �ļ��Ĵ�С
  char m_ModifyTime[21];       // �ļ����һ�α��޸ĵ�ʱ��
  char m_CreateTime[21];       // �ļ����ɵ�ʱ��
  char m_AccessTime[21];       // �ļ����һ�α����ʵ�ʱ��

  int m_uPOS;                 // �Ѷ�ȡm_vFileName������λ��
  vector<string> m_vFileName;  //  ���OpenDir������ȡ�����ļ��б�

  CDir();

  // ������ʼ��
  void initdata();

  // ��������ʱ��ĸ�ʽ��֧��"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"���ָ�ʽ��ȱʡ��ǰ��
  char m_DateFMT[21];
  void SetDateFMT(const char *in_DateFMT);

  // ��Ŀ¼����ȡ�ļ�����Ϣ�������m_vFileName������
  // in_DirName�����򿪵�Ŀ¼��
  // in_MatchStr������ȡ�ļ�����ƥ�����
  // in_MaxCount����ȡ�ļ����������
  // bAndChild���Ƿ�򿪸�����Ŀ¼
  // bSort���Ƿ�Խ������������
  bool OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount=10000,const bool bAndChild=false,bool bSort=false);

  // ����һ���ݹ麯��������OpenDir()�ĵ��á�
  bool _OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const bool bAndChild);

  // �����ȡĿ¼�е��ļ���Ϣ
  bool ReadDir();

  ~CDir();
};


// STRCPY��STRNCPY��STRCAT��STRNCAT�ĸ��������ֲ��⺯����ȱ��
// �Ժ���������ĸ�����ȡ��strcpy��strncpy��strcat��strncat
// �����ĵڶ��������ǵ�һ������dest��Ч���ȣ���ռ���ڴ���ֽ���-1��
// ��ϵ�к�������������⣺1��������ʼ����2���ڴ������3���޸�strncpy��ȱ�ݡ�
char *STRCPY(char* dest,const size_t destlen,const char* src);
char *STRNCPY(char* dest,const size_t destlen,const char* src,size_t n);
char *STRCAT(char* dest,const size_t destlen,const char* src);
char *STRNCAT(char* dest,const size_t destlen,const char* src,size_t n);

int SNPRINTF(char *str, size_t size, const char *fmt, ...);

// ��������ʱ��ת��Ϊ�ַ�����ʽ��ʱ�䣬��ʽ�磺"2019-02-08 12:05:08"�����ת���ɹ���������0��ʧ�ܷ���-1���������������£�
int timetostr(const time_t ti,char *strtime);

// ���ַ�����ʽ��ʱ��ת��Ϊ������ʱ�䣬�������������£�
int strtotime(const char *strtime,time_t *ti);

// ���ļ��ļ��ж�ȡһ��
// strEndStr��һ�����ݵĽ�����־�����Ϊ�գ����Ի��з�"\n"Ϊ������־��
bool FGETS(const FILE *fp,char *strBuffer,const int ReadSize,const char *strEndStr=0);

// �ļ�����������
class CFile
{
private:
  FILE *m_fp;        // �ļ�ָ��
  bool  m_bEnBuffer; // �Ƿ����û�������true-���ã�false-������
  char  m_filename[301]; // �ļ���
  char  m_filenametmp[301]; // ��ʱ�ļ���

public:
  CFile();   // ��Ĺ��캯��
 ~CFile();   // �����������

  bool IsOpened();  // �ж��ļ��Ƿ��Ѵ�

  // ���ļ���������fopen��ͬ���򿪳ɹ�true��ʧ�ܷ���false          
  bool Open(const char *filename,const char *openmode,bool bEnBuffer=true);
  // �ر��ļ�ָ�룬��ɾ���ļ�
  bool CloseAndRemove();

  // רΪ�����������ļ���������fopen��ͬ���򿪳ɹ�true��ʧ�ܷ���false          
  // �������ᴴ��filename���.tmp����ʱ�ļ�������CloseAndRename()��Ű���ʱ�ļ�����Ϊ��ʽ�ļ�
  bool OpenForRename(const char *filename,const char *openmode,bool bEnBuffer=true);
  // �ر��ļ�������
  bool CloseAndRename();

  // ����fprintf���ļ�д������
  void Fprintf(const char *fmt, ... );

  // ����fgets���ļ��ж�ȡһ�У�bDelCRT=trueɾ�����з���false��ɾ��
  bool Fgets(char *strBuffer,const int ReadSize,bool bDelCRT=false);

  // ���ļ��ļ��ж�ȡһ��
  // strEndStr��һ�����ݵĽ�����־�����Ϊ�գ����Ի��з�"\n"Ϊ������־��
  // ��Fgets��ͬ����������ɾ��������־
  bool FFGETS(char *strBuffer,const int ReadSize,const char *strEndStr=0);

  // �ر��ļ�ָ�룬���������ʱ�ļ�����ɾ������
  void Close();
};

// ����ַ�������
// �ַ����ĸ�ʽΪ:����1+�ָ��ַ���+����2+�ָ��ַ���+����3
// ��:num~!~name~!~address,�ָ���Ϊ"~!~"
class CCmdStr
{
public:
  vector<string> m_vCmdStr;  // ��Ų�ֺ���ֶ����ݡ�

  CCmdStr();

  // ����ַ�����������
  void SplitToCmd(const string in_string,const char *in_sep,const bool bdeletespace=true);

  int CmdCount();

  // ��ȡ�ֶε�ֵ��ȡÿ���ֶε�ֵinum��0��ʼ
  bool GetValue(const int inum,char   *in_return);
  bool GetValue(const int inum,char   *in_return,const int in_len);
  bool GetValue(const int inum,int    *in_return);
  bool GetValue(const int inum,long   *in_return);
  bool GetValue(const int inum,double *in_return);

  ~CCmdStr();
};

// ɾ���ַ������ָ�����ַ�
void DeleteLChar(char *in_string,const char in_char);

// ɾ���ַ����ұ�ָ�����ַ�
void DeleteRChar(char *in_string,const char in_char);

// ɾ���ַ�������ָ�����ַ�
void DeleteLRChar(char *in_string,const char in_char);

/*
  ȡ����ϵͳ��ʱ��
  out_stime��������
  in_interval��ƫ�Ƴ�������λ����
  ���صĸ�ʽ��fmt������fmtĿǰ��ȡֵ���£������Ҫ���������ӣ�
  yyyy-mm-dd hh:mi:ss���˸�ʽ��ȱʡ��ʽ
  yyyymmddhhmiss
  yyyy-mm-dd
  yyyymmdd
  hh:mi:ss
  hhmiss
  hh:mi
  hhmi
  hh
  mi
*/
void LocalTime(char *out_stime,const char *in_fmt=0,const int in_interval=0);

///////////////////////////////////////////////////////////////////////////////////////////////////
// ��������־�ļ�������

// ��־�ļ�������
class CLogFile
{
public:
  FILE   *m_tracefp;           // ��־�ļ�ָ��
  char    m_filename[301];     // ��־�ļ�ȫ��
  char    m_openmode[11];      // ��־�ļ��Ĵ򿪷�ʽ
  bool    m_bBackup;           // ��־�ļ�����100M���Ƿ��Զ�����
  bool    m_bEnBuffer;         // д����־ʱ���Ƿ����ò���ϵͳ�Ļ������

  CLogFile();

  // filename��־�ļ���
  // openmode���ļ��ķ�ʽ��������־�ļ���Ȩ��,ͬ���ļ�����(fopen)ʹ�÷���һ�£�һ�����"a+"
  // bBackup��true-���ݣ�false-�����ݣ��ڶ���̵ķ�������У����������й���һ����־�ļ���bBackup����Ϊfalse
  // bEnBuffer:true-���û�������false-�����û�������������û���������ôд����־�ļ��е����ݲ�������д���ļ�
  bool Open(const char *in_filename,const char *in_openmode,bool bBackup=true,bool bEnBuffer=false);

  // �����־�ļ�����100M���ͱ�����
  // ���ݺ���ļ�����Դ�ļ������������ʱ��
  bool BackupLogFile();

  // д��־�ļ�,���Ǹ��ɱ�����ķ���,ͬprintf������
  // Write()������д��ʱ�䣬WriteEx()������дʱ�䡣
  bool Write(const char *fmt,...);
  bool WriteEx(const char *fmt,...);

  // �ر���־�ļ�
  void Close();

  ~CLogFile();
};

// �ر�ȫ�����źź��������
void CloseIOAndSignal();

// ��ĳ�ļ���Ŀ¼��ȫ·���е�Ŀ¼����Ŀ¼���Լ���Ŀ¼�µĸ�����Ŀ¼
// pathorfilename Ŀ¼�����ļ���
// bisfilename true-pathorfilename���ļ�����������Ŀ¼��
bool MKDIR(const char *pathorfilename,bool bisfilename=true);

// ɾ���ļ������ɾ��ʧ�ܣ��᳢��in_times��
bool REMOVE(const char *in_filename,const int in_times=3);

// ��in_srcfilename����Ϊin_dstfilename���������ʧ�ܣ��᳢��in_times��
bool RENAME(const char *in_srcfilename,const char *in_dstfilename,const int in_times=3);

// ����fopen�������ļ�������ļ����а�����Ŀ¼�����ڣ��ʹ���Ŀ¼
FILE *FOPEN(const char *filename,const char *mode);

// ��ȡ�ļ��Ĵ�С�������ֽ���
int FileSize(const char *in_FullFileName);

// ��ȡ�ļ���ʱ�䣬��modtime
void FileMTime(const char *in_FullFileName,char *out_ModTime);

// �����ļ����޸�ʱ�����ԣ�mtimeָ����ʱ�䣬��ʽ���ޣ�ֻҪ������yyyy,mm,dd,hh24,mi,ss���ɡ�
int UTime(const char *filename,const char *mtime);


// ���ַ�����ʽ��ʱ��ת��Ϊtime_t
// stimeΪ�����ʱ�䣬��ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss
time_t UTCTime(const char *stime);

// ��һ���ַ�������ȡ���֡����ź�С���㣬���ж��Ƿ���һ���Ϸ�����
// ������Ϸ��������ؿ��ַ���
// bWithSign==true��ʾ�������ţ�bWithDOT==true��ʾ����Բ��
void PickNumber(const char *strSrc,char *strDst,const bool bWithSign,const bool bWithDOT);

// �ж��ַ����еĸ��ź�Բ���Ƿ�Ϸ�
bool JudgeSignDOT(const char *strSrc,const char *strBZ);

/*
  ��һ���ַ�������ʱ�����һ��ƫ�������õ�ƫ�ƺ��ʱ��
  in_stime�Ǵ����ʱ�䣬�����ʽ������һ��Ҫ����yyyymmddhh24miss���Ƿ��зָ���û�й�ϵ��
  ��yyyy-mm-dd hh24:mi:ssƫ��in_interval��
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
int AddTime(const char *in_stime,char *out_stime,const int in_interval,const char *in_fmt=0);

// ������XML��ʽ�ַ�������ز�����������

// ����XMLBuffer�ĺ���
// in_XMLBuffer��XML��ʽ���ַ��������£�
// <filename>/tmp/readme.txt</filename><mtime>2018-01-01 12:20:35</mtime><size>10241</size>
// in_FieldName���ֶεı�ǩ��
// out_Value����ȡ���ݴ�ŵı�����ָ��
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,bool   *out_Value);
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,int    *out_Value);
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned int *out_Value);
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,long   *out_Value);
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned long *out_Value);
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,double *out_Value);
bool GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,char   *out_Value,const int in_StrLen=0);

// �ж��ļ����Ƿ�ƥ��in_MatchStrָ���Ĺ���
// in_FileName�ļ���
// in_MatchStr������ʽ����"*.txt,*.xml"���м��ö��ŷָ�
bool MatchFileName(const string in_FileName,const string in_MatchStr);

// ��Сдת���ɴ�д�����Բ�����ĸ���ַ�
void ToUpper(char *str);
void ToUpper(string &str);

// �Ѵ�дת����Сд�����Բ�����ĸ���ַ�
void ToLower(char *str);
void ToLower(string &str);

// �ַ����滻����
// ��in_string�е�in_str1�滻Ϊin_str2
// bLoop�Ƿ�ѭ��ִ���滻
// ע��
// 1�����in_str2��in_str1Ҫ�����滻��in_string��䳤�����Ա��뱣֤in_string���㹻�ĳ��ȣ� �����ڴ�����
// 2�����in_str2�а�����in_str1�����ݣ���bLoopΪtrue���ͻ������ѭ�������յ����ڴ����
void UpdateStr(char *in_string,const char *in_str1,const char *in_str2,bool bLoop=true);

///////////////////////////////////// /////////////////////////////////////
// ������TCP/IPͨѶ�ĺ�������

#define TCPHEADLEN     4  // TCP����ͷ���ĳ���
#define TCPBUFLEN  10240  // TCP���ĵ���󳤶ȣ����ܳ���TCPHEADLEN�ܱ��ĳ��ȡ�

// ����TCPͨѶ�Ŀͻ���
class CTcpClient
{
public:
  int  m_sockfd;        // socket id
  char m_ip[21];        // ����˵�IP
  int  m_port;          // ������ͨѶ�Ķ˿�
  bool m_state;         // ����״̬
  bool m_btimeout;      // Read()�����Ƿ�ʱ��true-δ��ʱ��false-�ѳ�ʱ
  int  m_buflen;        // ���յ��ı��ĵİ���С

  CTcpClient();

  // �����˷�������
  bool ConnectToServer(const char *in_ip,const int in_port);                // ���ӷ�����

  // û�г�ʱ���ƵĽ��պ���
  bool Read(char *strRecvBuffer);
  // �г�ʱ���ƵĽ��պ���
  bool Read(char *strRecvBuffer,const int iTimeOut);

  // �����˷����ַ���������Ϊ�ַ����ĳ���
  bool Write(char *strSendBuffer);

  // �����˷��������ݣ���Ҫָ������
  bool Write(char *strSendBuffer,const int buflen);

  // �Ͽ������˵�����
  void Close();

  ~CTcpClient();
};

// ����TCPͨѶ�ķ�����
class CTcpServer
{
private:
  int m_socklen;                    // struct sockaddr_in�Ĵ�С
  struct sockaddr_in m_clientaddr;  // �ͻ��˵ĵ�ַ��Ϣ
  struct sockaddr_in m_servaddr;    // ����˵ĵ�ַ��Ϣ
public:
  int  m_listenfd;                  // ��������ڼ�����socket
  int  m_connfd;                    // �ͻ��˵�socket
  bool m_btimeout;                  // Read()�����Ƿ�ʱ��true-δ��ʱ��false-�ѳ�ʱ
  int  m_buflen;                    // ���յ��ı��ĵİ���С

  CTcpServer();
  bool InitServer(const unsigned int port); // ��ʼ��TcpServer
  bool Accept();                            // ���ܿͻ��˵�����
  char *GetIP();                            // ��ȡ�ͻ��˵�ip��ַ


  // û�г�ʱ���ƵĽ��պ���
  bool Read(char *strRecvBuffer);
  // �г�ʱ���ƵĽ��պ���
  bool Read(char *strRecvBuffer,const int iTimeOut);

  // ��ͻ��˷����ַ���������Ϊ�ַ����ĳ���
  bool Write(char *strSendBuffer);

  // ��ͻ��˷��������ݣ���Ҫָ������
  bool Write(char *strSendBuffer,const int buflen);

  // �رռ�����sock�����ڶ���̷��������ӽ��̴�����
  void CloseListen();

  // �رտͻ��˵�sock�����ڶ���̷������ĸ����̴�����
  void CloseClient();

  ~CTcpServer();
};

// ��ȡsocket������
// fd socket
// strRecvBuffer ��ȡ�����ŵĻ�����
// buflen ��ȡ���ı��ĵĳ���
// iTimeOut ��ȡ��ʱ��ʱ��
bool TcpRead(const int fd,char *strRecvBuffer,int *buflen,const int iTimeOut=0);

// ������д��socket
// fd socket
// strSendBuffer ��д�����ݴ�ŵĻ�����
// buflen ��д�����ݵĳ��ȣ����Ϊ0������ΪstrSendBuffer��һ���ַ�����
// iTimeOut д�볬ʱ��ʱ��
bool TcpWrite(const int fd,const char *strSendBuffer,const int buflen=0,const int iTimeOut=0);

// ��socket�ж�ȡ���ݣ�ר����TcpRead����
bool Readn(const int fd,char *vptr,const size_t n);

// ��socket��д�����ݣ�ר����TcpWrite����
bool Writen(const int fd,const char *vptr,const size_t n);

// ���ļ�ͨ��sockfd���͸��Զ�
bool SendFile(CLogFile *logfile,int sockfd,struct st_fileinfo *stfileinfo);

// ����ͨ��socdfd���͹������ļ�
bool RecvFile(CLogFile *logfile,int sockfd,struct st_fileinfo *stfileinfo);


// ������TCP/IPͨѶ�ĺ�������
///////////////////////////////////// /////////////////////////////////////






#endif
