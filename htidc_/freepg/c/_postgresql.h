#ifndef _POSTGRESQL_H
#define _POSTGRESQL_H

// C/C++�ⳣ��ͷ�ļ�
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

// postgresql�ӿڿ��ͷ�ļ�
#include <libpq-fe.h>

// ����SQL����У����������ֶθ��������ֵ��256�Ǻܴ���ˣ����Ը���ʵ���������
#define MAXPARAMS  256

// ����������������󳤶�
#define MAXFIELDLENGTH 2000   

// OCIִ�еĽ��
struct CDA_DEF
{
  int      rc;       // ִ�н��
  unsigned long rpc; // ִ��SQL����Ӱ���¼������
  char     message[2048];
};

/* ���ݿ������� */
class connection
{
public:
  PGconn *m_conn;

  // ����״̬��0-δ���ӣ�1-������
  int m_state;

  // ���ݿ����࣬�̶�ȡֵΪpostgresql
  char m_dbtype[21];

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt;

  // ���ڴ��connection�������ݿ�Ĵ�������һ��ִ��SQL���Ľ��
  CDA_DEF m_cda;

  connection();

 ~connection();

  // �������ݿ⣬0-�ر��Զ��ύ��1-�����Զ��ύ
  // connstrΪ�������ݿ�Ĳ��������£�
  // "host=127.0.0.1 user=postgres password=oracle dbname=fsqx port=5432"
  int connecttodb(char *connstr,char *charset,unsigned int autocommitopt=0);

  // �����ַ�����Ҫ�����ݿ��һ�£��������Ļ��������
  void character(char *charset);

  // �Ͽ������ݿ������
  int  disconnect();

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
  // �����ݿ����ӳص�״̬��0-δ�󶨣�1-�Ѱ�
  int m_state;

  connection *m_conn;

  PGresult *m_res;

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

  // �趨���ݿ�����
  int  connect(connection *conn);

  // �Ͽ������ݿ����ӵ�����,��Tuxedo��CICS�ķ�����,
  // һ��Ҫ��disconnect()�Ͽ������ݿ������,������þ����ݿ�Ĺ����Դ
  int  disconnect();

  // ����SQL���,֧�ִ洢����,�������̷����ķ�ʽ,��ʱ���ط������
  int  prepare(char *fmt,...);

  int   m_nParamsIn;
  char *m_paramValuesIn[MAXPARAMS];
  char  m_paramValuesInVal[MAXPARAMS][MAXFIELDLENGTH+1];
  char *m_paramValuesInPtr[MAXPARAMS];
  char  m_paramTypesIn[MAXPARAMS][15];
  int   m_paramLengthsIn[MAXPARAMS];

  // �����������ĵ�ַ
  int  bindin(unsigned int position,int    *value);
  int  bindin(unsigned int position,long   *value);
  int  bindin(unsigned int position,unsigned int  *value);
  int  bindin(unsigned int position,unsigned long *value);
  int  bindin(unsigned int position,float *value);
  int  bindin(unsigned int position,double *value);
  int  bindin(unsigned int position,char   *value,unsigned int len);

  int   m_nParamsOut;
  char  m_paramTypesOut[MAXPARAMS][15];
  char *m_paramValuesOut[MAXPARAMS];
  int   m_paramLengthsOut[MAXPARAMS];

  // �����������ĵ�ַ
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

  int  m_respos;
  int  m_restotal;

  int  next();
};

#endif

