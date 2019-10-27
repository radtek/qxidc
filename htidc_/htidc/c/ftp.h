#ifndef __FTP_H
#define __FTP_H

#include "_public.h"
#include "ftplib.h"

class Cftp
{
public:
  netbuf *m_ftpconn;

  int  m_size;
  char m_mtime[21];

  Cftp();
 ~Cftp();

  BOOL m_connectfailed;
  BOOL m_loginfailed;
  BOOL m_optionfailed;

  void initdata();

  BOOL login(const char *in_ip,const int in_port,const char *in_username,const char *in_password,const int in_mode=FTPLIB_PASSIVE);
  
  BOOL logout();

  BOOL mtime(const char *in_remotefilename);

  BOOL size(const char *in_remotefilename);

  BOOL site(const char *in_command);

  BOOL chdir(const char *in_remotedir);

  BOOL mkdir(const char *in_remotedir);

  BOOL rmdir(const char *in_remotedir);

  // 该函数用处不大
  // BOOL dir(const char *in_remotedir,const char *out_listfilename);

  // 如果是列出当前目录，in_remotedir用"","*","."都行。
  BOOL nlist(const char *in_remotedir,const char *out_listfilename);

  BOOL get(const char *in_remotefilename,const char *in_localfilename,const BOOL bCheckSize=TRUE);

  BOOL put(const char *in_localfilename,const char *in_remotefilename,const BOOL bCheckSize=TRUE);

  BOOL ftpdelete(const char *in_remotefilename);

  BOOL ftprename(const char *in_srcremotefilename,const char *in_dstremotefilename);

  char *response();
};

#endif
