#ifndef _public_H
#define _public_H

#include "_cmpublic.h"

// 读取参数配置文件内容，参数文件并不是传统的INI文件，而是采用xml文件格式。
class CIniFile
{
public:
  // 存放参数文件全部的内容，由LoadFile载入到本变量中。
  char m_XMLBuffer[204801];

  CIniFile();

  // 把参数文件的内容载入到m_XMLBuffer变量中。
  BOOL LoadFile(const char *in_FileName);
  
  // 获取参数文件字段的内容。
  BOOL GetValue(const char *in_FieldName,char   *out_Value,int in_len=0);
  BOOL GetValue(const char *in_FieldName,int    *out_Value);
  BOOL GetValue(const char *in_FieldName,long   *out_Value);
  BOOL GetValue(const char *in_FieldName,UINT   *out_Value);
  BOOL GetValue(const char *in_FieldName,double *out_Value);
};

// 读取某目录下的全部的文件
class CDir
{
public:
  char m_DirName[301];         // 目录名
  char m_FileName[301];        // 文件名
  char m_UpperFileName[301];   // 大写的文件名
  char m_FullFileName[301];    // 文件全名，包括路径
  UINT m_FileSize;             // 文件的大小
  char m_ModifyTime[21];       // 文件最后一次被修改的时间
  char m_CreateTime[21];       // 文件生成的时间
  char m_AccessTime[21];       // 文件最后一次被访问的时间
  UINT m_uPOS;                 // 已读取m_vFileName列表的位置
  vector<string> m_vFileName;  //  存放OpenDir方法获取到的文件列表
  BOOL m_bOnlyDir;             // TRUE-只获取目录信息；FALSE-获取目录和文件信息，缺省为FALSE
  vector<string> m_vDirName;   // 存放各级目录的信息
  UINT m_uMaxFileCount;        // 每次扫描目录，读入的文件数，缺省是不限制
  BOOL m_bAndTMPFiles;         // 扫描目录的文件时，是否包括tmp文件，TRUE-包括；FALSE-不包括，缺省是FALSE

  CDir();

  void initdata();

  // 设置日期时间的格式，支持"yyyy-mm-dd hh24:mi:ss"和"yyyymmddhh24miss"两种格式，缺省是前者
  char m_DateFMT[31];
  void SetDateFMT(const char *strDateFMT);

  // 打开目录，并按文件名排序
  BOOL OpenDir(const char *in_dirname,const BOOL bAndChild=FALSE);

  // 打开目录，不排序
  BOOL OpenDirNoSort(const char *in_dirname,const BOOL bAndChild=FALSE);

  BOOL _OpenDir(const char *in_dirname,const BOOL bAndChild=FALSE);

  // 逐个读取目录中的文件
  BOOL ReadDir();

  ~CDir();
};

/* 写日志文件类 */
class CLogFile 
{
public:
  FILE   *m_tracefp;           // 日志文件指针
  char    m_filename[301];     // 日志文件全名
  char    m_openmode[11];      // 日志文件的打开方式
  char    m_stime[21];         // 日志文件写入时的当前时间变量
  char    m_message[20480];    // 被写入的日志内容
  BOOL    m_bBackup;           // 日志文件超出100M，是否自动备份
  BOOL    m_bEnBuffer;         // 写入日志时，是否启用操作系统的缓冲机制
  va_list m_ap;

  CLogFile();

  // filename日志文件名
  // openmode打开文件的方式，操作日志文件的权限,同打开文件函数(fopen)使用方法一致
  // bBackup，TRUE-备份，FALSE-不备份，在多进程的服务程序中，如果多个进行共用一个日志文件，bBackup必须为FALSE
  // bEnBuffer:TRUE-启用缓冲区，FALSE-不启用缓冲区，如果启用缓冲区，那么写进日志文件中的内容不会立即写入文件是
  BOOL Open(const char *in_filename,const char *in_openmode,BOOL bBackup=TRUE,BOOL bEnBuffer=FALSE);

  // 如果日志文件大于100M，就备份它
  BOOL BackupLogFile();

  // 写日志文件,它是个可变参数的方法,同printf函数
  BOOL Write(const char *fmt,...);
  BOOL WriteEx(const char *fmt,...);

  // 关闭日志文件,如果文件太大(超过100M),则把原日志文件备份
  void Close();

  // 设置告警参数
  BOOL m_balarmbz;
  char m_programname[101];
  void SetAlarmOpt(const char *fmt,...);

  // 把m_message写入告警日志文件。
  BOOL WriteAlarmFile();

  ~CLogFile();
};

