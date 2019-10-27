#ifndef __MYSQL_H
#define __MYSQL_H

// C/C++�ⳣ��ͷ�ļ�
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* mysqlͷ�ļ� */
#include <mysql.h>

// mysql��¼����
struct LOGINENV
{
  char ip[32];
  char user[32];     // ���ݿ���û���
  char pass[32];     // ���ݿ������
  char dbname[51];  
  int  port;
};

// mysqlִ�еĽ��
struct CDA_DEF
{
  int      rc;             // ִ�н��
  unsigned long rpc;       // ִ��SQL����Ӱ���¼������
  char     message[2048];  // ִ��SQL������ʧ�ܣ���Ŵ�����Ϣ
};

/* ���ݿ����ӳ��� */
class connection
{
public:
  MYSQL     *m_conn;

  // ����״̬��0-δ���ӣ�1-������
  int m_state;

  // �������������
  LOGINENV m_env;

  // �Զ��ύ��־��0-�ر��Զ��ύ��1-�����Զ��ύ
  int m_autocommitopt; 

  // ���ݿ����࣬�̶�ȡֵΪmysql
  char m_dbtype[21];

  connection();

 ~connection();

  // �������ݿ⣬
  // connstr�ĸ�ʽΪ��ip,username,password,dbname,port
  // autocommitopt��0-�ر��Զ��ύ��1-�����Զ��ύ
  int connecttodb(char *connstr,char *charset,unsigned int autocommitopt=0);

  // ��connstr�н���ip,username,password,dbname,port
  void setdbopt(char *connstr);

  // �Ͽ������ݿ������
  int  disconnect();

  // �����ַ�����Ҫ�����ݿ��һ�£��������Ļ��������
  void character(char *charset);

  // �ύ����
  int  commit(); 

  // �ع�����
  int  rollback();

  // ��ȡ������Ϣ
  void err_report();

  // ���ڴ��connection�������ݿ�Ĵ�������һ��ִ��SQL���Ľ��
  CDA_DEF m_cda;
};

// ����SQL����У����������ֶθ��������ֵ��256�Ǻܴ���ˣ����Ը���ʵ���������
#define MAXPARAMS  256

/* SQL�������ݲ����� */
class sqlstatement
{
public:
  // �����ݿ����ӳص�״̬��0-δ�󶨣�1-�Ѱ�
  int m_state;

  // �����
  MYSQL_STMT *m_handle;
  
  // �������
  MYSQL_BIND params_in[MAXPARAMS];
  // �������
  MYSQL_BIND params_out[MAXPARAMS];

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

  void initial();

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
};

/*
delimiter $$
drop function if exists to_null;

create function to_null(in_value varchar(10)) returns decimal
begin
if (length(in_value)=0) then
  return null;
else
  return in_value;
end if;
end;
$$
*/

#endif

