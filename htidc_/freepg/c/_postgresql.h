#ifndef _POSTGRESQL_H
#define _POSTGRESQL_H

// C/C++库常用头文件
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

// postgresql接口库的头文件
#include <libpq-fe.h>

// 定义SQL语句中，输入和输出字段个数的最大值，256是很大的了，可以根据实际情况调整
#define MAXPARAMS  256

// 输入或输出参数的最大长度
#define MAXFIELDLENGTH 2000   

// OCI执行的结果
struct CDA_DEF
{
  int      rc;       // 执行结果
  unsigned long rpc; // 执行SQL语句后，影响记录的行数
  char     message[2048];
};

/* 数据库连接类 */
class connection
{
public:
  PGconn *m_conn;

  // 连接状态，0-未连接，1-已连接
  int m_state;

  // 数据库种类，固定取值为postgresql
  char m_dbtype[21];

  // 自动提交标志，0-关闭自动提交；1-开启自动提交
  int m_autocommitopt;

  // 用于存放connection操作数据库的错误或最后一次执行SQL语句的结果
  CDA_DEF m_cda;

  connection();

 ~connection();

  // 连接数据库，0-关闭自动提交；1-开启自动提交
  // connstr为连接数据库的参数，如下：
  // "host=127.0.0.1 user=postgres password=oracle dbname=fsqx port=5432"
  int connecttodb(char *connstr,char *charset,unsigned int autocommitopt=0);

  // 设置字符集，要与数据库的一致，否则中文会出现乱码
  void character(char *charset);

  // 断开与数据库的连接
  int  disconnect();

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
  // 与数据库连接池的状态，0-未绑定，1-已绑定
  int m_state;

  connection *m_conn;

  PGresult *m_res;

  // SQL语句
  char m_sql[10240];

  // 执行结果
  CDA_DEF m_cda;

  int m_sqltype;  // 待执行的SQL语句的类型，0-查询语句；1-非查询语句

  // 自动提交标志，0-关闭自动提交；1-开启自动提交
  int m_autocommitopt;

  sqlstatement();
  sqlstatement(connection *conn);

 ~sqlstatement();

  // 设定数据库连接
  int  connect(connection *conn);

  // 断开与数据库连接的连接,在Tuxedo和CICS的服务中,
  // 一定要用disconnect()断开与数据库的连接,否则会用尽数据库的光标资源
  int  disconnect();

  // 分析SQL语句,支持存储过程,采用立刻分析的方式,即时返回分析结果
  int  prepare(char *fmt,...);

  int   m_nParamsIn;
  char *m_paramValuesIn[MAXPARAMS];
  char  m_paramValuesInVal[MAXPARAMS][MAXFIELDLENGTH+1];
  char *m_paramValuesInPtr[MAXPARAMS];
  char  m_paramTypesIn[MAXPARAMS][15];
  int   m_paramLengthsIn[MAXPARAMS];

  // 结合输入变量的地址
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

  // 结合输出变量的地址
  int  bindout(unsigned int position,int    *value);
  int  bindout(unsigned int position,long   *value);
  int  bindout(unsigned int position,unsigned int  *value);
  int  bindout(unsigned int position,unsigned long *value);
  int  bindout(unsigned int position,float *value);
  int  bindout(unsigned int position,double *value);
  int  bindout(unsigned int position,char   *value,unsigned int len);

  // 执行SQL语句
  int  execute();

  // 如果SQL语句不需要输入和输出变量，可以直接执行。
  int  execute(const char *fmt,...);

  int  m_respos;
  int  m_restotal;

  int  next();
};

#endif