// 拆分字符串的类,
// 字符串的格式为:内容1+分隔字符串+内容2+分隔字符串+内容3
// 如:num~!~name~!~address,分隔符为"~!~"
class CCmdStr 
{
public:
  vector<string> m_vCmdStr;  // 存放拆分后的字段内容。

  CCmdStr();

  // 拆分字符串到容器中
  void SplitToCmd(const string in_string,const char *in_sep,const BOOL bdeletespace=TRUE);

  UINT CmdCount();

  // 获取字段的值，取每个字段的值inum从0开始
  BOOL GetValue(const int inum,char   *in_return);
  BOOL GetValue(const int inum,char   *in_return,const int in_len);
  BOOL GetValue(const int inum,int    *in_return);
  BOOL GetValue(const int inum,long   *in_return);
  BOOL GetValue(const int inum,UINT   *in_return);
  BOOL GetValue(const int inum,float  *in_return);
  BOOL GetValue(const int inum,double *in_return);

  ~CCmdStr();
};

// 计时器
class CTimer
{
public:
  // 调用Beginning函数时，m_BeginningPOS用于记录这个时间点
  time_t  m_BeginningPOS;

  CTimer();

  // 开始计时
  void Beginning();

  // 计算已逝去的时间，单位：秒。
  long Elapsed();
};


class CProgramActive
{
public:
  CLogFile *m_logfile;
  CTimer    m_Timer;                 // 一个时间计数器

  int    m_PID;                      // 进程的ID
  char   m_ProgramName[51];          // 程序名
  time_t m_MaxTimeOut;               // 每次活动之间允许的最大时间长，以秒为单位
  time_t m_LatestActiveTime;        // 最近一次活动时间，用整数表示
  char   m_FileName[301];
  time_t m_Elapsed;                  // 最近一次活动时间与当前时间的时间差
  time_t m_NowTime;

  CProgramActive();

  void initdata();

  // 设置进程参数
  void SetProgramInfo(const CLogFile *in_logfile,const char *in_ProgramName,const int in_MaxTimeOut);

  // 把最近一次的进程活动信息写入文件
  BOOL WriteToFile(const BOOL bIsFirstTime=FALSE);

  // 从文件中读取进程活动信息
  BOOL ReadFromFile(const char *in_FileName);

  ~CProgramActive();
};


// 操作XMLBuffer的类
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,char   *out_Value,const int in_StrLen=0);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,int    *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned int *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,long   *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned long *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,float  *out_Value);
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,double *out_Value);


/* 删除字符串左边指定的字符 */
void DeleteLChar(char *in_string,const char in_char);
/* 删除左边的空格 */
void LTrim(char *in_string);
/* 删除字符串右边指定的字符 */
void DeleteRChar(char *in_string,const char in_char);
/* 删除右边的空格 */
void RTrim(char *in_string);
/* 删除字符串两边指定的字符 */
void DeleteChar(char *in_string,const char in_char);
/* 删除两边的空格 */
void Trim(char *in_string);
// 删除字符串中间的字符
void DeleteMChar(char *in_string,const char in_char);

// 删除字符串中间的字符串
void DeleteMStr(char *in_string,const char *in_str);
void SDeleteMStr(string &in_string,const string in_str);

// 把字符串中的某字符串用另一个字符串代替，bLoop是否循环执行替换
void UpdateStr(char *in_string,const char *in_str1,const char *in_str2,BOOL bLoop=TRUE);
// 把字符串中的某字符串用另一个字符串代替，bLoop是否循环执行替换
void UpdateStr(string &in_string,const char *in_str1,const char *in_str2,BOOL bLoop=TRUE);

// 把小写转换成大写，如果其中有不是字母的字符就不改变
void ToUpper(char *str);
// 把小写转换成大写，如果其中有不是字母的字符就不改变
void ToUpper(string &str);

// 把大写转换成小写，如果其中有不是字母的字符就不改变
void ToLower(char *str);
void ToLower(string &str);

// 从一个字符串中提取数字，bWithSign==TRUE表示包括符号，bWithDOT==TRUE表示包括圆点
void PickNumber(const char *strSrc,char *strDst,const BOOL bWithSign,const BOOL bWithDOT);

