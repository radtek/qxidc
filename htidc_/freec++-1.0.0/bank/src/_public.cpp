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

#include "_public.h"

// ����XMLBuffer�ĺ���
// in_XMLBuffer��XML��ʽ���ַ���
// in_FieldName���ֶεı�ǩ��
// out_Value����ȡ���ݴ�ŵı�����ָ��
BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,char *out_Value,const int in_Len)
{
  strcpy(out_Value,"");

  char *start=NULL,*end=NULL;
  char m_SFieldName[51],m_EFieldName[51];

  int m_NameLen = strlen(in_FieldName);
  memset(m_SFieldName,0,sizeof(m_SFieldName));
  memset(m_EFieldName,0,sizeof(m_EFieldName));

  snprintf(m_SFieldName,50,"<%s>",in_FieldName);
  snprintf(m_EFieldName,50,"</%s>",in_FieldName);

  start=0; end=0;

  start = (char *)strstr(in_XMLBuffer,m_SFieldName);

  if (start != 0)
  {
    end   = (char *)strstr(start,m_EFieldName);
  }

  if ((start==0) || (end == 0))
  {
    return FALSE;
  }

  int   m_ValueLen = end - start - m_NameLen - 2 + 1 ;

  if ( ((m_ValueLen-1) <= in_Len) || (in_Len == 0) )
  {
    strncpy(out_Value,start+m_NameLen+2,m_ValueLen-1);
  }
  else
  {
    strncpy(out_Value,start+m_NameLen+2,in_Len);
  }

  DeleteLRChar(out_Value,' ');

  return TRUE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,int *out_Value)
{
  (*out_Value) = 0;

  char strTemp[51]; 

  memset(strTemp,0,sizeof(strTemp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,strTemp,50) == TRUE)
  {
    (*out_Value) = atoi(strTemp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned int *out_Value)
{
  (*out_Value) = 0;

  char strTemp[51]; 

  memset(strTemp,0,sizeof(strTemp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,strTemp,50) == TRUE)
  {
    (*out_Value) = (unsigned int)atoi(strTemp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,long *out_Value)
{
  (*out_Value) = 0;

  char strTemp[51]; 

  memset(strTemp,0,sizeof(strTemp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,strTemp,50) == TRUE)
  {
    (*out_Value) = atol(strTemp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned long *out_Value)
{
  (*out_Value) = 0;

  char strTemp[51]; 

  memset(strTemp,0,sizeof(strTemp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,strTemp,50) == TRUE)
  {
    (*out_Value) = (unsigned long)atol(strTemp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,double *out_Value)
{
  (*out_Value) = 0;

  char strTemp[51]; 

  memset(strTemp,0,sizeof(strTemp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,strTemp,50) == TRUE)
  {
    (*out_Value) = atof(strTemp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,float *out_Value)
{
  (*out_Value) = 0;

  char strTemp[51]; 

  memset(strTemp,0,sizeof(strTemp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,strTemp,50) == TRUE)
  {
    (*out_Value) = atof(strTemp); return TRUE;
  }

  return FALSE;
}

// ��ȡXML�ļ��е�����
CXmlFile::CXmlFile()
{
  memset(m_XMLBuffer,0,sizeof(m_XMLBuffer));
}

BOOL CXmlFile::LoadFile(const char *in_FileName)
{
  memset(m_XMLBuffer,0,sizeof(m_XMLBuffer));

  char strLine[1024]; 

  FILE *fp=0;

  if ( (fp=fopen(in_FileName,"rt")) == NULL) return FALSE;

  memset(strLine,0,sizeof(strLine));

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    if (FGETS(fp,strLine,1000) == FALSE) break;

    // ��ֹm_XMLBuffer���
    if ( (strlen(m_XMLBuffer)+strlen(strLine)) >= (UINT)20000 ) break;
  
    strcat(m_XMLBuffer,strLine);
  }

  fclose(fp);

  fp=0;

  if (strlen(m_XMLBuffer) < 10) return FALSE;

  return TRUE;
}

BOOL CXmlFile::GetValue(const char *in_FieldName,char *out_Value,int in_len)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value,in_len);
}

BOOL CXmlFile::GetValue(const char *in_FieldName,int *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

BOOL CXmlFile::GetValue(const char *in_FieldName,long *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

BOOL CXmlFile::GetValue(const char *in_FieldName,UINT *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

BOOL CXmlFile::GetValue(const char *in_FieldName,double *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

CDir::CDir()
{
  m_uPOS=0;

  memset(m_DateFMT,0,sizeof(m_DateFMT));
  strcpy(m_DateFMT,"yyyy-mm-dd hh24:mi:ss");

  m_vFileName.clear();

  initdata();
}

void CDir::initdata()
{
  memset(m_DirName,0,sizeof(m_DirName));
  memset(m_FileName,0,sizeof(m_FileName));
  memset(m_FullFileName,0,sizeof(m_FullFileName));
  m_FileSize=0;
  memset(m_CreateTime,0,sizeof(m_CreateTime));
  memset(m_ModifyTime,0,sizeof(m_ModifyTime));
  memset(m_AccessTime,0,sizeof(m_AccessTime));
}

// ��������ʱ��ĸ�ʽ��֧��"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"���ָ�ʽ��ȱʡ��ǰ��
void CDir::SetDateFMT(const char *in_DateFMT)
{
  memset(m_DateFMT,0,sizeof(m_DateFMT));
  strcpy(m_DateFMT,in_DateFMT);
}

// ��Ŀ¼����ȡ�ļ�����Ϣ�������m_vFileName������
// in_dirname�����򿪵�Ŀ¼��
// in_MatchStr������ȡ�ļ�����ƥ�����
// in_MaxCount����ȡ�ļ����������
// bAndChild���Ƿ�򿪸�����Ŀ¼
// bSort���Ƿ�Խ��ʱ������
BOOL CDir::OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const BOOL bAndChild,BOOL bSort)
{
  m_uPOS=0;
  m_vFileName.clear();

  // ���Ŀ¼�����ڣ��ʹ�����Ŀ¼
  if (MKDIR(in_DirName,FALSE) == FALSE) return FALSE;

  BOOL bRet=_OpenDir(in_DirName,in_MatchStr,in_MaxCount,bAndChild);

  if (bSort==TRUE)
  {
    sort(m_vFileName.begin(), m_vFileName.end());
  }

  return bRet;
}

// ��Ŀ¼�����Ǹ��ݹ麯��
BOOL CDir::_OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const BOOL bAndChild)
{
  DIR *dir;

  if ( (dir=opendir(in_DirName)) == NULL ) return FALSE;

  char strTempFileName[1024];

  struct dirent *st_fileinfo;
  struct stat st_filestat;

  while ((st_fileinfo=readdir(dir)) != NULL)
  {
    // ��"."��ͷ���ļ�������
    if (st_fileinfo->d_name[0]=='.') continue;
        
    memset(strTempFileName,0,sizeof(strTempFileName));

    snprintf(strTempFileName,300,"%s//%s",in_DirName,st_fileinfo->d_name);

    UpdateStr(strTempFileName,"//","/");

    stat(strTempFileName,&st_filestat);

    // �ж��Ƿ���Ŀ¼
    if (S_ISDIR(st_filestat.st_mode))
    {
      if (bAndChild == TRUE)
      {
        if (_OpenDir(strTempFileName,in_MatchStr,in_MaxCount,bAndChild) == FALSE) 
        {
          closedir(dir); return FALSE;
        }
      }
    }
    else
    {
      if (MatchFileName(st_fileinfo->d_name,in_MatchStr) == FALSE) continue;

      m_vFileName.push_back(strTempFileName);

      if ( m_vFileName.size()>in_MaxCount ) break;
    }
  }

  closedir(dir);

  return TRUE;
}

/*
st_gid 

Numeric identifier of group that owns file (UNIX-specific) This field will always be zero on NT systems. A redirected file is classified as an NT file.

st_atime

Time of last access of file.

st_ctime

Time of creation of file.

st_dev

Drive number of the disk containing the file (same as st_rdev).

st_ino

Number of the information node (the inode) for the file (UNIX-specific). On UNIX file systems, the inode describes the file date and time stamps, permissions, and content. When files are hard-linked to one another, they share the same inode. The inode, and therefore st_ino, has no meaning in the FAT, HPFS, or NTFS file systems.

st_mode

Bit mask for file-mode information. The _S_IFDIR bit is set if path specifies a directory; the _S_IFREG bit is set if path specifies an ordinary file or a device. User read/write bits are set according to the file��s permission mode; user execute bits are set according to the filename extension.

st_mtime

Time of last modification of file.

st_nlink

Always 1 on non-NTFS file systems.

st_rdev

Drive number of the disk containing the file (same as st_dev).

st_size

Size of the file in bytes; a 64-bit integer for _stati64 and _wstati64

st_uid

Numeric identifier of user who owns file (UNIX-specific). This field will always be zero on NT systems. A redirected file is classified as an NT file.
*/

BOOL CDir::ReadDir()
{
  initdata();

  // ����Ѷ��꣬�������
  if (m_uPOS >= m_vFileName.size()) 
  {
    m_uPOS=0; m_vFileName.clear(); return FALSE;
  }

  int pos=0;

  pos=m_vFileName[m_uPOS].find_last_of("/");

  // Ŀ¼��
  memset(m_DirName,0,sizeof(m_DirName));
  strcpy(m_DirName,m_vFileName[m_uPOS].substr(0,pos).c_str());

  // �ļ���
  memset(m_FileName,0,sizeof(m_FileName));
  strcpy(m_FileName,m_vFileName[m_uPOS].substr(pos+1,m_vFileName[m_uPOS].size()-pos-1).c_str());

  // �ļ�ȫ��������·��
  snprintf(m_FullFileName,300,"%s",m_vFileName[m_uPOS].c_str());

  struct stat st_filestat;

  stat(m_FullFileName,&st_filestat);

  m_FileSize=st_filestat.st_size;

  struct tm nowtimer;

  if (strcmp(m_DateFMT,"yyyy-mm-dd hh24:mi:ss") == 0)
  {
    nowtimer = *localtime(&st_filestat.st_mtime); nowtimer.tm_mon++;
    snprintf(m_ModifyTime,20,"%04u-%02u-%02u %02u:%02u:%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    nowtimer = *localtime(&st_filestat.st_ctime); nowtimer.tm_mon++;
    snprintf(m_CreateTime,20,"%04u-%02u-%02u %02u:%02u:%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    nowtimer = *localtime(&st_filestat.st_atime); nowtimer.tm_mon++;
    snprintf(m_AccessTime,20,"%04u-%02u-%02u %02u:%02u:%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);
  }

  if (strcmp(m_DateFMT,"yyyymmddhh24miss") == 0)
  {
    nowtimer = *localtime(&st_filestat.st_mtime); nowtimer.tm_mon++;
    snprintf(m_ModifyTime,20,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    nowtimer = *localtime(&st_filestat.st_ctime); nowtimer.tm_mon++;
    snprintf(m_CreateTime,20,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    nowtimer = *localtime(&st_filestat.st_atime); nowtimer.tm_mon++;
    snprintf(m_AccessTime,20,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);
  }

  m_uPOS++;

  return TRUE;
}

CDir::~CDir()
{
  m_vFileName.clear();

  // m_vDirName.clear();
}

/* ɾ���ַ������ָ�����ַ� */
void DeleteLChar(char *in_string,const char in_char)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;

  char strTemp[strlen(in_string)+1];

  int iTemp=0;

  memset(strTemp,0,sizeof(strTemp));
  strcpy(strTemp,in_string);

  while ( strTemp[iTemp] == in_char )  iTemp++; 
  
  memset(in_string,0,strlen(in_string)+1);

  strcpy(in_string,strTemp+iTemp);

  return;
}

/* ɾ���ַ����ұ�ָ�����ַ� */
void DeleteRChar(char *in_string,const char in_char)
{
  if (in_string == 0) return;

  int istrlen = strlen(in_string);

  while (istrlen>0)
  {
    if (in_string[istrlen-1] != in_char) break;

    in_string[istrlen-1]=0;

    istrlen--;
  }
}

/* ɾ���ַ�������ָ�����ַ� */
void DeleteLRChar(char *in_string,const char in_char)
{
  DeleteLChar(in_string,in_char);
  DeleteRChar(in_string,in_char);
}

// ���ַ�������߲��ַ���ָ������
void LPad(char *in_string,const char in_char,unsigned int in_len)
{
  if (strlen(in_string)>=in_len) return;

  char str[in_len+1];

  memset(str,0,sizeof(str));

  for (unsigned int ii=0;ii<in_len-strlen(in_string);ii++)
  {
    str[ii]=in_char;
  }

  strcpy(str+in_len-strlen(in_string),in_string);

  strcpy(in_string,str);
}

// ���ַ������ұ߲��ַ���ָ������
void RPad(char *in_string,const char in_char,unsigned int in_len)
{
  if (strlen(in_string)>=in_len) return;

  for (unsigned int ii=strlen(in_string);ii<in_len;ii++)
  {
    in_string[ii]=in_char;
  }

  in_string[in_len]=0;
}


CLogFile::CLogFile() 
{ 
  m_tracefp = 0;
  memset(m_filename,0,sizeof(m_filename));
  memset(m_openmode,0,sizeof(m_openmode));
  m_bBackup=TRUE;
  m_bEnBuffer=FALSE;
}

CLogFile::~CLogFile() 
{ 
  Close();
}

void CLogFile::Close() 
{ 
  if (m_tracefp != 0) 
  {
    fclose(m_tracefp); m_tracefp=0;
  }
}

/*
  �й��ļ���λ�ĺ������÷�:
  rewind(FILE *fp),ʹλ��ָ���ط��ļ��Ŀ�ͷ,�����޷���ֵ.
  fseek(FILE *fp,λ����,��ʼ��),��ʼ���ȡֵ��:SEEK_SET(0)-�ļ���ʼ;
  SEEK_CUR(1)-��ǰλ��;SEEK_END(2)-�ļ�δβ,λ����ָ����ʼ��Ϊ����,
  �ƶ����ֽ���(��ֵ������,��ֵ��ǰ��),Ϊlong��������,ANSI C��׼�涨
  �����ֵ�δβ������ĸL,�ͱ�ʾ��long��.
  ftell(FILE *fp),ȡ���ļ��еĵ�ǰλ��,����-1L��ʾ����.
  ferror(FILE *fp),�ڵ��ø��������������(��putc,getc,fread,fwrite)ʱ,
  ��������˴���,���˺�������ֵ������Ӧ��,������ferror�������,�����
  ��0��ʾδ����,��0��ʾ����,���ڵ���clearerr(FILE *fp)��,ferror����0.

*/
// filename��־�ļ���
// openmode���ļ��ķ�ʽ��������־�ļ���Ȩ��,ͬ���ļ�����(FOPEN)ʹ�÷���һ��
// bBackup��TRUE-���ݣ�FALSE-�����ݣ��ڶ���̵ķ�������У����������й���һ����־�ļ���bBackup����ΪFALSE
// bEnBuffer:TRUE-���û�������FALSE-�����û�������������û���������ôд����־�ļ��е����ݲ�������д���ļ���
BOOL CLogFile::Open(const char *in_filename,const char *in_openmode,BOOL bBackup,BOOL bEnBuffer)
{
  if (m_tracefp != 0) { fclose(m_tracefp); m_tracefp=0; }

  m_bEnBuffer=bEnBuffer;

  memset(m_filename,0,sizeof(m_filename));
  strcpy(m_filename,in_filename);

  memset(m_openmode,0,sizeof(m_openmode));
  strcpy(m_openmode,in_openmode);

  if ((m_tracefp=FOPEN(m_filename,m_openmode)) == NULL) return FALSE;

  m_bBackup=bBackup;

  return TRUE;
}

// �����־�ļ�����100M���ͱ�����
BOOL CLogFile::BackupLogFile()
{
  // ������
  if (m_bBackup == FALSE) return TRUE;

  if (m_tracefp == 0) return TRUE;

  fseek(m_tracefp,0L,2); 

  if (ftell(m_tracefp) > 100*1024*1024) 
  {
    fclose(m_tracefp); m_tracefp=0;

    char strLocalTime[21];
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyymmddhh24miss");

    char bak_filename[301]; 
    memset(bak_filename,0,sizeof(bak_filename));
    snprintf(bak_filename,300,"%s.%s",m_filename,strLocalTime);
    rename(m_filename,bak_filename);

    if ((m_tracefp=FOPEN(m_filename,m_openmode)) == NULL) return FALSE;
  }

  return TRUE;
}

BOOL CLogFile::Write(const char *fmt,...)
{
  if (BackupLogFile() == FALSE) return FALSE;

  memset(m_stime,0,sizeof(m_stime));
  memset(m_message,0,sizeof(m_message));

  LocalTime(m_stime);

  va_start(m_ap,fmt);
  vsnprintf(m_message,10000,fmt,m_ap);
  va_end(m_ap);

  if (m_tracefp == 0) 
  {
    fprintf(stdout,"%s %s",m_stime,m_message);
    if (m_bEnBuffer==FALSE) fflush(stdout);
  }
  else
  {
    fprintf(m_tracefp,"%s %s",m_stime,m_message);
    if (m_bEnBuffer==FALSE) fflush(m_tracefp);
  }

  return TRUE;  
}

BOOL CLogFile::WriteEx(const char *fmt,...)
{
  memset(m_stime,0,sizeof(m_stime));
  memset(m_message,0,sizeof(m_message));

  LocalTime(m_stime);

  va_start(m_ap,fmt);
  vsnprintf(m_message,10000,fmt,m_ap);
  va_end(m_ap);

  if (m_tracefp == 0) 
  {
    fprintf(stdout,"%s",m_message);
    if (m_bEnBuffer==FALSE) fflush(stdout);
  }
  else
  {
    fprintf(m_tracefp,"%s",m_message);
    if (m_bEnBuffer==FALSE) fflush(m_tracefp);
  }

  return TRUE;  
}

CCmdStr::CCmdStr()
{
  m_vCmdStr.clear();
}

void CCmdStr::SplitToCmd(const string in_string,const char *in_sep,const BOOL bdeletespace)
{
  // ������еľ�����
  m_vCmdStr.clear();

  int iPOS=0;
  string srcstr,substr;

  srcstr=in_string;

  char str[2048];

  while ( (iPOS=srcstr.find(in_sep)) >= 0)
  {
    substr=srcstr.substr(0,iPOS);

    if (bdeletespace == TRUE)
    {
      memset(str,0,sizeof(str));

      strncpy(str,substr.c_str(),2000);

      DeleteLRChar(str,' ');

      substr=str;
    }

    m_vCmdStr.push_back(substr);

    iPOS=iPOS+strlen(in_sep);

    srcstr=srcstr.substr(iPOS,srcstr.size()-iPOS);
  }

  substr=srcstr;

  if (bdeletespace == TRUE)
  {
    memset(str,0,sizeof(str));

    strncpy(str,substr.c_str(),2000);

    DeleteLRChar(str,' ');

    substr=str;
  }

  m_vCmdStr.push_back(substr);

  return;    
}

UINT CCmdStr::CmdCount()
{
  return m_vCmdStr.size();
}

BOOL CCmdStr::GetValue(const int inum,char *in_return)
{
  strcpy(in_return,"");

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  strcpy(in_return,m_vCmdStr[inum].c_str());

  return TRUE;
}

BOOL CCmdStr::GetValue(const int inum,char *in_return,const int in_len)
{
  strcpy(in_return,"");

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  if (m_vCmdStr[inum].length() > (unsigned int)in_len)
  {
    strncpy(in_return,m_vCmdStr[inum].c_str(),in_len);
  }
  else
  {
    strcpy(in_return,m_vCmdStr[inum].c_str());
  }

  return TRUE;
}

BOOL CCmdStr::GetValue(const int inum,int *in_return)
{
  (*in_return) = 0;

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  (*in_return) = atoi(m_vCmdStr[inum].c_str()); 

  return TRUE;
}

BOOL CCmdStr::GetValue(const int inum,long *in_return)
{
  (*in_return) = 0;

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  (*in_return) = atol(m_vCmdStr[inum].c_str()); 

  return TRUE;
}

BOOL CCmdStr::GetValue(const int inum,UINT *in_return)
{
  (*in_return) = 0;

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  (*in_return) = (UINT) atol(m_vCmdStr[inum].c_str()); 

  return TRUE;
}


BOOL CCmdStr::GetValue(const int inum,float *in_return)
{
  (*in_return) = 0;

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  (*in_return) = (float)atof(m_vCmdStr[inum].c_str()); 

  return TRUE;
}

BOOL CCmdStr::GetValue(const int inum,double *in_return)
{
  (*in_return) = 0;

  if ((UINT)inum >= m_vCmdStr.size()) return FALSE;

  (*in_return) = (double)atof(m_vCmdStr[inum].c_str()); 

  return TRUE;
}

CCmdStr::~CCmdStr()
{
  m_vCmdStr.clear();
}



void ToUpper(char *str)
{
  if (str == 0) return;

  if (strlen(str) == 0) return;

  UINT istrlen=strlen(str);

  for (UINT ii=0;ii<istrlen;ii++) 
  {
    if ( (str[ii] >= 97) && (str[ii] <= 122) ) str[ii]=str[ii] - 32;
  }
}

void ToUpper(string &str)
{
  if (str.empty()) return;

  char strtemp[str.size()+1];

  memset(strtemp,0,sizeof(strtemp));
  strcpy(strtemp,str.c_str());

  ToUpper(strtemp);

  str=strtemp;

  return;
}

void ToLower(char *str)
{
  if (str == 0) return;

  if (strlen(str) == 0) return;

  UINT istrlen=strlen(str);

  for (UINT ii=0;ii<istrlen;ii++) 
  {
    if ( (str[ii] >= 65) && (str[ii] <= 90) ) str[ii]=str[ii] + 32;
  }
}

void ToLower(string &str)
{
  if (str.empty()) return;

  char strtemp[str.size()+1];

  memset(strtemp,0,sizeof(strtemp));
  strcpy(strtemp,str.c_str());

  ToLower(strtemp);

  str=strtemp;

  return;
}

// ɾ���ַ����м���ַ���
void DeleteMStr(char *in_string,const char *in_str)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;
  
  char strTemp[strlen(in_string)+1];  
  char *strPos=0;

  while (TRUE)
  {
    strPos=strstr(in_string,in_str);
    if (strPos != 0)
    {
      memset(strTemp,0,sizeof(strTemp));
      strncpy(strTemp,in_string,strPos-in_string);
      strcat(strTemp,strPos+strlen(in_str));
      strcpy(in_string,strTemp);
    }
    else
    {
      break;
    }
  }
}

// ���ַ����е�ĳ�ַ�������һ���ַ�������
void UpdateStr(char *in_string,const char *in_str1,const char *in_str2,BOOL bLoop)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;

  char strTemp[2048];  

  char *strStart=in_string;

  char *strPos=0;

  while (TRUE)
  {
    if (strlen(in_string) >2000) break;

    if (bLoop == TRUE)
    {
      strPos=strstr(in_string,in_str1);
    }
    else
    {
      strPos=strstr(strStart,in_str1);
    }

    if (strPos == 0) break;

    memset(strTemp,0,sizeof(strTemp));
    strncpy(strTemp,in_string,strPos-in_string);
    strcat(strTemp,in_str2);
    strcat(strTemp,strPos+strlen(in_str1));
    strcpy(in_string,strTemp);

    strStart=strPos+strlen(in_str2);
  }
}

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
void LocalTime(char *out_stime,const char *in_fmt,const int in_interval)
{
  time_t  timer;
  struct tm nowtimer;

  time( &timer ); timer=timer+in_interval;
  nowtimer = *localtime ( &timer ); nowtimer.tm_mon++;

  if (in_fmt==0)
  {
    snprintf(out_stime,20,"%04u-%02u-%02u %02u:%02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24:mi:ss") == 0)
  {
    snprintf(out_stime,20,"%04u-%02u-%02u %02u:%02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24:mi") == 0)
  {
    snprintf(out_stime,17,"%04u-%02u-%02u %02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24") == 0)
  {
    snprintf(out_stime,14,"%04u-%02u-%02u %02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd") == 0)
  {
    snprintf(out_stime,11,"%04u-%02u-%02u",nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday); return;
  }

  if (strcmp(in_fmt,"yyyy-mm") == 0)
  {
    snprintf(out_stime,8,"%04u-%02u",nowtimer.tm_year+1900,nowtimer.tm_mon); return;
  }

  if (strcmp(in_fmt,"hh24:mi:ss") == 0)
  {
    snprintf(out_stime,9,"%02u:%02u:%02u",nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec); return;
  }

  if (strcmp(in_fmt,"hh24:mi") == 0)
  {
    snprintf(out_stime,5,"%02u:%02u",nowtimer.tm_hour,nowtimer.tm_min); return;
  }

  if (strcmp(in_fmt,"yyyymmddhh24miss") == 0)
  {
    snprintf(out_stime,15,"%04u%02u%02u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec);
    return;
  }

  if (strcmp(in_fmt,"yyyymmddhh24mi") == 0)
  {
    snprintf(out_stime,13,"%04u%02u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min);
    return;
  }

  if (strcmp(in_fmt,"yyyymmddhh24") == 0)
  {
    snprintf(out_stime,11,"%04u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour);
    return;
  }

  if (strcmp(in_fmt,"yyyymmdd") == 0)
  {
    snprintf(out_stime,9,"%04u%02u%02u",nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday); return;
  }

  if (strcmp(in_fmt,"hh24miss") == 0)
  {
    snprintf(out_stime,7,"%02u%02u%02u",nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec); return;
  }

  if (strcmp(in_fmt,"hh24mi") == 0)
  {
    snprintf(out_stime,5,"%02u%02u",nowtimer.tm_hour,nowtimer.tm_min); return;
  }

  if (strcmp(in_fmt,"hh24") == 0)
  {
    snprintf(out_stime,3,"%02u",nowtimer.tm_hour); return;
  }

  if (strcmp(in_fmt,"mi") == 0)
  {
    snprintf(out_stime,3,"%02u",nowtimer.tm_min); return;
  }
}


// ���ַ�����ʽ��ʱ��ת��Ϊtime_t
// stimeΪ�����ʱ�䣬��ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss
time_t UTCTime(const char *stime)
{
  char strtime[21],yyyy[5],mm[3],dd[3],hh[3],mi[3],ss[3];
  memset(strtime,0,sizeof(strtime));
  memset(yyyy,0,sizeof(yyyy));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(hh,0,sizeof(hh));
  memset(mi,0,sizeof(mi));
  memset(ss,0,sizeof(ss));

  PickNumber(stime,strtime,FALSE,FALSE);

  if (strlen(strtime) != 14) return -1;

  strncpy(yyyy,strtime,4);
  strncpy(mm,strtime+4,2);
  strncpy(dd,strtime+6,2);
  strncpy(hh,strtime+8,2);
  strncpy(mi,strtime+10,2);
  strncpy(ss,strtime+12,2);

  struct tm time_str;

  time_str.tm_year = atoi(yyyy) - 1900;
  time_str.tm_mon = atoi(mm) - 1;
  time_str.tm_mday = atoi(dd);
  time_str.tm_hour = atoi(hh);
  time_str.tm_min = atoi(mi);
  time_str.tm_sec = atoi(ss);
  time_str.tm_isdst = 0;

  return mktime(&time_str);
}

CTimer::CTimer()
{
  memset(&m_start,0,sizeof(struct timeval));
  memset(&m_end,0,sizeof(struct timeval));

  // ��ʼ��ʱ
  Start();
}

// ��ʼ��ʱ
void CTimer::Start()
{
  gettimeofday( &m_start, NULL );
}

// ��������ȥ��ʱ�䣬��λ���룬С���������΢��
double CTimer::Elapsed()
{

  gettimeofday( &m_end, NULL );

  double dstart,dend;

  dstart=dend=0;

  char strtemp[51];
  memset(strtemp,0,sizeof(strtemp));
  snprintf(strtemp,30,"%ld.%ld",m_start.tv_sec,m_start.tv_usec);
  dstart=atof(strtemp);

  memset(strtemp,0,sizeof(strtemp));
  snprintf(strtemp,30,"%ld.%ld",m_end.tv_sec,m_end.tv_usec);
  dend=atof(strtemp);

  // ���¿�ʼ��ʱ
  Start();

  return dend-dstart;
}

// �ж��ı��ļ��Ƿ���strEnd����
BOOL CheckFileEnd(const char *strFileName,const char *strEnd)
{
  FILE *fp=0;

  if ( (fp=fopen(strFileName,"rt")) == NULL ) return FALSE;

  char strBuf[301];
  memset(strBuf,0,sizeof(strBuf));

  fseek(fp,-30L,2);
  fread(strBuf,1,30,fp);
  fclose(fp);

  fp=0;

  if ( strstr(strBuf,strEnd) == NULL) return FALSE;

  return TRUE;
}

// ���ļ��ļ��ж�ȡһ��
// strEndStr��һ�����ݵĽ�����־�����Ϊ�գ����Ի��з�"\n"Ϊ������־��
BOOL FGETS(const FILE *fp,char *strBuffer,const int ReadSize,const char *strEndStr)
{
  char strLine[ReadSize+1];

  memset(strLine,0,sizeof(strLine));

  while (TRUE)
  {
    memset(strLine,0,ReadSize+1);

    if (fgets(strLine,ReadSize,(FILE *)fp) == 0) break;

    for (int ii=0;ii<3;ii++)
    {
      DeleteRChar(strLine,'\n'); 
      DeleteRChar(strLine,'\r'); 
    }

    // ��ֹstrBuffer���
    if ( (strlen(strBuffer)+strlen(strLine)) >= (UINT)ReadSize ) break;

    if (strEndStr != 0) 
    {
      if (strlen(strBuffer) > 0) strcat(strBuffer,"\n");
    }

    strcat(strBuffer,strLine);

    if (strEndStr == 0) return TRUE; 

    if (strncmp(strLine+strlen(strLine)-strlen(strEndStr),strEndStr,strlen(strEndStr)) == 0) 
    {
      strBuffer[strlen(strBuffer)-strlen(strEndStr)]=0; return TRUE;
    }
  }

  return FALSE;
}

// �ж��ַ����еĸ��ź�Բ���Ƿ�Ϸ�
BOOL JudgeSignDOT(const char *strSrc,const char *strBZ)
{
  char *pos=0;
  pos=(char *)strstr(strSrc,strBZ);

  // ���û�а������������ַ������ͷ���TRUE
  if (pos == 0) return TRUE;

  // ���strlen(pos)==1����ʾ�����ֻ�з��ţ�û�������ַ�������FALSE
  if (strlen(pos)==1) return FALSE;

  // ������������ַ�����+�ţ���һ��Ҫ�ǵ�һ���ַ�
  if ( (strcmp(strBZ,"+") == 0) && (strncmp(strSrc,"+",1) != 0) ) return FALSE;

  // ������������ַ�����-�ţ���һ��Ҫ�ǵ�һ���ַ�
  if ( (strcmp(strBZ,"-") == 0) && (strncmp(strSrc,"-",1) != 0) ) return FALSE;

  // �������������������ַ������ͷ���FALSE
  if (strstr(pos+1,strBZ) > 0) return FALSE;

  return TRUE;
}

// ��һ���ַ�������ȡ���֣�bWithSign==TRUE��ʾ�������ţ�bWithDOT==TRUE��ʾ����Բ��
void PickNumber(const char *strSrc,char *strDst,const BOOL bWithSign,const BOOL bWithDOT)
{
  char strtemp[1024];
  memset(strtemp,0,sizeof(strtemp));
  strncpy(strtemp,strSrc,1000);
  DeleteLRChar(strtemp,' ');

  // Ϊ�˷�ֹstrSrc��strDstΪͬһ���������������strDst���ܳ�ʼ��

  // �ж��ַ����еĸ����Ƿ�Ϸ�
  if ( (bWithSign==TRUE) && (JudgeSignDOT(strtemp,"-") == FALSE) )
  {
    strcpy(strDst,""); return;
  }

  // �ж��ַ����е������Ƿ�Ϸ�
  if ( (bWithSign==TRUE) && (JudgeSignDOT(strtemp,"+") == FALSE) )
  {
    strcpy(strDst,""); return;
  }

  // �ж��ַ����е�Բ���Ƿ�Ϸ�
  if ( (bWithDOT==TRUE) && (JudgeSignDOT(strtemp,".") == FALSE) )
  {
    strcpy(strDst,""); return;
  }

  UINT iPosSrc,iPosDst,iLen;
  iPosSrc=iPosDst=iLen=0;

  iLen=strlen(strtemp);

  for (iPosSrc=0;iPosSrc<iLen;iPosSrc++)
  {
    if ( (bWithSign==TRUE) && (strtemp[iPosSrc] == '+') )
    {
      strDst[iPosDst++]=strtemp[iPosSrc]; continue;
    }

    if ( (bWithSign==TRUE) && (strtemp[iPosSrc] == '-') )
    {
      strDst[iPosDst++]=strtemp[iPosSrc]; continue;
    }

    if ( (bWithDOT==TRUE) && (strtemp[iPosSrc] == '.') )
    {
      strDst[iPosDst++]=strtemp[iPosSrc]; continue;
    }
      
    if (isdigit(strtemp[iPosSrc])) strDst[iPosDst++]=strtemp[iPosSrc];
  }

  strDst[iPosDst]=0;

  return;
}

// ��ȡ�ļ��Ĵ�С�������ֽ���
UINT FileSize(const char *in_FullFileName)
{
  struct stat st_filestat;

  if (stat(in_FullFileName,&st_filestat) < 0) return -1;

  return st_filestat.st_size;
}

// ��ȡ�ļ���ʱ�䣬��modtime
void FileMTime(const char *in_FullFileName,char *out_ModTime)
{
  struct tm nowtimer;
  struct stat st_filestat;

  stat(in_FullFileName,&st_filestat);

  nowtimer = *localtime(&st_filestat.st_mtime); 
  nowtimer.tm_mon++;

  snprintf(out_ModTime,15,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);
}

// �ж��ļ����Ƿ��MatchFileNameƥ�䣬�����ƥ�䣬����ʧ��
BOOL MatchFileName(const string in_FileName,const string in_MatchStr) 
{ 
  // ������ڱȽϵ��ַ��ǿյģ�����FALSE
  if (in_MatchStr.size() == 0) return FALSE;

  // ������Ƚϵ��ַ����ǡ�*��������TRUE
  if (in_MatchStr == "*") return TRUE;

  // �����ļ���ƥ������е�ʱ��ƥ��dd-nn.mm
  char strTemp[2049];
  memset(strTemp,0,sizeof(strTemp));
  strncpy(strTemp,in_MatchStr.c_str(),2000);

  UINT ii,jj;
  INT  iPOS1,iPOS2;
  CCmdStr CmdStr,CmdSubStr;

  string strFileName,strMatchStr;

  strFileName=in_FileName;
  strMatchStr=strTemp;

  // ���ַ�����ת���ɴ�д�������Ƚ�
  ToUpper(strFileName);
  ToUpper(strMatchStr);

  CmdStr.SplitToCmd(strMatchStr,",");

  for (ii=0;ii<CmdStr.CmdCount();ii++)
  {
    // ���Ϊ�գ���һ��Ҫ����������ͻᱻ����
    if (CmdStr.m_vCmdStr[ii].empty() == TRUE) continue;

    iPOS1=iPOS2=0;
    CmdSubStr.SplitToCmd(CmdStr.m_vCmdStr[ii],"*");

    for (jj=0;jj<CmdSubStr.CmdCount();jj++)
    {
      // ������ļ������ײ�
      if (jj == 0)
      {
        if (strncmp(strFileName.c_str(),CmdSubStr.m_vCmdStr[jj].c_str(),CmdSubStr.m_vCmdStr[jj].size()) != 0) break;
      }

      // ������ļ�����β��
      if (jj == CmdSubStr.CmdCount()-1)
      {
        if (strcmp(strFileName.c_str()+strFileName.size()-CmdSubStr.m_vCmdStr[jj].size(),CmdSubStr.m_vCmdStr[jj].c_str()) != 0) break;
      }

      iPOS2=strFileName.find(CmdSubStr.m_vCmdStr[jj],iPOS1);

      if (iPOS2 < 0) break;

      iPOS1=iPOS2+CmdSubStr.m_vCmdStr[jj].size();
    }

    if (jj==CmdSubStr.CmdCount()) return TRUE;
  }

  return FALSE;
}

// �жϸ��е������Ƿ�ȫ��������
BOOL IsDigit(const char *strBuffer)
{
  int len=strlen(strBuffer);

  for (int i=0; i<len;i++)
  {
    if (isdigit(strBuffer[i]) == 0) return FALSE;
  }

  return TRUE;
}

// �ж������Ƿ�ȫ���Ǵ�д��ĸ
BOOL IsUpper(const char *strBuffer)
{
  int len=strlen(strBuffer);

  for (int i=0; i<len;i++)
  {
    if (isupper(strBuffer[i]) == 0) return FALSE;
  }

  return TRUE;
}

// �ж������Ƿ�ȫ����ASCII�ַ�
BOOL IsASCII(const char *strBuffer)
{
  int len=strlen(strBuffer);

  for (int i=0; i<len;i++)
  {
    if (isascii(strBuffer[i]) == 0) return FALSE;
  }

  return TRUE;
}


// �жϸ��е������Ƿ�ȫ�������ֺͿո�
BOOL IsDigitOrSpace(const char *strLine)
{
  int len=strlen(strLine);

  for (int i=0; i<len;i++)
  {
    if ( (strLine[i] != ' ') && (isdigit(strLine[i]) == 0) ) return FALSE;
  }

  return TRUE;
}


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
int AddTime(const char *in_stime,char *out_stime,const int in_interval,const char *in_fmt)
{
  time_t  timer;
  struct tm nowtimer;

  timer=UTCTime(in_stime)+in_interval;

  nowtimer = *localtime ( &timer ); nowtimer.tm_mon++;

  // Ϊ�˷�ֹin_stime��out_stimeΪͬһ���������������out_stime�ڴ˴���ʼ�������벻����ǰ
  out_stime[0]=0;

  if (in_fmt==0)
  {
    snprintf(out_stime,20,"%04u-%02u-%02u %02u:%02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec); return 0;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24:mi:ss") == 0)
  {
    snprintf(out_stime,20,"%04u-%02u-%02u %02u:%02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec); return 0;
  }

  if (strcmp(in_fmt,"yyyymmddhh24miss") == 0)
  {
    snprintf(out_stime,15,"%04u%02u%02u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec); return 0;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd") == 0)
  {
    snprintf(out_stime,11,"%04u-%02u-%02u",nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday); return 0;
  }
  if (strcmp(in_fmt,"yyyymmdd") == 0)
  {
    snprintf(out_stime,9,"%04u%02u%02u",nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday); return 0;
  }

  if (strcmp(in_fmt,"hh24:mi:ss") == 0)
  {
    snprintf(out_stime,9,"%02u:%02u:%02u",nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec); return 0;
  }

  if (strcmp(in_fmt,"hh24:mi") == 0)
  {
    snprintf(out_stime,9,"%02u:%02u",nowtimer.tm_hour,nowtimer.tm_min); return 0;
  }

  if (strcmp(in_fmt,"hh24mi") == 0)
  {
    snprintf(out_stime,7,"%02u%02u",nowtimer.tm_hour,nowtimer.tm_min); return 0;
  }

  return -1;
}

CTcpClient::CTcpClient()
{
  m_sockfd=-1;
  memset(m_IP,0,sizeof(m_IP));
  m_Port=0;
  m_State=FALSE;
  memset(m_ConnStr,0,sizeof(m_ConnStr));
  m_bTimeOut=FALSE;
}

BOOL CTcpClient::ConnectToServer(const char *in_ConnStr)
{
  m_State=FALSE;

  if (m_sockfd != -1)
  {
    close(m_sockfd); m_sockfd = -1;
  }

  memset(m_ConnStr,0,sizeof(m_ConnStr));
  strcpy(m_ConnStr,in_ConnStr);

  struct hostent* h;
  struct sockaddr_in servaddr;

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(m_ConnStr,",");

  UINT ii;

  for (ii=0; ii<CmdStr.CmdCount(); ii=ii+2)
  {
    CmdStr.GetValue(ii,m_IP);
    CmdStr.GetValue(ii+1,&m_Port);

    if ( (m_sockfd = socket(AF_INET,SOCK_STREAM,0) ) < 0)
      return FALSE;

    if ( !(h = gethostbyname(m_IP)) )
    {
      close(m_sockfd);  m_sockfd = -1; return FALSE;
    }

    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_Port);  // ָ������˵�ͨѶ�˿�
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);

    if (connect(m_sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) == 0)
    {
      m_State=TRUE; return TRUE;
    }
    else
    {
      close(m_sockfd);  m_sockfd = -1;
    }
  }

  return FALSE;
}

BOOL CTcpClient::Read(char *strRecvBuffer)
{
  if (m_sockfd == -1) return FALSE;

  m_BufLen = 0;
  return(TcpRead(m_sockfd,strRecvBuffer,&m_BufLen));
}

BOOL CTcpClient::Read(char *strRecvBuffer,const int iTimeOut)
{
  if (m_sockfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_sockfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = iTimeOut; timeout.tv_usec = 0;

  m_bTimeOut = FALSE;

  int i;
  if ( (i = select(m_sockfd+1,&tmpfd,NULL,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  m_BufLen = 0;
  return(TcpRead(m_sockfd,strRecvBuffer,&m_BufLen));
}

BOOL CTcpClient::Write(char *strSendBuffer)
{
  if (m_sockfd == -1) return FALSE;

  int buflen = strlen(strSendBuffer);

  return(Write(strSendBuffer,buflen));
}

BOOL CTcpClient::Write(char *strSendBuffer,const int buflen)
{
  if (m_sockfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_sockfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = 5; timeout.tv_usec = 0;
  
  m_bTimeOut = FALSE;

  int i;
  if ( (i=select(m_sockfd+1,NULL,&tmpfd,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  return(TcpWrite(m_sockfd,strSendBuffer,buflen));
}

void CTcpClient::Close()
{
  if (m_sockfd > 0)
  {
    close(m_sockfd); m_sockfd=-1;
  }

  m_State=FALSE;
}

CTcpClient::~CTcpClient()
{
  Close();
}

CTcpServer::CTcpServer()
{
  m_listenfd=-1;
  m_connfd=-1;
  m_socklen=0;
  m_bTimeOut=FALSE;
}

BOOL CTcpServer::InitServer(const unsigned int port)
{
  m_listenfd = socket(AF_INET,SOCK_STREAM,0);

  // WINDOWSƽ̨����
  //char b_opt='1';
  //setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&b_opt,sizeof(b_opt));
  //setsockopt(m_listenfd,SOL_SOCKET,SO_KEEPALIVE,&b_opt,sizeof(b_opt));

  // Linux����
  int opt = 1; unsigned int len = sizeof(opt);
  setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,len);
  setsockopt(m_listenfd,SOL_SOCKET,SO_KEEPALIVE,&opt,len);

  memset(&m_servaddr,0,sizeof(m_servaddr));
  m_servaddr.sin_family = AF_INET;
  m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  m_servaddr.sin_port = htons(port);
  if (bind(m_listenfd,(struct sockaddr *)&m_servaddr,sizeof(m_servaddr)) != 0 )
  {
    CloseListen(); return FALSE;
  }

  if (listen(m_listenfd,5) != 0 )
  {
    CloseListen(); return FALSE;
  }

  m_socklen = sizeof(struct sockaddr_in);

  return TRUE;
}

BOOL CTcpServer::Accept()
{
  if (m_listenfd == -1) return FALSE;

  if ((m_connfd=accept(m_listenfd,(struct sockaddr *)&m_clientaddr,(socklen_t*)&m_socklen)) < 0)
      return FALSE;

  return TRUE;
}

char *CTcpServer::GetIP()
{
  return(inet_ntoa(m_clientaddr.sin_addr));
}

BOOL CTcpServer::Read(char *strRecvBuffer)
{
  if (m_connfd == -1) return FALSE;

  m_BufLen = 0;
  return(TcpRead(m_connfd,strRecvBuffer,&m_BufLen));
}

BOOL CTcpServer::Read(char *strRecvBuffer,const int iTimeOut)
{
  if (m_connfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_connfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = iTimeOut; timeout.tv_usec = 0;

  m_bTimeOut = FALSE;

  int i;
  if ( (i = select(m_connfd+1,&tmpfd,NULL,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  m_BufLen = 0;
  return(TcpRead(m_connfd,strRecvBuffer,&m_BufLen));
}

BOOL CTcpServer::Write(char *strSendBuffer)
{
  if (m_connfd == -1) return FALSE;

  int buflen = strlen(strSendBuffer);

  return(Write(strSendBuffer,buflen));
}

BOOL CTcpServer::Write(char *strSendBuffer,const int buflen)
{
  if (m_connfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_connfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = 5; timeout.tv_usec = 0;
  
  m_bTimeOut = FALSE;

  int i;
  if ( (i=select(m_connfd+1,NULL,&tmpfd,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  return(TcpWrite(m_connfd,strSendBuffer,buflen));
}

void CTcpServer::CloseListen()
{
  if (m_listenfd > 0)
  {
    close(m_listenfd); m_listenfd=-1;
  }
}

void CTcpServer::CloseClient()
{
  if (m_connfd > 0)
  {
    close(m_connfd); m_connfd=-1; 
  }
}

CTcpServer::~CTcpServer()
{
  CloseListen(); CloseClient();
}

BOOL TcpRead(const int fd,char *strRecvBuffer,int *buflen,const int iTimeOut)
{
  if (fd == -1) return FALSE;

  if (iTimeOut > 0)
  {
    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(fd,&tmpfd);

    struct timeval timeout;
    timeout.tv_sec = iTimeOut; timeout.tv_usec = 0;

    int i;
    if ( (i = select(fd+1,&tmpfd,NULL,NULL,&timeout)) <= 0 ) return FALSE;
  }

  (*buflen) = 0;

  char strBufLen[11]; memset(strBufLen,0,sizeof(strBufLen));

  if (Readn(fd,(char*)strBufLen,TCPHEADLEN) == FALSE) return FALSE;

  (*buflen) = atoi(strBufLen);

  if ( (*buflen) > TCPBUFLEN ) return FALSE;

  if (Readn(fd,strRecvBuffer,(*buflen)) == FALSE) return FALSE;

  return TRUE;
}

BOOL TcpWrite(const int fd,const char *strSendBuffer,const int buflen,const int iTimeOut)
{
  if (fd == -1) return FALSE;

  if (iTimeOut > 0)
  {
    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(fd,&tmpfd);

    struct timeval timeout;
    timeout.tv_sec = 5; timeout.tv_usec = 0;

    if ( select(fd+1,NULL,&tmpfd,NULL,&timeout) <= 0 ) return FALSE;
  }
  
  int ibuflen=0;

  // �������Ϊ0���Ͳ����ַ����ĳ���
  if (buflen==0) ibuflen=strlen(strSendBuffer);
  else ibuflen=buflen;

  if (ibuflen>TCPBUFLEN) return FALSE;

  char strBufLen[11]; 
  memset(strBufLen,0,sizeof(strBufLen));
  sprintf(strBufLen,"%d",ibuflen);

  char strTBuffer[TCPHEADLEN+ibuflen];
  memset(strTBuffer,0,sizeof(strTBuffer));
  memcpy(strTBuffer,strBufLen,TCPHEADLEN);
  memcpy(strTBuffer+TCPHEADLEN,strSendBuffer,ibuflen);

  if (Writen(fd,strTBuffer,TCPHEADLEN+ibuflen) == FALSE) return FALSE;

  return TRUE;
}

BOOL Readn(const int fd,char *vptr,const size_t n)
{
  int nLeft,nread,idx;

  nLeft = n;
  idx = 0;

  while(nLeft > 0)
  {
    if ( (nread = recv(fd,vptr + idx,nLeft,0)) <= 0) return FALSE;

    idx += nread;
    nLeft -= nread;
  }

  return TRUE;
}

BOOL Writen(const int fd,const char *vptr,const size_t n)
{
  int nLeft,idx,nwritten;
  nLeft = n;  
  idx = 0;
  while(nLeft > 0 )
  {    
    if ( (nwritten = send(fd, vptr + idx,nLeft,0)) <= 0) return FALSE;      
    
    nLeft -= nwritten;
    idx += nwritten;
  }

  return TRUE;
}

CFile::CFile()
{
  initdata();
}

CFile::~CFile()
{
  CloseOnly();
}

void CFile::initdata()
{
  memset(m_filename, 0, sizeof(m_filename) );
  memset(m_filenametmp, 0, sizeof(m_filenametmp) );

  m_fp = 0;

  m_bEnBuffer = FALSE;

  m_filetype=0; 
}

// ��һ���ļ����ڶ���filenameΪȫ·�����ļ�����
// openmodeΪ���ļ��ķ�ʽ��ֻ����"rt"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
BOOL CFile::OpenForRead(const char *filename,const char *openmode)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  CloseOnly();

  if ( (strcmp(openmode,"r")!=0) && (strcmp(openmode,"rt")!=0) && (strcmp(openmode,"rb")!=0) ) return FALSE;

  if ( (m_fp=FOPEN(filename,openmode)) == NULL ) return FALSE;

  strncpy(m_filename,filename,300);

  m_filetype=1;

  return TRUE;
}

// ��ȡ�ļ���ÿ�У�����ÿһ�еĻس����з�ȥ�����������ж�ÿ�е������Ƿ���strEndStr������
// �Ƿ���ÿ�е���󲹳�strPadStr�ַ�
BOOL CFile::FFGETS(char *strBuffer,const int ReadSize,const char *strEndStr)
{
  if ( m_filetype != 1 ) return FALSE;

  return FGETS(m_fp,strBuffer,ReadSize,strEndStr);
}

// ����fread���ļ��ж�ȡ���ݡ�
size_t CFile::Fread(void *ptr, size_t size)
{
  if ( m_filetype != 1 ) return 0;

  return fread(ptr,1,size,m_fp);
}


// ��һ���ļ�����д��filenameΪ������·�����ļ���
// openmodeΪ���ļ��ķ�ʽ��ͬFOPENʹ�÷���һ�£�������"rt"��"rb"
// bEnBuffer:TRUE-���û�������FALSE-�����û�����
BOOL CFile::OpenForWrite(const char *filename, const char *openmode, BOOL bEnBuffer)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  CloseOnly();

  if ( (strcmp(openmode,"r")==0) || (strcmp(openmode,"rt")==0) || (strcmp(openmode,"rb")==0) ) return FALSE;

  if ( (m_fp = FOPEN(filename, openmode )) == NULL ) return FALSE;

  strncpy( m_filename, filename, 300);

  m_bEnBuffer = bEnBuffer;

  m_filetype=2;

  return TRUE;
}


BOOL CFile::OpenForRename(const char *filename,const char *openmode,BOOL bEnBuffer)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  CloseOnly();

  if ( (strcmp(openmode,"r")==0) || (strcmp(openmode,"rt")==0) || (strcmp(openmode,"rb")==0) ) return FALSE;

  strncpy(m_filename,filename,300);
  snprintf(m_filenametmp,300,"%s.tmp",filename);

  if ( (m_fp = FOPEN(m_filenametmp, openmode )) == NULL ) return FALSE;

  m_bEnBuffer = bEnBuffer;

  m_filetype=3;

  return TRUE;
}

void CFile::Fprintf(const char *fmt, ... )
{
  if ( m_filetype==1 ) return;

  va_list arg;

  if ( m_fp == 0 ) return;

  va_start( arg, fmt );
  vfprintf( m_fp, fmt, arg );
  va_end( arg );

  if ( m_bEnBuffer == FALSE ) fflush(m_fp);
}

// ���m_filetype==1��m_filetype==2���͹ر�����
// ���m_filetype==3���ر��ļ�ָ�룬��ɾ������ʱ�ļ���
void CFile::CloseOnly()
{
  if (m_fp == 0) return;

  fclose( m_fp );

  m_fp=0;

  if (m_filetype==3)
  {
    // ɾ������ʱ�ļ�
    remove(m_filenametmp); 
  }

  initdata();
}

BOOL CFile::IsOpened()
{
  if (m_fp == NULL) return FALSE;

  return TRUE;
}

// �ر��ļ�ָ�룬������ʱ�ļ�����Ϊ��ʽ�ļ������������ʧ�ܣ�������FALSE���ɹ�����TRUE
BOOL CFile::CloseAndRename()
{
  if (m_filetype!=3) return FALSE;

  if (m_fp==0) return FALSE;

  fclose( m_fp );

  m_fp=0;

  if (rename(m_filenametmp,m_filename) != 0) return FALSE;

  return TRUE;
}


// ����fwrite���ļ���д����
size_t CFile::Fwrite(const void *ptr, size_t size )
{
  if ( m_filetype == 1 ) return 0;

  size_t tt=fwrite(ptr,1,size,m_fp);

  if ( m_bEnBuffer == FALSE ) fflush(m_fp); 

  return tt;
}

// �����Ѵ򿪵�ֻ���ļ�����ɾ������
BOOL CFile::CloseAndRemove()
{
  if (m_filetype!=1) return FALSE;
  
  if (m_fp==0) return FALSE;

  // �ȹر��ļ�
  if ( m_fp != 0 ) fclose( m_fp );

  m_fp=0;

  if (remove(m_filename) != 0) return FALSE; 

  return TRUE;
}

// ����fopen�������ļ�������ļ����а�����Ŀ¼�����ڣ��ʹ���Ŀ¼
FILE *FOPEN(const char *filename,const char *mode)
{
  if (MKDIR(filename) == FALSE) return NULL;

  return fopen(filename,mode);
}

// ��ĳ�ļ���Ŀ¼��ȫ·���е�Ŀ¼����Ŀ¼���Լ���Ŀ¼�µĸ�����Ŀ¼
BOOL MKDIR(const char *filename,BOOL bisfilename)
{
  // ���Ŀ¼�Ƿ���ڣ���������ڣ��𼶴�����Ŀ¼
  char strPathName[301];

  for (UINT ii=1; ii<strlen(filename);ii++)
  {
    if (filename[ii] != '/') continue;

    memset(strPathName,0,sizeof(strPathName));
    strncpy(strPathName,filename,ii);

    if (access(strPathName,F_OK) == 0) continue;

    if (mkdir(strPathName,00755) != 0) return FALSE;
  }

  if (bisfilename==FALSE)
  {
    if (access(filename,F_OK) != 0) 
    {
      if (mkdir(filename,00755) != 0) return FALSE;
    }
  }

  return TRUE;
}

// �����ļ����޸�ʱ������
int UTime(const char *filename,const char *mtime)
{
  struct utimbuf stutimbuf;
  
  stutimbuf.actime=stutimbuf.modtime=UTCTime(mtime);

  return utime(filename,&stutimbuf);
}

// �ر�ȫ�����źź��������
void CloseIOAndSignal()
{
  int ii=0;

  for (ii=0;ii<50;ii++)
  {
    signal(ii,SIG_IGN); close(ii);
  }
}

