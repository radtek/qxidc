#ifndef _oracle_H
#define _oracle_H

#include "_cmpublic.h"

/* Oracle头文件 */

#define LOBMAXBUFLEN  10240

#include <oratypes.h>
#include <ocidfn.h>
#include <ocidem.h>
#include <oci.h>

// 环境
struct LOGINENV
{
  char user[32];
  char pass[32];
  char tnsname[51];

  OCIEnv *envhp; // 环境变量的句柄
};

// 上下文
struct OCI_CXT
{
  OCISvcCtx  *svchp;
  OCIError   *errhp;
  OCIEnv     *envhp;   // 环境变量的句柄
};

// 语句
struct HANDLE
{
  OCISvcCtx  *svchp;  // 服务器上下文的句柄引用context句柄
  OCIStmt    *smthp;

  OCIBind    *bindhp;
  OCIDefine  *defhp;

  OCIError   *errhp;  // 错误句柄引用context句柄

  OCIEnv     *envhp; // 环境变量的句柄 引用context句柄
};

// OCI执行的结果
struct CDA_DEF
{
  int      rc;       // 执行结果
  unsigned long rpc; // 执行SQL语句后，影响记录的行数
  char     message[4097];
};

int oci_init(LOGINENV *env);
int oci_close(LOGINENV *env); 
int oci_context_create(LOGINENV *env,OCI_CXT *cxt);
int oci_context_close(OCI_CXT *cxt);

int oci_stmt_create(OCI_CXT *cxt,HANDLE *handle);
int oci_stmt_close(HANDLE *handle);

/* 数据库连接类 */
class connection
{
public:
  enum conn_state { not_connected, connected };

  // 连接状态
  conn_state state;

  // 服务器环境句柄
  LOGINENV env;

  // 服务器上下文
  OCI_CXT cxt;

  // 自动提交标志，0-关闭自动提交；1-开启自动提交
  int m_autocommitopt; 

  // 数据库种类
  char m_dbtype[21];

  connection();

 ~connection();

  // 连接数据库
  int connecttodb(char *connstr,int in_autocommitopt=0);

  // 从connstr中解析username,password,tnsname
  void setdbopt(char *connstr);

  // 断开与数据库的连接
  int  disconnect();

  // 提交事务
  int  commitwork(); 

  // 回滚事务
  int  rollbackwork();

  void err_report();

  // 用于存放connection操作数据库的错误或最后一次执行SQL语句的结果
  CDA_DEF lda;
};

/* SQL语言数据操作类 */
class sqlstatement
{
public:
  enum cursor_state { not_opened, opened };
  cursor_state state;

  // 语句句柄
  HANDLE handle;

  connection *m_conn;

  // SQL语句
  char m_sql[20480];

  char m_dtime[31];

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

  // 结合输入变量的地址
  int  bindin(int position,int    *value);
  int  bindin(int position,long   *value);
  int  bindin(int position,unsigned int  *value);
  int  bindin(int position,unsigned long *value);
  int  bindin(int position,float *value);
  int  bindin(int position,double *value);
  int  bindin(int position,char   *value,int len=2000);

  // 结合输出变量的地址
  int  bindout(int position,int    *value);
  int  bindout(int position,long   *value);
  int  bindout(int position,unsigned int  *value);
  int  bindout(int position,unsigned long *value);
  int  bindout(int position,float *value);
  int  bindout(int position,double *value);
  int  bindout(int position,char   *value,int len=2000);

  // 执行SQL语句
  int  execute();

  // 取下一条记录,只有当SQL语句是查询语句时才有意义 
  int  next();

  // 错误报告
  void err_report();

  // 指向LOB字段的指针，在执行execute后，用BindBLOB或BindCLOB绑定，再用next获取指针
  OCILobLocator *lob;

  // 初始化lob指针，在sqlstatement::connect()中调用
  int  alloclob();
  
  // 释放lob指针，在sqlstatement::disconnect()中调用
  void freelob();

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

// 用表的唯一字段查找该记录的其它字段的值
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,char *othfieldvalue,int valuelen);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,UINT pkvalue,long *othfieldvalue);
long findbypk(connection *conn,char *tablename,char *pkfiledname,char *othfieldname,char *pkvalue,char *othfieldvalue,int valuelen);

// 把表中的BLOB字段卸出到文件中，要求表一定要用keyid字段
// 如果被提取的字段为空，就会返回1405，没有目的文件生成
// 如果被提出的字段是empty_blob，就会生成一个空文件
long expblobfile(connection *conn,char *tablename,char *fieldname,UINT keyid,char *dstfilename);

// 把表中的CLOB字段卸出到文件中，要求表一定要用keyid字段
// 如果被提取的字段为空，就会返回1405，没有目的文件生成
// 如果被提出的字段是empty_clob，就会生成一个空文件
long expclobfile(connection *conn,char *tablename,char *fieldname,UINT keyid,char *dstfilename);

// 把BLOB文件导入到表的字段内容中，要求表一定要用keyid字段，0-成功，1-记录不存在，>1-其它错误
long filetoblob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);

// 把CLOB文件导入到表的字段内容中，要求表一定要用keyid字段，0-成功，1-记录不存在，>1-其它错误
long filetoclob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename);

// 获取数据库的时间
long LocalTime(connection *conn,char *stime,const char *in_fmt=0,const int interval=0);

#endif //_oracle_H

