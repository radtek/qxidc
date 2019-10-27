#ifndef __SSH_H
#define __SSH_H

#include "_public.h"
#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

class Cssh
{
public:
  int m_sock;
  struct sockaddr_in sockin;

  LIBSSH2_SESSION * m_session;
  LIBSSH2_CHANNEL * m_channel;

  CLogFile  *m_logfile;

  char m_username[51];     // 远程服务器用户名。
  char m_password[51];     // 远程服务器密码。
  char m_hostip[101];      // 远程服务器IP。
  int  m_port;             // 远程服务器SSH端口。

  static int waitsocket(int socket_fd, LIBSSH2_SESSION *session);

  Cssh();
 ~Cssh();

  // 初始化
  int ssh_init(char *username,char *password,char *hostip,int prot=22);

  // 连接远程服务器
  int ssh_connect();

  // 绑定日志
  void ssh_bindlog(CLogFile *logfile);

  // 执行远程命令
  int ssh_write(char *cmdstr);

  // 读取返回结果
  int ssh_read(char *outbuffer);

  // 退出
  void ssh_close();
};

#endif
