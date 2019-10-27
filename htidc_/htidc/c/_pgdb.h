#ifndef _PGDB_H
#define _PGDB_H

#include "_cmpublic.h"
#include "/opt/PostgresPlus/9.0SS/include/libpq-fe.h"

#define PGMAXFIELDCOUNT   150   // ��������������������
#define PGMAXFIELDLENGTH 2000   // ����������������󳤶�

// ����
struct LOGINENV
{
  char user[32];
  char pass[32];
  char tnsname[51];
};

// OCIִ�еĽ��
struct CDA_DEF
{
  int      rc;       // ִ�н��
  unsigned long rpc; // ִ��SQL����Ӱ���¼������
  char     message[4097];
};


/* ���ݿ������� */
class connection
{
public:
  PGconn *m_conn;

  // �������������
  LOGINENV env;

  // ����״̬
  enum conn_state { not_connected, connected };
  conn_state state;

  // ���ݿ�����
  char m_dbtype[21];

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt;

  // ���ڴ��connection�������ݿ�Ĵ�������һ��ִ��SQL���Ľ��
  CDA_DEF lda;

  connection();

 ~connection();

  // �������ݿ⣬0-�ر��Զ��ύ��1-�����Զ��ύ
  // connstrΪ�������ݿ�Ĳ��������£�
  // "host=127.0.0.1 user=postgres password=oracle dbname=fsqx port=5432"
  int connecttodb(char *connstr,int in_autocommitopt=0);

  // �Ͽ������ݿ������
  int  disconnect();

  PGresult *m_res;

  // ��ʼ����
  int  beginwork();

  // �ύ����
  int  commitwork();

  // �ع�����
  int  rollbackwork();
};

/* SQL�������ݲ����� */
class sqlstatement
{
public:
  enum cursor_state { not_opened, opened };
  cursor_state state;

  connection *m_conn;

  PGresult *m_res;

  // SQL���
  char m_sql[20481];

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

  int   m_nParamsIn;
  char *m_paramValuesIn[PGMAXFIELDCOUNT];
  char  m_paramValuesInVal[PGMAXFIELDCOUNT][PGMAXFIELDLENGTH+1];
  char *m_paramValuesInPtr[PGMAXFIELDCOUNT];
  char  m_paramTypesIn[PGMAXFIELDCOUNT][15];
  int   m_paramLengthsIn[PGMAXFIELDCOUNT];
  //int   m_paramFormatsIn[PGMAXFIELDCOUNT];

  // �����������ĵ�ַ��PG���ݿ���ʱ��֧��
  int  bindin(int position,int    *value);
  int  bindin(int position,long   *value);
  int  bindin(int position,unsigned int  *value);
  int  bindin(int position,unsigned long *value);
  int  bindin(int position,float *value);
  int  bindin(int position,double *value);
  int  bindin(int position,char   *value,int len);

  int   m_nParamsOut;
  char  m_paramTypesOut[PGMAXFIELDCOUNT][15];
  char *m_paramValuesOut[PGMAXFIELDCOUNT];
  int   m_paramLengthsOut[PGMAXFIELDCOUNT];

  // �����������ĵ�ַ
  int  bindout(int position,int    *value);
  int  bindout(int position,long   *value);
  int  bindout(int position,unsigned int  *value);
  int  bindout(int position,unsigned long *value);
  int  bindout(int position,float *value);
  int  bindout(int position,double *value);
  int  bindout(int position,char   *value,int len);

  // ִ��SQL���
  int  execute();

  int  m_respos;
  int  m_restotal;

  int  next();

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

// ��BLOB�ļ����뵽����ֶ������У�Ҫ���һ��Ҫ��keyid�ֶΣ�0-�ɹ���1-��¼�����ڣ�>1-��������
long filetoblob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);

// ��CLOB�ļ����뵽����ֶ������У�Ҫ���һ��Ҫ��keyid�ֶΣ�0-�ɹ���1-��¼�����ڣ�>1-��������
long filetoclob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);


// �ñ��Ψһ�ֶβ��Ҹü�¼�������ֶε�ֵ
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,char *othfieldvalue,int valuelen);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,long *othfieldvalue);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,char *pkvalue,char *othfieldvalue,int valuelen);

// ��ȡ���ݿ��ʱ��
long LocalTime(connection *conn,char *stime,const char *in_fmt=0,const int interval=0);


#endif

