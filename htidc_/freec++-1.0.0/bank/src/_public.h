////////////////////////////////////////////////////////
// ���������ũ��Դ���ṩ����Դ�����Ҫ��ҵ�Ŭ����
// �����������QQȺ701117364������һ��ѿ�Դ������ø��á�
//
// ����ٷ�����Э��Լ��
//
// 1) ��ֹ���ڼ�����ѵ���������ҵ��;��
// 2) ��ֹ�κ���Ȩ��Ϊ�����������Դ����������ĵ����ļ���Ϣ��
// 3) ��ֹ�����Ʒ����������ٷ��޹ص��κι����Ϣ���������֡�ͼ��ý����Ϣ��
// 4) ��ֹ�����Ʒ����������ٷ�����Ӧ���޹ص��κε�����������������
// 5) ��ֹ�����������κβ������û���ɵ�Զ����Ӧִ�еĿ��Ƴ���
// 6) ��ֹ�������ʹ���κ��ֶ��ռ��û���˽��Ϣ���û���������ݡ�
//
//
//  �û�����Э��Լ��
//  1) ��ֹ�û��޸�����ٷ��κεİ�Ȩ˵����Ϣ����������İ�ȨЭ��˵��������ٷ����ӡ�����˵����ͼ���־��ý����Ϣ��
//  2) ��ֹ�û�ͨ���κη�ʽ�ƻ����ַ�����ٷ���������չ��Ӫ����������ٷ������������������ҵ��Ȩ���ơ�
//
////////////////////////////////////////////////////////

#ifndef _public_H
#define _public_H

#include "_cmpublic.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// ������XML��ʽ�ַ�������ز�����������

// ����XMLBuffer�ĺ���
// in_XMLBuffer��XML��ʽ���ַ��������£�
// <filename>/tmp/readme.txt</filename><mtime>2018-01-01 12:20:35</mtime><size>10241</size>
// in_FieldName���ֶεı�ǩ��
// out_Value����ȡ���ݴ�ŵı�����ָ��
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,int    *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned int *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,long   *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned long *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,double *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,char   *out_Value,const int in_StrLen=0);

// ��ȡ���������ļ�����
// �����ļ�������׼��XML������һ���򵥵ĵ���ǩXML��
class CXmlFile
{
public:
  // ��Ų����ļ�ȫ�������ݣ���LoadFile���뵽�������С�
  // ע�⣬����ļ������ݴ���20480����Ҫ�Ӵ�m_XMLBuffer���ڴ�
  char m_XMLBuffer[20480];

  CXmlFile();

  // �Ѳ����ļ����������뵽m_XMLBuffer�����С�
  BOOL LoadFile(const char *in_FileName);
  
  // ��ȡ�����ļ��ֶε����ݡ�
  BOOL GetValue(const char *in_FieldName,int    *out_Value);
  BOOL GetValue(const char *in_FieldName,long   *out_Value);
  BOOL GetValue(const char *in_FieldName,UINT   *out_Value);
  BOOL GetValue(const char *in_FieldName,double *out_Value);
  BOOL GetValue(const char *in_FieldName,char   *out_Value,int in_len=0);
};

// ������XML��ʽ�ַ�������ز�����������
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ������Ŀ¼�����ĺ�������

// ��ĳ�ļ���Ŀ¼��ȫ·���е�Ŀ¼����Ŀ¼���Լ���Ŀ¼�µĸ�����Ŀ¼
// pathorfilename Ŀ¼�����ļ���
// bisfilename TRUE-pathorfilename���ļ�����������Ŀ¼��
BOOL MKDIR(const char *pathorfilename,BOOL bisfilename=TRUE);

