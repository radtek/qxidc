#include "_public.h"
#include "_oracle.h"

connection::connection()
{
  m_conn=0;

  state = not_connected;

  memset(&lda,0,sizeof(lda));

  lda.rc=-1;
  strcpy(lda.message,"database not opend.");

  // 数据库种类
  memset(m_dbtype,0,sizeof(m_dbtype));
  strcpy(m_dbtype,"pg");
}

connection::~connection()
{
  disconnect();
}

int connection::connecttodb(char *connstr,int in_autocommitopt)
{
  // 如果已连接上数据库，就不再连接
  // 所以，如果想重连数据库，必须显示的调用disconnect()方法后才能重连
  if (state == connected) return 0;

  m_conn=PQconnectdb(connstr);

  if (PQstatus(m_conn) != CONNECTION_OK)
  {
    lda.rc=-1; strcpy(lda.message,PQerrorMessage(m_conn)); return -1;
  }

  PQsetClientEncoding(m_conn,"GBK");

  state = connected;

  m_autocommitopt=in_autocommitopt;

  beginwork();

  return 0;
}

int connection::disconnect()
{
  memset(&lda,0,sizeof(lda));

  if (state == not_connected)
  {
    lda.rc=-1; strcpy(lda.message,"database not opend."); return -1;
  }

  rollbackwork();

  PQfinish(m_conn);

  m_conn=0;

  state = not_connected;

  return 0;
}

// 开始事务
int connection::beginwork()
{
  memset(&lda,0,sizeof(lda));

  if (state == not_connected)
  {
    lda.rc=-1; strcpy(lda.message,"database not opend."); return -1;
  }

  if (m_autocommitopt==1) return 0;

  m_res = PQexec(m_conn,"BEGIN");

  if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
  {
    lda.rc=-1; strcpy(lda.message,PQerrorMessage(m_conn)); 
  }

  PQclear(m_res); m_res=0;

  return lda.rc;
}

int connection::commitwork()
{
  memset(&lda,0,sizeof(lda));

  if (state == not_connected)
  {
    lda.rc=-1; strcpy(lda.message,"database not opend."); return -1;
  }

  if (m_autocommitopt==1) return 0;

  m_res = PQexec(m_conn,"COMMIT");

  if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
  {
    lda.rc=-1; strcpy(lda.message,PQerrorMessage(m_conn));
  }

  PQclear(m_res); m_res=0;

  beginwork();

  return lda.rc;
}

int connection::rollbackwork()
{
  memset(&lda,0,sizeof(lda));

  if (state == not_connected)
  {
    lda.rc=-1; strcpy(lda.message,"database not opend."); return -1;
  }

  if (m_autocommitopt==1) return 0;

  m_res = PQexec(m_conn,"ROLLBACK");

  if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
  {
    lda.rc=-1; strcpy(lda.message,PQerrorMessage(m_conn));
  }

  PQclear(m_res); m_res=0;

  beginwork();

  return lda.rc;
}


sqlstatement::sqlstatement()
{
  state=not_opened;

  memset(&cda,0,sizeof(cda));

  memset(m_sql,0,sizeof(m_sql));

  m_res=0;

  m_conn=0;

  cda.rc=-1;

  strcpy(cda.message,"sqlstatement not connect to connection.\n");

  m_nParamsIn=0;
  memset(m_paramValuesIn,0,sizeof(m_paramValuesIn));
  memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));
  memset(m_paramValuesInPtr,0,sizeof(m_paramValuesInPtr));
  memset(m_paramLengthsIn,0,sizeof(m_paramLengthsIn));
  //memset(m_paramFormatsIn,0,sizeof(m_paramFormatsIn));

  for (int ii=0;ii<PGMAXFIELDCOUNT;ii++)
  {
    //m_paramValuesIn[ii]=&m_paramValuesInVal[ii][0];
    m_paramValuesIn[ii]=m_paramValuesInVal[ii];
  }

  m_respos=0;

  m_restotal=0;

  m_nParamsOut=0;
}

sqlstatement::~sqlstatement()
{
  disconnect();
}

