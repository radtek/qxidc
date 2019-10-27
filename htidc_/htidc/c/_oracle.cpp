#include "_public.h"
#include "_oracle.h"

/*
OCI_SUCCESS                0  // maps to SQL_SUCCESS of SAG CLI  函数执行成功
OCI_SUCCESS_WITH_INFO      1  // maps to SQL_SUCCESS_WITH_INFO   执行成功，但有诊断消息返回，
                              // 可能是警告信息，但是，在测试的时候，我还从未见
                              // 识到OCI_SUCCESS_WITH_INFO是怎么回事
OCI_RESERVED_FOR_INT_USE 200  // reserved 
OCI_NO_DATA              100  // maps to SQL_NO_DATA 函数执行完成，但没有其他数据 
OCI_ERROR                 -1  // maps to SQL_ERROR 函数执行错误 
OCI_INVALID_HANDLE        -2  // maps to SQL_INVALID_HANDLE 传递给函数的参数为无效句柄，
                              // 或传回的句柄无效 
OCI_NEED_DATA             99  // maps to SQL_NEED_DATA 需要应用程序提供运行时刻的数据
OCI_STILL_EXECUTING    -3123  // OCI would block error 服务环境建立在非阻塞模式，
                              // OCI函数调用正在执行中

OCI_CONTINUE          -24200  // Continue with the body of the OCI function 
                              // 说明回调函数需要OCI库恢复其正常的处理操作 
OCI_ROWCBK_DONE       -24201  // done with user row callback 
*/

int oci_init(LOGINENV *env)
{
  //初始化Oracle 环境变量
  int  oci_ret;

  oci_ret = OCIEnvCreate(&env->envhp,OCI_DEFAULT,NULL,NULL,NULL,NULL,0,NULL);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO ) 
  {
    oci_close(env); return -1;
  }

  return 0;
} 

int oci_close(LOGINENV *env)
{
  int oci_ret;

  // 释放Oracle 环境变量
  oci_ret=OCIHandleFree(env->envhp,OCI_HTYPE_ENV);

  env->envhp = 0;

  return oci_ret;
}

int oci_context_create(LOGINENV *env,OCI_CXT *cxt )
{
  // 创建数据库连接的上下文对象，连接服务器，认证并建立会话

  if (env->envhp == 0) return -1;

  int oci_ret;
    
  oci_ret = OCIHandleAlloc(env->envhp,(dvoid**)&cxt->errhp,OCI_HTYPE_ERROR,(size_t) 0,NULL);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO ) 
  {
    oci_context_close(cxt); return -1;
  }

  // 登录
  oci_ret = OCILogon(env->envhp,cxt->errhp,&cxt->svchp,(OraText*)env->user,strlen(env->user),
                     (OraText*)env->pass,strlen(env->pass),(OraText*)env->tnsname,strlen(env->tnsname));

  if( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    oci_context_close(cxt); return -1;
  }

  cxt->envhp = env->envhp;

  return 0;    
}

int oci_context_close(OCI_CXT *cxt)
{
  // 关闭数据库连接的上下文

  int oci_ret;
    
  oci_ret = OCILogoff(cxt->svchp,cxt->errhp);

  oci_ret = OCIHandleFree(cxt->svchp,OCI_HTYPE_SVCCTX);

  oci_ret = OCIHandleFree(cxt->errhp, OCI_HTYPE_ERROR);

  cxt->svchp=0;

  cxt->errhp=0;

  return oci_ret;
}

int oci_stmt_create(OCI_CXT *cxt,HANDLE *handle )
{
  //创建语句
  int  oci_ret;
    
  oci_ret = OCIHandleAlloc( cxt->envhp, (dvoid**)&handle->smthp, OCI_HTYPE_STMT,(size_t)0, NULL);

  if( oci_ret == OCI_SUCCESS || oci_ret == OCI_SUCCESS_WITH_INFO )
  {
    handle->svchp  = cxt->svchp;
    handle->errhp  = cxt->errhp;
    handle->envhp = cxt->envhp;

    oci_ret = OCI_SUCCESS;
  }

  return oci_ret;
}

int oci_stmt_close(HANDLE *handle)
{
  // 关闭语句
  int oci_ret=0;

  oci_ret = OCIHandleFree(handle->smthp,OCI_HTYPE_STMT);

  return oci_ret;
}