// ��ȡĳĿ¼�µ�ȫ�����ļ�
class CDir
{
public:
  char m_DirName[301];         // Ŀ¼��
  char m_FileName[301];        // �ļ�����������Ŀ¼��
  char m_FullFileName[301];    // �ļ�ȫ��������Ŀ¼��
  UINT m_FileSize;             // �ļ��Ĵ�С
  char m_ModifyTime[21];       // �ļ����һ�α��޸ĵ�ʱ��
  char m_CreateTime[21];       // �ļ����ɵ�ʱ��
  char m_AccessTime[21];       // �ļ����һ�α����ʵ�ʱ��

  UINT m_uPOS;                 // �Ѷ�ȡm_vFileName������λ��
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
  BOOL OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount=10000,const BOOL bAndChild=FALSE,BOOL bSort=FALSE);

  // ����һ���ݹ麯��������OpenDir()�ĵ��á�
  BOOL _OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const BOOL bAndChild);

  // �����ȡĿ¼�е��ļ���Ϣ
  BOOL ReadDir();

  ~CDir();
};

// ������Ŀ¼�����ĺ�������
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ��������־�ļ�������

// ��־�ļ�������
class CLogFile 
{
public:
  FILE   *m_tracefp;           // ��־�ļ�ָ��
  char    m_filename[301];     // ��־�ļ�ȫ��
  char    m_openmode[11];      // ��־�ļ��Ĵ򿪷�ʽ
  char    m_stime[20];         // ��־�ļ�д��ʱ�ĵ�ǰʱ�����
  char    m_message[10240];    // ��д�����־����
  BOOL    m_bBackup;           // ��־�ļ�����100M���Ƿ��Զ�����
  BOOL    m_bEnBuffer;         // д����־ʱ���Ƿ����ò���ϵͳ�Ļ������
  va_list m_ap;

  CLogFile();

  // filename��־�ļ���
  // openmode���ļ��ķ�ʽ��������־�ļ���Ȩ��,ͬ���ļ�����(fopen)ʹ�÷���һ�£�һ�����"a+"
  // bBackup��TRUE-���ݣ�FALSE-�����ݣ��ڶ���̵ķ�������У����������й���һ����־�ļ���bBackup����ΪFALSE
  // bEnBuffer:TRUE-���û�������FALSE-�����û�������������û���������ôд����־�ļ��е����ݲ�������д���ļ�
  BOOL Open(const char *in_filename,const char *in_openmode,BOOL bBackup=TRUE,BOOL bEnBuffer=FALSE);

  // �����־�ļ�����100M���ͱ�����
  // ���ݺ���ļ�����Դ�ļ������������ʱ��
  BOOL BackupLogFile();

  // д��־�ļ�,���Ǹ��ɱ�����ķ���,ͬprintf������
  // Write()������д��ʱ�䣬WriteEx()������дʱ�䡣
  BOOL Write(const char *fmt,...);
  BOOL WriteEx(const char *fmt,...);

  // �ر���־�ļ�
  void Close();

  ~CLogFile();
};

// ��������־�ļ�������
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////// /////////////////////////////////////
// �������ַ���������صĺ�������

// ɾ���ַ������ָ�����ַ�
void DeleteLChar(char *in_string,const char in_char);

// ɾ���ַ����ұ�ָ�����ַ�
void DeleteRChar(char *in_string,const char in_char);

// ɾ���ַ�������ָ�����ַ�
void DeleteLRChar(char *in_string,const char in_char);

// ɾ���ַ����м���ַ���
void DeleteMStr(char *in_string,const char *in_str);

// ���ַ�������߲��ַ���ָ������
void LPad(char *in_string,const char in_char,unsigned int in_len);

// ���ַ������ұ߲��ַ���ָ������
void RPad(char *in_string,const char in_char,unsigned int in_len);

// �ַ����滻����
// ��in_string�е�in_str1�滻Ϊin_str2
// bLoop�Ƿ�ѭ��ִ���滻
// ע��
// 1�����in_str2��in_str1Ҫ�����滻��in_string��䳤�����Ա��뱣֤in_string���㹻�ĳ��ȣ������ڴ�����
// 2�����in_str2�а�����in_str1�����ݣ���bLoopΪTRUE���ͻ������ѭ�������յ����ڴ����
void UpdateStr(char *in_string,const char *in_str1,const char *in_str2,BOOL bLoop=TRUE);