/*
  取操作系统的时间,interval是偏移常量,单位是秒,返回的格式由fmt决定
  fmt目前的取值如下，如果需要，可以增加：
  yyyy-mm-dd hh24:mi:ss，此格式是缺省格式
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

// 把time_t类型的世界时(字符串)转化为数据库标准时 yyyy-mm-dd hh24:mi:ss
void STDTime(char *strtime, char *buff);
void STDTime(char *strtime, long buff);

// 把yyyy-mm-dd hh24:mi:ss转换为以整数表示的UTC时间，函数在处理的过程中将忽略日期串中的格式
long UTCTime(const char *stime);

/*
  in_stime是传入的时间，任意格式，但是一定要包括yyyymmddhh24miss，是否有分隔符没有关系。
  把yyyy-mm-dd hh24:mi:ss偏移interval秒
  传出的格式由fmt决定，fmt目前的取值如下，如果需要，可以增加：
  yyyy-mm-dd hh24:mi:ss（此格式是缺省格式）
  yyyymmddhh24miss
  yyyymmddhh24miss
  yyyy-mm-dd
  yyyymmdd
  hh24:mi:ss
  hh24miss
  hh24:mi
  hh24mi
  返回值：0-成功，-1-失败。
*/
int AddTime(const char *in_stime,char *out_stime,const int interval=0,const char *in_fmt=0);

// 在当月的年月上减少一个月
void SubtractAMonth(const char *strLocalYM,char *strPreYM);

// 关闭全部的信号和输入输出
void CloseIOAndSignal();

// 判断文件是否以strBZ结束
BOOL CheckFileSTS(const char *strFileName,const char *strBZ);

// 读取文件的每行，并把每一行的回车换行符去掉，还可以判断每行的内容是否以strEndStr结束，
// 是否在每行的最后补充strPadStr字符
BOOL FGETS(char *strBuffer,const int ReadSize,const FILE *fp,const char *strEndStr=0,const char *strPadStr=0);

// 判断文件的大小，返回字节数
long FileSize(const char *in_FullFileName);

// 判断文件的时间，即modtime
void FileModTime(const char *in_FullFileName,char *out_ModTime);

// 判断文件名是否和MatchFileName匹配，如果不匹配，返回失败
// in_MatchStr中的字母采用大写，如果需要匹配yyyymmdd-xx.yyhh24mi，时间表达示要采用小写。
BOOL MatchFileName(const string in_FileName,const string in_MatchStr,BOOL bPMatch=TRUE);

// 处理字符串中匹配年月日的情况，并可以处理格式为dd-xx.yy的匹配，xx.yy的单位是天
// 注意格式，xx一定要是2位长度，yy一定要两位长度，不足的补0。
void ProcMatchDTime(char *in_MatchStr);

// 判断内容是否全部是数字
BOOL IsDigit(const char *strBuffer);

// 判断内容是否全部是大写字母
BOOL IsUpper(const char *strBuffer);

// 判断内容是否全部是ASCII字符
BOOL IsASCII(const char *strBuffer);

// 判断内容是否全部是数字或空格
BOOL IsDigitOrSpace(const char *strLine);

// 统计字符串字的个数，一个汉字，或英文或数字都算一个字。
int SumWord(const char *in_Content);

class CSplitSMS
{
public:
  vector<string> m_vContent;

  // 最后一条短信的字数
  UINT m_EndCharCount;

  CSplitSMS();

  void initdata();

  void SplitToCmd(const char *in_Content,const int in_smslen);
};

// 把浮点数的经纬度转换为度分秒，分别存放在strd,strf,strm中
void doubletodfm(const double dd,char *strd,char *strf,char *strm);

// 把字符串方式的经纬度（如111°23'45"或1112345）转换为度，结果存放在double或char中
void dfmtodouble(const char *strdfm,double *dd);
void dfmtochar(const char *strdfm,char *strdd);

// 计算x1,y1到x2,y2之间的距离，x1,y1,x2,y2的单位都是度
float x1y1tox2y2(const float x1,const float x2,const float y1,const float y2);

// 把短信的有效时间和定时时间转换
// valid_time和at_time的格式是yyyy-mm-dd hh24:mi:ss
// strvalidtime和strattime的格式是yymmddhhmisst32+
void ConvertTime(char *valid_time,char *strvalidtime,char *at_time,char *strattime);

#define TCPBUFLEN 8192  // TCP报文的最大长度，受通讯底层类封装的限制，不允许超过9999

// 用于TCP通讯的客户类
class CTcpClient
{
public:
  int  m_sockfd;
  char m_IP[17];
  int  m_Port;
  BOOL m_State;
  char m_ConnStr[200];
  BOOL m_bTimeOut;
  long m_BufLen;       // 接收到的报文的包大小
  CLogFile *m_logfile;

  CTcpClient();
  void SetConnectOpt(char *in_ConnStr);  // 设置服务端的参数，格式为：IP,Port
  BOOL ConnectToServer();                // 连接服务器
  