connection::connection()
{ 
  state = not_connected; 

  memset(&cxt,0,sizeof(OCI_CXT));
  memset(&env,0,sizeof(LOGINENV));

  memset(&lda,0,sizeof(lda));

  lda.rc=-1;
  strncpy(lda.message,"database not opend.",128);

  // 数据库种类
  memset(m_dbtype,0,sizeof(m_dbtype));
  strcpy(m_dbtype,"oracle");
}

connection::~connection()
{
  disconnect();
}

// 从connstr中解析username,password,tnsname
void connection::setdbopt(char *connstr)
{
  char strtemp[201];

  memset(strtemp,0,sizeof(strtemp));

  strncpy(strtemp,connstr,128);

  char *pos=0;

  // tnsname
  pos = strstr(strtemp,"@");
  if (pos > 0) 
  {
    strncpy(env.tnsname,pos+1,50); pos[0]=0;
  }

  // password
  pos = strstr(strtemp,"/");
  if (pos > 0) 
  {
    strncpy(env.pass,pos+1,30); pos[0]=0;
  }

  // username
  strncpy(env.user,strtemp,30); 

  // 把username中的字母转换为大写
  int istrlen=strlen(env.user);

  for (int ii=0;ii<istrlen;ii++)
  {
    if ( (env.user[ii] >= 97) && (env.user[ii] <= 122) ) env.user[ii]=env.user[ii] - 32;
  }
}

int connection::connecttodb(char *connstr,int in_autocommitopt)
{
  // 如果已连接上数据库，就不再连接
  // 所以，如果想重连数据库，必须显示的调用disconnect()方法后才能重连
  if (state == connected) return 0;

  setenv("NLS_DATE_FORMAT","yyyymmddhh24miss",1);

  // 从connstr中解析username,password,tnsname
  setdbopt(connstr);

  memset(&lda,0,sizeof(lda));

  // 初始化环境
  int oci_ret = oci_init(&env);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    oci_close(&env); lda.rc=-1; strncpy(lda.message,"initialize oracle call interface failed.\n",128); return -1;
  }

  // 创建句柄，登录数据库
  oci_ret = oci_context_create(&env,&cxt);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    oci_close(&env); lda.rc=1017; strncpy(lda.message,"ORA-01017: invalid username/password,logon denied.\n",128); return -1;
  }

  state = connected;

  m_autocommitopt=in_autocommitopt;

  return 0;
}

int connection::disconnect()
{
  memset(&lda,0,sizeof(lda));

  if (state == not_connected) 
  { 
    lda.rc=-1; strncpy(lda.message,"database not opend.",128); return -1;
  }

  rollbackwork();

  oci_context_close(&cxt);

  oci_close(&env);

  state = not_connected;    

  return 0;
}

int connection::rollbackwork()
{ 
  memset(&lda,0,sizeof(lda));

  if (state == not_connected) 
  { 
    lda.rc=-1; strncpy(lda.message,"database not opend.",128); return -1;
  }

  int oci_ret = OCITransRollback( cxt.svchp, cxt.errhp, OCI_DEFAULT ); 

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    err_report(); return lda.rc;
  }

  return 0;    
}

int connection::commitwork()
{ 
  memset(&lda,0,sizeof(lda));

  if (state == not_connected) 
  { 
    lda.rc=-1; strncpy(lda.message,"database not opend.",128); return -1;
  }

  int oci_ret = OCITransCommit( cxt.svchp, cxt.errhp, OCI_DEFAULT );

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    err_report(); return lda.rc;
  }

  return 0;
}

void connection::err_report()
{
  if (state == not_connected) 
  { 
    lda.rc=-1; strncpy(lda.message,"database not opend.",128); return;
  }

  memset(&lda,0,sizeof(lda));

  lda.rc=-1;
  strncpy(lda.message,"call err_report failed.",128);

  if (cxt.errhp != NULL)
  {
    if (OCIErrorGet(cxt.errhp,1,NULL,&lda.rc,(OraText*)lda.message,sizeof(lda.message),OCI_HTYPE_ERROR) == OCI_NO_DATA)
    {
      // 如果获取不到错误信息，就返回正确的
      memset(&lda,0,sizeof(lda)); return;
    }
  }
}