// ��Сдת���ɴ�д�����Բ�����ĸ���ַ�
void ToUpper(char *str);
void ToUpper(string &str);

// �Ѵ�дת����Сд�����Բ�����ĸ���ַ�
void ToLower(char *str);
void ToLower(string &str);

// ��һ���ַ�������ȡ���֡����ź�С���㣬���ж��Ƿ���һ���Ϸ�����
// ������Ϸ��������ؿ��ַ���
// bWithSign==TRUE��ʾ�������ţ�bWithDOT==TRUE��ʾ����Բ��
void PickNumber(const char *strSrc,char *strDst,const BOOL bWithSign,const BOOL bWithDOT);

// �ж��ļ����Ƿ�ƥ��in_MatchStrָ���Ĺ���
// in_FileName�ļ���
// in_MatchStr������ʽ����"*.txt,*.xml"���м��ö��ŷָ�
BOOL MatchFileName(const string in_FileName,const string in_MatchStr);

// �ж������Ƿ�ȫ��������
BOOL IsDigit(const char *strBuffer);

// �ж������Ƿ�ȫ���Ǵ�д��ĸ
BOOL IsUpper(const char *strBuffer);

// �ж������Ƿ�ȫ����ASCII�ַ�
BOOL IsASCII(const char *strBuffer);

// �ж������Ƿ�ȫ�������ֺͿո�
BOOL IsDigitOrSpace(const char *strLine);

// ����ַ�������
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

// �������ַ���������صĺ�������
///////////////////////////////////// /////////////////////////////////////


///////////////////////////////////// /////////////////////////////////////
// ����������ʱ�������������

/*
  ȡ����ϵͳ��ʱ��
  out_stime��������
  in_interval��ƫ�Ƴ�������λ����
  ���صĸ�ʽ��fmt������fmtĿǰ��ȡֵ���£������Ҫ���������ӣ�
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
void LocalTime(char *out_stime,const char *in_fmt=0,const int in_interval=0);

// ���ַ�����ʽ��ʱ��ת��Ϊtime_t
// stimeΪ�����ʱ�䣬��ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss
time_t UTCTime(const char *stime);

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

// ����һ����ȷ��΢��ļ�ʱ��
class CTimer
{
public:
  struct timeval m_start,m_end;

  CTimer();

  // ��ʼ��ʱ
  void Start();

  // ��������ȥ��ʱ�䣬��λ���룬С���������΢��
  double Elapsed();
};

// ����������ʱ�������������
///////////////////////////////////// /////////////////////////////////////


///////////////////////////////////// /////////////////////////////////////
// ������TCP/IPͨѶ�ĺ�������

#define TCPHEADLEN     5  // TCP����ͷ���ĳ��ȣ����ܳ���10��
#define TCPBUFLEN  10240  // TCP���ĵ���󳤶ȣ����ܳ���TCPHEADLEN�ܱ��ĳ��ȡ�

// ����TCPͨѶ�Ŀͻ���
class CTcpClient
{
public:
  int  m_sockfd;        // socket id
  char m_IP[21];        // ����˵�IP
  int  m_Port;          // ������ͨѶ�Ķ˿�
  BOOL m_State;         // ����״̬
  char m_ConnStr[200];  // �����˵����Ӳ�������ʽ��ip1,port1,ip2,port2,...
  BOOL m_bTimeOut;      // Read()�����Ƿ�ʱ��TRUE-δ��ʱ��FALSE-�ѳ�ʱ
  int  m_BufLen;        // ���յ��ı��ĵİ���С

  CTcpClient();

  // �����˷�������
  // in_ConnStr����˲�������ʽ��ip1,port1,ip2,port2,...
  // ע�⣬in_ConnStr֧�ֶ����������ô����Ŀ����Ϊ��֧�ֶ�������
  BOOL ConnectToServer(const char *in_ConnStr);                // ���ӷ�����
  
  // û�г�ʱ���ƵĽ��պ���
  BOOL Read(char *strRecvBuffer);
  // �г�ʱ���ƵĽ��պ���
  BOOL Read(char *strRecvBuffer,const int iTimeOut);

  // �����˷����ַ���������Ϊ�ַ����ĳ���
  BOOL Write(char *strSendBuffer);             

  // �����˷����ַ�������Ҫָ������
  BOOL Write(char *strSendBuffer,const int buflen);
  
  // �Ͽ������˵�����
  void Close();

  ~CTcpClient();
};

// ����TCPͨѶ�ķ�����
class CTcpServer
{
public:
  int m_socklen;                    // struct sockaddr_in�Ĵ�С
  struct sockaddr_in m_clientaddr;  // �ͻ��˵ĵ�ַ��Ϣ
  struct sockaddr_in m_servaddr;    // ����˵ĵ�ַ��Ϣ
  int  m_listenfd;                  // ��������ڼ�����socket
  int  m_connfd;                    // �ͻ��˵�socket
  BOOL m_bTimeOut;                  // Read()�����Ƿ�ʱ��TRUE-δ��ʱ��FALSE-�ѳ�ʱ
  int  m_BufLen;                    // ���յ��ı��ĵİ���С

  CTcpServer();
  BOOL InitServer(const unsigned int port); // ��ʼ��TcpServer
  BOOL Accept();                            // ���ܿͻ��˵�����
  char *GetIP();                            // ��ȡ�ͻ��˵�ip��ַ
  
  // û�г�ʱ���ƵĽ��պ���
  BOOL Read(char *strRecvBuffer);
  // �г�ʱ���ƵĽ��պ���
  BOOL Read(char *strRecvBuffer,const int iTimeOut);

  // �����˷����ַ���������Ϊ�ַ����ĳ���
  BOOL Write(char *strSendBuffer);             

  // �����˷����ַ�������Ҫָ������
  BOOL Write(char *strSendBuffer,const int buflen);
  
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
BOOL TcpRead(const int fd,char *strRecvBuffer,int *buflen,const int iTimeOut=0);

// ������д��socket
// fd socket
// strSendBuffer ��д�����ݴ�ŵĻ�����
// buflen ��д�����ݵĳ��ȣ����Ϊ0������ΪstrSendBuffer��һ���ַ�����
// iTimeOut д�볬ʱ��ʱ��
BOOL TcpWrite(const int fd,const char *strSendBuffer,const int buflen=0,const int iTimeOut=0);

// ��socket�ж�ȡ���ݣ�ר����TcpRead����
BOOL Readn(const int fd,char *vptr,const size_t n);

// ��socket��д�����ݣ�ר����TcpWrite����
BOOL Writen(const int fd,const char *vptr,const size_t n);

// ������TCP/IPͨѶ�ĺ�������
///////////////////////////////////// /////////////////////////////////////


///////////////////////////////////// /////////////////////////////////////
// �������ļ���������ͺ���

// �ļ�������
class CFile
{
public:
  FILE *m_fp;                 // �ļ�ָ��
  char  m_filename[301];      // ������·�����ļ���
  char  m_filenametmp[301];   // �������ļ�����ʱ�ļ���
  BOOL  m_bEnBuffer;          // �Ƿ����û������ı�־��TRUE-���ã�FALSE-�����á�
  int   m_filetype;           // �ļ��򿪵ķ�ʽ:1-ֻ����2-ֻд��3-д�����������

  CFile();

 ~CFile();  // ���������е���CloseOnly����

  void initdata();

  // �����ļ��Ƿ�򿪣�TRUE��ʾ�Ѿ��򿪣�FALSE��ʾû�д�
  BOOL IsOpened();

  // ��һ���ļ����ڶ�
  // filenameΪȫ·�����ļ�����
  // openmodeΪ���ļ��ķ�ʽ��ֻ����"rt"��"rb"��
  // �ô˷����򿪵��ļ���������CloseAndRemove��CloseOnly�ر�
  // �˷�������m_filetype��ֵ��Ϊ1
  BOOL OpenForRead(const char *filename, const char *openmode);

  // �ر��Ѵ򿪵�ֻ���ļ�����ɾ������
  BOOL CloseAndRemove();

  // ��һ���ļ�����д����������ʱ�ļ����棬����Ӧ�õĳ����Ƚ���
  // �����������ʱ�ļ����棬������������׶�ȡ���������ļ�
  // filenameΪȫ·�����ļ�����
  // openmodeΪ���ļ��ķ�ʽ��ͬfopenʹ�÷���һ�£�������"rt"��"rb"
  // bEnBuffer:TRUE-���û�������FALSE-�����û�����
  // �ô˷����򿪵��ļ���ֻ����CloseOnly�ر�
  // �˷�������m_filetype��ֵ��Ϊ2
  BOOL OpenForWrite(const char *filename, const char *openmode, BOOL bEnBuffer=FALSE);

  // ��һ���ļ�����д��������ʱ�ļ�����
  // �˷����Ȱ�����д��"*.tmp"�ļ����ر�ʱ����CloseAndRename��Ϊ��ʽ���ļ���
  // filenameΪȫ·�����ļ�����
  // openmodeΪ���ļ��ķ�ʽ��ͬfopenʹ�÷���һ�£�������"rt"��"rb"
  // bEnBuffer:TRUE-���û�������FALSE-�����û�����
  // �ô˷����򿪵��ļ���������CloseAndRename�رգ������CloseOnly�رգ��Ͳ���������ʽ�ļ�
  // �˷�������m_filetype��ֵ��Ϊ3
  BOOL OpenForRename(const char *filename, const char *openmode, BOOL bEnBuffer=FALSE);

  // �ر��ļ�ָ�룬������ʱ�ļ�����Ϊ��ʽ�ļ������������ʧ�ܣ�������FALSE���ɹ�����TRUE
  BOOL CloseAndRename();

  // �ر��ļ�ָ��
  // ���������OpenForRename���ļ�����ɾ������ʱ�ļ�
  void CloseOnly();

  // ����FGETS������ȡһ��
  BOOL FFGETS(char *strBuffer,const int ReadSize,const char *strEndStr=0);

  // ����fread���ļ��ж�ȡ����
  size_t Fread(void *ptr,size_t size);

  // ����fwrite���ļ���д����
  size_t Fwrite(const void *ptr,size_t size);
  
  // ����fprintf���Ѵ򿪵��ļ�д�����ݡ�
  void Fprintf(const char *fmt, ... );
};

// ����fopen�������ļ�������ļ����а�����Ŀ¼�����ڣ��ʹ���Ŀ¼
FILE *FOPEN(const char *filename,const char *mode);

// �����ļ����޸�ʱ�����ԣ�mtimeָ����ʱ�䣬��ʽ���ޣ�ֻҪ������yyyy,mm,dd,hh24,mi,ss���ɡ�
int UTime(const char *filename,const char *mtime);

// �ж��ı��ļ��Ƿ���strEnd����
// �����ж��ļ���������
BOOL CheckFileEnd(const char *strFileName,const char *strEnd);

// ���ļ��ļ��ж�ȡһ��
// strEndStr��һ�����ݵĽ�����־�����Ϊ�գ����Ի��з�"\n"Ϊ������־��
BOOL FGETS(const FILE *fp,char *strBuffer,const int ReadSize,const char *strEndStr=0);

// ��ȡ�ļ��Ĵ�С�������ֽ���
UINT FileSize(const char *in_FullFileName);

// ��ȡ�ļ���ʱ�䣬��modtime
void FileMTime(const char *in_FullFileName,char *out_ModTime);

// �������ļ���������ͺ���
///////////////////////////////////// /////////////////////////////////////

// �ر�ȫ�����źź��������
void CloseIOAndSignal();


#endif