int sqlstatement::disconnect()
{
  if (state == not_opened) return 0;

  memset(&cda,0,sizeof(cda));

  if (m_res!= 0) { PQclear(m_res); m_res=0; }

  state=not_opened;

  memset(&cda,0,sizeof(cda));

  memset(m_sql,0,sizeof(m_sql));

  cda.rc=-1;
  strcpy(cda.message,"cursor not opend.");

  PQclear(m_res); m_res=0;

  m_nParamsIn=0;
  //memset(m_paramValuesIn,0,sizeof(m_paramValuesIn));
  memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));
  memset(m_paramLengthsIn,0,sizeof(m_paramLengthsIn));
  memset(m_paramTypesIn,0,sizeof(m_paramTypesIn));
  //memset(m_paramFormatsIn,0,sizeof(m_paramFormatsIn));

  m_respos=0;
  m_restotal=0;
  m_nParamsOut=0;
  memset(m_paramValuesOut,0,sizeof(m_paramValuesOut));
  memset(m_paramTypesOut,0,sizeof(m_paramTypesOut));
  memset(m_paramLengthsOut,0,sizeof(m_paramLengthsOut));

  return 0;
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
    cda.rc=-1; strcpy(cda.message,"database not opened.\n"); return -1;
  }

  // 如果数据库没有连接好，直接返回失败
  if (m_conn->state == m_conn->not_connected)
  {
    cda.rc=-1; strcpy(cda.message,"database not opend.\n"); return -1;
  }

  state = opened;  // 光标成功打开

  return 0;
}

int sqlstatement::prepare(char *fmt,...)
{
  memset(&cda,0,sizeof(cda));

  if (state == not_opened)
  {
    cda.rc=-1; strcpy(cda.message,"cursor not opend.\n"); return -1;
  }

  // 该代码不能启用，如果m_conn为空，就会造成Segmentation fault
  // memset(&m_conn->lda,0,sizeof(m_conn->lda));

  memset(m_sql,0,sizeof(m_sql));

  va_list ap;
  va_start(ap,fmt);
  vsnprintf(m_sql,20480,fmt,ap);

  char seqname[51],strsrc[51],strdst[51];

  char *beginpos,*endpos;

  // 替换序列生成器的语法
  while (TRUE)
  {
    beginpos=endpos=0;

    beginpos=strstr(m_sql,"SEQ_");
    endpos=strstr(m_sql,".nextval");

    if ( (beginpos==0) || (endpos==0) ) break;

    memset(seqname,0,sizeof(seqname));
    strncpy(seqname,beginpos,endpos-beginpos);

    memset(strsrc,0,sizeof(strsrc));
    snprintf(strsrc,50,"%s.nextval",seqname);

    memset(strdst,0,sizeof(strdst));
    snprintf(strdst,50,"nextval('%s')",seqname);

    UpdateStr(m_sql,strsrc,strdst);
  }

  // 把rowid替换成ctid
  UpdateStr(m_sql,"rowid","ctid");

  // 把sysdate替换成now()
  UpdateStr(m_sql,"sysdate","now()");

  // 把to_date转换成to_timestamp
  UpdateStr(m_sql,"to_date","to_timestamp");

  int ilen=strlen(m_sql);

  // PG数据库与Oracle不同，是采用$，不是:
  for (int ii=0;ii<ilen;ii++)
  {
    if ( (m_sql[ii]==':') && 
         ( (m_sql[ii+1]=='1') || (m_sql[ii+1]=='2')  ||
           (m_sql[ii+1]=='3') || (m_sql[ii+1]=='4')  ||
           (m_sql[ii+1]=='5') || (m_sql[ii+1]=='6')  ||
           (m_sql[ii+1]=='7') || (m_sql[ii+1]=='8')  ||
           (m_sql[ii+1]=='9') || (m_sql[ii+1]=='0') ) )
      m_sql[ii]='$';
  }

  PQclear(m_res); m_res=0;

  m_nParamsIn=0;
  // m_paramValuesIn在这里不能被初始化
  //memset(m_paramValuesIn,0,sizeof(m_paramValuesIn));
  memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));
  memset(m_paramValuesInPtr,0,sizeof(m_paramValuesInPtr));
  memset(m_paramLengthsIn,0,sizeof(m_paramLengthsIn));
  memset(m_paramTypesIn,0,sizeof(m_paramTypesIn));
  //memset(m_paramFormatsIn,0,sizeof(m_paramFormatsIn));

  m_respos=0;
  m_restotal=0;
  m_nParamsOut=0;
  memset(m_paramValuesOut,0,sizeof(m_paramValuesOut));
  memset(m_paramTypesOut,0,sizeof(m_paramTypesOut));
  memset(m_paramLengthsOut,0,sizeof(m_paramLengthsOut));

  return 0;
}