  BOOL Read(char *strRead); // 没有超时机制的读取函数
  BOOL Read(char *strRead,long iSecond); // 有超时机制的读取函数
  BOOL Write(char *strWrite); // 发送字符串，长度为字符串的长度
  BOOL Write(char *strWrite,long buflen); // 发送串，需要指定长度
  
  void Close();
  ~CTcpClient();
};

// 用于TCP通讯的服务类
class CTcpServer
{
public:
  int m_socklen;              // struct sockaddr_in的大小
  struct sockaddr_in m_servaddr,m_clientaddr;
  int  m_listenfd;            // 用于监听的描述符
  int  m_connfd;              // 用于客户端连接的描述符
  BOOL m_bTimeOut;
  long m_BufLen;             // 接收到的报文的包大小
  CLogFile *m_logfile;

  CTcpServer();
  BOOL InitServer(int port); // 初始化TcpServer
  BOOL Accept();             // 接受客户端的请求
  char *GetIP();             // 获取对方的IP
  
  BOOL Read(char *strRead);               // 没有超时机制
  BOOL Read(char *strRead,long iSecond);  // 有超时机制
  BOOL Write(char *strWrite);             // 发送字符串，长度为字符串的长度
  BOOL Write(char *strWrite,long buflen); // 发送串，需要指定长度
  
  void CloseListen();              // 关闭TCP连接
  void CloseClient();              // 关闭TCP连接

  ~CTcpServer();
};

BOOL TcpRead(int fd,char *strRead,long *buflen,int iSecond=0);

BOOL TcpWrite(int fd,char *strWrite,long buflen=0,int iSecond=0);

BOOL Readn(int fd,char *vptr,size_t n);
BOOL Writen(int fd,const char *vptr,size_t n);

// 文件操作类
class CFile
{
public:
  char  m_fullfilename[301];  // 全路径的文件名
  char  m_filename[301];      // 不包括路径的文件名
  FILE *m_fp;                 // 文件指针
  BOOL  m_bEnBuffer;          // 是否启用缓冲区的标志，TRUE-启用；FALSE-不启用。

  // 以下成员变量只适用于打开文件用于写操作的情况。
  char  m_tmppathname[301];   // 待生成文件存放的临时目录
  char  m_pathname[301];      // 待生成文件存放的正式目录
  char  m_tmpfullfilename[301];   // 待生成文件的文件名，不包括路径。

  int   m_filetype;               // 文件打开的方式，1-文件打开只读；2-文件打开可写；3-文件打开为改名

  CFile();

 ~CFile();  // 析构函数中调用Fclose()函数

  void initdata();

  // 返回文件是否打开，TRUE表示已经打开，FALSE表示没有打开
  BOOL IsOpened();

  // 打开一个文件用于读，fullfilename为全路径的文件名。
  // openmode为打开文件的方式，只能用"r"或"rb"，返回TRUE说明打开成功，FALSE说明打开文件失败
  BOOL OpenForRead(const char *fullfilename, const char *openmode);

  // 调用fread从文件中读取数据
  size_t Fread(void *ptr, size_t size, size_t nitems=1);

  // 读取文件的每行，并把每一行的回车换行符去掉，还可以判断每行的内容是否以strEndStr结束，
  // 是否在每行的最后补充strPadStr字符
  BOOL FFGETS(char *strBuffer,const int ReadSize,const char *strEndStr=NULL,const char *strPadStr=NULL);

  // 关闭已打开的只读文件，并删除它。
  BOOL CloseAndRemove();

  // 打开一个文件用于写，fullfilename为不包括路径的文件名
  // openmode为打开文件的方式，同fopen使用方法一致，不包括"r"和"rb"，返回TRUE说明打开成功，FALSE说明打开文件失败
  // bEnBuffer:TRUE-启用缓冲区，FALSE-不启用缓冲区
  BOOL OpenForWrite(const char *fullfilename, const char *openmode, BOOL bEnBuffer=FALSE);

  // 打开一个文件用于写，tmppathname文件存放的临时目录，pathname文件存放的正式目录，filename为不包括路径的文件名
  // openmode为打开文件的方式，同fopen使用方法一致，不包括"r"和"rb"，返回TRUE说明打开成功，FALSE说明打开文件失败
  // bEnBuffer:TRUE-启用缓冲区，FALSE-不启用缓冲区
  // 这个函数是为了兼容旧的程序
  BOOL OpenForRename(const char *tmppathname,const char *pathname,const char *filename, const char *openmode, BOOL bEnBuffer=FALSE);
  // 以后都用这个新函数
  BOOL OpenForRename(const char *fullfilename, const char *openmode, BOOL bEnBuffer=FALSE);