sqlstatement::sqlstatement()
{
  state=not_opened; 

  memset(&handle,0,sizeof(handle));

  memset(&cda,0,sizeof(cda));

  memset(m_sql,0,sizeof(m_sql));

  memset(m_dtime,0,sizeof(m_dtime));

  cda.rc=-1;
  strncpy(cda.message,"sqlstatement not connect to connection.\n",128);
}

sqlstatement::~sqlstatement()
{
  disconnect();
}

int sqlstatement::connect(connection *in_conn)
{
  // 注意，一个sqlstatement在程序中只能连一个connection，不允许连多个connection
  // 所以，只要这个光标已打开，就不允许再次打开，直接返回成功
  if ( state == opened ) return 0;

  memset(&cda,0,sizeof(cda));

  m_conn=in_conn;

  // 如果数据库连接类的指针为空，直接返回失败
  if (m_conn == 0) 
  {
    cda.rc=-1; strncpy(cda.message,"database not opened.\n",128); return -1;
  }

  // 如果数据库没有连接好，直接返回失败
  if (m_conn->state == m_conn->not_connected) 
  {
    cda.rc=-1; strncpy(cda.message,"database not opend.\n",128); return -1;
  }
    
  cda.rc = oci_stmt_create(&m_conn->cxt,&handle);

  if ( cda.rc != OCI_SUCCESS && cda.rc != OCI_SUCCESS_WITH_INFO )
  {
    err_report(); return cda.rc;
  }

  state = opened;  // 光标成功打开

  m_autocommitopt=m_conn->m_autocommitopt;

  cda.rc = OCI_SUCCESS; 

  alloclob();

  return 0;
}

int sqlstatement::disconnect()
{
  if (state == not_opened) return 0;

  memset(&cda,0,sizeof(cda));

  freelob();

  cda.rc = oci_stmt_close(&handle);

  state=not_opened;

  memset(&handle,0,sizeof(handle));

  memset(&cda,0,sizeof(cda));

  memset(m_sql,0,sizeof(m_sql));

  cda.rc=-1;
  strncpy(cda.message,"cursor not opend.",128);

  return 0;
}

// 在新的OCI方法中，当SQL语句有错误时，OCIStmtPrepare返回的是0，不是失败
// 所以，程序员在程序中一般不必处理prepare的结果
int sqlstatement::prepare(char *fmt,...)
{ 
  memset(&cda,0,sizeof(cda));

  if (state == not_opened) 
  {
    cda.rc=-1; strncpy(cda.message,"cursor not opend.\n",128); return -1;
  }

  memset(&m_conn->lda,0,sizeof(m_conn->lda));

  memset(m_sql,0,sizeof(m_sql));

  va_list ap;
  va_start(ap,fmt);
  vsnprintf(m_sql,20000,fmt,ap);

  int oci_ret = OCIStmtPrepare(handle.smthp,handle.errhp,(OraText*)m_sql,strlen(m_sql),OCI_NTV_SYNTAX,OCI_DEFAULT);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    err_report(); return cda.rc;
  }

  cda.rc = OCI_SUCCESS; 

  return 0;
}

int sqlstatement::bindin(int position,int *value)
{
  return OCIBindByPos(handle.smthp, &handle.bindhp, handle.errhp, (ub4)position, value, sizeof(int),
                        SQLT_INT, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);  
}

int sqlstatement::bindin(int position,long *value)
{
  return OCIBindByPos(handle.smthp, &handle.bindhp, handle.errhp, (ub4)position, value, sizeof(long),
                        SQLT_INT, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);  
}

int sqlstatement::bindin(int position,unsigned int *value)
{
  return OCIBindByPos(handle.smthp, &handle.bindhp,handle.errhp,(ub4)position,value,sizeof(unsigned int),
                        SQLT_INT, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);  
}

int sqlstatement::bindin(int position,unsigned long *value)
{
  return OCIBindByPos(handle.smthp,&handle.bindhp,handle.errhp,(ub4)position,value,sizeof(unsigned long),
                        SQLT_INT, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);  
}

int sqlstatement::bindin(int position,char *value,int len)
{
  return OCIBindByPos(handle.smthp, &handle.bindhp, handle.errhp, (ub4)position, value, len+1,
                        SQLT_STR, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);  
}

int sqlstatement::bindin(int position,float *value)
{
  return OCIBindByPos(handle.smthp, &handle.bindhp, handle.errhp, (ub4)position, value, sizeof(float),
                        SQLT_FLT, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);
}