int sqlstatement::bindin(int position,int *value)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"int");
  //m_paramFormatsIn[m_nParamsIn]=0;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(int position,long *value)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"long");
  //m_paramFormatsIn[m_nParamsIn]=0;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(int position,unsigned int *value)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"unsigned int");
  //m_paramFormatsIn[m_nParamsIn]=0;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(int position,unsigned long *value)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"unsigned long");
  //m_paramFormatsIn[m_nParamsIn]=0;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(int position,float *value)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"float");
  //m_paramFormatsIn[m_nParamsIn]=0;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(int position,double *value)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=0; // 此字段没有意义
  strcpy(m_paramTypesIn[m_nParamsIn],"double");
  //m_paramFormatsIn[m_nParamsIn]=0;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindin(int position,char *value,int len)
{
  if (m_nParamsIn>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesInPtr[m_nParamsIn]=(char *)value;
  m_paramLengthsIn[m_nParamsIn]=len;
  strcpy(m_paramTypesIn[m_nParamsIn],"char");
  //m_paramFormatsIn[m_nParamsIn]=0;

  if (len>PGMAXFIELDLENGTH) m_paramLengthsIn[m_nParamsIn]=PGMAXFIELDLENGTH;

  m_nParamsIn++;
  
  return 0;
}

int sqlstatement::bindout(int position,int *value)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"int");
  m_paramLengthsOut[m_nParamsOut]=sizeof(int);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(int position,long *value)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"long");
  m_paramLengthsOut[m_nParamsOut]=sizeof(long);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(int position,unsigned int *value)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"unsigned int");
  m_paramLengthsOut[m_nParamsOut]=sizeof(unsigned int);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(int position,unsigned long *value)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"unsigned long");
  m_paramLengthsOut[m_nParamsOut]=sizeof(unsigned long);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(int position,float *value)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"float");
  m_paramLengthsOut[m_nParamsOut]=sizeof(float);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(int position,double *value)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"double");
  m_paramLengthsOut[m_nParamsOut]=sizeof(double);

  m_nParamsOut++;

  return 0;
}

int sqlstatement::bindout(int position,char *value,int len)
{
  if (m_nParamsOut>=PGMAXFIELDCOUNT) return -1;

  m_paramValuesOut[m_nParamsOut]=(char *)value;
  strcpy(m_paramTypesOut[m_nParamsOut],"char");
  m_paramLengthsOut[m_nParamsOut]=len;

  m_nParamsOut++;

  return 0;
}