  // 往已打开的文件写入数据。
  void Fprintf(const char *fmt, ... );

  // 调用fwrite向文件中写数据，不知道怎么搞的，该函数有问题，暂时不启用
  // size_t Fwrite(const void *ptr, size_t size, size_t nitems=1);

  // 关闭文件指针，并把临时文件名改为正式文件名，如果改名失败，将返回FALSE，成功返回TRUE
  BOOL CloseAndRename();

  // 如果m_filetype==1或m_filetype==2，就关闭它。
  // 如果m_filetype==3，关闭文件指针，并删除掉临时文件。
  void Fclose();
};

// 删除文件，如果删除失败，会尝试in_times次
BOOL REMOVE(const char *in_filename,const int in_times=3);

// 把in_srcfilename改名为in_dstfilename，如果改名失败，会尝试in_times次
BOOL RENAME(const char *in_srcfilename,const char *in_dstfilename,const int in_times=3);

// 打开文件，如果文件的目录不存在，就创建该目录，但是，如果打开方式为"r"或"rb"，就不会创建目录。
FILE *FOPEN(const char *filename,const char *mode);

int OPEN(const char *filename, int flags);
int OPEN(const char *filename, int flags, mode_t mode);

// 用某文件或目录的全路径中的目录创建目录，以级该目录下的各级子目录
BOOL MKDIR(const char *filename,BOOL bisfilename=TRUE);

// 把某一个文件复制到另一个文件
BOOL COPY(const char *srcfilename,const char *dstfilename);

// 更改文件的修改时间属性，mtime指定了时间，格式不限，只要包括了yyyy,mm,dd,hh24,mi,ss即可。
int UTime(const char *filename,const char *mtime);

// 把度度度分分秒秒格式的字符串转换为单位为度的字符串。
// strAzimuth表示经度或纬度，经度的长度必须7，纬度的长度必须是6位。
// strNumber存放8位的浮点数，精确到小数点后5位。
BOOL AzimuthToNumber(char *strAzimuth,char *strNumber);

// lon_rd雷达的经度，lat_rd雷达的纬度，h_rd雷达的高度
// 指定格点经度lon_p，纬度lat_p，高度h_p
// angle是雷达站与指定格点连接地心的夹角，斜距range，仰角elev，方位角azim
int get_sea(double lon_rd, double lat_rd, double h_rd ,
            double lon_p , double lat_p , double h_p  ,
            double *angle, double *range, double *elev, double *azim);


// lon_rd雷达的经度，lat_rd雷达的纬度，h_rd雷达的高度（单位：米）
// 目的点的rang斜距，elev仰角，azim方位角
// 目的点的lon_p经度，lat_p纬度，h_p高度（单位：米）
int get_llh(double lon_rd, double lat_rd, double h_rd,
            double rang  , double elev  , double azim,
            double *lon_p, double *lat_p, double *h_p);

// 判断是否需要处理buffer中的时间变量
// 可以处理以下时间变量：{yyyy}（4位的年）、{yyy}（后三位的年）、
// {yy}（后两位的年）、{mm}（月月）、{dd}（日日）、{hh}（时时）、{mi}（分分）、{ss}（秒秒）
void MatchBuffer(char *buffer,int timetvl=0);

// 压缩文件为zip格式
void ZIPFiles(char *strFileName,BOOL bKeep=FALSE);

// 把字符串转换为double
double ATOF(const char *nptr);

// 把二进制的字符串转换为十进制
int bintodec(char *strbin,int length);

// 从日志文件中读取已处理记录的位置
long ReadLogFile(const char *strFileName);

// 把本次已处理的文件的位置写入日志文件
BOOL WriteToLogFile(FILE *fpSend,const char *strFileName);

// 把本次已处理完成的的日志文件删除
BOOL RemoveLogFile(char *strFileName);

// 从文件中加载一条记录，如果文件没有新记录，strBuffer将为空
BOOL LoadOneLine(const char *strFileName,char *strBuffer);

// 把风速转换为等级
// in_wf-风速，单位：0.1m/s，out_wdj-风速等级，2-小于三级；3-三级；4-四级...12-十二级；13-大于12级
BOOL wftowdj(char *in_wf,char *out_wdj);

// 转码,UTF-8 转 GB18030
// dest:目的格式, src:原格式, input:输入字符串, output:输出字符串
int conv_charset(const char *dest, const char *src, char *input, size_t inlen, char *output, size_t outlen);

// 调用此函数，需要设置服务器的hostname及在hosts文件中添加
BOOL getLocalIP(char *localip);

#endif