int sqlstatement::bindin(int position,double *value)
{
  return OCIBindByPos(handle.smthp, &handle.bindhp, handle.errhp, (ub4)position, value, sizeof(double),
                        SQLT_FLT, NULL, NULL,NULL,0, NULL, OCI_DEFAULT);
}

int sqlstatement::bindout(int position,int *value)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, sizeof(int), 
                          SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindout(int position,long *value)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, sizeof(long), 
                          SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindout(int position,unsigned int *value)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, sizeof(unsigned int), 
                          SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT );
}
int sqlstatement::bindout(int position,unsigned long *value)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, sizeof(unsigned long), 
                          SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindout(int position,float *value)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, sizeof(float), 
                          SQLT_FLT, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindout(int position,double *value)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, sizeof(double), 
                          SQLT_FLT, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindout(int position,char *value,int len)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position, value, len+1, 
                          SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindblob(int position)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position,(dvoid *) &lob,-1, 
                          SQLT_BLOB, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::bindclob(int position)
{
  return OCIDefineByPos(handle.smthp, &handle.defhp, handle.errhp, position,(dvoid *) &lob,-1, 
                          SQLT_CLOB, NULL, NULL, NULL, OCI_DEFAULT );
}

int sqlstatement::execute() 
{
  memset(&cda,0,sizeof(cda));

  if (state == not_opened) 
  {
    cda.rc=-1; strncpy(cda.message,"cursor not opend.\n",128); return -1;
  }

  // 判断是否是查询语句，如果是，把m_sqltype设为0，其它语句设为1。
  m_sqltype=1;

  // 从待执行的SQL语句中截取15个字符，如果这15个字符中包括了“select”，就认为是查询语句
  char strtemp[16]; memset(strtemp,0,sizeof(strtemp)); strncpy(strtemp,m_sql,15);

  if ( (strstr(strtemp,"select") > 0) || (strstr(strtemp,"SELECT") > 0) ) m_sqltype=0; 

  ub4 mode=OCI_DEFAULT;

  if (m_sqltype==1 && m_autocommitopt==1) mode=OCI_COMMIT_ON_SUCCESS;

  int oci_ret = OCIStmtExecute(handle.svchp,handle.smthp,handle.errhp,m_sqltype,0,NULL,NULL,mode);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    // 发生了错误或查询没有结果
    err_report(); return cda.rc;
  }

  // 如果不是查询语句，就获取影响记录的行数
  if (m_sqltype == 1)
  {
    OCIAttrGet((CONST dvoid *)handle.smthp,OCI_HTYPE_STMT,(dvoid *)&cda.rpc, (ub4 *)0,
                OCI_ATTR_ROW_COUNT, handle.errhp);
  }

  return 0;
}

int sqlstatement::next() 
{ 
  // 注意，在该函数中，不可随意用memset(&cda,0,sizeof(cda))，否则会清空cda.rpc的内容
  if (state == not_opened) 
  {
    cda.rc=-1; strncpy(cda.message,"cursor not opend.\n",128); return -1;
  }

  // 如果语句未执行成功，直接返回失败。
  if (cda.rc != 0) return cda.rc;

  // 判断是否是查询语句，如果不是，直接返回错误
  if (m_sqltype != 0)
  {
    cda.rc=-1; strncpy(cda.message,"no recordset found.\n",128); return -1;
  }

  int oci_ret = OCIStmtFetch(handle.smthp,handle.errhp,1,OCI_FETCH_NEXT,OCI_DEFAULT);

  if ( oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO )
  {
    err_report(); 

    // 只有当cda.rc不是1405和1406的时候，才返回错误，1405和1406不算错
    // 并且，返回错误的时候，不要清空了cda.rpc
    if (cda.rc != 1405 && cda.rc != 1406) 
    {
      // 如果返回代码不是0,1403,1405,1406，就表是next出现了其它异常
      // 必须关闭数据库连接，让程序错误退出。
      if ( (cda.rc!=0) && (cda.rc!=1403) && (cda.rc!=1405) && (cda.rc!=1406) ) m_conn->disconnect();

      return cda.rc;
    }
  }

  // 获取结果集成功

  // 如果返回的是1405或1406，就把它强置为0
  if (cda.rc == 1405 || cda.rc == 1406) cda.rc=0;

  // 获取影响记录的行数据
  OCIAttrGet((CONST dvoid *)handle.smthp,OCI_HTYPE_STMT,(dvoid *)&cda.rpc, (ub4 *)0,
              OCI_ATTR_ROW_COUNT, handle.errhp);

  return 0;
}

