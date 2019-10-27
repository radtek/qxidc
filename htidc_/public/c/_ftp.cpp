////////////////////////////////////////////////////////

#include "_ftp.h"

Cftp::Cftp()
{
  m_ftpconn=0;

  initdata();

  FtpInit();

  m_connectfailed=false;
  m_loginfailed=false;
  m_optionfailed=false;
}

Cftp::~Cftp()
{
  logout();
}

void Cftp::initdata()
{
  m_size=0;

  memset(m_mtime,0,sizeof(m_mtime));
}

bool Cftp::login(const char *in_host,const char *in_username,const char *in_password,const int in_mode)
{
  if (m_ftpconn != 0) { FtpQuit(m_ftpconn); m_ftpconn=0; }

  m_connectfailed=m_loginfailed=m_optionfailed=false;

  if (FtpConnect(in_host,&m_ftpconn) == false)  { m_connectfailed=true; return false; }

  if (FtpLogin(in_username,in_password,m_ftpconn) == false)  { m_loginfailed=true; return false; }

  if (FtpOptions(FTPLIB_CONNMODE,(long)in_mode,m_ftpconn) == false) { m_optionfailed=true; return false; }

  return true;
}

bool Cftp::logout()
{
  if (m_ftpconn == 0) return false;

  FtpQuit(m_ftpconn);

  m_ftpconn=0;

  return true;
}

bool Cftp::get(const char *in_remotefilename,const char *in_localfilename,const bool bCheckMTime)
{
  if (m_ftpconn == 0) return false;

  // ���������ļ�Ŀ¼
  MKDIR(in_localfilename);

  char strlocalfilenametmp[301];
  memset(strlocalfilenametmp,0,sizeof(strlocalfilenametmp));
  snprintf(strlocalfilenametmp,300,"%s.tmp",in_localfilename);

  // ��ȡԶ�̷��������ļ���ʱ��
  if (mtime(in_remotefilename) == false) return false;

  // ȡ�ļ�
  if (FtpGet(strlocalfilenametmp,in_remotefilename,FTPLIB_IMAGE,m_ftpconn) == false) return false;
  
  // �ж��ļ���ȡǰ�ͻ�ȡ���ʱ�䣬���ʱ�䲻ͬ����ʾ�ļ��Ѹı䣬����ʧ��
  if (bCheckMTime==false)
  {
    char strmtime[21];
    strcpy(strmtime,m_mtime);

    if (mtime(in_remotefilename) == false) return false;

    if (strcmp(m_mtime,strmtime) != 0) return false;
  }

  // �����ļ�ʱ��
  UTime(strlocalfilenametmp,m_mtime);

  // ��Ϊ��ʽ���ļ�
  if (rename(strlocalfilenametmp,in_localfilename) != 0) return false; 

  // ��ȡ�ļ��Ĵ�С
  m_size=FileSize(in_localfilename);

  return true;
}

bool Cftp::mtime(const char *in_remotefilename)
{
  if (m_ftpconn == 0) return false;
  
  memset(m_mtime,0,sizeof(m_mtime));
  
  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));

  if (FtpModDate(in_remotefilename,strmtime,14,m_ftpconn) == false) return false;

  AddTime(strmtime,m_mtime,0+8*60*60,"yyyymmddhh24miss");

  return true;
}

bool Cftp::size(const char *in_remotefilename)
{
  if (m_ftpconn == 0) return false;

  m_size=0;
  
  if (FtpSize(in_remotefilename,&m_size,FTPLIB_IMAGE,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::site(const char *in_command)
{
  if (m_ftpconn == 0) return false;
  
  if (FtpSite(in_command,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::chdir(const char *in_remotedir)
{
  if (m_ftpconn == 0) return false;
  
  if (FtpChdir(in_remotedir,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::mkdir(const char *in_remotedir)
{
  if (m_ftpconn == 0) return false;
  
  if (FtpMkdir(in_remotedir,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::rmdir(const char *in_remotedir)
{
  if (m_ftpconn == 0) return false;
  
  if (FtpRmdir(in_remotedir,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::dir(const char *in_remotedir,const char *out_listfilename)
{
  if (m_ftpconn == 0) return false;
  
  if (FtpDir(out_listfilename,in_remotedir,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::nlist(const char *in_remotedir,const char *out_listfilename)
{
  if (m_ftpconn == 0) return false;

  // ���������ļ�Ŀ¼
  MKDIR(out_listfilename);
  
  if (FtpNlst(out_listfilename,in_remotedir,m_ftpconn) == false) return false;

  return true;
}

bool Cftp::put(const char *in_localfilename,const char *in_remotefilename,const bool bCheckSize)
{
  if (m_ftpconn == 0) return false;

  char strremotefilenametmp[301];
  memset(strremotefilenametmp,0,sizeof(strremotefilenametmp));
  snprintf(strremotefilenametmp,300,"%s.tmp",in_remotefilename);

  if (FtpPut(in_localfilename,strremotefilenametmp,FTPLIB_IMAGE,m_ftpconn) == false) return false;

  if (FtpRename(strremotefilenametmp,in_remotefilename,m_ftpconn) == false) return false;

  // �ж����ϴ����ļ��Ĵ�С�뱾���ļ��Ƿ���ͬ��ȷ���ϴ��ɹ���
  if (bCheckSize==true)
  {
    if (size(in_remotefilename) == false) return false;

    if (m_size != FileSize(in_localfilename)) return false; 
  }

  return true;
}

bool Cftp::ftpdelete(const char *in_remotefilename)
{
  if (m_ftpconn == 0) return false;

  if (FtpDelete(in_remotefilename,m_ftpconn) == false) return false;
  
  return true;
}

bool Cftp::ftprename(const char *in_srcremotefilename,const char *in_dstremotefilename)
{
  if (m_ftpconn == 0) return false;

  if (FtpRename(in_srcremotefilename,in_dstremotefilename,m_ftpconn) == false) return false;
  
  return true;
}

char *Cftp::response()
{
  if (m_ftpconn == 0) return 0;

  return FtpLastResponse(m_ftpconn);
}

