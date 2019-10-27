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

  char m_username[51];     // Զ�̷������û�����
  char m_password[51];     // Զ�̷��������롣
  char m_hostip[101];      // Զ�̷�����IP��
  int  m_port;             // Զ�̷�����SSH�˿ڡ�

  static int waitsocket(int socket_fd, LIBSSH2_SESSION *session);

  Cssh();
 ~Cssh();

  // ��ʼ��
  int ssh_init(char *username,char *password,char *hostip,int prot=22);

  // ����Զ�̷�����
  int ssh_connect();

  // ����־
  void ssh_bindlog(CLogFile *logfile);

  // ִ��Զ������
  int ssh_write(char *cmdstr);

  // ��ȡ���ؽ��
  int ssh_read(char *outbuffer);

  // �˳�
  void ssh_close();
};

#endif