void sqlstatement::err_report()
{
  // 注意，在该函数中，不可随意用memset(&cda,0,sizeof(cda))，否则会清空cda.rpc的内容
  if (state == not_opened) 
  {
    cda.rc=-1; strncpy(cda.message,"cursor not opend.\n",128); return;
  }

  memset(&m_conn->lda,0,sizeof(m_conn->lda));

  cda.rc=-1;
  strncpy(cda.message,"call sqlstatement::err_report() failed.\n",128);

  if (handle.errhp != NULL)
  {
    if (OCIErrorGet(handle.errhp,1,NULL,&cda.rc,(OraText*)cda.message,sizeof(cda.message),OCI_HTYPE_ERROR) == OCI_NO_DATA)
    {
      // 如果没有获取到任何错误信息，就返回正确的
      // 这里可以用memset清空cda，因为如果没有任何错误cda.rpc在next中会重新赋值
      cda.rc=0; memset(cda.message,0,sizeof(cda.message)); return;
    }
  }

  // 把cda中的内容复制到lda中
  memcpy(&m_conn->lda,&cda,sizeof(cda));
}

int sqlstatement::alloclob()
{
  return OCIDescriptorAlloc((dvoid *)handle.envhp,(dvoid **)&lob,(ub4)OCI_DTYPE_LOB,(size_t)0,(dvoid **)0);
}

void sqlstatement::freelob()
{
  OCIDescriptorFree((dvoid *)lob, (ub4)OCI_DTYPE_LOB);
}

ub4 file_length(FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  return (ub4) (ftell(fp));
}

int  sqlstatement::filetolob(char *filename)
{
  FILE *fp=0;

  if ( (fp = FOPEN(filename,"rb")) == 0) 
  {
    cda.rc=-1; strncpy(cda.message,"FOPEN failed",128); return -1;
  }
  
  int iret = filetolob(fp);

  fclose(fp);

  return iret;
}

