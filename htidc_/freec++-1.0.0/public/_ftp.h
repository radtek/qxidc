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
  BOOL m_connectfailed;
  BOOL m_loginfailed;
  BOOL m_optionfailed;

  void initdata();

  // ��¼ftp������
  // in_host ��������ַ�Ͷ˿ڣ��м���":"�ָ�����"192.168.1.1:21"
  // in_username ftp�û���
  // in_password ftp������
  // in_mode ����ģʽ��FTPLIB_PASSIVE�Ǳ���ģʽ��FTPLIB_PORT������ģʽ
  BOOL login(const char *in_host,const char *in_username,const char *in_password,const int in_mode=FTPLIB_PASSIVE);
  
  // ע��
  BOOL logout();

  // ��ȡftp���������ļ���ʱ��
  BOOL mtime(const char *in_remotefilename);

  // ��ȡftp���������ļ��Ĵ�С
  BOOL size(const char *in_remotefilename);

  // �����˷���site����
  BOOL site(const char *in_command);

  // �ı�ftpԶ��Ŀ¼
  BOOL chdir(const char *in_remotedir);

  // ��ftp�������ϴ���Ŀ¼
  BOOL mkdir(const char *in_remotedir);

  // ɾ��ftp�������ϵ�Ŀ¼
  BOOL rmdir(const char *in_remotedir);

  // ����list�����г�ftp������Ŀ¼�е��ļ���������浽�����ļ���
  // ������г���ǰĿ¼��in_remotedir��"","*","."���С�
  BOOL nlist(const char *in_remotedir,const char *out_listfilename);

  // ����dir�����г�ftp������Ŀ¼�е��ļ���������浽�����ļ���
  BOOL dir(const char *in_remotedir,const char *out_listfilename);

  // ��ftp�������ϻ�ȡ�ļ�
  // in_remotefilename ����ȡ��Զ���ļ���
  // in_localfilename  �����ļ�����������in_remotefilename��ͬ
  // bCheckSize �ļ�������ɺ��Ƿ�˶Ա��غ�Զ���ļ��Ĵ�С����֤�ļ���������
  // ע�⣬�ļ��ڴ���Ĺ����У�������ʱ�ļ������ķ���������in_localfilename���".tmp"���ڴ���
  // ��ɺ����ʽ��Ϊin_localfilename
  BOOL get(const char *in_remotefilename,const char *in_localfilename,const BOOL bCheckSize=TRUE);

  // ��ftp�����������ļ�
  // in_localfilename ���ش����͵��ļ���
  // in_remotefilename Զ���ļ���
  // bCheckSize �ļ�������ɺ��Ƿ�˶Ա��غ�Զ���ļ��Ĵ�С����֤�ļ���������
  // ע�⣬�ļ��ڴ���Ĺ����У�������ʱ�ļ������ķ���������in_remotefilename���".tmp"���ڴ���
  // ��ɺ����ʽ��Ϊin_remotefilename
  BOOL put(const char *in_localfilename,const char *in_remotefilename,const BOOL bCheckSize=TRUE);

  // ɾ��ftp�������ϵ��ļ�
  BOOL ftpdelete(const char *in_remotefilename);

  // ��ftp�������ϵ��ļ�����
  BOOL ftprename(const char *in_srcremotefilename,const char *in_dstremotefilename);

  // return a pointer to the last response received
  char *response();
};

#endif
