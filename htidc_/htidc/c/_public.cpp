#include "_public.h"


// ��ȡINI�ļ��е�����
CIniFile::CIniFile()
{
  memset(m_XMLBuffer,0,sizeof(m_XMLBuffer));
}

BOOL CIniFile::LoadFile(const char *in_FileName)
{
  memset(m_XMLBuffer,0,sizeof(m_XMLBuffer));

  char strLine[8193]; 

  FILE *fp=0;

  if ( (fp=FOPEN(in_FileName,"r")) == NULL) return FALSE;

  memset(strLine,0,sizeof(strLine));

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    if (FGETS(strLine,5000,fp) == FALSE) break;

    if (strLine[0] == '#') continue;

    // ��ֹm_XMLBuffer���
    if ( (strlen(m_XMLBuffer)+strlen(strLine)) >= (UINT)204800 ) break;
  
    strcat(m_XMLBuffer,strLine);
  }

  fclose(fp);

  fp=0;

  if (strlen(m_XMLBuffer) < 10) return FALSE;

  return TRUE;
}

BOOL CIniFile::GetValue(const char *in_FieldName,char *out_Value,int in_len)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value,in_len);
}

BOOL CIniFile::GetValue(const char *in_FieldName,int *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

BOOL CIniFile::GetValue(const char *in_FieldName,long *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

BOOL CIniFile::GetValue(const char *in_FieldName,UINT *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

BOOL CIniFile::GetValue(const char *in_FieldName,double *out_Value)
{
  return GetXMLBuffer(m_XMLBuffer,in_FieldName,out_Value);
}

CDir::CDir()
{
  m_uPOS=0;

  memset(m_DateFMT,0,sizeof(m_DateFMT));
  strcpy(m_DateFMT,"yyyy-mm-dd hh24:mi:ss");

  m_vFileName.clear();

  m_vDirName.clear();

  m_bOnlyDir=FALSE;

  m_bAndTMPFiles=FALSE;

  m_uMaxFileCount=0;
}

void CDir::initdata()
{
  memset(m_DirName,0,sizeof(m_DirName));
  memset(m_FileName,0,sizeof(m_FileName));
  memset(m_UpperFileName,0,sizeof(m_UpperFileName));
  memset(m_FullFileName,0,sizeof(m_FullFileName));
  m_FileSize=0;
  memset(m_CreateTime,0,sizeof(m_CreateTime));
  memset(m_ModifyTime,0,sizeof(m_ModifyTime));
  memset(m_AccessTime,0,sizeof(m_AccessTime));
}

// ��������ʱ��ĸ�ʽ��֧��"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"���ָ�ʽ��ȱʡ��ǰ��
void CDir::SetDateFMT(const char *strDateFMT)
{
  memset(m_DateFMT,0,sizeof(m_DateFMT));
  strcpy(m_DateFMT,strDateFMT);
}

// ��Ŀ¼�������ļ�������
BOOL CDir::OpenDir(const char *in_dirname,const BOOL bAndChild)
{
  m_uPOS=0;
  m_vFileName.clear();

  m_vDirName.clear();

  // ���Ŀ¼�����ڣ��ʹ�����Ŀ¼
  if (MKDIR(in_dirname,FALSE) == FALSE) return FALSE;

  BOOL bRet=_OpenDir(in_dirname,bAndChild);

  sort(m_vFileName.begin(), m_vFileName.end());

  sort(m_vDirName.begin(), m_vDirName.end());

  return bRet;
}

// ��Ŀ¼��������
BOOL CDir::OpenDirNoSort(const char *in_dirname,const BOOL bAndChild)
{
  m_uPOS=0;
  m_vFileName.clear();

  m_vDirName.clear();

  // ���Ŀ¼�����ڣ��ʹ�����Ŀ¼
  if (MKDIR(in_dirname,FALSE) == FALSE) return FALSE;

  m_vDirName.push_back(in_dirname);

  BOOL bRet=_OpenDir(in_dirname,bAndChild);

  sort(m_vDirName.begin(), m_vDirName.end());

  return bRet;
}

// ��Ŀ¼�����Ǹ��ݹ麯��
BOOL CDir::_OpenDir(const char *in_dirname,const BOOL bAndChild)
{
  DIR *dir;

  if ( (dir=opendir(in_dirname)) == NULL ) return FALSE;

  char strTempFileName[1025];

  struct dirent *st_fileinfo;
  struct stat st_filestat;

  while ((st_fileinfo=readdir(dir)) != NULL)
  {
    // ��"."��ͷ���ļ�������
    if (st_fileinfo->d_name[0]=='.') continue;

    if (m_bAndTMPFiles==FALSE)
    {
      // *.tmp���ļ�������
      if (MatchFileName(st_fileinfo->d_name,"*.TMP") == TRUE) continue;

      // *.TMP���ļ�������
      if (strcmp(st_fileinfo->d_name+strlen(st_fileinfo->d_name)-4,".TMP") == 0) continue;
    }

    memset(strTempFileName,0,sizeof(strTempFileName));

    snprintf(strTempFileName,300,"%s//%s",in_dirname,st_fileinfo->d_name);

    UpdateStr(strTempFileName,"//","/");

    stat(strTempFileName,&st_filestat);

    // �ж��Ƿ���Ŀ¼
    if (S_ISDIR(st_filestat.st_mode))
    {
      if (bAndChild == TRUE)
      {
        if (_OpenDir(strTempFileName,bAndChild) == FALSE) 
        {
          closedir(dir); return FALSE;
        }
      }

      m_vDirName.push_back(strTempFileName);
    }
    else
    {
      if (m_bOnlyDir==FALSE) 
      {
        m_vFileName.push_back(strTempFileName);

        if ( (m_uMaxFileCount!=0) && (m_vFileName.size()>m_uMaxFileCount) ) break;
      }
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

  // �ļ�����д
  memset(m_UpperFileName,0,sizeof(m_UpperFileName));
  strcpy(m_UpperFileName,m_FileName);
  ToUpper(m_UpperFileName);

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

  m_vDirName.clear();
}

/* ɾ���ַ������ָ�����ַ� */
void DeleteLChar(char *in_string,const char in_char)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;

  int istrlen = strlen(in_string) + 1;

  char *strTemp = new char[istrlen]; 

  int iTemp=0;

  memset(strTemp,0,istrlen);
  strcpy(strTemp,in_string);

  while ( strTemp[iTemp] == in_char )  iTemp++; 
  
  memset(in_string,0,istrlen);

  strcpy(in_string,strTemp+iTemp);

  delete strTemp;

  return;
}

/* ɾ����ߵĿո� */
void LTrim(char *in_string)
{
  DeleteLChar(in_string,' ');
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

/* ɾ���ұߵĿո� */
void RTrim(char *in_string)
{
  DeleteRChar(in_string,' ');
}

/* ɾ���ַ�������ָ�����ַ� */
void DeleteChar(char *in_string,const char in_char)
{
  DeleteLChar(in_string,in_char);
  DeleteRChar(in_string,in_char);
}

/* ɾ���ұߵĿո� */
void Trim(char *in_string)
{
  DeleteLChar(in_string,' ');
  DeleteRChar(in_string,' ');
}

CLogFile::CLogFile() 
{ 
  m_tracefp = 0;
  memset(m_filename,0,sizeof(m_filename));
  memset(m_openmode,0,sizeof(m_openmode));
  m_bBackup=TRUE;
  m_bEnBuffer=FALSE;
  m_balarmbz=FALSE;
  memset(m_programname,0,sizeof(m_programname));
}

// ���ø澯����
void CLogFile::SetAlarmOpt(const char *fmt,...) 
{
  m_balarmbz=FALSE;

  memset(m_programname,0,sizeof(m_programname));

  va_start(m_ap,fmt);
  vsnprintf(m_programname,100,fmt,m_ap);
  va_end(m_ap);

  if (strlen(m_programname) == 0) return;

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(m_programname,"/");

  memset(m_programname,0,sizeof(m_programname));

  strcpy(m_programname,CmdStr.m_vCmdStr[CmdStr.CmdCount()-1].c_str());

  m_balarmbz=TRUE;
}

BOOL CLogFile::WriteAlarmFile()
{
  char strFileName[301],strLocalTime[21];
  memset(strFileName,0,sizeof(strFileName));
  memset(strLocalTime,0,sizeof(strLocalTime));
  PickNumber(m_stime,strLocalTime,FALSE,FALSE);
  snprintf(strFileName,300,"/tmp/htidc/alarmxml/%s_%d_%s.xml",m_programname,getpid(),strLocalTime);

  FILE *fp=0;

  if ((fp=FOPEN(strFileName,"w+")) == NULL) return FALSE;

  fprintf(fp,"<data>\n<crttime>%s</crttime><progname>%s</progname><alarmtext>%s</alarmtext><endl/>\n</data>\n",
             strLocalTime,m_programname,m_message);

  fclose(fp);

  fp=0;

  return TRUE;
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

  // ֻҪ�Ǵ���־�ļ�������bBackup��־��ʲô����ִ�б���
  BackupLogFile();

  // Ϊ��BackupLogFile()�ɹ������д���һ��Ҫ����BackupLogFile()֮��
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
    RENAME(m_filename,bak_filename);

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
  vsnprintf(m_message,20000,fmt,m_ap);
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

  // д��澯��־
  if ( (m_balarmbz==TRUE) && (strstr(m_message,"failed")!=0) ) WriteAlarmFile();

  return TRUE;  
}

BOOL CLogFile::WriteEx(const char *fmt,...)
{
  memset(m_stime,0,sizeof(m_stime));
  memset(m_message,0,sizeof(m_message));

  LocalTime(m_stime);

  va_start(m_ap,fmt);
  vsnprintf(m_message,20000,fmt,m_ap);
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

  // д��澯��־
  if ( (m_balarmbz==TRUE) && (strstr(m_message,"failed")!=0) ) WriteAlarmFile();

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

  char str[8193];

  while ( (iPOS=srcstr.find(in_sep)) >= 0)
  {
    substr=srcstr.substr(0,iPOS);

    if (bdeletespace == TRUE)
    {
      memset(str,0,sizeof(str));

      strncpy(str,substr.c_str(),8192);

      Trim(str);

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

    strncpy(str,substr.c_str(),4095);

    Trim(str);

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

  Trim(out_Value);

  return TRUE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,int *out_Value)
{
  (*out_Value) = 0;

  char m_Temp[51]; 

  memset(m_Temp,0,sizeof(m_Temp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,m_Temp,50) == TRUE)
  {
    (*out_Value) = atoi(m_Temp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned int *out_Value)
{
  (*out_Value) = 0;

  char m_Temp[51]; 

  memset(m_Temp,0,sizeof(m_Temp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,m_Temp,50) == TRUE)
  {
    (*out_Value) = (unsigned int)atoi(m_Temp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,long *out_Value)
{
  (*out_Value) = 0;

  char m_Temp[51]; 

  memset(m_Temp,0,sizeof(m_Temp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,m_Temp,50) == TRUE)
  {
    (*out_Value) = atol(m_Temp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,unsigned long *out_Value)
{
  (*out_Value) = 0;

  char m_Temp[51]; 

  memset(m_Temp,0,sizeof(m_Temp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,m_Temp,50) == TRUE)
  {
    (*out_Value) = (unsigned long)atol(m_Temp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,double *out_Value)
{
  (*out_Value) = 0;

  char m_Temp[51]; 

  memset(m_Temp,0,sizeof(m_Temp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,m_Temp,50) == TRUE)
  {
    (*out_Value) = atof(m_Temp); return TRUE;
  }

  return FALSE;
}

BOOL GetXMLBuffer(const char *in_XMLBuffer,const char *in_FieldName,float *out_Value)
{
  (*out_Value) = 0;

  char m_Temp[51]; 

  memset(m_Temp,0,sizeof(m_Temp));
  
  if (GetXMLBuffer(in_XMLBuffer,in_FieldName,m_Temp,50) == TRUE)
  {
    (*out_Value) = atof(m_Temp); return TRUE;
  }

  return FALSE;
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

  char *strtemp=new char[str.size()+1];

  memset(strtemp,0,str.size()+1);
  strcpy(strtemp,str.c_str());

  ToUpper(strtemp);

  str=strtemp;

  delete strtemp;

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

  char *strtemp=new char[str.size()+1];

  memset(strtemp,0,str.size()+1);
  strcpy(strtemp,str.c_str());

  ToLower(strtemp);

  str=strtemp;

  delete strtemp;

  return;
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

// ɾ���ַ����м���ַ�
void DeleteMChar(char *in_string,const char in_char)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;

  char strTemp[8193]; 

  memset(strTemp,0,sizeof(strTemp));

  int jj=0;

  int iLen=strlen(in_string);

  for (int ii=0; ii<iLen; ii++)
  {
    if (in_string[ii] != in_char)
    {
      strTemp[jj] = in_string[ii]; jj++;
    }
  }

  strcpy(in_string,strTemp);
}

// ɾ���ַ����м���ַ���
void DeleteMStr(char *in_string,const char *in_str)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;
  
  char strTemp[8193];  // 8193�Ǻܳ��ˣ���Ҳ�п��ܲ�����
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

// ɾ���ַ����м���ַ���
void SDeleteMStr(string &in_string,const string in_str)
{
  if (in_string.empty()) return;
  
  char *strtemp=new char[in_string.size()+1];

  memset(strtemp,0,in_string.size()+1);

  strcpy(strtemp,in_string.c_str());

  DeleteMStr(strtemp,(char*)in_str.c_str());

  in_string=strtemp;

  delete strtemp;

  return;
}

// ���ַ����е�ĳ�ַ�������һ���ַ�������
void UpdateStr(string &in_string,const char *in_str1,const char *in_str2,BOOL bLoop)
{
  if (in_string.empty()) return;
  
  int len=in_string.size()+strlen(in_str1)+strlen(in_str2)+1;

  char *strtemp=new char[len];

  memset(strtemp,0,len);

  strcpy(strtemp,in_string.c_str());

  UpdateStr(strtemp,in_str1,in_str2,bLoop);

  in_string = strtemp;

  delete strtemp;
  
  return;
}

// ���ַ����е�ĳ�ַ�������һ���ַ�������
void UpdateStr(char *in_string,const char *in_str1,const char *in_str2,BOOL bLoop)
{
  if (in_string == 0) return;

  if (strlen(in_string) == 0) return;

  char strTemp[204801];   // 204801�Ǻܳ��ˣ���Ҳ�п��ܲ�����

  char *strStart=in_string;

  char *strPos=0;

  while (TRUE)
  {
    if (strlen(in_string) >200000) break;

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
void LocalTime(char *stime,const char *in_fmt,const int interval)
{
  time_t  timer;
  struct tm nowtimer;

  time( &timer ); timer=timer+interval;
  nowtimer = *localtime ( &timer ); nowtimer.tm_mon++;

  if (in_fmt==0)
  {
    snprintf(stime,20,"%04u-%02u-%02u %02u:%02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24:mi:ss") == 0)
  {
    snprintf(stime,20,"%04u-%02u-%02u %02u:%02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24:mi") == 0)
  {
    snprintf(stime,17,"%04u-%02u-%02u %02u:%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd hh24") == 0)
  {
    snprintf(stime,14,"%04u-%02u-%02u %02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour);
    return;
  }

  if (strcmp(in_fmt,"yyyy-mm-dd") == 0)
  {
    snprintf(stime,11,"%04u-%02u-%02u",nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday); return;
  }

  if (strcmp(in_fmt,"yyyy-mm") == 0)
  {
    snprintf(stime,8,"%04u-%02u",nowtimer.tm_year+1900,nowtimer.tm_mon); return;
  }

  if (strcmp(in_fmt,"hh24:mi:ss") == 0)
  {
    snprintf(stime,9,"%02u:%02u:%02u",nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec); return;
  }

  if (strcmp(in_fmt,"hh24:mi") == 0)
  {
    snprintf(stime,5,"%02u:%02u",nowtimer.tm_hour,nowtimer.tm_min); return;
  }

  if (strcmp(in_fmt,"yyyymmddhh24miss") == 0)
  {
    snprintf(stime,15,"%04u%02u%02u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min,nowtimer.tm_sec);
    return;
  }

  if (strcmp(in_fmt,"yyyymmddhh24mi") == 0)
  {
    snprintf(stime,13,"%04u%02u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour,
                    nowtimer.tm_min);
    return;
  }

  if (strcmp(in_fmt,"yyyymmddhh24") == 0)
  {
    snprintf(stime,11,"%04u%02u%02u%02u",nowtimer.tm_year+1900,
                    nowtimer.tm_mon,nowtimer.tm_mday,nowtimer.tm_hour);
    return;
  }

  if (strcmp(in_fmt,"yyyymmdd") == 0)
  {
    snprintf(stime,9,"%04u%02u%02u",nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday); return;
  }

  if (strcmp(in_fmt,"hh24miss") == 0)
  {
    snprintf(stime,7,"%02u%02u%02u",nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec); return;
  }

  if (strcmp(in_fmt,"hh24mi") == 0)
  {
    snprintf(stime,5,"%02u%02u",nowtimer.tm_hour,nowtimer.tm_min); return;
  }

  if (strcmp(in_fmt,"hh24") == 0)
  {
    snprintf(stime,3,"%02u",nowtimer.tm_hour); return;
  }

  if (strcmp(in_fmt,"mi") == 0)
  {
    snprintf(stime,3,"%02u",nowtimer.tm_min); return;
  }
}

// ��time_t���͵�����ʱת��Ϊ���ݿ��׼ʱ yyyy-mm-dd hh24:mi:ss
 void STDTime(char *strtime, char *buff)
 {	
    time_t ti = (atol)(buff);
    strtime[0]=0;
    struct tm *newtime;
    newtime = localtime(&ti);
    strftime(strtime,100,"%F %H:%M:%S",newtime);
    return;
 }
 void STDTime(char *strtime, long buff)
 {	
    time_t ti = buff;
    strtime[0]=0;
    struct tm *newtime;
    newtime = localtime(&ti);
    strftime(strtime,100,"%F %H:%M:%S",newtime);
    return;
 }

// ��yyyy-mm-dd hh24:mi:ss�������ڴ���Ĺ����н��������ڴ��еĸ�ʽ
long UTCTime(const char *stime)
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

  //return mktime(&time_str)+8*60*60;
  return mktime(&time_str);
}

CTimer::CTimer()
{
  Beginning();
}

// ��ʼ��ʱ
void CTimer::Beginning()
{
  m_BeginningPOS=0;
  time(&m_BeginningPOS);
}

// ��������ȥ��ʱ��
long CTimer::Elapsed()
{
  time_t NowPOS;

  time(&NowPOS);

  return NowPOS-m_BeginningPOS;
}

CProgramActive::CProgramActive()
{
  m_logfile=0;
}

void CProgramActive::initdata()
{
  m_PID=0;                      // ���̵�ID
  memset(m_ProgramName,0,sizeof(m_ProgramName));
  m_MaxTimeOut=0;               // ÿ�λ֮����������ʱ�䳤������Ϊ��λ
  m_LatestActiveTime=0;        // ���һ�λʱ�䣬��������ʾ
  m_Elapsed=0;                  // ���һ�λʱ���뵱ǰʱ���ʱ���
  memset(m_FileName,0,sizeof(m_FileName));
  m_NowTime=0;
}

void CProgramActive::SetProgramInfo(const CLogFile *in_logfile,const char *in_ProgramName,const int in_MaxTimeOut)
{
  m_logfile=(CLogFile *)in_logfile;
  m_PID=getpid();
  strcpy(m_ProgramName,in_ProgramName);
  m_MaxTimeOut=in_MaxTimeOut;
  if (m_MaxTimeOut < 30) m_MaxTimeOut=30; // ���̻�����30�롣
  snprintf(m_FileName,300,"/tmp/htidc/proc/_P_A_%s_%d",m_ProgramName,m_PID);
  UpdateStr(m_FileName,"//","/");

  WriteToFile(TRUE);
}

// �����һ�εĽ��̻��Ϣд���ļ�
BOOL CProgramActive::WriteToFile(const BOOL bIsFirstTime)
{
  if ( (bIsFirstTime==FALSE) && (m_Timer.Elapsed()<(int)m_MaxTimeOut/3) ) return TRUE;

  // ��ʼ��ʱ
  m_Timer.Beginning();

  // ��ȡ��ǰʱ��
  time(&m_LatestActiveTime);

  CLogFile logfiletmp;

  if (logfiletmp.Open(m_FileName,"w") == FALSE)
  {
    m_logfile->Write("Open %s failed.\n",m_FileName); return FALSE;
  }

  logfiletmp.Write("<pid>%d</pid><programname>%s</programname><latestactivetime>%d</latestactivetime><maxtimeout>%d</maxtimeout>\n",m_PID,m_ProgramName,m_LatestActiveTime,m_MaxTimeOut);

  return TRUE;
}

BOOL CProgramActive::ReadFromFile(const char *in_FileName)
{
  initdata();

  CIniFile IniFileTmp;

  if (IniFileTmp.LoadFile(in_FileName) == FALSE) return FALSE;

  IniFileTmp.GetValue("pid",&m_PID);
  IniFileTmp.GetValue("programname",m_ProgramName);
  IniFileTmp.GetValue("latestactivetime",&m_LatestActiveTime);
  IniFileTmp.GetValue("maxtimeout",&m_MaxTimeOut);

  time(&m_NowTime);

  m_Elapsed=m_NowTime-m_LatestActiveTime;

  if ( (m_PID==0) || (m_MaxTimeOut==0) ) return FALSE;

  return TRUE;
}

CProgramActive::~CProgramActive()
{
  if (strlen(m_FileName) > 0) REMOVE(m_FileName);
}

// �ж��ļ��Ƿ���strBZ����
BOOL CheckFileSTS(const char *strFileName,const char *strBZ)
{
  FILE *fp=0;

  if ( (fp=FOPEN(strFileName,"r")) == NULL ) return FALSE;

  char strBuf[301];
  memset(strBuf,0,sizeof(strBuf));

  fseek(fp,-256L,2);
  fread(strBuf,1,256,fp);
  fclose(fp);

  fp=0;

  if ( strstr(strBuf,strBZ) == NULL) return FALSE;

  return TRUE;
}

// ��ȡ�ļ���ÿ�У�����ÿһ�еĻس����з�ȥ�����������ж�ÿ�е������Ƿ���strEndStr������
// �Ƿ���ÿ�е���󲹳�strPadStr�ַ�
BOOL FGETS(char *strBuffer,const int ReadSize,const FILE *fp,const char *strEndStr,const char *strPadStr)
{
  char *strLine = new char[ReadSize+1];

  strcpy(strBuffer,"");

  while (TRUE)
  {
    memset(strLine,0,ReadSize+1);

    if (fgets(strLine,ReadSize,(FILE *)fp) == 0) break;

    DeleteRChar(strLine,'\n'); DeleteRChar(strLine,'\r'); 
    DeleteRChar(strLine,'\n'); DeleteRChar(strLine,'\r');

    DeleteLChar(strLine,' ');  
    DeleteRChar(strLine,' ');

    // ��ֹstrBuffer���
    if ( (strlen(strBuffer)+strlen(strLine)) >= (UINT)ReadSize ) break;

    if ( (strEndStr != 0) && (strPadStr != 0) )
    {
      if (strlen(strBuffer) > 0) strcat(strBuffer,strPadStr);
    }

    strcat(strBuffer,strLine);

    if (strEndStr == 0) { delete strLine; return TRUE; }

    if (strncmp(strLine+strlen(strLine)-strlen(strEndStr),strEndStr,strlen(strEndStr)) == 0) 
    {
      strBuffer[strlen(strBuffer)-strlen(strEndStr)]=0; delete strLine; return TRUE;
    }
  }

  delete strLine;

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
  char strtemp[1025];
  memset(strtemp,0,sizeof(strtemp));
  strncpy(strtemp,strSrc,1024);
  Trim(strtemp);

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

// �ж��ļ��Ĵ�С�������ֽ���
long FileSize(const char *in_FullFileName)
{
  struct stat st_filestat;

  if (stat(in_FullFileName,&st_filestat) < 0) return -1;

  return st_filestat.st_size;
}

// �ж��ļ���ʱ�䣬��modtime
void FileModTime(const char *in_FullFileName,char *out_ModTime)
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
// in_MatchStr�е���ĸ���ô�д�������Ҫƥ��yyyymmdd-xx.yyhh24mi��ʱ����ʾҪ����Сд��
BOOL MatchFileName(const string in_FileName,const string in_MatchStr,BOOL bPMatch) 
{ 
  // ������ڱȽϵ��ַ��ǿյģ�����FALSE
  if (in_MatchStr.size() == 0) return FALSE;

  // ������Ƚϵ��ַ����ǡ�*��������TRUE
  if (in_MatchStr == "*") return TRUE;

  // �����ļ���ƥ������е�ʱ��ƥ��dd-nn.mm
  char strTemp[2049];
  memset(strTemp,0,sizeof(strTemp));
  strncpy(strTemp,in_MatchStr.c_str(),2000);

  if (bPMatch==TRUE) ProcMatchDTime(strTemp);

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

// ��ĳ�µ������ϼ���һ����
void SubtractAMonth(const char *strLocalYM,char *strPreYM)
{
  // Ϊ�˷�ֹstrLocalYM��strPreYMΪͬһ�����������strPreYM���ܱ���ʼ��
  char strYear[5],strMon[3];

  memset(strYear,0,sizeof(strYear));
  memset(strMon,0,sizeof(strMon));

  strncpy(strYear,strLocalYM,4);
  strncpy(strMon ,strLocalYM+4,2);

  if (atoi(strMon) == 1)
  {
    snprintf(strPreYM,7,"%d12",atoi(strYear)-1);
  }
  else
  {
    snprintf(strPreYM,7,"%s%02d",strYear,atoi(strMon)-1);
  }

  strPreYM[6]=0;
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


// �жϸ��е������Ƿ�ȫ�������ֻ�ո�
BOOL IsDigitOrSpace(const char *strLine)
{
  int len=strlen(strLine);

  for (int i=0; i<len;i++)
  {
    if ( (strLine[i] != ' ') && (isdigit(strLine[i]) == 0) ) return FALSE;
  }

  return TRUE;
}

// �����ַ�����ƥ�������յ�����������Դ����ʽΪdd-xx.yy��ƥ�䣬xx.yy�ĵ�λ����
// ע���ʽ��xxһ��Ҫ��2λ���ȣ�yyһ��Ҫ��λ���ȣ�����Ĳ�0��
void ProcMatchDTime(char *in_MatchStr)
{
  char  *pos=0;
  char   strtimetvl[9];
  double dtimetvl=0;

  char strField[201];
  char strDstMatchStr[2048];

  char strLocalTime[21],stryyyy[5],stryyy[4],stryy[3],strmm[3],strdd[3],strhh24[3],strmi[3];
  
  CCmdStr CmdStr;
  CmdStr.SplitToCmd(in_MatchStr,",");

  memset(strDstMatchStr,0,sizeof(strDstMatchStr));
  
  for (UINT ii=0; ii<CmdStr.CmdCount(); ii++)
  {
    memset(strField,0,sizeof(strField));

    CmdStr.GetValue(ii,strField,200);

    pos=0;
    dtimetvl=0;
    memset(strtimetvl,0,sizeof(strtimetvl));
  
    pos=strstr(strField,"dd-");

    if (pos > 0) 
    {
      strncpy(strtimetvl,pos,8);
      dtimetvl=atof(strtimetvl+3);
    }

    memset(strLocalTime,0,sizeof(strLocalTime));
    memset(stryyyy,0,sizeof(stryyyy));
    memset(stryyy,0,sizeof(stryyy));
    memset(stryy,0,sizeof(stryy));
    memset(strmm,0,sizeof(strmm));
    memset(strdd,0,sizeof(strdd));
    memset(strhh24,0,sizeof(strhh24));
    memset(strmi,0,sizeof(strmi));

    // ��ȡ��ǰƫ�ƺ��ʱ��
    LocalTime(strLocalTime,"yyyymmddhh24miss",(int)(0-dtimetvl*24*60*60));

    strncpy(stryyyy,strLocalTime,4);
    strncpy(stryyy,strLocalTime+1,3);
    strncpy(stryy,strLocalTime+2,2);
    strncpy(strmm,strLocalTime+4,2);
    strncpy(strdd,strLocalTime+6,2);
    strncpy(strhh24,strLocalTime+8,2);
    strncpy(strmi,strLocalTime+10,2);

    UpdateStr(strField,"yyyy",stryyyy);
    UpdateStr(strField,"yyy",stryyy);
    UpdateStr(strField,"yy",stryy);
    UpdateStr(strField,"mm",strmm);
    UpdateStr(strField,"hh24",strhh24);
    UpdateStr(strField,"mi",strmi);

    // ����dd-xx.yy
    if ( strlen(strtimetvl) > 0)
    {
      UpdateStr(strField,strtimetvl,strdd);
    }
    else
    {
      UpdateStr(strField,"dd",strdd);
    }

    strcat(strDstMatchStr,strField);
    strcat(strDstMatchStr,",");
  }

  strDstMatchStr[strlen(strDstMatchStr)-1] = 0;

  strcpy(in_MatchStr,strDstMatchStr);
}

// ͳ���ַ����ֵĸ�����һ�����֣���Ӣ�Ļ����ֶ���һ���֡�
int SumWord(const char *in_Content)
{
  int  SMSTotal=0,SMSNo=0;
  char SMSContent[8193];
  memset(SMSContent,0,sizeof(SMSContent));

  // �ж��Ƿ񳬳���ʽ
  GetXMLBuffer(in_Content,"1",&SMSTotal);
  GetXMLBuffer(in_Content,"2",&SMSNo);
  GetXMLBuffer(in_Content,"3", SMSContent);

  if (SMSTotal == 0) strncpy(SMSContent,in_Content,8192);

  int  WideLen=0;
  int  iDouble=0;
  int  iTotalLen=strlen(SMSContent);

  for (int ii=0;ii<iTotalLen;ii++)
  {
    if ( (unsigned int )SMSContent[ii] < 128)
    {
      WideLen = WideLen + 1;
    }
    else
    {
      if (iDouble==1)
      {
        WideLen = WideLen + 1;
        iDouble=0;
      }
      else
      {
        iDouble=1;
      }
    }
  }

  return WideLen;
}

CSplitSMS::CSplitSMS()
{
  initdata();
}

void CSplitSMS::initdata()
{
  m_vContent.clear();
}

void CSplitSMS::SplitToCmd(const char *in_Content,const int in_smslen)
{
  initdata();

  int  WideLen=0;
  int  iDouble=0;
  int  iTotalLen=strlen(in_Content);
  int  ipos=0;

  char strContent[201];
  memset(strContent,0,sizeof(strContent));

  for (int ii=0;ii<iTotalLen;ii++)
  {
    if (WideLen == in_smslen)
    {
      m_EndCharCount=WideLen;
      m_vContent.push_back(strContent);

      ipos=WideLen=iDouble=0;
      memset(strContent,0,sizeof(strContent));
    }

    if ( (unsigned int )in_Content[ii] < 128)
    {
      WideLen = WideLen + 1;
    }
    else
    {
      if (iDouble==1)
      {
        WideLen = WideLen + 1; iDouble=0;
      }
      else
      {
        iDouble=1;
      }
    }

    strContent[ipos] = in_Content[ii];

    ipos++;
  }

  if (strlen(strContent) > 0) 
  {
    m_EndCharCount=WideLen;
    m_vContent.push_back(strContent);
  }
}


// �Ѹ������ľ�γ��ת��Ϊ�ȷ��룬�ֱ�����strd,strf,strm��
void doubletodfm(const double dd,char *strd,char *strf,char *strm)
{
  strd[0]=0; strf[0]=0; strm[0]=0;
 
  snprintf(strd,4,"%03d",(int)floor(dd));
 
  double dtemp=(dd-floor(dd))*60;
 
  snprintf(strf,3,"%02d",(int)floor(dtemp));
 
  snprintf(strm,3,"%02d",(int)((dtemp-floor(dtemp))*60));
}


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
int AddTime(const char *in_stime,char *out_stime,const int interval,const char *in_fmt)
{
  time_t  timer;
  struct tm nowtimer;

  //timer=UTCTime(in_stime)+interval-8*60*60;
  timer=UTCTime(in_stime)+interval;

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


// ����x1,y1��x2,y2֮��ľ��룬x1,y1,x2,y2�ĵ�λ���Ƕ�
float x1y1tox2y2(const float x1,const float x2,const float y1,const float y2)
{
  float x1tox2=(x1*111-x2*111);
  float y1toy2=(y1*111-y2*111);

  if (x1tox2<0) x1tox2=0-x1tox2;

  if (y1toy2<0) y1toy2=0-y1toy2;

  return sqrt(x1tox2*x1tox2+y1toy2*y1toy2);
}

// ���ַ�����ʽ�ľ�γ�ȣ���111��23'45"��1112345��ת��Ϊ�ȣ���������double��char��
void dfmtodouble(const char *strdfm,double *dd)
{
  (*dd)=0;

  BOOL bsplitbz=FALSE;

  if (strstr(strdfm,"��") > 0) bsplitbz=TRUE;

  // �������Դ�ַ����ĳ���Ϊ�գ���ֱ�ӷ���
  if (strlen(strdfm) == 0) return;

  double d,f,m;

  d=f=m=0;

  char strtemp[51];
  memset(strtemp,0,sizeof(strtemp));
  strncpy(strtemp,strdfm,20);

  UpdateStr(strtemp,"E","");
  UpdateStr(strtemp,"W","");
  UpdateStr(strtemp,"S","");
  UpdateStr(strtemp,"N","");

  if (bsplitbz==FALSE)
  {
    char strtemp1[11];
    memset(strtemp1,0,sizeof(strtemp1));
    switch (strlen(strtemp))
    {
      case 7:
        strncpy(strtemp1,strtemp,3);  strcat(strtemp1,"|");
        strncat(strtemp1,strtemp+3,2);strcat(strtemp1,"|");
        strncat(strtemp1,strtemp+5,2);
        break;
      case 6:
        strncpy(strtemp1,strtemp,2);  strcat(strtemp1,"|");
        strncat(strtemp1,strtemp+2,2);strcat(strtemp1,"|");
        strncat(strtemp1,strtemp+4,2);
        break;
      case 5:
        strncpy(strtemp1,strtemp,3);  strcat(strtemp1,"|");
        strncat(strtemp1,strtemp+3,2);
        break;
      case 4:
        strncpy(strtemp1,strtemp,2);  strcat(strtemp1,"|");
        strncat(strtemp1,strtemp+2,2);
        break;
      default:
        strncpy(strtemp1,strtemp,3); 
        break;
    }

    memset(strtemp,0,sizeof(strtemp));
    strcpy(strtemp,strtemp1);
  }

  UpdateStr(strtemp,"��","|");
  UpdateStr(strtemp,"'" ,"|");
  UpdateStr(strtemp,"\"" ,"");

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strtemp,"|");

  if (CmdStr.CmdCount()>=1) CmdStr.GetValue(0,&d);
  if (CmdStr.CmdCount()>=2) CmdStr.GetValue(1,&f);
  if (CmdStr.CmdCount()>=3) CmdStr.GetValue(2,&m);

  (*dd)=d+f/60+m/3600;

  if ( ((*dd)>+180.01) || ((*dd)<-180.01) ) (*dd)=0;

  return;
}

// ���ַ�����ʽ�ľ�γ�ȣ���111��23'45"��1112345��ת��Ϊ�ȣ���������double��char��
void dfmtochar(const char *strdfm,char *strdd)
{
  double dd=0;

  dfmtodouble(strdfm,&dd);

  snprintf(strdd,10,"%.5f",dd);
}

// �Ѷ��ŵ���Чʱ��Ͷ�ʱʱ��ת��
// valid_time��at_time�ĸ�ʽ��yyyy-mm-dd hh24:mi:ss
// strvalidtime��strattime�ĸ�ʽ��yymmddhhmisst32+
void ConvertTime(char *valid_time,char *strvalidtime,char *at_time,char *strattime)
{
  PickNumber(valid_time,strvalidtime,FALSE,FALSE);

  // ��ʽ��yymmddhhmisst32+
  if (strlen(strvalidtime) == 14) 
  {
    strcat(strvalidtime,"032+");
  }
  else
  {
    strcpy(strvalidtime,"");
  }

  PickNumber(at_time,strattime,FALSE,FALSE);

  // ��ʽ��yymmddhhmisst32+
  if (strlen(strattime) == 14) 
  {
    strcat(strattime,"032+");
  }
  else
  {
    strcpy(strattime,"");
  }
}

CTcpClient::CTcpClient()
{
  m_sockfd=-1;
  memset(m_IP,0,sizeof(m_IP));
  m_Port=0;
  m_State=FALSE;
  memset(m_ConnStr,0,sizeof(m_ConnStr));
  m_bTimeOut=FALSE;
  m_logfile=0;
}

void CTcpClient::SetConnectOpt(char *in_ConnStr)
{
  memset(m_ConnStr,0,sizeof(m_ConnStr));
  strcpy(m_ConnStr,in_ConnStr);
}

BOOL CTcpClient::ConnectToServer()
{
  m_State=FALSE;

  if (m_sockfd != -1)
  {
    close(m_sockfd); m_sockfd = -1;
  }

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
      close(m_sockfd); return FALSE;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_Port);
    bcopy(h->h_addr,&servaddr.sin_addr,h->h_length);

    if (connect(m_sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) == 0)
    {
      m_State=TRUE; return TRUE;
    }
    else
    {
      close(m_sockfd);
    }
  }

  return FALSE;
}

BOOL CTcpClient::Read(char *strRead)
{
  if (m_sockfd == -1) return FALSE;

  m_BufLen = 0;
  return(TcpRead(m_sockfd,strRead,&m_BufLen));
}

BOOL CTcpClient::Read(char *strRead,long iSecond)
{
  if (m_sockfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_sockfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = iSecond; timeout.tv_usec = 0;

  m_bTimeOut = FALSE;

  int i;
  if ( (i = select(m_sockfd+1,&tmpfd,NULL,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  m_BufLen = 0;
  return(TcpRead(m_sockfd,strRead,&m_BufLen));
}

BOOL CTcpClient::Write(char *strWrite)
{
  if (m_sockfd == -1) return FALSE;

  long buflen = strlen(strWrite);

  return(Write(strWrite,buflen));
}

BOOL CTcpClient::Write(char *strWrite,long buflen)
{
  if (m_sockfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_sockfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = 30; timeout.tv_usec = 0;
  
  m_bTimeOut = FALSE;

  int i;
  if ( (i=select(m_sockfd+1,NULL,&tmpfd,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  return(TcpWrite(m_sockfd,strWrite,buflen));
}

void CTcpClient::Close()
{
  if (m_sockfd > 0)
  {
    // ����ʹ��shutdown���������Ŀǰ�����������ԭ��
    // shutdown(m_sockfd,2); 
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
  m_logfile=0;
}

BOOL CTcpServer::InitServer(int port)
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

BOOL CTcpServer::Read(char *strRead)
{
  if (m_connfd == -1) return FALSE;

  m_BufLen = 0;
  return(TcpRead(m_connfd,strRead,&m_BufLen));
}

BOOL CTcpServer::Read(char *strRead,long iSecond)
{
  if (m_connfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_connfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = iSecond; timeout.tv_usec = 0;

  m_bTimeOut = FALSE;

  int i;
  if ( (i = select(m_connfd+1,&tmpfd,NULL,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  m_BufLen = 0;
  return(TcpRead(m_connfd,strRead,&m_BufLen));
}

BOOL CTcpServer::Write(char *strWrite)
{
  if (m_connfd == -1) return FALSE;

  long buflen = strlen(strWrite);

  return(Write(strWrite,buflen));
}

BOOL CTcpServer::Write(char *strWrite,long buflen)
{
  if (m_connfd == -1) return FALSE;

  fd_set tmpfd;

  FD_ZERO(&tmpfd);
  FD_SET(m_connfd,&tmpfd);

  struct timeval timeout;
  timeout.tv_sec = 30; timeout.tv_usec = 0;
  
  m_bTimeOut = FALSE;

  int i;
  if ( (i=select(m_connfd+1,NULL,&tmpfd,NULL,&timeout)) <= 0 )
  {
    if (i==0) m_bTimeOut = TRUE;
    return FALSE;
  }

  return(TcpWrite(m_connfd,strWrite,buflen));
}

void CTcpServer::CloseListen()
{
  if (m_listenfd > 0)
  {
    // ����ʹ��shutdown���������Ŀǰ�����������ԭ��
    // shutdown(m_listenfd,2); 
    close(m_listenfd); m_listenfd=-1;
  }
}

void CTcpServer::CloseClient()
{
  if (m_connfd > 0)
  {
    // ����ʹ��shutdown���������Ŀǰ�����������ԭ��
    // shutdown(m_connfd,2); 
    close(m_connfd); m_connfd=-1; 
  }
}

CTcpServer::~CTcpServer()
{
  CloseListen(); CloseClient();
}

BOOL TcpRead(int fd,char *strRead,long *buflen,int iSecond)
{
  if (fd == -1) return FALSE;

  if (iSecond > 0)
  {
    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(fd,&tmpfd);

    struct timeval timeout;
    timeout.tv_sec = iSecond; timeout.tv_usec = 0;

    int i;
    if ( (i = select(fd+1,&tmpfd,NULL,NULL,&timeout)) <= 0 ) return FALSE;
  }

  (*buflen) = 0;

  char strBufLen[5]; memset(strBufLen,0,sizeof(strBufLen));

  if (Readn(fd,(char*)strBufLen,4) == FALSE) return FALSE;

  (*buflen) = atoi(strBufLen);

  if ( (*buflen) > TCPBUFLEN ) return FALSE;

  if (Readn(fd,strRead,(*buflen)-4) == FALSE)
    return FALSE;

  (*buflen) = (*buflen) - 4;

  return TRUE;
}

BOOL TcpWrite(int fd,char *strWrite,long buflen,int iSecond)
{
  if (fd == -1) return FALSE;

  if (iSecond > 0)
  {
    fd_set tmpfd;

    FD_ZERO(&tmpfd);
    FD_SET(fd,&tmpfd);

    struct timeval timeout;
    timeout.tv_sec = 5; timeout.tv_usec = 0;

    if ( select(fd+1,NULL,&tmpfd,NULL,&timeout) <= 0 ) return FALSE;
  }

  // �������Ϊ0���Ͳ����ַ����ĳ���
  if (buflen==0) buflen=strlen(strWrite);

  if (buflen>TCPBUFLEN) return FALSE;

  buflen = buflen + 4;

  char strBufLen[5]; memset(strBufLen,0,sizeof(strBufLen));
  sprintf(strBufLen,"%04ld",buflen);

  if (Writen(fd,(char *)strBufLen,4) == FALSE)
  {
    return FALSE;
  }
//printf("=%s=\n",strBufLen);

  if (Writen(fd,strWrite,buflen-4) == FALSE)
  {
    return FALSE;
  }
//printf("=%s=\n",strWrite);

  return TRUE;
}

BOOL Readn(int fd,char *vptr,size_t n)
{
  int nLeft,nread,idx;

  nLeft = n;
  idx = 0;

  /*
  // ����sock�ķ��ͺͽ��յĳ�ʱ����ʱ�����ã�����ûʲô�á�
  struct timeval timeout;
  timeout.tv_sec = 3; timeout.tv_usec = 0;

  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)) == -1) return FALSE;
  */

  while(nLeft > 0)
  {
    if ( (nread = recv(fd,vptr + idx,nLeft,0)) <= 0) return FALSE;

    idx += nread;
    nLeft -= nread;
  }

  return TRUE;
}

BOOL Writen(int fd,const char *vptr,size_t n)
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
  Fclose();
}

void CFile::initdata()
{
  memset(m_fullfilename, 0, sizeof(m_fullfilename) );
  memset(m_filename, 0, sizeof(m_filename) );
  //memset( m_openmode, 0, sizeof(m_openmode) );

  m_fp = 0;

  m_bEnBuffer = FALSE;

  memset(m_tmppathname,0,sizeof(m_tmppathname));
  memset(m_pathname,0,sizeof(m_pathname));
  memset(m_tmpfullfilename,0,sizeof(m_tmpfullfilename));

  m_filetype=0; 
}

// ��һ���ļ����ڶ���filenameΪȫ·�����ļ�����
// openmodeΪ���ļ��ķ�ʽ��ֻ����"r"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
BOOL CFile::OpenForRead(const char *fullfilename,const char *openmode)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  Fclose();

  if ( (strcmp(openmode,"r")!=0) && (strcmp(openmode,"rb")!=0) ) return FALSE;

  if ( (m_fp=FOPEN(fullfilename,openmode)) == NULL ) return FALSE;

  strncpy(m_fullfilename,fullfilename,300);

  //strncpy(m_openmode,openmode,30);

  m_filetype=1;

  return TRUE;
}

// ��ȡ�ļ���ÿ�У�����ÿһ�еĻس����з�ȥ�����������ж�ÿ�е������Ƿ���strEndStr������
// �Ƿ���ÿ�е���󲹳�strPadStr�ַ�
BOOL CFile::FFGETS( char *strBuffer,const int ReadSize,const char *strEndStr,const char *strPadStr)
{
  if ( m_filetype != 1 ) return FALSE;

  return FGETS(strBuffer,ReadSize,m_fp,strEndStr,strPadStr);
}

// ����fread���ļ��ж�ȡ���ݡ�
size_t CFile::Fread(void *ptr, size_t size, size_t nitems)
{
  if ( m_filetype != 1 ) return 0;

  return fread(ptr,size,nitems,m_fp);
}


// ��һ���ļ�����д��fullfilenameΪ������·�����ļ���
// openmodeΪ���ļ��ķ�ʽ��ͬFOPENʹ�÷���һ�£�������"r"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
// bEnBuffer:TRUE-���û�������FALSE-�����û�����
BOOL CFile::OpenForWrite(const char *fullfilename, const char *openmode, BOOL bEnBuffer)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  Fclose();

  if ( (strcmp(openmode,"r")==0) || (strcmp(openmode,"rb")==0) ) return FALSE;

  if ( (m_fp = FOPEN(fullfilename, openmode )) == NULL ) return FALSE;

  strncpy( m_fullfilename, fullfilename, 300);
  //strncpy( m_openmode, openmode, 30);

  m_bEnBuffer = bEnBuffer;

  m_filetype=2;

  return TRUE;
}


// ��һ���ļ�����д��tmppathname�ļ���ŵ���ʱĿ¼��pathname�ļ���ŵ���ʽĿ¼��filenameΪ������·�����ļ���
// openmodeΪ���ļ��ķ�ʽ��ͬFOPENʹ�÷���һ�£�������"r"��"rb"������TRUE˵���򿪳ɹ���FALSE˵�����ļ�ʧ��
// bEnBuffer:TRUE-���û�������FALSE-�����û�����
BOOL CFile::OpenForRename(const char *tmppathname,const char *pathname,const char *filename,const char *openmode,BOOL bEnBuffer)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  Fclose();

  if ( (strcmp(openmode,"r")==0) || (strcmp(openmode,"rb")==0) ) return FALSE;

  snprintf(m_tmpfullfilename,300,"%s/%s.tmp",tmppathname,filename);
  snprintf(m_fullfilename   ,300,"%s/%s"    ,pathname,filename);

  if ( (m_fp = FOPEN(m_tmpfullfilename, openmode )) == NULL ) return FALSE;

  strncpy(m_filename, filename, 300);
  //strncpy(m_openmode, openmode, 30);

  m_bEnBuffer = bEnBuffer;

  m_filetype=3;

  return TRUE;
}

BOOL CFile::OpenForRename(const char *fullfilename,const char *openmode,BOOL bEnBuffer)
{
  // �ȹر��Ѵ��ļ�,�����ظ����ļ�
  Fclose();

  if ( (strcmp(openmode,"r")==0) || (strcmp(openmode,"rb")==0) ) return FALSE;

  snprintf(m_tmpfullfilename,300,"%s.tmp",fullfilename);
  strncpy(m_fullfilename,fullfilename,300);

  if ( (m_fp = FOPEN(m_tmpfullfilename, openmode )) == NULL ) return FALSE;

  strncpy(m_filename, fullfilename, 300);

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
void CFile::Fclose()
{
  if (m_fp == 0) return;

  fclose( m_fp );

  m_fp=0;

  if (m_filetype==3)
  {
    // ���ڸ������ļ�
    REMOVE(m_tmpfullfilename); 
  }

  initdata();
}

BOOL CFile::IsOpened()
{
  if (m_fp == 0) return FALSE;

  return TRUE;
}

// �ر��ļ�ָ�룬������ʱ�ļ�����Ϊ��ʽ�ļ������������ʧ�ܣ�������FALSE���ɹ�����TRUE
BOOL CFile::CloseAndRename()
{
  if (m_filetype!=3) return FALSE;

  if (m_fp==0) return FALSE;

  fclose( m_fp );

  m_fp=0;

  if (RENAME(m_tmpfullfilename,m_fullfilename) == FALSE) { initdata(); return FALSE; }

  initdata();

  return TRUE;
}


/*
const char *CFile::FullFileName()
{
  return m_fullfilename;
}

const char *CFile::FileName()
{
  return m_filename;
}
*/

// ����fwrite���ļ���д���ݣ���֪����ô��ģ��ú��������⣬��ʱ������
/*
size_t CFile::Fwrite(const void *ptr, size_t size, size_t nitems)
{
  if ( m_filetype == 1 ) return 0;

  size_t tt=fwrite(ptr,size,nitems,m_fp);

  if ( m_bEnBuffer == FALSE ) fflush(m_fp); 

  return tt;
}
*/

// �����Ѵ򿪵�ֻ���ļ�����ɾ������
BOOL CFile::CloseAndRemove()
{
  if (m_filetype!=1) return FALSE;
  
  if (m_fp==0) return FALSE;

  // �ȹر��ļ�
  if ( m_fp != 0 ) fclose( m_fp );

  if (REMOVE(m_fullfilename) == FALSE) { initdata(); return FALSE; }

  initdata();

  return TRUE;
}


// ɾ���ļ������ɾ��ʧ�ܣ��᳢��in_times��
BOOL REMOVE(const char *in_filename,const int in_times)
{
  // ����ļ������ڣ�ֱ�ӷ���ʧ��
  if (access(in_filename,R_OK) != 0) return FALSE;

  for (int ii=0;ii<in_times;ii++)
  {
    if (remove(in_filename) == 0) return TRUE;

    usleep(100000);
  }

  return FALSE;
}

// ��in_srcfilename����Ϊin_dstfilename���������ʧ�ܣ��᳢��in_times��
BOOL RENAME(const char *in_srcfilename,const char *in_dstfilename,const int in_times)
{
  // ����ļ������ڣ�ֱ�ӷ���ʧ��
  if (access(in_srcfilename,R_OK) != 0) return FALSE;

  if (MKDIR(in_dstfilename) == FALSE) return FALSE;

  /*
  // ���Ŀ¼�Ƿ���ڣ���������ڣ��𼶴�����Ŀ¼
  char strPathName[301];

  for (UINT ii=1; ii<strlen(in_dstfilename);ii++)
  {
    if (in_dstfilename[ii] != '/') continue;

    memset(strPathName,0,sizeof(strPathName));
    strncpy(strPathName,in_dstfilename,ii);

    if (access(strPathName,F_OK) == 0) continue;

    // ����Ŀ¼���ɹ�������-1������0��ע���FOPEN������
    if (mkdir(strPathName,00777) != 0) return FALSE;
  }
  */


  for (int ii=0;ii<in_times;ii++)
  {
    if (rename(in_srcfilename,in_dstfilename) == 0) return TRUE;

    usleep(100000);
  }

  return FALSE;
}

// ���ļ�������ļ���Ŀ¼�����ڣ��ʹ�����Ŀ¼�����ǣ�����򿪷�ʽΪ"r"��"rb"���Ͳ��ᴴ��Ŀ¼��
FILE *FOPEN(const char *filename,const char *mode)
{
  if ((strcmp(mode,"r") != 0) && (strcmp(mode,"rb") != 0))
  {
    if (MKDIR(filename) == FALSE) return 0;

    /*
    // ���Ŀ¼�Ƿ���ڣ���������ڣ��𼶴�����Ŀ¼
    char strPathName[301];

    for (UINT ii=1; ii<strlen(filename);ii++)
    {
      if (filename[ii] != '/') continue;

      memset(strPathName,0,sizeof(strPathName));
      strncpy(strPathName,filename,ii);

      if (access(strPathName,F_OK) == 0) continue;

      // ����Ŀ¼���ɹ������ؿյ��ļ�ָ�롣
      if (mkdir(strPathName,00777) != 0) return 0;
    }
    */
  }

  return fopen(filename,mode);
}

int OPEN(char *filename, int flags)
{
  if (MKDIR(filename) == FALSE) return -1;

  /*
  // ���Ŀ¼�Ƿ���ڣ���������ڣ��𼶴�����Ŀ¼
  char strPathName[301];

  for (UINT ii=1; ii<strlen(filename);ii++)
  {
    if (filename[ii] != '/') continue;

    memset(strPathName,0,sizeof(strPathName));
    strncpy(strPathName,filename,ii);

    if (access(strPathName,F_OK) == 0) continue;

    // ����Ŀ¼���ɹ�������-1������0��ע���FOPEN������
    if (mkdir(strPathName,00777) != 0) return -1;
  }
  */

  return open(filename,flags);
}

int OPEN(const char *filename, int flags, mode_t mode)
{
  if (MKDIR(filename) == FALSE) return -1;

  /*
  // ���Ŀ¼�Ƿ���ڣ���������ڣ��𼶴�����Ŀ¼
  char strPathName[301];

  for (UINT ii=1; ii<strlen(filename);ii++)
  {
    if (filename[ii] != '/') continue;

    memset(strPathName,0,sizeof(strPathName));
    strncpy(strPathName,filename,ii);

    if (access(strPathName,F_OK) == 0) continue;

    // ����Ŀ¼���ɹ�������-1������0��ע���FOPEN������
    if (mkdir(strPathName,00777) != 0) return -1;
  }
  */

  return open(filename,flags,mode);
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

    if (mkdir(strPathName,00777) != 0) return FALSE;
  }

  if (bisfilename==FALSE)
  {
    if (access(filename,F_OK) != 0) 
    {
      if (mkdir(filename,00777) != 0) return FALSE;
    }
  }

  return TRUE;
}

// ��ĳһ���ļ����Ƶ���һ���ļ�
BOOL COPY(const char *srcfilename,const char *dstfilename)
{
  if (MKDIR(dstfilename) == FALSE) return FALSE;

  char strdstfilenametmp[301];
  memset(strdstfilenametmp,0,sizeof(strdstfilenametmp));
  snprintf(strdstfilenametmp,300,"%s.tmp",dstfilename);

  int  srcfd,dstfd;

  srcfd=dstfd=-1;

  int iFileSize=FileSize(srcfilename);

  int  bytes=0;
  int  total_bytes=0;
  int  onread=0;
  char buffer[5000];

  if ( (srcfd=open(srcfilename,O_RDONLY)) < 0 ) return FALSE;

  if ( (dstfd=open(strdstfilenametmp,O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IXUSR)) < 0) { close(srcfd); return FALSE; }

  while (TRUE)
  {
    memset(buffer,0,sizeof(buffer));

    if ((iFileSize-total_bytes) > 5000) onread=5000;
    else onread=iFileSize-total_bytes;

    bytes=read(srcfd,buffer,onread);

    if (bytes > 0) write(dstfd,buffer,bytes);

    total_bytes = total_bytes + bytes;

    if (total_bytes == iFileSize) break;
  }

  close(srcfd);

  close(dstfd);

  // �����ļ����޸�ʱ������
  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));
  FileModTime(srcfilename,strmtime);
  UTime(strdstfilenametmp,strmtime);

  if (RENAME(strdstfilenametmp,dstfilename) == FALSE) return FALSE;

  return TRUE;
}

// �����ļ����޸�ʱ������
int UTime(const char *filename,const char *mtime)
{
  struct utimbuf stutimbuf;
  
  //stutimbuf.actime=stutimbuf.modtime=UTCTime(mtime)-8*60*60;;
  stutimbuf.actime=stutimbuf.modtime=UTCTime(mtime);

  return utime(filename,&stutimbuf);
}


// �Ѷȶȶȷַ������ʽ���ַ���ת��Ϊ��λΪ�ȵ��ַ�����
// strAzimuth��ʾ���Ȼ�γ�ȣ����ȵĳ��ȱ���7��γ�ȵĳ��ȱ�����6λ��
// strNumber���8λ�ĸ���������ȷ��С�����5λ��
BOOL AzimuthToNumber(char *strAzimuth,char *strNumber)
{
  strcpy(strNumber,"");

  if ( (strlen(strAzimuth)!=6) && (strlen(strAzimuth)!=7) ) return FALSE;

  char strdd[4],strmi[3],strss[3];
  memset(strdd,0,sizeof(strdd));
  memset(strmi,0,sizeof(strmi));
  memset(strss,0,sizeof(strss));

  strncpy(strss,strAzimuth+strlen(strAzimuth)-2,2);
  strncpy(strmi,strAzimuth+strlen(strAzimuth)-4,2);
  strncpy(strdd,strAzimuth,strlen(strAzimuth)-4);

  sprintf(strNumber,"%3.3f",atof(strdd)+atof(strmi)/60.0+atof(strss)/3600.0);

  return TRUE;
}

/*
#ifndef RM
#define RM 6374131.3
#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif
*/


// lon_rd�״�ľ��ȣ�lat_rd�״��γ�ȣ�h_rd�״�ĸ߶�
// ָ����㾭��lon_p��γ��lat_p���߶�h_p
// angle���״�վ��ָ��������ӵ��ĵļнǣ�б��range������elev����λ��azim
int get_sea(double lon_rd, double lat_rd, double h_rd ,
            double lon_p , double lat_p , double h_p  ,
            double *angle, double *range, double *elev, double *azim)
{
  double delt;
  double rbi, rbj, rbk, rdi, rdj, rdk, tmp;
  double z, h;
  double i_lon, j_lat;
  double coss, max_r=800000;
  double RM = 6374131.3;    //Radius of Earth
  double PI = 3.14159265358979323846;

  // �жϾ�γ�ȵĺϷ���
  if ( (lon_rd>180.000001)||(lon_rd<-0.000001)||(lat_rd>90.000001)||(lat_rd<-0.000001) ) return -1;
  if ( (lon_p >180.000001)||(lon_p <-0.000001)||(lat_p >90.000001)||(lat_p <-0.000001) ) return -1;

  z=h_p+RM;
  h=h_rd+RM;

  lon_rd *= PI/180;
  lat_rd *= PI/180;
  i_lon  = lon_p * PI/180;
  j_lat  = lat_p * PI/180;

  // delt  longitude difference between radar and point to be interpolated
  delt   = i_lon - lon_rd;
  coss   = cos(j_lat)*cos(lat_rd)*cos(delt)+sin(j_lat)*sin(lat_rd);

  *angle   = acos(coss);
  rbi=cos(j_lat)*cos(delt)/coss-cos(lat_rd);
  rbj=cos(j_lat)*sin(delt)/coss;
  rbk=sin(j_lat)/coss-sin(lat_rd);
  rdi=-cos(lat_rd);
  rdj=0;
  rdk=1./sin(lat_rd)-sin(lat_rd);

  tmp=(rdi*rbi+rdj*rbj+rdk*rbk)/sqrt(rdi*rdi+rdj*rdj+rdk*rdk)/sqrt(rbi*rbi+rbj*rbj+rbk*rbk);

  *azim=acos(tmp);

  (*azim)*=180/PI;

  if(delt<-0.000001)
    *azim=360-*azim;

  if ( (*azim-360 >-0.000001) && (*azim-360 <+0.000001) )
    *azim=0;

  // ����*azimΪnan����������*azimΪnan����ǿ��Ϊ0
  // ע�⣬��azimΪnanʱ������������ֵ�Ƚ϶��᷵�ؼ٣�����Ҫ����!���㡣
  if (!(*azim<360.000001)) *azim=0;

  // h_p  distance between point A(i,j,k) and earth's center
  *range = sqrt(h*h+z*z-2*h*z*coss);

  if(*range>max_r)
    return 0;
  else if(*range>0.000001) //Ipara
    *elev  = acos(z*sin(*angle)/(*range))*180/PI;
  else
    return 0;  //skip when srange is small?

  if(z*coss<h)  (*elev)*=-1;

  (*angle)*=180/PI;

  return 0;
}


// get position of A according to radar position and monitiored s/e/a of A
// lon_rd�״�ľ��ȣ�lat_rd�״��γ�ȣ�h_rd�״�ĸ߶ȣ���λ���ף�
// Ŀ�ĵ��rangб�࣬elev���ǣ�azim��λ��
// Ŀ�ĵ��lon_p���ȣ�lat_pγ�ȣ�h_p�߶ȣ���λ���ף�
int get_llh(double lon_rd, double lat_rd, double h_rd,
            double rang  , double elev  , double azim,
            double *lon_p, double *lat_p, double *h_p)
{
  double angle, d;
  double temp;

  double RM = 6374131.3;    //Radius of Earth
  double PI = 3.14159265358979323846;

  // �жϾ�γ�ȵĺϷ���
  if ( (lon_rd>180.000001)||(lon_rd<-0.000001)||(lat_rd>90.000001)||(lat_rd<-0.000001) ) return -1;
  // �ж�б��ĺϷ���
  if (rang<-0.000001) return -1;
  // �ж����Ǻͷ�λ�ǵĺϷ���
  if ( (azim>360.000001)||(azim<-0.000001)||(elev>90.000001)||(elev<-0.000001) ) return -1;

  lon_rd  *=  PI/180;
  lat_rd  *=  PI/180;
  elev  *=  PI/180;
  azim  *=  PI/180;

  angle=atan(rang*cos(elev)/(RM+h_rd+rang*sin(elev)));

  // get A's height
  *h_p=(RM+h_rd+rang*sin(elev))/cos(angle)-RM;

  // get A's latitude
  if(fabs(cos(azim)-1)<0.000001)    // to the north
  {
    *lat_p=lat_rd+angle;
    d=0;
  }
  else if(fabs(cos(azim)+1)<0.000001)  // to the south
  {
    *lat_p=lat_rd-angle;
    d=0;
  }
  else
  {
    *lat_p=asin(cos(angle)*sin(lat_rd)+sin(angle)*cos(lat_rd)*cos(azim));
    temp=(cos(angle)-sin(lat_rd)*sin(*lat_p))/cos(lat_rd)/cos(*lat_p);
    d=acos(temp);
  }

  if(azim>PI) d*=-1;    // A is to the west of SHENZHEN radar


  // get A's longitude
  *lon_p  = lon_rd+d;
  *lat_p *= 180/PI;
  *lon_p *= 180/PI;

  return 0;
}

// �ж��Ƿ���Ҫ����buffer�е�ʱ�����
// ���Դ�������ʱ�������{yyyy}��4λ���꣩��{yyy}������λ���꣩��
// {yy}������λ���꣩��{mm}�����£���{dd}�����գ���{hh}��ʱʱ����{mi}���ַ֣���{ss}�����룩
void MatchBuffer(char *buffer,int timetvl)
{
  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss",0+timetvl*60*60);

  char YYYY[5],YYY[4],YY[3],MM[3],DD[3],HH[3],MI[3],SS[3];

  memset(YYYY,0,sizeof(YYYY));
  memset(YYY,0,sizeof(YYY));
  memset(YY,0,sizeof(YY));
  memset(MM,0,sizeof(MM));
  memset(DD,0,sizeof(DD));
  memset(HH,0,sizeof(HH));
  memset(MI,0,sizeof(MI));
  memset(SS,0,sizeof(SS));

  strncpy(YYYY,strLocalTime,4);
  strncpy(YYY,strLocalTime+1,3);
  strncpy(YY,strLocalTime+2,2);
  strncpy(MM,strLocalTime+4,2);
  strncpy(DD,strLocalTime+6,2);
  strncpy(HH,strLocalTime+8,2);
  strncpy(MI,strLocalTime+10,2);
  strncpy(SS,strLocalTime+12,2);


  UpdateStr(buffer,"{YYYY}",YYYY);
  UpdateStr(buffer,"{YYY}",YYY);
  UpdateStr(buffer,"{YY}",YY);
  UpdateStr(buffer,"{MM}",MM);
  UpdateStr(buffer,"{DD}",DD);
  UpdateStr(buffer,"{HH}",HH);
  UpdateStr(buffer,"{MI}",MI);
  UpdateStr(buffer,"{SS}",SS);


  UpdateStr(buffer,"{yyyy}",YYYY);
  UpdateStr(buffer,"{yyy}",YYY);
  UpdateStr(buffer,"{yy}",YY);
  UpdateStr(buffer,"{mm}",MM);
  UpdateStr(buffer,"{dd}",DD);
  UpdateStr(buffer,"{hh}",HH);
  UpdateStr(buffer,"{mi}",MI);
  UpdateStr(buffer,"{ss}",SS);

  // logfile.Write("xmlbuffer=%s\n",buffer);
}

// ѹ���ļ�Ϊzip��ʽ
// ���ܵ���CloseIOAndSignal�ú���������ZIPFiles�������ò��ܳɹ�
void ZIPFiles(char *strFileName,BOOL bKeep)
{
  char strPathName[301];
  memset(strPathName,0,sizeof(strPathName));

  int ii=0;

  for (ii=strlen(strFileName)-1; ii>=0;ii--)
  {
    if (strFileName[ii] == '/') 
    {
      strncpy(strPathName,strFileName,ii);
      chdir(strPathName);
      break;
    }
  }

  if (ii==0) return;

  char strTmpFileName[301],strZipFileName[301];

  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strZipFileName,0,sizeof(strZipFileName));

  snprintf(strTmpFileName,300,"%s.tmp",strFileName+ii+1);
  snprintf(strZipFileName,300,"%s.zip",strFileName+ii+1);

  char strCmd[1024];
  memset(strCmd,0,sizeof(strCmd));
  snprintf(strCmd,1000,"/usr/bin/zip %s %s 1>/dev/null 2>/dev/null",strTmpFileName,strFileName+ii+1);
  system(strCmd);

  RENAME(strTmpFileName,strZipFileName);

  if (bKeep==TRUE) return;

  REMOVE(strFileName);
}

// ���ַ���ת��Ϊdouble
double ATOF(const char *nptr)
{
  if (atof(nptr)>0.00001) return atof(nptr)+0.000001;
  if (atof(nptr)<0.00001) return atof(nptr)-0.000001;

  return atof(nptr);
}

// �Ѷ����Ƶ��ַ���ת��Ϊʮ����
int bintodec(char *strbin,int length)
{
  int i,j,sum,s;
  i=j=sum=s=0;

  for(i=0;i<length;i++)
  {
    if(strbin[i]=='1')
    {
      s=1;
      for(j=length-i-1;j>0;j--)s=s*2;
      sum+=s;
    }
  }

  return sum;
}

// ��SendLog�ļ��ж�ȡ�Ѵ����¼��λ��
long ReadLogFile(const char *strFileName)
{
  char strLogFileName[301];
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,300,"%s.log",strFileName);

  FILE *fpLog;

  if ( (fpLog=FOPEN(strLogFileName,"r")) == 0 ) return 0;

  char strBuf[301];
  memset(strBuf,0,sizeof(strBuf));

  fseek(fpLog,-256L,2);
  fread(strBuf,1,256,fpLog);
  fclose(fpLog);

  return atol(strBuf);
}

// �ѱ����Ѵ�����ļ���λ��д����־�ļ�
BOOL WriteToLogFile(FILE *fpSend,const char *strFileName)
{
  char strLogFileName[301];
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,300,"%s.log",strFileName);

  FILE *fpLog=0;

  if ( (fpLog=FOPEN(strLogFileName,"w")) == 0 )
  {
    sleep(1); if ( (fpLog=FOPEN(strLogFileName,"w")) == 0 ) return FALSE;
  }

  fprintf(fpLog,"%ld",ftell(fpSend));

  fclose(fpLog);

  return TRUE;
}

// �ѱ����Ѵ�����ɵĵ���־�ļ�ɾ��
BOOL RemoveLogFile(char *strFileName)
{
  char strLogFileName[301];
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,300,"%s.log",strFileName);

  if (REMOVE(strLogFileName) == FALSE) return FALSE;

  return TRUE;
}

// ���ļ��м���һ����¼������ļ�û���¼�¼��strBuffer��Ϊ��
BOOL LoadOneLine(const char *strFileName,char *strBuffer)
{
  CFile File;

  if (File.OpenForRead(strFileName,"r")==FALSE) return FALSE;

  // �ӷ�����־�ļ��ж�ȡ�Ѵ����¼��λ��
  long POS=ReadLogFile(strFileName);

  // ��λ���ϴ��Ѵ������¼��λ��
  fseek(File.m_fp,POS,0);

  char strLine[2048];

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    if (FGETS(strLine,2000,File.m_fp) == FALSE) return FALSE;

    if (strlen(strLine)>1) break;
  }

  strcpy(strBuffer,strLine);

  // �ѱ����Ѵ�����ļ���λ��д����־�ļ�
  WriteToLogFile(File.m_fp,strFileName);
  
  return TRUE;
}

// �ѷ���ת��Ϊ�ȼ�
// wf-���٣���λ��0.1m/s��wdj-���ٵȼ���2-С��������3-������4-�ļ�...12-ʮ������13-����12��
BOOL wftowdj(char *in_wf,char *out_wdj)
{
  if (strlen(in_wf)==0) { out_wdj[0]=0; return TRUE; }

  int wf=0;

  wf=atoi(in_wf);

  // ��������
  if ( (wf  >= 00) && (wf  <=33) )   strcpy(out_wdj,"2");
  // ����
  if ( (wf  >= 34) && (wf  <=54) )   strcpy(out_wdj,"3");
  // �ļ�
  if ( (wf  >= 55) && (wf  <=79) )   strcpy(out_wdj,"4");
  // �弶
  if ( (wf  >= 80) && (wf  <=107) )  strcpy(out_wdj,"5");
  // ����
  if ( (wf  >= 108) && (wf  <=138) ) strcpy(out_wdj,"6");
  // �߼�
  if ( (wf  >= 139) && (wf  <=171) ) strcpy(out_wdj,"7");
  // �˼�
  if ( (wf  >= 172) && (wf  <=207) ) strcpy(out_wdj,"8");
  // �ż�
  if ( (wf  >= 208) && (wf  <=244) ) strcpy(out_wdj,"9");
  // ʮ��
  if ( (wf  >= 245) && (wf  <=284) ) strcpy(out_wdj,"10");
  // ʮһ��
  if ( (wf  >= 285) && (wf  <=326) ) strcpy(out_wdj,"11");
  // ʮ����
  if ( (wf  >= 327) && (wf  <=369) ) strcpy(out_wdj,"12");
  // ʮ��������
  if ( (wf  >= 370) && (wf  <=800) ) strcpy(out_wdj,"13");

  return TRUE;
}

int conv_charset(const char *dest, const char *src, char *input, size_t inlen, char *output, size_t outlen)
{
  if(strlen(src) == 0 || strlen(dest) == 0)  return -1;

  char **inbuf = &input;
  char **outbuf = &output;

  iconv_t conv = iconv_open(dest, src);

  if (conv == (iconv_t)-1) return -2;

  memset(output, 0, outlen);

  if (iconv(conv, inbuf, &inlen, outbuf, &outlen) == (size_t)-1)  return -3;

  iconv_close(conv);

  return 0;
}

// ���ô˺�������Ҫ���÷�������hostname����hosts�ļ������
BOOL getLocalIP(char *localip)
{
  char hostname[128];
  struct hostent *hent;

  memset(hostname,0,sizeof(hostname));
  gethostname(hostname, sizeof(hostname));

  if(strlen(hostname) == 0) return FALSE;

  hent = gethostbyname(hostname);

  snprintf(localip,20,"%s",inet_ntoa(*(struct in_addr*)(hent->h_addr_list[0])));

  return TRUE;
}