int sqlstatement::filetolob(FILE *fp)
{
  ub4   offset = 1;
  ub4   loblen = 0;
  ub1   bufp[LOBMAXBUFLEN+1];
  ub4   amtp;
  ub1   piece;
  sword retval;
  ub4   nbytes;
  ub4   remainder;

  ub4  filelen = file_length(fp);

  if (filelen == 0) return 0;

  amtp = filelen;

  remainder = filelen;

  (void) OCILobGetLength(handle.svchp, handle.errhp, lob, &loblen);

  (void) fseek(fp, 0, 0);

  if (filelen > LOBMAXBUFLEN)
  {
    nbytes = LOBMAXBUFLEN;
  }
  else
  {
    nbytes = filelen;
  }

  memset(bufp,0,sizeof(bufp));

  if (fread((void *)bufp, (size_t)nbytes, 1, fp) != 1)
  {
    cda.rc=-1; strncpy(cda.message,"fread failed",128); return -1;
  }

  remainder -= nbytes;

  if (remainder == 0)       
  {
    // exactly one piece in the file
    // Only one piece, no need for stream write
    if ( (retval = OCILobWrite(handle.svchp, handle.errhp, lob, &amtp, offset, (dvoid *) bufp,
                            (ub4) nbytes, OCI_ONE_PIECE, (dvoid *)0,
                            (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
                            (ub2) 0, (ub1) SQLCS_IMPLICIT)) != OCI_SUCCESS)
    {
      err_report(); return cda.rc;
    }
  }
  else
  {
    // more than one piece
    if (OCILobWrite(handle.svchp, handle.errhp, lob, &amtp, offset, (dvoid *) bufp,
                    (ub4) LOBMAXBUFLEN, OCI_FIRST_PIECE, (dvoid *)0,
                    (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
                    (ub2) 0, (ub1) SQLCS_IMPLICIT) != OCI_NEED_DATA)
    {
      err_report(); return cda.rc;
    }

    piece = OCI_NEXT_PIECE;

    do
    {
      if (remainder > LOBMAXBUFLEN)
      {
        nbytes = LOBMAXBUFLEN;
      }
      else
      {
        nbytes = remainder; piece = OCI_LAST_PIECE;
      }

       memset(bufp,0,sizeof(bufp));

      if (fread((void *)bufp, (size_t)nbytes, 1, fp) != 1)
      {
        cda.rc=-1; strncpy(cda.message,"fread failed",128); return -1;
      }

      retval = OCILobWrite(handle.svchp, handle.errhp, lob, &amtp, offset, (dvoid *) bufp,
                          (ub4) nbytes, piece, (dvoid *)0,
                          (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
                          (ub2) 0, (ub1) SQLCS_IMPLICIT);
      remainder -= nbytes;

    } while (retval == OCI_NEED_DATA && !feof(fp));
  }

  if (retval != OCI_SUCCESS)
  {
    err_report(); return cda.rc;
  }

  (void) OCILobGetLength(handle.svchp, handle.errhp, lob, &loblen);

  return 0;
}

// 把LOB字段中的内容写入文件
int  sqlstatement::lobtofile(char *filename)
{
  FILE *fp=0;

  if ( (fp = FOPEN(filename,"wb")) == 0)
  {
    // 不知道是什么原因，经常会出现FOPEN文件失败的情况。
    // 所以，如果出现了这种情况，以下代码再重试一次，如果仍失败，就不再尝试了。
    if ( (fp = FOPEN(filename,"wb")) == 0)
    {
      cda.rc=-1; strncpy(cda.message,"FOPEN failed",128); return -1;
    }
  }

  fseek(fp, 0, 0);

  int iret = lobtofile(fp);

  fclose(fp);

  // 如果文件在生成的过程中发生了错误，就删除该文件，因为它是一个不完整的文件
  if (iret != 0) REMOVE(filename); 

  return iret;
}

int sqlstatement::lobtofile(FILE *fp)
{
  ub4   offset = 1;
  ub4   loblen = 0;
  ub1   bufp[LOBMAXBUFLEN+1];
  ub4   amtp = 0;
  sword retval;

  OCILobGetLength(handle.svchp, handle.errhp, lob, &loblen);

  if (loblen == 0) return 0;

  amtp = loblen;

  memset(bufp,0,sizeof(bufp));

  retval = OCILobRead(handle.svchp, handle.errhp, lob, &amtp, offset, (dvoid *) bufp,
                     (loblen < LOBMAXBUFLEN ? loblen : LOBMAXBUFLEN), (dvoid *)0,
                     (sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0,
                     (ub2) 0, (ub1) SQLCS_IMPLICIT);

  switch (retval)
  {
    case OCI_SUCCESS:             /* only one piece */
      fwrite((void *)bufp, (size_t)amtp, 1, fp);
      break;

    case OCI_NEED_DATA:           /* there are 2 or more pieces */
      fwrite((void *)bufp, amtp, 1, fp); 

      while (TRUE)
      {
        amtp = 0;
        memset(bufp,0,sizeof(bufp));

        retval = OCILobRead(handle.svchp, handle.errhp, lob, &amtp, offset, (dvoid *) bufp,
                           (ub4) LOBMAXBUFLEN, (dvoid *)0,
                           (sb4 (*)(dvoid *, const dvoid *, ub4, ub1)) 0,
                           (ub2) 0, (ub1) SQLCS_IMPLICIT);

        fwrite((void *)bufp, (size_t)amtp, 1, fp);

        if (retval != OCI_NEED_DATA) break;
      } 

      break;

    case OCI_ERROR:
      err_report(); 
      return cda.rc;
  }

  return 0;
}

long findbypk(connection *conn,char *tablename,char *pkfieldname,char *othfieldname,UINT pkvalue,char *othfieldvalue,int valuelen)
{
  othfieldvalue[0]=0;

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select %s from %s where %s=:1",othfieldname,tablename,pkfieldname);
  stmt.bindin(1,&pkvalue);
  stmt.bindout(1,othfieldvalue,valuelen);

  if (stmt.execute() != 0) return stmt.cda.rc;

  return stmt.next();
}

long findbypk(connection *conn,char *tablename,char *pkfieldname,char *othfieldname,UINT pkvalue,long *othfieldvalue)
{
  (*othfieldvalue)=0;

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select %s from %s where %s=:1",othfieldname,tablename,pkfieldname);
  stmt.bindin(1,&pkvalue);
  stmt.bindout(1,othfieldvalue);

  if (stmt.execute() != 0) return stmt.cda.rc;

  return stmt.next();
}

long findbypk(connection *conn,char *tablename,char *pkfieldname,char *othfieldname,char *pkvalue,char *othfieldvalue,int valuelen)
{
  othfieldvalue[0]=0;

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select %s from %s where %s=:1",othfieldname,tablename,pkfieldname);
  stmt.bindin(1,pkvalue,strlen(pkvalue));
  stmt.bindout(1,othfieldvalue,valuelen);

  if (stmt.execute() != 0) return stmt.cda.rc;

  return stmt.next();
}

// 把表中的BLOB字段卸出到文件中，要求表一定要用keyid字段
// 如果被提取的字段为空，就会返回1405，没有目的文件生成
// 如果被提出的字段是empty_blob，就会生成一个空文件
long expblobfile(connection *conn,char *tablename,char *fieldname,UINT keyid,char *dstfilename)
{
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select %s from %s where keyid=%lu and %s is not null",fieldname,tablename,keyid,fieldname);
  stmt.bindblob(1);

  if (stmt.execute() != 0) return stmt.cda.rc;

  if (stmt.next() != 0) return stmt.cda.rc;

  return stmt.lobtofile(dstfilename);
}

// 把表中的CLOB字段卸出到文件中，要求表一定要用keyid字段
// 如果被提取的字段为空，就会返回1405，没有目的文件生成
// 如果被提出的字段是empty_clob，就会生成一个空文件
long expclobfile(connection *conn,char *tablename,char *fieldname,UINT keyid,char *dstfilename)
{
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select %s from %s where keyid=%lu and %s is not null",fieldname,tablename,keyid,fieldname);
  stmt.bindclob(1);

  if (stmt.execute() != 0) return stmt.cda.rc;

  if (stmt.next() != 0) return stmt.cda.rc;

  return stmt.lobtofile(dstfilename);
}

// 把BLOB文件导入到表的字段内容中，0-成功，1-记录不存在，>1-其它错误
long filetoblob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename)
{
  sqlstatement stmt;
  stmt.connect(conn);
 
  stmt.prepare("update %s set %s=empty_blob() where keyid=%lu",tablename,fieldname,keyid);
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n",stmt.m_sql); return stmt.cda.rc;
  }

  if (stmt.cda.rpc == 0) return 1;
 
  stmt.prepare("select %s from %s where keyid=%lu for update",fieldname,tablename,keyid);
  stmt.bindblob(1);
 
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n",stmt.m_sql); return stmt.cda.rc;
  }
 
  if (stmt.next() != 0) return stmt.cda.rc;
 
  // 把文件写入LOB字段
  stmt.filetolob(filename);
 
  return stmt.cda.rc;
}

 
// 把CLOB文件导入到表的字段内容中，0-成功，1-记录不存在，>1-其它错误
long filetoclob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename)
{
  sqlstatement stmt;
  stmt.connect(conn);
 
  stmt.prepare("update %s set %s=empty_clob() where keyid=%lu",tablename,fieldname,keyid);
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n",stmt.m_sql); return stmt.cda.rc;
  }
 
  if (stmt.cda.rpc == 0) return 1;
 
  stmt.prepare("select %s from %s where keyid=%lu for update",fieldname,tablename,keyid);
  stmt.bindclob(1);
 
  if (stmt.execute() != 0)
  {
    printf("%s failed.\n",stmt.m_sql); return stmt.cda.rc;
  }
 
  if (stmt.next() != 0) return stmt.cda.rc;
 
  // 把文件写入LOB字段
  stmt.filetolob(filename);
 
  return stmt.cda.rc;
}

long LocalTime(connection *conn,char *stime,const char *in_fmt,const int interval)
{
  if (conn == 0) return -1;

  if (conn->state == connection::not_connected) return -1;

  char strfmt[31];

  memset(strfmt,0,sizeof(strfmt));

  strncpy(strfmt,"yyyy-mm-dd hh24:mi:ss",21);

  if (in_fmt != 0) strncpy(strfmt,in_fmt,30);

  // stime变量不需要初始化，不太好写。

  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("select to_char(sysdate+%d/86400,'%s') from dual",interval,strfmt);
  stmt.bindout(1,stime,strlen(strfmt));

  if (stmt.execute() != 0) return stmt.cda.rc;

  stmt.next();

  return stmt.cda.rc;
}
