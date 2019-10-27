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

#ifndef __OOCI_H
#define __OOCI_H

// C/C++�ⳣ��ͷ�ļ�
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// oracle���ݿ�ӿڿ�ͷ�ļ�
#include <oci.h>

// OCI��¼����
struct LOGINENV
{
  char user[32];     // ���ݿ���û���
  char pass[32];     // ���ݿ������
  char tnsname[51];  // ���ݿ��tnsname����ORACLE_HOME/network/admin/tnsnames.ora������

  OCIEnv *envhp; // ���������ľ��
};

// OCI������
struct OCI_CXT
{
  OCISvcCtx  *svchp;
  OCIError   *errhp;
  OCIEnv     *envhp;   // ���������ľ��
};

// ���
struct OCI_HANDLE
{
  OCISvcCtx  *svchp;  // �����������ĵľ������context���
  OCIStmt    *smthp;

  OCIBind    *bindhp;
  OCIDefine  *defhp;

  OCIError   *errhp;  // ����������context���

  OCIEnv     *envhp; // ���������ľ�� ����context���
};

// OCIִ�еĽ��
struct CDA_DEF
{
  int      rc;             // ִ�н��
  unsigned long rpc;       // ִ��SQL����Ӱ���¼������
  char     message[2048];  // ִ��SQL������ʧ�ܣ���Ŵ�����Ϣ
};

int oci_init(LOGINENV *env);
int oci_close(LOGINENV *env); 
int oci_context_create(LOGINENV *env,OCI_CXT *cxt);
int oci_context_close(OCI_CXT *cxt);

int oci_stmt_create(OCI_CXT *cxt,OCI_HANDLE *handle);
int oci_stmt_close(OCI_HANDLE *handle);

// ���ݿ����ӳ���
class connection
{
private:
  // �������������
  LOGINENV m_env;

public:
  // ������������
  OCI_CXT m_cxt;

  // ����״̬��0-δ���ӣ�1-������
  int m_state;

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt; 

  // ���ݿ����࣬�̶�ȡֵΪoracle
  char m_dbtype[21];

  // ���ڴ��connection�������ݿ�Ĵ�������һ��ִ��SQL���Ľ��
  CDA_DEF m_cda;

  connection();

 ~connection();

  // �����ַ���������ͻ��˵��ַ��������ݿ�Ĳ�һ�£��ͻ�������롣
  // ������Ҫ��connecttodb()֮ǰ���á�
  void character(char *charset);

  // �������ݿ⣬connstr�ĸ�ʽΪ��username/password@tnsname��autocommitopt��0-�ر��Զ��ύ��1-�����Զ��ύ
  int connecttodb(char *connstr,char *charset,int autocommitopt=0);

  // ��connstr�н���username,password,tnsname
  void setdbopt(char *connstr);

  // �Ͽ������ݿ������
  int  disconnect();

  // �ύ����
  int  commit(); 

  // �ع�����
  int  rollback();

  // ��ȡ������Ϣ
  void err_report();
};

// SQL�������ݲ�����
class sqlstatement
{
public:
  // �����ݿ����ӳص�״̬��0-δ�󶨣�1-�Ѱ�
  int m_state;    

  // �����
  OCI_HANDLE m_handle;

  connection *m_conn;

  // SQL���
  char m_sql[10240];

  // ִ�н��
  CDA_DEF m_cda;

  int m_sqltype;  // ��ִ�е�SQL�������ͣ�0-��ѯ��䣻1-�ǲ�ѯ���

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt; 

  sqlstatement();
  sqlstatement(connection *conn);

 ~sqlstatement();

  // �趨���ݿ����ӳ�
  int  connect(connection *conn); 

  // �Ͽ������ݿ����ӳص�����
  int  disconnect();

  // ����SQL���,֧�ִ洢����,�������̷����ķ�ʽ,��ʱ���ط������
  int  prepare(const char *fmt,...);

  // ����������ĵ�ַ
  int  bindin(unsigned int position,int    *value);
  int  bindin(unsigned int position,long   *value);
  int  bindin(unsigned int position,unsigned int  *value);
  int  bindin(unsigned int position,unsigned long *value);
  int  bindin(unsigned int position,float *value);
  int  bindin(unsigned int position,double *value);
  int  bindin(unsigned int position,char   *value,unsigned int len);

  // ����������ĵ�ַ
  int  bindout(unsigned int position,int    *value);
  int  bindout(unsigned int position,long   *value);
  int  bindout(unsigned int position,unsigned int  *value);
  int  bindout(unsigned int position,unsigned long *value);
  int  bindout(unsigned int position,float *value);
  int  bindout(unsigned int position,double *value);
  int  bindout(unsigned int position,char   *value,unsigned int len);

  // ִ��SQL���
  int  execute();
  
  // ���SQL��䲻��Ҫ������������������ֱ��ִ�С�
  int  execute(const char *fmt,...);

  // ȡ��һ����¼,ֻ�е�SQL����ǲ�ѯ���ʱ�������� 
  int  next();

  // ���󱨸�
  void err_report();

  // ָ��LOB�ֶε�ָ�룬��ִ��execute����bindblob��bindclob�󶨣�����next��ȡָ��
  OCILobLocator *m_lob;

  // ��ʼ��lobָ�룬��sqlstatement::connect()�е���
  int  alloclob();
  
  // �ͷ�lobָ�룬��sqlstatement::disconnect()�е���
  void freelob();

  // ��lobָ��
  int  bindblob();
  int  bindclob();

  // ���������������ǰ��ļ��е�����д��LOB�ֶΣ��ڶ��������Ǳ���һ���������õ�
  int  filetolob(char *filename);
  int  filetolob(FILE *fp);

  // ��LOB�ֶ��е�����д���ļ�
  int  lobtofile(char *filename);
  int  lobtofile(FILE *fp);
};

#endif 

