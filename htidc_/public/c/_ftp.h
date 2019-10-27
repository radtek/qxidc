////////////////////////////////////////////////////////

#ifndef __FTP_H
#define __FTP_H

#include "_public.h"
#include "ftplib.h"

class Cftp
{
public:
  // ftp���Ӿ��
  netbuf *m_ftpconn;

  // �ļ��Ĵ�С
  unsigned int m_size;

  // �ļ���ʱ��modifytime
  char m_mtime[21];

  Cftp();
 ~Cftp();

  // ���login()������¼ʧ�ܵ�ԭ��
  bool m_connectfailed;
  bool m_loginfailed;
  bool m_optionfailed;

  void initdata();

  // ��¼ftp������
  // in_host ��������ַ�Ͷ˿ڣ��м���":"�ָ�����"192.168.1.1:21"
  // in_username ftp�û���
  // in_password ftp������
  // in_mode ����ģʽ��FTPLIB_PASSIVE�Ǳ���ģʽ��FTPLIB_PORT������ģʽ
  bool login(const char *in_host,const char *in_username,const char *in_password,const int in_mode=FTPLIB_PASSIVE);
  
  // ע��
  bool logout();

  // ��ȡftp���������ļ���ʱ��
  bool mtime(const char *in_remotefilename);

  // ��ȡftp���������ļ��Ĵ�С
  bool size(const char *in_remotefilename);

  // �����˷���site����
  bool site(const char *in_command);

  // �ı�ftpԶ��Ŀ¼
  bool chdir(const char *in_remotedir);

  // ��ftp�������ϴ���Ŀ¼
  bool mkdir(const char *in_remotedir);

  // ɾ��ftp�������ϵ�Ŀ¼
  bool rmdir(const char *in_remotedir);

  // ����list�����г�ftp������Ŀ¼�е��ļ���������浽�����ļ���
  // ������г���ǰĿ¼��in_remotedir��"","*","."���С�
  bool nlist(const char *in_remotedir,const char *out_listfilename);

  // ����dir�����г�ftp������Ŀ¼�е��ļ���������浽�����ļ���
  bool dir(const char *in_remotedir,const char *out_listfilename);

  // ��ftp�������ϻ�ȡ�ļ�
  // in_remotefilename ����ȡ��Զ���ļ���
  // in_localfilename  �����ļ�����������in_remotefilename��ͬ
  // bCheckMTime �ļ�������ɺ��Ƿ�˶��ļ�����ǰ���ʱ�䣬��֤�ļ���������
  // ע�⣬�ļ��ڴ���Ĺ����У�������ʱ�ļ������ķ���������in_localfilename���".tmp"���ڴ���
  // ��ɺ����ʽ��Ϊin_localfilename
  bool get(const char *in_remotefilename,const char *in_localfilename,const bool bCheckMTime=true);

  // ��ftp�����������ļ�
  // in_localfilename ���ش����͵��ļ���
  // in_remotefilename Զ���ļ���
  // bCheckSize �ļ�������ɺ��Ƿ�˶Ա��غ�Զ���ļ��Ĵ�С����֤�ļ���������
  // ע�⣬�ļ��ڴ���Ĺ����У�������ʱ�ļ������ķ���������in_remotefilename���".tmp"���ڴ���
  // ��ɺ����ʽ��Ϊin_remotefilename
  bool put(const char *in_localfilename,const char *in_remotefilename,const bool bCheckSize=true);

  // ɾ��ftp�������ϵ��ļ�
  bool ftpdelete(const char *in_remotefilename);

  // ��ftp�������ϵ��ļ�����
  bool ftprename(const char *in_srcremotefilename,const char *in_dstremotefilename);

  // ��ȡ������������Ϣ�����һ�� return a pointer to the last response received
  char *response();
};

#endif
