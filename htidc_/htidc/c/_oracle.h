#ifndef _oracle_H
#define _oracle_H

#include "_cmpublic.h"

/* Oracleͷ�ļ� */

#define LOBMAXBUFLEN  10240

#include <oratypes.h>
#include <ocidfn.h>
#include <ocidem.h>
#include <oci.h>

// ����
struct LOGINENV
{
  char user[32];
  char pass[32];
  char tnsname[51];

  OCIEnv *envhp; // ���������ľ��
};

// ������
struct OCI_CXT
{
  OCISvcCtx  *svchp;
  OCIError   *errhp;
  OCIEnv     *envhp;   // ���������ľ��
};

// ���
struct HANDLE
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
  int      rc;       // ִ�н��
  unsigned long rpc; // ִ��SQL����Ӱ���¼������
  char     message[4097];
};

int oci_init(LOGINENV *env);
int oci_close(LOGINENV *env); 
int oci_context_create(LOGINENV *env,OCI_CXT *cxt);
int oci_context_close(OCI_CXT *cxt);

int oci_stmt_create(OCI_CXT *cxt,HANDLE *handle);
int oci_stmt_close(HANDLE *handle);

/* ���ݿ������� */
class connection
{
public:
  enum conn_state { not_connected, connected };

  // ����״̬
  conn_state state;

  // �������������
  LOGINENV env;

  // ������������
  OCI_CXT cxt;

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt; 

  // ���ݿ�����
  char m_dbtype[21];

  connection();

 ~connection();

  // �������ݿ�
  int connecttodb(char *connstr,int in_autocommitopt=0);

  // ��connstr�н���username,password,tnsname
  void setdbopt(char *connstr);

  // �Ͽ������ݿ������
  int  disconnect();

  // �ύ����
  int  commitwork(); 

  // �ع�����
  int  rollbackwork();

  void err_report();

  // ���ڴ��connection�������ݿ�Ĵ�������һ��ִ��SQL���Ľ��
  CDA_DEF lda;
};

/* SQL�������ݲ����� */
class sqlstatement
{
public:
  enum cursor_state { not_opened, opened };
  cursor_state state;

  // �����
  HANDLE handle;

  connection *m_conn;

  // SQL���
  char m_sql[20480];

  char m_dtime[31];

  // ִ�н��
  CDA_DEF cda;

  int m_sqltype;  // ��ִ�е�SQL�������ͣ�0-��ѯ��䣻1-�ǲ�ѯ���

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt; 

  sqlstatement();
 ~sqlstatement();

  // �趨���ݿ�����
  int  connect(connection *conn); 

  // �Ͽ������ݿ����ӵ�����,��Tuxedo��CICS�ķ�����,
  // һ��Ҫ��disconnect()�Ͽ������ݿ������,������þ����ݿ�Ĺ����Դ
  int  disconnect();

  // ����SQL���,֧�ִ洢����,�������̷����ķ�ʽ,��ʱ���ط������
  int  prepare(char *fmt,...);

  // �����������ĵ�ַ
  int  bindin(int position,int    *value);
  int  bindin(int position,long   *value);
  int  bindin(int position,unsigned int  *value);
  int  bindin(int position,unsigned long *value);
  int  bindin(int position,float *value);
  int  bindin(int position,double *value);
  int  bindin(int position,char   *value,int len=2000);

  // �����������ĵ�ַ
  int  bindout(int position,int    *value);
  int  bindout(int position,long   *value);
  int  bindout(int position,unsigned int  *value);
  int  bindout(int position,unsigned long *value);
  int  bindout(int position,float *value);
  int  bindout(int position,double *value);
  int  bindout(int position,char   *value,int len=2000);

  // ִ��SQL���
  int  execute();

  // ȡ��һ����¼,ֻ�е�SQL����ǲ�ѯ���ʱ�������� 
  int  next();

  // ���󱨸�
  void err_report();

  // ָ��LOB�ֶε�ָ�룬��ִ��execute����BindBLOB��BindCLOB�󶨣�����next��ȡָ��
  OCILobLocator *lob;

  // ��ʼ��lobָ�룬��sqlstatement::connect()�е���
  int  alloclob();
  
  // �ͷ�lobָ�룬��sqlstatement::disconnect()�е���
  void freelob();

  // ��lobָ��
  int  bindblob(int position);
  int  bindclob(int position);

  // ���������������ǰ��ļ��е�����д��LOB�ֶΣ��ڶ��������Ǳ���һ���������õ�
  int  filetolob(char *filename);
  int  filetolob(FILE *fp);

  // ��LOB�ֶ��е�����д���ļ�
  int  lobtofile(char *filename);
  int  lobtofile(FILE *fp);
};

// �ñ��Ψһ�ֶβ��Ҹü�¼�������ֶε�ֵ
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,char *othfieldvalue,int valuelen);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,long *othfieldvalue);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,char *pkvalue,char *othfieldvalue,int valuelen);

// �ѱ��е�BLOB�ֶ�ж�����ļ��У�Ҫ���һ��Ҫ��keyid�ֶ�
// �������ȡ���ֶ�Ϊ�գ��ͻ᷵��1405��û��Ŀ���ļ�����
// �����������ֶ���empty_blob���ͻ�����һ�����ļ�
long expblobfile(connection *conn,char *tablename,char *fieldname,UINT keyid,char *dstfilename);

// �ѱ��е�CLOB�ֶ�ж�����ļ��У�Ҫ���һ��Ҫ��keyid�ֶ�
// �������ȡ���ֶ�Ϊ�գ��ͻ᷵��1405��û��Ŀ���ļ�����
// �����������ֶ���empty_clob���ͻ�����һ�����ļ�
long expclobfile(connection *conn,char *tablename,char *fieldname,UINT keyid,char *dstfilename);

// ��BLOB�ļ����뵽����ֶ������У�Ҫ���һ��Ҫ��keyid�ֶΣ�0-�ɹ���1-��¼�����ڣ�>1-��������
long filetoblob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);

// ��CLOB�ļ����뵽����ֶ������У�Ҫ���һ��Ҫ��keyid�ֶΣ�0-�ɹ���1-��¼�����ڣ�>1-��������
long filetoclob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);

// ��ȡ���ݿ��ʱ��
long LocalTime(connection *conn,char *stime,const char *in_fmt=0,const int interval=0);

#endif //_oracle_H