int sqlstatement::execute()
{
  memset(&cda,0,sizeof(cda));

  if (state == not_opened)
  {
    // 该代码不能启用，如果m_conn为空，就会造成Segmentation fault
    // memcpy(&m_conn->lda,&cda,sizeof(cda)); 

    cda.rc=-1; strcpy(cda.message,"cursor not opend.\n"); return -1;
  }

  // 判断是否是查询语句，如果是，把m_sqltype设为0，其它语句设为1。
  m_sqltype=1;

  // 从待执行的SQL语句中截取15个字符，如果这15个字符中包括了“select”，就认为是查询语句
  char strtemp[16]; memset(strtemp,0,sizeof(strtemp)); strncpy(strtemp,m_sql,15);

  for (UINT ii=0;ii<strlen(strtemp);ii++)
  {
    if ( (strtemp[ii] >= 97) && (strtemp[ii] <= 122) ) strtemp[ii]=strtemp[ii] - 32;
  }

  if ( (strstr(strtemp,"select") > 0) || (strstr(strtemp,"SELECT") > 0) ) m_sqltype=0;

  PQclear(m_res); m_res=0;

  m_respos=0;
  m_restotal=0;

  // 如果有输入参数
  if (m_nParamsIn > 0)
  {
    memset(m_paramValuesInVal,0,sizeof(m_paramValuesInVal));

    for (int ii=0;ii<m_nParamsIn;ii++)
    {
      if (strcmp(m_paramTypesIn[ii],"int") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%d",(int)(*(int *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"long") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%ld",(long)(*(long *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"unsigned int") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%u",(unsigned int)(*(unsigned int *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"unsigned long") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%lu",(unsigned long)(*(unsigned long *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"float") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%f",(float)(*(float *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"double") == 0)
      {
        sprintf(m_paramValuesInVal[ii],"%f",(double)(*(double *)m_paramValuesInPtr[ii]));
      }
      if (strcmp(m_paramTypesIn[ii],"char") == 0)
      {
        strncpy(m_paramValuesInVal[ii],m_paramValuesInPtr[ii],m_paramLengthsIn[ii]); 
        m_paramValuesInVal[ii][m_paramLengthsIn[ii]]=0;
      }
    }
  }

  m_res = PQexecParams(\
             (PGconn *)m_conn->m_conn,
         (const char *)m_sql,
                  (int)m_nParamsIn,       /* one param */
          (const Oid *)0,    /* let the backend deduce param type */
 (const char * const *)m_paramValuesIn,
          (const int *)m_paramLengthsIn,    /* don't need param lengths since text */
          (const int *)0,    /* default to all text params */
                  (int)0);      /* ask for binary results */

  // 如果不是查询语句
  if (m_sqltype == 1)
  {
    if (PQresultStatus(m_res) != PGRES_COMMAND_OK)
    {
      cda.rc=-1; strcpy(cda.message,PQerrorMessage(m_conn->m_conn)); 

      memcpy(&m_conn->lda,&cda,sizeof(cda)); 

      PQclear(m_res); m_res=0;

      return -1;
    }

    // 影响记录的行数
    cda.rpc = atoi(PQcmdTuples(m_res));
  }
  else
  {
    // 如果是查询语句
    if (PQresultStatus(m_res) != PGRES_TUPLES_OK)
    {
      cda.rc=-1; strcpy(cda.message,PQerrorMessage(m_conn->m_conn)); 

      memcpy(&m_conn->lda,&cda,sizeof(cda)); 

      return -1;
    }

    m_respos=0; m_restotal=PQntuples(m_res); 
  }

  return 0;
}

int sqlstatement::next()
{
  // 注意，在该函数中，不可随意用memset(&cda,0,sizeof(cda))，否则会清空cda.rpc的内容
  if (state == not_opened)
  {
    cda.rc=-1; strcpy(cda.message,"cursor not opend.\n"); return -1;
  }

  // 如果语句未执行成功，直接返回失败。
  if (cda.rc != 0) return cda.rc;

  // 判断是否是查询语句，如果不是，直接返回错误
  if (m_sqltype != 0)
  {
    cda.rc=-1; strcpy(cda.message,"no recordset found.\n"); return -1;
  }

  if (cda.rpc>=(unsigned long)PQntuples(m_res))
  {
    cda.rc=1403; strcpy(cda.message,"no data found"); return cda.rc;
  }

  for (int ii=0;ii<m_nParamsOut;ii++)
  {
    if (strcmp(m_paramTypesOut[ii],"int" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,cda.rpc,ii);
      int ival = atoi(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"long" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,cda.rpc,ii);
      long ival = atol(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"unsigned int" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,cda.rpc,ii);
      unsigned int ival = (unsigned int)atoi(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"unsigned long" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,cda.rpc,ii);
      unsigned long ival = (unsigned long)atol(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"float" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,cda.rpc,ii);
      float ival = atof(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"double" ) == 0) 
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);

      char *iptr=PQgetvalue(m_res,cda.rpc,ii);
      double ival = atof(iptr); 
      memcpy(m_paramValuesOut[ii],&ival,m_paramLengthsOut[ii]);
    }

    if (strcmp(m_paramTypesOut[ii],"char") == 0)  
    {
      memset(m_paramValuesOut[ii],0,m_paramLengthsOut[ii]);
      
      int len=0;
      if (m_paramLengthsOut[ii]>PQgetlength(m_res,cda.rpc,ii))
        len=PQgetlength(m_res,cda.rpc,ii);
      else
        len=m_paramLengthsOut[ii];
      
      strncpy(m_paramValuesOut[ii],PQgetvalue(m_res,cda.rpc,ii),len);

      m_paramValuesOut[ii][len]=0;
    }
  }

  cda.rpc++;

  return cda.rc;
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
  stmt.prepare("select to_char(now()+%d/86400,'%s')",interval,strfmt);
  stmt.bindout(1,stime,strlen(strfmt));

  if (stmt.execute() != 0) return stmt.cda.rc;

  stmt.next();

  return stmt.cda.rc;
}

// 绑定lob指针
int  sqlstatement::bindblob(int position)
{
  return 0;
}

int  sqlstatement::bindclob(int position)
{
  return 0;
}


// 以下两个函数都是把文件中的内容写入LOB字段，第二个函数是被第一个函数调用的
int  sqlstatement::filetolob(char *filename)
{
  return 0;
}

int  sqlstatement::filetolob(FILE *fp)
{
  return 0;
}


// 把LOB字段中的内容写入文件
int  sqlstatement::lobtofile(char *filename)
{
  return 0;
}

int  sqlstatement::lobtofile(FILE *fp)
{
  return 0;
}


// 把BLOB文件导入到表的字段内容中，要求表一定要用keyid字段，0-成功，1-记录不存在，>1-其它错误
long filetoblob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename)
{
  return 0;
}

// 把CLOB文件导入到表的字段内容中，要求表一定要用keyid字段，0-成功，1-记录不存在，>1-其它错误
long filetoclob(connection *conn,char *tablename,char *fieldname,UINT keyid,char *filename)
{
  return 0;
}

