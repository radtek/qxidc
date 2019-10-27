#ifndef _PGDB_H
#define _PGDB_H

#include "_cmpublic.h"
#include "/opt/PostgresPlus/9.0SS/include/libpq-fe.h"

#define PGMAXFIELDCOUNT   150   // 输入或输出参数的最大个数
#define PGMAXFIELDLENGTH 2000   // 输入或输出参数的最大长度

// 环境
struct LOGINENV
{
  char user[32];
  char pass[32];
  char tnsname[51];
};

// OCI执行的结果
struct CDA_DEF
{
  int      rc;       // 执行结果
  unsigned long rpc; // 执行SQL语句后，影响记录的行数
  char     message[4097];
};


/* 数据库连接类 */
class connection
{
public:
  PGconn *m_conn;

  // 服务器环境句柄
  LOGINENV env;

  // 连接状态
  enum conn_state { not_connected, connected };
  conn_state state;

  // 数据库种类
  char m_dbtype[21];

  // 自动提交标志，0-关闭自动提交；1-开启自动提交
  int m_autocommitopt;

  // 用于存放connection操作数据库的错误或最后一次执行SQL语句的结果
  CDA_DEF lda;

  connection();

 ~connection();

  // 连接数据库，0-关闭自动提交；1-开启自动提交
  // connstr为连接数据库的参数，如下：
  // "host=127.0.0.1 user=postgres password=oracle dbname=fsqx port=5432"
  int connecttodb(char *connstr,int in_autocommitopt=0);

  // 断开与数据库的连接
  int  disconnect();

  PGresult *m_res;

  // 开始事务
  int  beginwork();

  // 提交事务
  int  commitwork();

  // 回滚事务
  int  rollbackwork();
};

/* SQL语言数据操作类 */
class sqlstatement
{
public:
  enum cursor_state { not_opened, opened };
  cursor_state state;

  connection *m_conn;

  PGresult *m_res;

  // SQL语句
  char m_sql[20481];

  // 执行结果
  CDA_DEF cda;

  int m_sqltype;  // 待执行的SQL语句的类型，0-查询语句；1-非查询语句

  // 自动提交标志，0-关闭自动提交；1-开启自动提交
  int m_autocommitopt;

  sqlstatement();
 ~sqlstatement();

  // 设定数据库连接
  int  connect(connection *conn);

  // 断开与数据库连接的连接,在Tuxedo和CICS的服务中,
  // 一定要用disconnect()断开与数据库的连接,否则会用尽数据库的光标资源
  int  disconnect();

  // 分析SQL语句,支持存储过程,采用立刻分析的方式,即时返回分析结果
  int  prepare(char *fmt,...);

  int   m_nParamsIn;
  char *m_paramValuesIn[PGMAXFIELDCOUNT];
  char  m_paramValuesInVal[PGMAXFIELDCOUNT][PGMAXFIELDLENGTH+1];
  char *m_paramValuesInPtr[PGMAXFIELDCOUNT];
  char  m_paramTypesIn[PGMAXFIELDCOUNT][15];
  int   m_paramLengthsIn[PGMAXFIELDCOUNT];
  //int   m_paramFormatsIn[PGMAXFIELDCOUNT];

  // 结合输入变量的地址，PG数据库暂时不支持
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

  // 结合输出变量的地址
  int  bindout(int position,int    *value);
  int  bindout(int position,long   *value);
  int  bindout(int position,unsigned int  *value);
  int  bindout(int position,unsigned long *value);
  int  bindout(int position,float *value);
  int  bindout(int position,double *value);
  int  bindout(int position,char   *value,int len);

  // 执行SQL语句
  int  execute();

  int  m_respos;
  int  m_restotal;

  int  next();

  // 绑定lob指针
  int  bindblob(int position);
  int  bindclob(int position);

  // 以下两个函数都是把文件中的内容写入LOB字段，第二个函数是被第一个函数调用的
  int  filetolob(char *filename);
  int  filetolob(FILE *fp);

  // 把LOB字段中的内容写入文件
  int  lobtofile(char *filename);
  int  lobtofile(FILE *fp);
};

// 把BLOB文件导入到表的字段内容中，要求表一定要用keyid字段，0-成功，1-记录不存在，>1-其它错误
long filetoblob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);

// 把CLOB文件导入到表的字段内容中，要求表一定要用keyid字段，0-成功，1-记录不存在，>1-其它错误
long filetoclob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);


// 用表的唯一字段查找该记录的其它字段的值
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,char *othfieldvalue,int valuelen);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,long *othfieldvalue);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,char *pkvalue,char *othfieldvalue,int valuelen);

// 获取数据库的时间
long LocalTime(connection *conn,char *stime,const char *in_fmt=0,const int interval=0);


#endif

