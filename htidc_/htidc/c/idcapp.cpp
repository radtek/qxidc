#include "idcapp.h"

CSYNCTABLE::CSYNCTABLE()
{
  m_conndst=m_connsrc=0;
  m_logfile=0;
  memset(m_dblink,0,sizeof(m_dblink));
}

CSYNCTABLE::~CSYNCTABLE()
{
}

void CSYNCTABLE::BindConnLog(connection *in_conndst,connection *in_connsrc,CLogFile *in_logfile)
{
  m_conndst=in_conndst;
  m_connsrc=in_connsrc;
  m_logfile=in_logfile;
}

// 从数据源数据库的数据字典中载入待同步的表的结构，用小写存放
long CSYNCTABLE::LoadTInfo(char *strTNameStr)
{
  memset(m_tnamessrc,0,sizeof(m_tnamessrc));
  memset(m_tnamesdst,0,sizeof(m_tnamesdst));
  memset(m_columns,0,sizeof(m_columns));

  CTABFIELD TABFIELD;

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strTNameStr,",");

  for (UINT ii=0;ii<CmdStr.m_vCmdStr.size();ii++)
  {
    strcpy(m_tnamessrc[ii],CmdStr.m_vCmdStr[ii].c_str());

    // 检查目的数据库表是否存在，目的表在源表名前加“T_”，如果源表已经是“T_”打头，就不加了
    memset(m_tnamesdst[ii],0,sizeof(m_tnamesdst[ii]));
    if (strncmp(m_tnamessrc[ii],"T_",2) == 0)
      strcpy(m_tnamesdst[ii],m_tnamessrc[ii]);
    else
      sprintf(m_tnamesdst[ii],"T_%s",m_tnamessrc[ii]);

    TABFIELD.GetALLField(m_connsrc,m_tnamessrc[ii],"crttime,sync_rowid,keyid");

    strcpy(m_columns[ii],TABFIELD.m_allfieldstr);
  }

  return 0;
}

// 从数据源数据库的T_SYNCLOG_同步标志表中载入全部未同步的日志
long CSYNCTABLE::LoadSyncLog(char *strSyncName)
{
  m_vSYNCTABLE.clear(); // 存放等同步的数据的日志

  sqlstatement stmt;
  stmt.connect(m_connsrc);
  stmt.prepare(\
    "select rowid,keyid,tname,ctype,sync_rowid,to_char(crttime,'yyyy-mm-dd hh24:mi:ss')\
       from T_SYNCLOG%s order by tname desc,keyid",strSyncName);
  stmt.bindout(1, m_stSYNCTABLE.rowid,30);
  stmt.bindout(2,&m_stSYNCTABLE.keyid);
  stmt.bindout(3, m_stSYNCTABLE.tname,30);
  stmt.bindout(4,&m_stSYNCTABLE.ctype);
  stmt.bindout(5, m_stSYNCTABLE.sync_rowid,30);
  stmt.bindout(6, m_stSYNCTABLE.crttime,19);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("select T_SYNCLOG%s failed.\n%s\n",strSyncName,stmt.cda.message); return stmt.cda.rc;
  }


  while (TRUE)
  {
    memset(&m_stSYNCTABLE,0,sizeof(m_stSYNCTABLE));

    if (stmt.next() != 0) break;

    m_vSYNCTABLE.push_back(m_stSYNCTABLE);

    //// 如果待同步的记录超过10万，将导致m_vSYNCTABLE容器过大，所以加上以下判断，以防万一。
    //if (stmt.cda.rpc > 100000) break;
    // 如果待同步的记录超过3000，将导致m_vSYNCTABLE容器过大，所以加上以下判断，以防万一。
    if (stmt.cda.rpc > 3000) break;
  }

  return 0;
}

// 根据m_vSYNCTABLE中的日志同步
long CSYNCTABLE::SyncDATA(CProgramActive *ProgramActive,char *strSyncName)
{
  UINT iCount=0;

  for (UINT ii=0;ii<m_vSYNCTABLE.size();ii++)
  {
    if (iCount>50) { iCount=0; ProgramActive->WriteToFile(); }

    iCount++;

    memset(&m_stSYNCTABLE,0,sizeof(m_stSYNCTABLE));

    memcpy(&m_stSYNCTABLE,&m_vSYNCTABLE[ii],sizeof(m_stSYNCTABLE));

    m_logfile->Write("keyid=%ld,tname=%s,ctype=%d,sync_rowid=%s,crttime=%s...",m_stSYNCTABLE.keyid,\
                      m_stSYNCTABLE.tname,m_stSYNCTABLE.ctype,m_stSYNCTABLE.sync_rowid,m_stSYNCTABLE.crttime);

    // 把数据源表的记录更新新数据中心的表
    if (SyncTableToIDC(strSyncName) != 0) return -1;

    m_logfile->WriteEx("sync ok...");

    // 从数据源数据库的T_SYNCLOG_同步标志表中删除一条已同步的记录的日志
    if (ClearSyncLog(strSyncName) != 0) return -1;

    m_logfile->WriteEx("clear log ok.\n");

    m_connsrc->commitwork(); m_conndst->commitwork();
  }

  return 0;
}

// 把数据源表的记录更新新数据中心的表
long CSYNCTABLE::SyncTableToIDC(char *strSyncName)
{
  UINT ii=0;
  BOOL bFindTable=FALSE;

  for (ii=0;strlen(m_tnamessrc[ii])>0;ii++)
  {
    if (strcmp(m_tnamessrc[ii],m_stSYNCTABLE.tname) == 0)
    {
      bFindTable=TRUE; break;
    }
  }

  if (bFindTable==FALSE) return 0;

  sqlstatement stmt;
  stmt.connect(m_conndst);

  // 新增记录
  if (m_stSYNCTABLE.ctype == 1)
  {
    stmt.prepare("\
      insert into %s(%s,crttime,keyid,sync_rowid) select %s,sysdate,SEQ_%s.nextval,rowid from %s.%s@%s where rowid='%s'",\
      m_tnamesdst[ii],m_columns[ii],m_columns[ii],m_tnamesdst[ii]+2,m_connsrc->env.user,m_tnamessrc[ii],m_dblink,m_stSYNCTABLE.sync_rowid);
  }

  // 修改记录
  if (m_stSYNCTABLE.ctype == 2)
  {
    stmt.prepare("\
      update %s set (%s,crttime)=(select %s,sysdate from %s.%s@%s where rowid='%s') where sync_rowid='%s'",\
      m_tnamesdst[ii],m_columns[ii],m_columns[ii],m_connsrc->env.user,m_tnamessrc[ii],m_dblink,m_stSYNCTABLE.sync_rowid,m_stSYNCTABLE.sync_rowid);
  }

  // 删除记录
  if (m_stSYNCTABLE.ctype == 3)
  {
    stmt.prepare("delete from %s where sync_rowid='%s'",m_tnamesdst[ii],m_stSYNCTABLE.sync_rowid);
  }

  if (stmt.execute() != 0)
  {
    // 如果是主键冲突，记录错误日志，但函数仍返回成功，不影响同步的继续
    if (stmt.cda.rc == 1)
    {
      m_logfile->Write("SyncTableToIDC FAILED.\n%s\n",stmt.cda.message); return 0;
    }

    m_logfile->Write("SyncTableToIDC failed.\n%s\n%s\n",stmt.cda.message,stmt.m_sql);
  }

  return stmt.cda.rc;
}

// 从数据源数据库的T_SYNCLOG_同步标志表中删除一条已同步的记录的日志
long CSYNCTABLE::ClearSyncLog(char *strSyncName)
{
  sqlstatement stmt;
  stmt.connect(m_connsrc);
  stmt.prepare("delete from T_SYNCLOG%s where rowid=:1",strSyncName);
  stmt.bindin(1, m_stSYNCTABLE.rowid,30);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("ClearSyncLog delete T_SYNCLOG%s failed.\n%s\n",strSyncName,stmt.cda.message);
  }

  return stmt.cda.rc;
}

// 检查表是否存在
BOOL CheckTExist(connection *in_conn,char *in_tablename)
{
  int iCount=0;
  sqlstatement stmt;
  stmt.connect(in_conn);
  stmt.prepare("select count(*) from USER_TABLES where table_name=upper(:1)");
  stmt.bindin(1,in_tablename,30);
  stmt.bindout(1,&iCount);
  stmt.execute();
  stmt.next();

  if (iCount == 0) return FALSE;

  return TRUE;
}

// 检查序列是否存在
BOOL CheckSEQExist(connection *in_conn,char *in_seqname)
{
  int iCount=0;
  sqlstatement stmt;
  stmt.connect(in_conn);
  stmt.prepare("select count(*) from USER_SEQUENCES where sequence_name=upper(:1)");
  stmt.bindin(1,in_seqname,30);
  stmt.bindout(1,&iCount);
  stmt.execute();
  stmt.next();

  if (iCount == 0) return FALSE;

  return TRUE;
}

// 检查触发器是否存在
BOOL CheckTRExist(connection *in_conn,char *in_trname)
{
  int iCount=0;
  sqlstatement stmt;
  stmt.connect(in_conn);
  stmt.prepare("select count(*) from USER_TRIGGERS where trigger_name=upper(:1)");
  stmt.bindin(1,in_trname,30);
  stmt.bindout(1,&iCount);
  stmt.execute();
  stmt.next();

  if (iCount == 0) return FALSE;

  return TRUE;
}

CTABFIELD::CTABFIELD()
{
  memset(m_pkfieldstr,0,sizeof(m_pkfieldstr));
  memset(m_allfieldstr,0,sizeof(m_allfieldstr));
  initdata();
}

CTABFIELD::~CTABFIELD()
{
  // 存放主键字段信息的容器
  m_vPKFIELD.clear();
  // 存放全部字段信息的容器
  m_vALLFIELD.clear();
}

void CTABFIELD::initdata()
{
  m_pkcount=0;    // 主键字段的个数
  m_fieldcount=0; // 全部字段的个数

  // 全部的主键字段，以字符串存放，中间用半角的逗号分隔
  memset(m_pkfieldstr,0,sizeof(m_pkfieldstr));
  // 全部的字段，以字符串存放，中间用半角的逗号分隔
  memset(m_allfieldstr,0,sizeof(m_allfieldstr));

  memset(&m_stTABFIELD,0,sizeof(m_stTABFIELD));

  // 存放主键字段信息的容器
  m_vPKFIELD.clear();
  // 存放全部字段信息的容器
  m_vALLFIELD.clear();
}

// 获取指定表的主键字段信息，TRUE-获取成功，FALSE-获取失败
BOOL CTABFIELD::GetPKField(connection *conn,char *tablename)
{
  // 主键字段的个数
  m_pkcount=0;
  // 全部的主键字段，以字符串存放，中间用半角的逗号分隔
  memset(m_pkfieldstr,0,sizeof(m_pkfieldstr));
  // 存放主键字段信息的容器
  m_vPKFIELD.clear();

  sqlstatement stmt;
  stmt.connect(conn);

  if (strcmp(conn->m_dbtype,"oracle")==0)
  {
    stmt.prepare("\
      select lower(column_name) from USER_CONS_COLUMNS\
       where table_name=upper(:1)\
         and constraint_name=(select constraint_name from USER_CONSTRAINTS\
                               where table_name=upper(:2) and constraint_type='P'\
                                 and generated='USER NAME')\
       order by position");
    stmt.bindin(1,tablename,30);
    stmt.bindin(2,tablename,30);
    stmt.bindout(1,m_stTABFIELD.fieldname,30);
  }
  if (strcmp(conn->m_dbtype,"pg")==0)
  {
    char strindkey[51];
    memset(strindkey,0,sizeof(strindkey));
  
    stmt.prepare("\
      select indkey from pg_index where indrelid=(select relid from pg_statio_user_tables\
       where relname=lower(:1) and indisprimary='t')");
    stmt.bindin(1,tablename,30);
    stmt.bindout(1,strindkey,30);
    stmt.execute();
    stmt.next();
  
    if (strlen(strindkey)==0) return FALSE;
  
    UpdateStr(strindkey," ",",");
  
    stmt.prepare("\
      select attname from PG_ATTRIBUTE where attrelid=(select relid from PG_STATIO_USER_TABLES where relname=lower(:1) and attnum in (%s))",strindkey);
    stmt.bindin(1,tablename,30);
    stmt.bindout(1,m_stTABFIELD.fieldname,30);
  }

  if (strcmp(conn->m_dbtype,"mysql")==0)
  {
    /*
    stmt.prepare("\
                   select lower(column_name) from information_schema.STATISTICS\
                     where table_schema = '%s' and table_name= '%s' and UPPER(index_name)='PRIMARY'\
       order by seq_in_index",conn->env.dbname,tablename); 
    */

    stmt.prepare("select lower(column_name) from information_schema.STATISTICS where table_name= '%s' and UPPER(index_name)='PRIMARY' order by seq_in_index",tablename);

    stmt.bindout(1,m_stTABFIELD.fieldname,30);
  }

  if (stmt.execute() != 0) return FALSE;
  
  for (UINT ii=0;ii<20;ii++)
  {
    memset(&m_stTABFIELD,0,sizeof(m_stTABFIELD));

    if (stmt.next() != 0) break;

    if (stmt.cda.rpc != 1) strcat(m_pkfieldstr,",");

    strcat(m_pkfieldstr,m_stTABFIELD.fieldname);

    m_vPKFIELD.push_back(m_stTABFIELD);
  }

  m_pkcount=stmt.cda.rpc;

  return TRUE;
}

// 获取指定表的全部字段信息，TRUE-获取成功，FALSE-获取失败
BOOL CTABFIELD::GetALLField(connection *conn,char *tablename,const char *exceptfields,BOOL ISDBLINK)
{
  // 全部字段的个数
  m_fieldcount=0;

  // 全部的字段，以字符串存放，中间用半角的逗号分隔
  memset(m_allfieldstr,0,sizeof(m_allfieldstr));

  // 存放全部字段信息的容器
  m_vALLFIELD.clear();

  UINT data_precision=0;

  sqlstatement stmt;
  stmt.connect(conn);

  if (strcmp(conn->m_dbtype,"oracle")==0)
  {
    if (ISDBLINK == FALSE)
    {
      stmt.prepare("\
        select lower(column_name),lower(data_type),data_length,data_precision from USER_TAB_COLUMNS\
         where table_name=upper('%s') order by column_id",tablename);
    }
    else
    {
      char strdblink[101],*strpos=0,strtablename[51];
      memset(strdblink,0,sizeof(strdblink));
      memset(strtablename,0,sizeof(strtablename));

      if ((strpos = strstr(tablename,"@")) == 0) return FALSE;

      strncpy(strtablename,tablename,strlen(tablename)-strlen(strpos));
      strncpy(strdblink,strpos,100);

      stmt.prepare("\
        select lower(column_name),lower(data_type),data_length,data_precision from USER_TAB_COLUMNS%s\
         where table_name=upper('%s') order by column_id",strdblink,strtablename);
    }
  }

  if (strcmp(conn->m_dbtype,"pg")==0)
  {
    // pg数据库不能用synctable_update3程序同步数据
    if (ISDBLINK == TRUE) return FALSE;
 
    stmt.prepare("\
      select attname,typname,2000 data_length,2000 data_precision from PG_CLASS C,PG_ATTRIBUTE A,PG_TYPE T where C.relname =lower('%s')  and C.oid = attrelid and atttypid = T.oid AND attnum > 0",tablename);
  }

  if (strcmp(conn->m_dbtype,"mysql")==0)
  {
    // mysql数据库不能用synctable_update3程序同步数据
    if (ISDBLINK == TRUE) return FALSE;
 
     // select lower(column_name),lower(data_type),character_maximum_length,2000 data_precision from information_schema.`COLUMNS` where table_schema='%s' and table_name='%s'",conn->env.dbname,tablename);
     
    stmt.prepare("select lower(column_name),lower(data_type),character_maximum_length,2000 data_precision from information_schema.COLUMNS where table_name='%s'",tablename);

  }

  //stmt.bindin(1,tablename,30);
  stmt.bindout(1, m_stTABFIELD.fieldname,30);
  stmt.bindout(2, m_stTABFIELD.datatype,106);
  stmt.bindout(3,&m_stTABFIELD.datalen);
  stmt.bindout(4,&data_precision);

  if (stmt.execute() != 0) return FALSE;

  while (TRUE)
  {
    data_precision=0;
    memset(&m_stTABFIELD,0,sizeof(m_stTABFIELD));

    if (stmt.next() != 0) break;

    if (strcmp(m_stTABFIELD.datatype,"bpchar")==0) strcpy(m_stTABFIELD.datatype,"char");
    if (strcmp(m_stTABFIELD.datatype,"timestamp")==0) strcpy(m_stTABFIELD.datatype,"date");
    if (strcmp(m_stTABFIELD.datatype,"datetime")==0) strcpy(m_stTABFIELD.datatype,"date");
    if (strcmp(m_stTABFIELD.datatype,"numeric")==0) strcpy(m_stTABFIELD.datatype,"number");
    if (strcmp(m_stTABFIELD.datatype,"decimal")==0) strcpy(m_stTABFIELD.datatype,"number");
    if (strcmp(m_stTABFIELD.datatype,"int")==0) strcpy(m_stTABFIELD.datatype,"number");
    if (strcmp(m_stTABFIELD.datatype,"bigint")==0) strcpy(m_stTABFIELD.datatype,"number");
    if (strcmp(m_stTABFIELD.datatype,"varchar")==0) strcpy(m_stTABFIELD.datatype,"varchar2");

    // 判断是否有不需要获取的字段
    if (exceptfields != 0)
    {
      if (strlen(exceptfields)>0)
      {
        BOOL bIn=FALSE;

        CCmdStr CmdStr;
        CmdStr.SplitToCmd(exceptfields,",");
        for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
        {
          if (strcmp(m_stTABFIELD.fieldname,CmdStr.m_vCmdStr[ii].c_str())==0) {bIn=TRUE; break; }
        }

        if (bIn==TRUE) continue;
      }
    }

    // 如果字段类型是date,把长度设置为19
    if (strcmp(m_stTABFIELD.datatype,"date")==0) m_stTABFIELD.datalen=19;

    // 如果字段类型是number,把长度设置为20,以前是用data_precision
    if (strcmp(m_stTABFIELD.datatype,"number")==0) m_stTABFIELD.datalen=20;

    // 字段总长度不能超过2000个字符长度
    if (m_stTABFIELD.datalen > 2000) m_stTABFIELD.datalen=2000;

    if (stmt.cda.rpc != 1) strcat(m_allfieldstr,",");

    strcat(m_allfieldstr,m_stTABFIELD.fieldname);


    m_vALLFIELD.push_back(m_stTABFIELD);
  }

  m_fieldcount=stmt.cda.rpc;

  return TRUE;
}

CFILEFTPTASK::CFILEFTPTASK()
{
  m_conn=0;
  m_logfile=0;

  initdata();

  memset(&m_stLocalFile,0,sizeof(m_stLocalFile));

  memset(m_validdays,0,sizeof(m_validdays));
}

void CFILEFTPTASK::initdata()
{
  taskid=0;
  memset(taskname,0,sizeof(taskname));
  ftptype=0;
  memset(remoteip,0,sizeof(remoteip));
  port=0;
  memset(username,0,sizeof(username));
  memset(password,0,sizeof(password));
  ftpcmode=0;
  memset(localpath,0,sizeof(localpath));
  dstpathmatchbz=0;
  memset(dstpath,0,sizeof(dstpath));
  memset(matchstr,0,sizeof(matchstr));
  checkend=0;
  cmodtime=0;
  cfilesize=0;
  validdays=0;
  memset(filesizestr,0,sizeof(filesizestr));
  deletebz=0;
  renamebz=0;
  memset(renamepath,0,sizeof(renamepath));
  synctvl=0;
  memset(synctime,0,sizeof(synctime));
  alarmbz=0;
  memset(mobilenostr,0,sizeof(mobilenostr));
  ftpsts=0;
  memset(faulttime,0,sizeof(faulttime));
  rsts=0;

  memset(m_validdays,0,sizeof(m_validdays));
};

void CFILEFTPTASK::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn = in_conn;
  m_logfile = in_logfile;
}

// 把已发送的文件的信息插入文件分发日志表
long CFILEFTPTASK::InsertPutList()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare(\
    "insert into T_FILEPUTLIST(filename,taskid,modtime,filesize,ddatetime) values(:1,:2,to_date(:3,'yyyy-mm-dd hh24:mi:ss'),:4,sysdate)");
  stmt.bindin(1, m_stLocalFile.filename,200);
  stmt.bindin(2,&m_stLocalFile.taskid);
  stmt.bindin(3, m_stLocalFile.modtime,19);
  stmt.bindin(4,&m_stLocalFile.filesize);

  if (stmt.execute() != 0)
  {
    if (stmt.cda.rc != 1)
    {
      m_logfile->Write("InsertPutList insert T_FILEPUTLIST failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
    }
  }

  return 0;
}

CFILEFTPTASK::~CFILEFTPTASK()
{
}

// 从FILEFTPTASK表中提取执行时间（synctime）小于等于当前时间的全部记录。
long CFILEFTPTASK::LoadFtpTask(char *strRemoteIP)
{
  if (selstmt.state == selstmt.not_opened)
  {
    selstmt.connect(m_conn);

    selstmt.prepare("\
      select taskid,taskname,ftptype,remoteip,port,username,password,trim(dstpath),\
             trim(localpath),matchstr,cfilesize,cmodtime,filesizestr,deletebz,renamebz,\
             trim(renamepath),validdays,to_char(sysdate-validdays,'yyyymmddhh24miss'),\
             dstpathmatchbz,checkend,ftpcmode\
        from T_FILEFTPTASK where remoteip='%s' and (synctime<=sysdate or synctime is null)\
         and rsts=1 order by synctime",strRemoteIP);
    selstmt.bindout( 1,&taskid);
    selstmt.bindout( 2, taskname,100);
    selstmt.bindout( 3,&ftptype);
    selstmt.bindout( 4, remoteip,50);
    selstmt.bindout( 5,&port);
    selstmt.bindout( 6, username,50);
    selstmt.bindout( 7, password,50);
    selstmt.bindout( 8, dstpath,200);
    selstmt.bindout( 9, localpath,200);
    selstmt.bindout(10, matchstr,1024);
    selstmt.bindout(11,&cfilesize);
    selstmt.bindout(12,&cmodtime);
    selstmt.bindout(13, filesizestr,50);
    selstmt.bindout(14,&deletebz);
    selstmt.bindout(15,&renamebz);
    selstmt.bindout(16, renamepath,200);
    selstmt.bindout(17,&validdays);
    selstmt.bindout(18, m_validdays,14);
    selstmt.bindout(19,&dstpathmatchbz);
    selstmt.bindout(20,&checkend);
    selstmt.bindout(21,&ftpcmode);
  }

  if (selstmt.execute() != 0)
  {
    m_logfile->Write("LoadFtpTask select T_FILEFTPTASK failed.\n%s\n",selstmt.cda.message);
  }

  return selstmt.cda.rc;
}

long CFILEFTPTASK::LoadFtpTaskNext()
{
  initdata();

  selstmt.next();

  // 如果对方服务器的目录为*，就清空它，方便后续的程序处理。
  if (strcmp(dstpath,"*") == 0) memset(dstpath,0,sizeof(dstpath));

  if (selstmt.cda.rc == 1405) return 0;

  return selstmt.cda.rc;
}

// 更新FILEFTPTASK表的ftpsts
long CFILEFTPTASK::UptFtpSTS()
{
  sqlstatement stmt;
  stmt.connect(m_conn);

  // 正常
  if (ftpsts == 1)
  {
    stmt.prepare("update T_FILEFTPTASK set synctime=sysdate+synctvl/(1*24*60*60),ftpsts=1,faulttime=null where taskid=:1");
    stmt.bindin(1,&taskid);
  }

  // socket连接失败
  if (ftpsts == 2)
  {
    stmt.prepare("update T_FILEFTPTASK set ftpsts=2,faulttime=sysdate where remoteip=:1");
    stmt.bindin(1, remoteip,50);
  }

  // login失败
  if (ftpsts == 3)
  {
    stmt.prepare("update T_FILEFTPTASK set ftpsts=3,faulttime=sysdate where taskid=:1");
    stmt.bindin(1,&taskid);
  }


  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptFtpSTS update T_FILEFTPTASK failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  // 如果状态是正常，还需要把与对方服务器有关的FTP状态为连接失败的任务全部更新为正常
  if (ftpsts == 1)
  {
    stmt.prepare("update T_FILEFTPTASK set ftpsts=1,faulttime=null where remoteip=:1 and ftpsts=2");
    stmt.bindin(1,remoteip,50);
    stmt.execute();
  }

  return stmt.cda.rc;
}

// 获取最近需要执行的任务记录的时间和当前时间间隔的秒数
long CFILEFTPTASK::SelLeastTimeTvl(char *strRemoteIP)
{
  m_sleeptvl=0;

  sqlstatement selstmttmp;
  selstmttmp.connect(m_conn);
  selstmttmp.prepare("\
       select (min(synctime)-sysdate)*1*24*60*60 from T_FILEFTPTASK where remoteip='%s' and rsts=1",strRemoteIP);
  selstmttmp.bindout(1,&m_sleeptvl);

  if (selstmttmp.execute() != 0)
  {
    m_logfile->Write("SelLeastTimeTvl select T_FILEFTPTASK failed.\n%s\n",selstmttmp.cda.message); return selstmttmp.cda.rc;
  }

  if (selstmttmp.next() != 0) m_sleeptvl=60;

  if (m_sleeptvl < 10) m_sleeptvl=10;

  if (m_sleeptvl > 60) m_sleeptvl=60;

  return 0;
}

CFILEFTPLIST::CFILEFTPLIST()
{
  m_conn=0; m_logfile=0;

  initdata();

  memset(&m_stRemoteFile,0,sizeof(m_stRemoteFile));
}

CFILEFTPLIST::~CFILEFTPLIST()
{
  m_vRemoteFile.clear();
}

void CFILEFTPLIST::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;
  m_logfile=in_logfile;
  m_taskid=0;
}

void CFILEFTPLIST::initdata()
{
  m_taskid=0;
}

// 从已获取的文件清单中获取文件的大小和修改日期
long CFILEFTPLIST::FindFList()
{
  if (selliststmt.state == selliststmt.not_opened)
  {
    selliststmt.connect(m_conn);
    selliststmt.prepare("select to_char(modtime,'yyyymmddhh24miss'),filesize from T_FILEFTPLIST where filename=:1 and taskid=:2");
    selliststmt.bindin(1, m_stRemoteFile.filename,200);
    selliststmt.bindin(2,&m_taskid);
    selliststmt.bindout(1, m_stRemoteFile.modtime,14);
    selliststmt.bindout(2,&m_stRemoteFile.filesize);
  }

  if (selliststmt.execute() != 0)
  {
    m_logfile->Write("FindFList select T_FILEFTPLIST failed.\n%s\n",selliststmt.cda.message);
  }

  return selliststmt.cda.rc;
}

long CFILEFTPLIST::FindFListNext()
{
  // 注意，在这个函数中，不能初始化以下结构，否则出错，它的初始化由ftpserver.cpp中执行
  // memset(&m_stRemoteFile,0,sizeof(m_stRemoteFile));

  return selliststmt.next();
}

// 把同步成功的文件信息插入FILEFTPLIST表中
long CFILEFTPLIST::InsertList()
{
  if (insliststmt.state == insliststmt.not_opened)
  {
    insliststmt.connect(m_conn);
    insliststmt.prepare("\
      BEGIN\
        delete from T_FILEFTPLIST where taskid=:1 and filename=:2;\
        insert into T_FILEFTPLIST(taskid,filename,modtime,filesize) values(:3,:4,to_date(:5,'yyyymmddhh24miss'),:6);\
      END;");
    insliststmt.bindin(1,&m_taskid);
    insliststmt.bindin(2, m_stRemoteFile.filename,200);
    insliststmt.bindin(3,&m_taskid);
    insliststmt.bindin(4, m_stRemoteFile.filename,200);
    insliststmt.bindin(5, m_stRemoteFile.modtime,14);
    insliststmt.bindin(6,&m_stRemoteFile.filesize);
  }

  if (insliststmt.execute() != 0)
  {
    m_logfile->Write("InsertList update T_FILEFTPLIST failed.\n%s\n",insliststmt.cda.message);
  }

  return insliststmt.cda.rc;
}


CQXDATA::CQXDATA()
{
  m_conn=0; m_logfile=0;
  // m_checksts=9;
  // m_bexistcheckfield=FALSE;
  // m_limitcheck=m_timecheck=m_spacecheck=2;
  m_bexistcrttimefield=FALSE;
  memset(m_dtypename,0,sizeof(m_dtypename));
  memset(m_pfilename,0,sizeof(m_pfilename));
  memset(m_tname,0,sizeof(m_tname));
  memset(m_dmintime,0,sizeof(m_dmintime));
  memset(m_addatetime,0,sizeof(m_addatetime));
  memset(m_filename,0,sizeof(m_filename));
  memset(m_KEYFIELDSTR,0,sizeof(m_KEYFIELDSTR));
  memset(m_rowid,0,sizeof(m_rowid));
  memset(m_SelectSQL,0,sizeof(m_SelectSQL));
  memset(m_UpdateSQL,0,sizeof(m_UpdateSQL));
  memset(m_InsertSQL,0,sizeof(m_InsertSQL));
}

// 根据T_ALLTABLE表的质控字段，生成质控标志字段的值
/*
long CQXDATA::AnalyCheckSTS()
{
  m_checksts=9;
  m_limitcheck=m_timecheck=m_spacecheck=2;

  // 如果基数据表没有checksts字段，就返回，不必判断质控标志字段
  if (m_bexistcheckfield==FALSE) return 0;

  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("select limitcheck,timecheck,spacecheck from T_ALLTABLE where tname=upper(:1)");
  stmt.bindin(1,m_tname,30);
  stmt.bindout(1,&m_limitcheck);
  stmt.bindout(2,&m_timecheck);
  stmt.bindout(3,&m_spacecheck);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("AnalyCheckSTS select %s failed.\n%s\n",m_tname,stmt.cda.message); 
    return stmt.cda.rc;
  }

  stmt.next();
  
  if (m_limitcheck==1) 
  {
    m_checksts=0; return 0;
  }

  if (m_timecheck==1) 
  {
    m_checksts=1; return 0;
  }

  if (m_spacecheck==1) 
  {
    m_checksts=2; return 0;
  }

  return 0;
}
*/

CQXDATA::~CQXDATA()
{
  m_vAPPDTYPE.clear();
  m_vTABCOLUMNS.clear();
}

void CQXDATA::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn; m_logfile=in_logfile;
}

void CQXDATA::initdata()
{
  m_dtypeid=0;
  memset(m_dtypename,0,sizeof(m_dtypename));
  memset(m_pfilename,0,sizeof(m_pfilename));
  memset(m_tname,0,sizeof(m_tname));
  memset(m_dmintime,0,sizeof(m_dmintime));
  memset(m_addatetime,0,sizeof(m_addatetime));
  memset(m_filename,0,sizeof(m_filename));
  m_upttype=0;
  m_upttlimit=0;
}

// 把全部的气象文件参数载入容器
long CQXDATA::LoadFileCFG()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  if (strcmp(m_conn->m_dbtype,"mysql")==0)
  {
    stmt.prepare("\
      select dtypeid,dtypename,pfilename,tname,date_format(dmintime,'%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s'),\
             upttype,upttlimit,addatetime,length(pfilename) pfilenamelen\
        from T_APPDTYPE where rsts=1 order by pfilenamelen desc");
  }
  else
  {
    stmt.prepare("\
      select dtypeid,dtypename,pfilename,tname,to_char(dmintime,'yyyymmddhh24miss'),\
             upttype,upttlimit,addatetime,length(pfilename) pfilenamelen\
        from T_APPDTYPE where rsts=1 order by pfilenamelen desc");
  }
  stmt.bindout( 1,&m_stAPPDTYPE.dtypeid);
  stmt.bindout( 2, m_stAPPDTYPE.dtypename,50);
  stmt.bindout( 3, m_stAPPDTYPE.pfilename,201);
  stmt.bindout( 4, m_stAPPDTYPE.tname,30);
  stmt.bindout( 5, m_stAPPDTYPE.dmintime,14);
  stmt.bindout( 6,&m_stAPPDTYPE.upttype);
  stmt.bindout( 7,&m_stAPPDTYPE.upttlimit);
  stmt.bindout( 8, m_stAPPDTYPE.addatetime,200);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("LoadFileCFG select T_APPDTYPE failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }

  m_vAPPDTYPE.clear();

  while (TRUE)
  {
    memset(&m_stAPPDTYPE,0,sizeof(m_stAPPDTYPE));

    if (stmt.next() != 0) break;

    // 判断是否需要处理buffer中的时间变量
    // 可以处理以下时间变量：YYYY（4位的年）、YYY（后三位的年）、
    // YY（后两位的年）、MM（月月）、DD（日日）、HH（时时）、MI（分分）、SS（秒秒）
    MatchBuffer(m_stAPPDTYPE.pfilename);
    MatchBuffer(m_stAPPDTYPE.tname);

    ToUpper(m_stAPPDTYPE.tname);

    m_vAPPDTYPE.push_back(m_stAPPDTYPE);
  }

  return 0;
}

// 从容器中获取某种类型文件的参数
BOOL CQXDATA::GETFILECFG()
{
  UINT uPOS;

  for (uPOS=0;uPOS<m_vAPPDTYPE.size();uPOS++)
  {
    if ( (strncmp(m_vAPPDTYPE[uPOS].pfilename,m_filename,strlen(m_vAPPDTYPE[uPOS].pfilename)) == 0) ||
         (MatchFileName(m_filename,m_vAPPDTYPE[uPOS].pfilename) == TRUE) )
    {
      m_dtypeid=m_vAPPDTYPE[uPOS].dtypeid;
      strcpy(m_dtypename,m_vAPPDTYPE[uPOS].dtypename);
      strcpy(m_pfilename,m_vAPPDTYPE[uPOS].pfilename);
      strcpy(m_addatetime,m_vAPPDTYPE[uPOS].addatetime);
      strcpy(m_tname,m_vAPPDTYPE[uPOS].tname);
      strcpy(m_dmintime,m_vAPPDTYPE[uPOS].dmintime);
      m_upttype=m_vAPPDTYPE[uPOS].upttype;
      m_upttlimit=m_vAPPDTYPE[uPOS].upttlimit;

      // 对PG数据库，该字段暂时不启用，强制为0。
      if (strcmp(m_conn->m_dbtype,"pg")==0) m_upttlimit=0;

      return TRUE;
    }
  }

  return FALSE;
}

// 获取表的全部的列信息，存放在m_vTABCOLUMNS容器中
long CQXDATA::GETTABCOLUMNS()
{
  m_ddtfieldpos=-1;
  m_bnokeyidfield=TRUE;
  m_bexistcrttimefield=FALSE;

  m_TotalCount=m_UptCount=m_InsCount=m_DiscardCount=0;

  // 清空字段信息容器
  m_vTABCOLUMNS.clear();

  CTABFIELD TABFIELD;

  // 获取该表全部的字段
  if (TABFIELD.GetALLField(m_conn,m_tname) == FALSE) 
  {
    m_logfile->Write("GetALLField() failed.\n"); return -1;
  }

  for (UINT ii=0;ii<TABFIELD.m_fieldcount;ii++)
  {
    memset(&m_stTABCOLUMNS,0,sizeof(m_stTABCOLUMNS));

    strcpy(m_stTABCOLUMNS.COLUMN_NAME,TABFIELD.m_vALLFIELD[ii].fieldname);
    strcpy(m_stTABCOLUMNS.DATA_TYPE,TABFIELD.m_vALLFIELD[ii].datatype);
    m_stTABCOLUMNS.DATA_LENGTH=TABFIELD.m_vALLFIELD[ii].datalen;

    // 判断字段名是否是keyid
    if (strcmp(m_stTABCOLUMNS.COLUMN_NAME,"keyid")==0) m_bnokeyidfield=FALSE;

    if ( (strcmp(m_stTABCOLUMNS.DATA_TYPE,"char")      != 0) && 
         (strcmp(m_stTABCOLUMNS.DATA_TYPE,"date")      != 0) && 
         (strcmp(m_stTABCOLUMNS.DATA_TYPE,"number")    != 0) &&
         (strcmp(m_stTABCOLUMNS.DATA_TYPE,"varchar")   != 0) && 
         (strcmp(m_stTABCOLUMNS.DATA_TYPE,"varchar2")  != 0) &&
         (strcmp(m_stTABCOLUMNS.DATA_TYPE,"nvarchar2") != 0) )
    {
      // 如果字段的数据类型不是以上种类，不处理
      continue;
    }

    // 判断是否存在质控字段
    /*
    if (strcmp(m_stTABCOLUMNS.COLUMN_NAME,"checksts")==0) 
    {
      m_bexistcheckfield=TRUE;
    }
    */

    // 判断是否存在入库时间
    if (strcmp(m_stTABCOLUMNS.COLUMN_NAME,"crttime")==0) 
    {
      m_bexistcrttimefield=TRUE;
    }

    if ( (strcmp(m_stTABCOLUMNS.COLUMN_NAME, "crttime")==0) || 
         (strcmp(m_stTABCOLUMNS.COLUMN_NAME,"sync_rowid")==0) || 
         (strcmp(m_stTABCOLUMNS.COLUMN_NAME,   "keyid")==0) )
    {
      // 以上字段是入库时间和数据同步字段，入库不处理
      continue;
    }

    m_vTABCOLUMNS.push_back(m_stTABCOLUMNS);
  }

  // 获取该表的主键字段，以字符串的形式存放，中间用逗号分隔
  if (TABFIELD.GetPKField(m_conn,m_tname) == FALSE) 
  {
    m_logfile->Write("GetPKField() failed.\n"); return -1;
  }

  strcpy(m_KEYFIELDSTR,TABFIELD.m_pkfieldstr);

  return 0;
}

// 生成查询、更新和插入表的SQL语句
void CQXDATA::CrtSQL()
{
  UINT ii,jj;
  char strTemp[101],strPart1[10240],strPart2[10240];

  // 生成select语句
  memset(m_SelectSQL,0,sizeof(m_SelectSQL));

  if (strcmp(m_conn->m_dbtype,"mysql")==0)
    snprintf(m_SelectSQL,1000,"select keyid from %s where 1=1",m_tname);
  else
    snprintf(m_SelectSQL,1000,"select rowid from %s where 1=1",m_tname);
  
  m_CmdKeyFields.SplitToCmd(m_KEYFIELDSTR,",");

  for (ii=0;ii<m_CmdKeyFields.m_vCmdStr.size();ii++)
  {
    for (jj=0;jj<m_vTABCOLUMNS.size();jj++)
    {
      if (m_CmdKeyFields.m_vCmdStr[ii] == m_vTABCOLUMNS[jj].COLUMN_NAME)
      {
        memset(strTemp,0,sizeof(strTemp));
        if (strcmp(m_vTABCOLUMNS[jj].DATA_TYPE,"date") == 0)
        {
          if (strcmp(m_conn->m_dbtype,"mysql")==0)
            snprintf(strTemp,100," and %s=str_to_date(:%ld,'%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s')",m_CmdKeyFields.m_vCmdStr[ii].c_str(),ii+1);
          else
            snprintf(strTemp,100," and %s=to_date(:%ld,'yyyymmddhh24miss')",m_CmdKeyFields.m_vCmdStr[ii].c_str(),ii+1);
        }
        else if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"number") == 0)
        {
          snprintf(strTemp,100," and %s=to_null(:%ld)",m_CmdKeyFields.m_vCmdStr[ii].c_str(),ii+1);
        }
        else
        {
          snprintf(strTemp,100," and %s=:%ld",m_CmdKeyFields.m_vCmdStr[ii].c_str(),ii+1);
        }
        strcat(m_SelectSQL,strTemp);
      }
    }
  }

  // 生成update语句
  memset(m_UpdateSQL,0,sizeof(m_UpdateSQL));

  if (m_upttype == 1)
  {
    snprintf(m_UpdateSQL,10000,"update %s set ",m_tname);
    for (ii=0;ii<m_vTABCOLUMNS.size();ii++)
    {
      memset(strTemp,0,sizeof(strTemp));
      if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"date") == 0)
      {
          if (strcmp(m_conn->m_dbtype,"mysql")==0)
            snprintf(strTemp,100,"%s=str_to_date(:%ld,'%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s'),",\
                 m_vTABCOLUMNS[ii].COLUMN_NAME,ii+1);
          else
            snprintf(strTemp,100,"%s=to_date(:%ld,'yyyymmddhh24miss'),",\
                 m_vTABCOLUMNS[ii].COLUMN_NAME,ii+1);
      }
      else if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"number") == 0)
      {
        snprintf(strTemp,100,"%s=to_null(:%ld),",m_vTABCOLUMNS[ii].COLUMN_NAME,ii+1);
      }
      else
      {
        snprintf(strTemp,100,"%s=:%ld,",m_vTABCOLUMNS[ii].COLUMN_NAME,ii+1);
      }
      strcat(m_UpdateSQL,strTemp);
    }

    // 处理入库时间字段
    if (m_bexistcrttimefield == TRUE) 
    {
      if (strcmp(m_conn->m_dbtype,"mysql")==0)
        strcat(m_UpdateSQL,"crttime=sysdate())");
      else
        strcat(m_UpdateSQL,"crttime=sysdate,");
    }

    // 删除update字段串的最后一个逗号
    m_UpdateSQL[strlen(m_UpdateSQL)-1]=0;

    memset(strTemp,0,sizeof(strTemp));
  
    // 判断更新时限
    if (m_upttlimit >0)
    {
      if (strcmp(m_conn->m_dbtype,"mysql")==0)
        snprintf(strTemp,100," where keyid=:%d and ddatetime>date_sub(now(),interval %d HOUR)",(int)m_vTABCOLUMNS.size()+1,m_upttlimit);
      else
        snprintf(strTemp,100," where rowid=:%d and ddatetime>sysdate-%d/24",(int)m_vTABCOLUMNS.size()+1,m_upttlimit);
    }
    else
    {
      if (strcmp(m_conn->m_dbtype,"mysql")==0)
        snprintf(strTemp,100," where keyid=:%d",(int)m_vTABCOLUMNS.size()+1);
      else
        snprintf(strTemp,100," where rowid=:%d",(int)m_vTABCOLUMNS.size()+1);
    }

    strcat(m_UpdateSQL,strTemp);
  }

  // 生成插入的SQL语句
  memset(strPart1,0,sizeof(strPart1));
  memset(strPart2,0,sizeof(strPart2));
  memset(m_InsertSQL,0,sizeof(m_InsertSQL));

  if (m_bnokeyidfield == TRUE)
  {
    snprintf(strPart1,10000,"insert into %s(",m_tname);
    snprintf(strPart2,10000,"values(");
  }
  else
  {
    snprintf(strPart1,10000,"insert into %s(keyid,",m_tname);

    if (strcmp(m_conn->m_dbtype,"mysql")==0)
      snprintf(strPart2,10000,"values(SEQ_%s.nextval,",m_tname+2);
    else
      snprintf(strPart2,10000,"values(SEQ_%s.nextval,",m_tname+2);
  }

  for (ii=0;ii<m_vTABCOLUMNS.size();ii++)
  {
    memset(strTemp,0,sizeof(strTemp));
    snprintf(strTemp,100,"%s,",m_vTABCOLUMNS[ii].COLUMN_NAME);
    strcat(strPart1,strTemp);

    memset(strTemp,0,sizeof(strTemp));
    if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"date") == 0)
    {
      if (strcmp(m_conn->m_dbtype,"mysql")==0)
        snprintf(strTemp,100,"str_to_date(:%ld,'%%%%Y%%%%m%%%%d%%%%H%%%%i%%%%s'),",ii+1);
      else
        snprintf(strTemp,100,"to_date(:%ld,'yyyymmddhh24miss'),",ii+1);
    }
    else if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"number") == 0)
    {
      snprintf(strTemp,100,"to_null(:%ld),",ii+1);
    }
    else
    {
      snprintf(strTemp,100,":%ld,",ii+1);
    }

    strcat(strPart2,strTemp);
  }

  // 处理质控字段
  /*
  if (m_bexistcheckfield == TRUE)
  {
    strcat(strPart1,"checksts,");

    memset(strTemp,0,sizeof(strTemp));
    snprintf(strTemp,10,"%lu,",m_checksts);
    strcat(strPart2,strTemp);
  }
  */

  // 处理入库时间字段
  if (m_bexistcrttimefield == TRUE) 
  {
    strcat(strPart1,"crttime,"); 

    if (strcmp(m_conn->m_dbtype,"mysql")==0)
      strcat(strPart2,"sysdate(),"); 
    else
      strcat(strPart2,"sysdate,"); 
  }

  // 把insert语句最后一个","号替换成")"号
  strPart1[strlen(strPart1)-1] = ')';
  strPart2[strlen(strPart2)-1] = ')';
  
  snprintf(m_InsertSQL,10000,"%s %s",strPart1,strPart2);

  //m_logfile->Write("m_SelectSQL=%s\nm_InsertSQL=%s\nm_UpdateSQL=%s\n",m_SelectSQL,m_InsertSQL,m_UpdateSQL);
}

// 把每行的内容解包到字段中
void CQXDATA::UNPackBuffer(char *strBuffer,int iFileVer)
{
  m_ddtfieldpos=-1;

  for (UINT ii=0;ii<m_vTABCOLUMNS.size();ii++)
  {
    memset(m_vTABCOLUMNS[ii].COLUMN_VALUE,0,sizeof(m_vTABCOLUMNS[ii].COLUMN_VALUE));
    if ( (iFileVer==1) || (iFileVer==2) )
    {
      GetXMLBuffer(strBuffer,m_vTABCOLUMNS[ii].COLUMN_NAME,m_vTABCOLUMNS[ii].COLUMN_VALUE,m_vTABCOLUMNS[ii].DATA_LENGTH);
    }
    if (iFileVer==3)
    {
      char strcolumnname[51];
      memset(strcolumnname,0,sizeof(strcolumnname));
      strcpy(strcolumnname,m_vTABCOLUMNS[ii].COLUMN_NAME);
      ToUpper(strcolumnname);
      GetXMLBuffer(strBuffer,strcolumnname,m_vTABCOLUMNS[ii].COLUMN_VALUE,m_vTABCOLUMNS[ii].DATA_LENGTH);
    }

    if (strcmp(m_vTABCOLUMNS[ii].COLUMN_NAME,"checksts") == 0)
    {
      strcpy(m_vTABCOLUMNS[ii].COLUMN_VALUE,"9");
    }

    // 截去值中的空格
    Trim(m_vTABCOLUMNS[ii].COLUMN_VALUE);

    // 如果是日期型字段，就只获取其中的数字，相当于把格式转换成yyyymmddhh24miss
    if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"date") == 0) 
    {
      PickNumber(m_vTABCOLUMNS[ii].COLUMN_VALUE,m_vTABCOLUMNS[ii].COLUMN_VALUE,FALSE,FALSE); 
      m_vTABCOLUMNS[ii].COLUMN_VALUE[14]=0;
    }

    // 如果是数字型字段，就只获取其中的数字、负号和圆点
    if (strcmp(m_vTABCOLUMNS[ii].DATA_TYPE,"number") == 0) 
    {
      PickNumber(m_vTABCOLUMNS[ii].COLUMN_VALUE,m_vTABCOLUMNS[ii].COLUMN_VALUE,TRUE,TRUE); 

      if(strlen(m_vTABCOLUMNS[ii].COLUMN_NAME) == 0) 
      {
        memset(m_vTABCOLUMNS[ii].COLUMN_NAME,0,sizeof(m_vTABCOLUMNS[ii].COLUMN_NAME));
        strcpy(m_vTABCOLUMNS[ii].COLUMN_NAME," ");
      }
    }

    // 获取数据时间字段的位置
    if (strcmp(m_vTABCOLUMNS[ii].COLUMN_NAME,"ddatetime") == 0) m_ddtfieldpos=ii;

    //m_logfile->Write("COLUMN_NAME=%s,COLUMN_VALUE=%s\n",m_vTABCOLUMNS[ii].COLUMN_NAME,m_vTABCOLUMNS[ii].COLUMN_VALUE);
  }

  // 总记录数加一
  m_TotalCount++;
}

// 准备插入表的SQL语句，把数据插入表中
long CQXDATA::PreSQL()
{
  // 预分析查询表的SQL
  stmtselecttable.connect(m_conn);
  if (stmtselecttable.prepare(m_SelectSQL) != 0)
  {
    m_logfile->Write("stmtselecttable.prepare failed.\n%s\n",stmtselecttable.cda.message);
    return stmtselecttable.cda.rc;
  }

  // 预分析更新表的SQL
  if (m_upttype == 1)
  {
    stmtupdatetable.connect(m_conn);
    if (stmtupdatetable.prepare(m_UpdateSQL) != 0) 
    {
      m_logfile->Write("stmtupdatetable.prepare failed.\n%s\n",stmtupdatetable.cda.message);
      return stmtupdatetable.cda.rc;
    }
  }

  // 预分析插入表的SQL
  stmtinserttable.connect(m_conn);
  if (stmtinserttable.prepare(m_InsertSQL) != 0) 
  {
    m_logfile->Write("stmtinserttable.prepare failed.\n%s\n",stmtinserttable.cda.message);
    m_logfile->Write("m_sql=%s\n",stmtinserttable.m_sql);
    return stmtinserttable.cda.rc;
  }

  return 0;
}

long CQXDATA::BindParams()
{
  UINT ii,jj;

  // 预分析查询表的SQL
  stmtselecttable.bindout(1,m_rowid,30);

  m_CmdKeyFields.SplitToCmd(m_KEYFIELDSTR,",");

  for (ii=0;ii<m_CmdKeyFields.m_vCmdStr.size();ii++)
  {
    for (jj=0;jj<m_vTABCOLUMNS.size();jj++)
    {
      if (m_CmdKeyFields.m_vCmdStr[ii] == m_vTABCOLUMNS[jj].COLUMN_NAME)
      {
        stmtselecttable.bindin(ii+1,m_vTABCOLUMNS[jj].COLUMN_VALUE,m_vTABCOLUMNS[jj].DATA_LENGTH);
      }
    }
  }

  // 预分析更新表的SQL
  if (m_upttype == 1)
  {
    for (ii=0;ii<m_vTABCOLUMNS.size();ii++)
    {
      stmtupdatetable.bindin(ii+1,m_vTABCOLUMNS[ii].COLUMN_VALUE,m_vTABCOLUMNS[ii].DATA_LENGTH);
    }
    stmtupdatetable.bindin(ii+1,m_rowid,30);
  }

  // 预分析插入表的SQL
  for (ii=0;ii<m_vTABCOLUMNS.size();ii++)
  {
    stmtinserttable.bindin(ii+1,m_vTABCOLUMNS[ii].COLUMN_VALUE,m_vTABCOLUMNS[ii].DATA_LENGTH);
  }

  return 0;
}

long CQXDATA::InsertTable(char *strBuffer,char *striflog)
{
  // 如果该表的数据时间这个字段，就判断数据的时间是否合法
  if (m_ddtfieldpos>=0)
  {
    // 判断数据时间是否合法，即是否在dmintime日到当前时间之后的10天
    if (CheckDDateTime(m_vTABCOLUMNS[m_ddtfieldpos].COLUMN_VALUE,m_dmintime) == FALSE) 
    {
      // if (strcmp(striflog,"true")==0) 
      // {
      //   m_logfile->Write("CheckDDateTime(%s) FAILED.\n%s\n",m_tname,strBuffer); 
      // }

      return 0;
    }
  }

  // 查询表记录
  if (stmtselecttable.execute() != 0)
  {
    m_logfile->Write("select %s FAILED.\n%s\n%s\n%s\n",\
                      m_tname,stmtselecttable.cda.message,strBuffer,stmtselecttable.m_sql); 

    // 一般来说，这里的select是不会出错的，如果出错，一定是数据库出了大问题，所以有必要返回失败的代码
    return stmtselecttable.cda.rc;
  }

  memset(m_rowid,0,sizeof(m_rowid));

  // 如果有旧记录，判断是否需要更新再操作
  if (stmtselecttable.next() == 0) 
  {
    // 必须把select出来的结果清空
    while(TRUE)
    {
      if (stmtselecttable.next() != 0) break;
    }

    // 如果不需要更新，就直接返回
    if (m_upttype == 2) return 0;

    // 更新数据
    if (stmtupdatetable.execute() != 0)
    {
      // 1-主键冲突
      if (stmtupdatetable.cda.rc == 1) return 0;

      if (strcmp(striflog,"true")==0) 
      {
        m_logfile->Write("update %s FAILED.\n%s\n%s\n%s\n",\
                          m_tname,stmtupdatetable.cda.message,strBuffer,stmtupdatetable.m_sql); 
      }
      return 0;
    }

    // 获取更新记录的行数
    m_UptCount=m_UptCount+stmtupdatetable.cda.rpc;
  }
  else
  {
    // 插入数据
    CTimer Timer;

    Timer.Beginning();;

    // 没有旧记录，插入新记录
    while (TRUE)
    {
      if (stmtinserttable.execute() == 54)
      {
        if (Timer.Elapsed() <= 10) { usleep(500000); continue; }

        if (Timer.Elapsed() >= 10) break;
      }

      if (stmtinserttable.cda.rc == 0) 
      { 
        m_InsCount=m_InsCount+stmtinserttable.cda.rpc; return 0; 
      }

      // 1-主键冲突
      if (stmtinserttable.cda.rc == 1) return 0;

      if (strcmp(striflog,"true")==0) 
      {
        m_logfile->Write("insert %s FAILED.\n%s\n%s\n%s\n",\
                          m_tname,stmtinserttable.cda.message,strBuffer,stmtinserttable.m_sql); 
      }

      return 0;
    }
  }

  return 0;
}


CALLTABLE::CALLTABLE()
{
  m_connidc=0;
  m_conndst=0;
  m_logfile=0;
  memset(m_COLUMNSSTR,0,sizeof(m_COLUMNSSTR));
}

CALLTABLE::~CALLTABLE()
{
  m_vALLTABLE.clear();
  m_vROWIDYEAR.clear();
}

void CALLTABLE::BindConnLog(connection *in_connidc,connection *in_conndst,CLogFile *in_logfile,CProgramActive *in_ProgramActive)
{
  m_connidc=in_connidc;

  m_conndst=in_conndst;

  m_logfile=in_logfile;

  m_ProgramActive=in_ProgramActive;
}



// 把TAB数据字典中存在，  但T_ALLTABLE表中不存在的记录插入T_ALLTABLE中
// 把TAB数据字典中不存在，但T_ALLTABLE表中存在的记录删除掉
// 把USER_SEQUENCES数据字典中存在，  但T_SEQANDTABLE表中不存在的记录插入T_SEQANDTABLE
// 把USER_SEQUENCES数据字典中不存在，但T_SEQANDTABLE表中存在的记录删除掉
long CALLTABLE::UpdateALLTABLEAndSEQ()
{
  sqlstatement stmt;
  stmt.connect(m_connidc);

  // 把USER_TABLES表中存在，但T_ALLTABLE表中不存在的记录插入T_ALLTABLE中
  stmt.prepare("\
    insert into T_ALLTABLE(tname,tcname,keyid)\
    select upper(table_name),upper(table_name),SEQ_ALLTABLE.nextval from USER_TABLES\
     where substr(table_name,1,2)='T_' and table_name not in (select upper(tname) from T_ALLTABLE)");
  if (stmt.execute() != 0)
  {
    // 当处理不是数据中心的数据库，而是其它数据库时，序列SEQ_ALLTABLE不一定存在
    // 所以这里要忽略这个错误
    // 02289, 00000, "sequence does not exist"
    if ( (stmt.cda.rc != 1) && (stmt.cda.rc != 2289) )
    {
      m_logfile->Write("UpdateALLTABLEAndSEQ insert T_ALLTABLE failed.\n%s\n",stmt.cda.message);
      return stmt.cda.rc;
    }
  }


  // 把USER_TABLES表中不存在，但T_APPTABLE和T_ALLTABLE表中存在的记录删除掉
  stmt.prepare("\
    BEGIN\
      delete from T_APPTABLE where tname not in (select upper(table_name) from USER_TABLES where substr(table_name,1,2)='T_');\
      delete from T_ALLTABLE where tname not in (select upper(table_name) from USER_TABLES where substr(table_name,1,2)='T_');\
    END;");
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UpdateALLTABLEAndSEQ update T_ALLTABLE failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  // 把USER_SEQUENCES表中存在，但T_SEQANDTABLE表中不存在的记录插入T_SEQANDTABLE
  stmt.prepare("\
    insert into T_SEQANDTABLE(seqname,keyid)\
    select sequence_name,SEQ_SEQANDTABLE.nextval from USER_SEQUENCES\
     where sequence_name not in (select seqname from T_SEQANDTABLE)");
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UpdateALLTABLEAndSEQ insert T_SEQANDTABLE failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  // 把USER_SEQUENCES表中不存在，但T_SEQANDTABLE表中存在的记录删除掉
  stmt.prepare("\
    delete from T_SEQANDTABLE where seqname not in (select sequence_name from USER_SEQUENCES)");
  if (stmt.execute() != 0)
  {
    m_logfile->Write("UpdateALLTABLEAndSEQ update T_SEQANDTABLE failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  m_connidc->commitwork();

  return 0;
}

// 从T_APPTABLE表中载入某集群服务器的全部记录，用于统计记录总数
long CALLTABLE::LoadForCount()
{
  if (loadstmt.state == loadstmt.not_opened)
  {
    loadstmt.connect(m_connidc);
    loadstmt.prepare("\
      select upper(tname),(select tabletype from T_ALLTABLE where tname=T_APPTABLE.tname) from T_APPTABLE where appid=:1 order by totalcount");
    loadstmt.bindin(1,&m_appid);
    loadstmt.bindout(1, m_stALLTABLE.tname,30);
    loadstmt.bindout(2,&m_stALLTABLE.tabletype);
  }

  if (loadstmt.execute() != 0)
  {
    m_logfile->Write("LoadForCount select T_APPTABLE failed.\n%s\n",loadstmt.cda.message);
  }

  return loadstmt.cda.rc;
}

/*
// 从T_APPTABLE表中载入需要核对数据的参数。
long CALLTABLE::LoadForCheck(int in_appid)
{
  memset(m_dblinkname,0,sizeof(m_dblinkname));

  if (loadstmt.state == loadstmt.not_opened)
  {
    loadstmt.connect(m_connidc);
    loadstmt.prepare("\
      select tname,(select dblinkname from T_DAPPSERVER where appid=:1) from T_APPTABLE\
       where appid=:2 and ifcheck=1\
       order by tname,appid");
    loadstmt.bindin(1,&in_appid);
    loadstmt.bindin(2,&in_appid);
    loadstmt.bindout(1,m_stALLTABLE.tname,30);
    loadstmt.bindout(2,m_dblinkname,50);

  }

  if (loadstmt.execute() != 0)
  {
    m_logfile->Write("LoadForCheck select T_APPTABLE failed.\n%s\n",loadstmt.cda.message);
  }

  return loadstmt.cda.rc;
}
*/

// 从T_APPTABLE表中载入某集群服务器全部的当前表，用于创建视图
long CALLTABLE::LoadForCrtView()
{
  if (loadstmt.state == loadstmt.not_opened)
  {
    loadstmt.connect(m_connidc);
    loadstmt.prepare(\
      "select tname from t_apptable where appid=:1 and tname not like '%%HIS' and tname not like '%%ARH%%'");
    loadstmt.bindin(1,&m_appid);
    loadstmt.bindout(1, m_stALLTABLE.tname,30);
  }

  if (loadstmt.execute() != 0)
  {
    m_logfile->Write("LoadForCrtView select T_APPTABLE failed.\n%s\n",loadstmt.cda.message);
  }

  return loadstmt.cda.rc;
}

// 从T_APPTABLE表中载入某集群服务器需要管理的表的记录，用于数据管理
long CALLTABLE::LoadForManager()
{
  if (loadstmt.state == loadstmt.not_opened)
  {
    loadstmt.connect(m_connidc);
    loadstmt.prepare("\
      select upper(tname),hdataptype,hdatapcfg,addterm from T_APPTABLE\
       where appid=:1 and tname in (select tname from T_ALLTABLE)\
         and hdataptype in (0,1,2)\
         and (aleasttime is null or aleasttime>sysdate-0.2) order by tname desc");
    loadstmt.bindin(1,&m_appid);
    loadstmt.bindout(1, m_stALLTABLE.tname,30);
    loadstmt.bindout(2,&m_stALLTABLE.hdataptype);
    loadstmt.bindout(3,&m_stALLTABLE.hdatapcfg);
    loadstmt.bindout(4, m_stALLTABLE.addterm,300);
  }

  if (loadstmt.execute() != 0)
  {
    m_logfile->Write("LoadForManager select T_APPTABLE failed.\n%s\n",loadstmt.cda.message);
  }

  return loadstmt.cda.rc;
}


// 获取数据表符合条件的记录的rowid，放入容器中
long CALLTABLE::LoadALLROWID()
{
  // 把容器中的记录清空
  m_vROWIDYEAR.clear();

  // 每次最多只处理50000条记录。
  sqlstatement stmt;
  stmt.connect(m_conndst);
  stmt.prepare("\
    select rowid,to_char(ddatetime,'yyyy') from %s\
     where ddatetime<=to_date(to_char(sysdate-:1,'yyyymmdd')||'000000','yyyymmddhh24miss') %s and rownum<=50000",m_stALLTABLE.tname,m_stALLTABLE.addterm);
  stmt.bindin(1,&m_stALLTABLE.hdatapcfg);
  stmt.bindout(1,m_stROWIDYEAR.rowid,30);
  stmt.bindout(2,m_stROWIDYEAR.yearstr,4);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("LoadALLROWID select %s failed.\n%s\n",m_stALLTABLE.tname,stmt.cda.message);
  }

  int iCount=0;

  while (TRUE)
  {
    memset(&m_stROWIDYEAR,0,sizeof(m_stROWIDYEAR));

    if (stmt.next() != 0) break;

    if (iCount++>5000) { iCount=0; m_ProgramActive->WriteToFile(); }

    m_vROWIDYEAR.push_back(m_stROWIDYEAR);
  }

  return 0;
}

// 备份到归档表
long CALLTABLE::BackupToATable()
{
  if (m_vROWIDYEAR.size() == 0)
  {
    m_logfile->Write("0 rows archive\n"); return 0;
  }

  memset(&m_stROWIDYEAR,0,sizeof(m_stROWIDYEAR));

  char strArhTName[31];
  sqlstatement stmtins,stmtdel;
  stmtins.connect(m_conndst);
  stmtdel.connect(m_conndst);

  UINT ivSize=m_vROWIDYEAR.size();

  int iCount=0;

  for (UINT ii=0;ii<ivSize;ii++)
  {
    // 判断是否需要重新准备SQL语句
    if (strcmp(m_vROWIDYEAR[ii].yearstr,m_stROWIDYEAR.yearstr) != 0)
    {
      // 由当前表名或历史表名组成归档表名
      memset(strArhTName,0,sizeof(strArhTName));
      if (strncmp(m_stALLTABLE.tname+strlen(m_stALLTABLE.tname)-3,"HIS",3) != 0)
        strcpy(strArhTName,m_stALLTABLE.tname); // 当前表
      else
        strncpy(strArhTName,m_stALLTABLE.tname,strlen(m_stALLTABLE.tname)-4); // 历史表
      strcat(strArhTName,"_ARH_"); strcat(strArhTName,m_vROWIDYEAR[ii].yearstr);
 

      // 判断归档表是否存在，如果不存在，就生成归档表
      if (CrtByTable(m_conndst,m_stALLTABLE.tname,strArhTName) != 0)
      {
        m_logfile->Write("CrtByTable(%s,%s) failed.\n",m_stALLTABLE.tname,strArhTName); return FALSE;
      }

      stmtins.prepare("\
        insert into %s(%s) select %s from %s where rowid=:1",\
               strArhTName,m_COLUMNSSTR,m_COLUMNSSTR,m_stALLTABLE.tname);
      stmtins.bindin(1,m_stROWIDYEAR.rowid,30);

      stmtdel.prepare("delete from %s where rowid=:1",m_stALLTABLE.tname);
      stmtdel.bindin(1,m_stROWIDYEAR.rowid,30);
    }

    memcpy(&m_stROWIDYEAR,&m_vROWIDYEAR[ii],sizeof(m_stROWIDYEAR));

    if (stmtins.execute() != 0)
    {
      if (stmtins.cda.rc != 1)
      {
        m_logfile->Write("BackupToATable insert %s failed.\n%s\n",strArhTName,stmtins.cda.message);
        return stmtins.cda.rc;
      }
    }

    if (stmtdel.execute() != 0)
    {
      m_logfile->Write("BackupToATable delete %s failed.\n%s\n",m_stALLTABLE.tname,stmtdel.cda.message);
      return stmtdel.cda.rc;
    }

    if (iCount++>5000) { iCount=0; m_ProgramActive->WriteToFile(); }
  }

  m_logfile->Write("%ld rows archive to %s\n",ivSize,strArhTName);

  m_stALLTABLE.totalcount=ivSize;

  return 0;
}

long CALLTABLE::LoadAllRecordNext()
{
  memset(&m_stALLTABLE,0,sizeof(m_stALLTABLE));

  return loadstmt.next();
}

// 获取全部需要备份的记录，并放入m_vALLTABLE中
long CALLTABLE::LoadEXPRecord()
{
  sqlstatement stmt;
  stmt.connect(m_connidc);
  stmt.prepare("\
    select upper(tname) from T_ALLTABLE\
     where ifbackup=1 and (sysdate-backuptime>backuptvl or backuptime is null)\
     order by tname");
  stmt.bindout(1, m_stALLTABLE.tname,30);

  m_vALLTABLE.clear();

  if (stmt.execute() != 0)
  {
    m_logfile->Write("LoadAllRecord select T_ALLTABLE failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  while (TRUE)
  {
    memset(&m_stALLTABLE,0,sizeof(m_stALLTABLE));

    if (stmt.next() != 0) break;

    m_vALLTABLE.push_back(m_stALLTABLE);
  }

  return 0;
}

// 统计表的记录数
long CALLTABLE::CountTable(char *in_DBLinkName)
{
  // analyze table t_bakfilelist compute statistics;

  sqlstatement stmt;
  stmt.connect(m_connidc);

  // 判断该表是否有ddatetime字段
  int iddatetimeexist=0;
  stmt.prepare(\
   "select count(*) from USER_TAB_COLUMNS where table_name=upper('%s')\
       and column_name=upper('DDATETIME') and data_type='DATE'",m_stALLTABLE.tname);
  stmt.bindout(1,&iddatetimeexist);
  stmt.execute();
  stmt.next();

  /*
  // 2019-09-10，吴从周修改
  // 现在，数据中心的数据量越来越大，如果按时间统计，效率很低，所以改为全表统计
  if ( (iddatetimeexist == 0) ||
       (MatchFileName(m_stALLTABLE.tname,"*_HIS")==TRUE) ||
       (MatchFileName(m_stALLTABLE.tname,"*_ARH_*")==TRUE) )
  {
    stmt.prepare("select count(rowid) from %s@%s",m_stALLTABLE.tname,in_DBLinkName);
  }
  else
  {
    char strendtime[20];
    memset(strendtime,0,sizeof(strendtime));
    LocalTime(strendtime,"yyyymmdd",0-1*24*60*60);
    strcat(strendtime,"000000");

    stmt.prepare("select count(rowid) from %s@%s where ddatetime<=to_date('%s','yyyymmddhh24miss')",m_stALLTABLE.tname,in_DBLinkName,strendtime);
  }
  */


  // 全表统计
  stmt.prepare("select count(rowid) from %s@%s",m_stALLTABLE.tname,in_DBLinkName);
  stmt.bindout(1,&m_stALLTABLE.totalcount);


  if (stmt.execute() != 0)
  {
    m_logfile->Write("CountTable select %s@%s FAILED.\n%s\n",m_stALLTABLE.tname,in_DBLinkName,stmt.cda.message);
  }

  stmt.next();

  //m_logfile->WriteEx("%s,%d\n",stmt.m_sql,m_stALLTABLE.totalcount);

  return stmt.cda.rc;
}

// 卸出表
BOOL CALLTABLE::ExpTable(char *strTmpPath,char *strStdPath,char *strConnStr,char *strTName)
{
  // 生成备份日期，格式是yyyymmdd000000
  char strDDateTime[20];
  memset(strDDateTime,0,sizeof(strDDateTime));
  LocalTime(strDDateTime,"yyyymmdd");
  strncpy(strDDateTime+8,"000000",6);

  char strTmpDFileName[301];     // 临时数据文件名
  char strTmpLFileName[301];     // 临时日志文件名
  memset(strTmpDFileName,0,sizeof(strTmpDFileName));
  memset(strTmpLFileName,0,sizeof(strTmpLFileName));
  snprintf(strTmpDFileName,200,"%s/DMPDAT_%s_%s.dmp",strTmpPath,strDDateTime,strTName+2);
  snprintf(strTmpLFileName,200,"%s/DMPLOG_%s_%s.log",strTmpPath,strDDateTime,strTName+2);

  char strGZTmpDFileName[301];   // 压缩后的临时数据文件名
  char strGZTmpLFileName[301];   // 压缩后的临时日志文件名
  memset(strGZTmpDFileName,0,sizeof(strGZTmpDFileName));
  memset(strGZTmpLFileName,0,sizeof(strGZTmpLFileName));
  snprintf(strGZTmpDFileName,200,"%s/DMPDAT_%s_%s.dmp.gz",strTmpPath,strDDateTime,strTName+2);
  snprintf(strGZTmpLFileName,200,"%s/DMPLOG_%s_%s.log.gz",strTmpPath,strDDateTime,strTName+2);

  char strStdDFileName[301];     // 标准数据文件名
  char strStdLFileName[301];     // 标准日志文件名
  memset(strStdDFileName,0,sizeof(strStdDFileName));
  memset(strStdLFileName,0,sizeof(strStdLFileName));
  snprintf(strStdDFileName,200,"%s/DMPDAT_%s_%s.dmp.gz",strStdPath,strDDateTime,strTName+2);
  snprintf(strStdLFileName,200,"%s/DMPLOG_%s_%s.log.gz",strStdPath,strDDateTime,strTName+2);

  char strCMD[2048];

  // 备份表到临时目录
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,2000,"exp %s file=%s log=%s tables=\\(%s\\) 1>/dev/null 2>/dev/null",strConnStr,strTmpDFileName,strTmpLFileName,strTName);
  system(strCMD);

  // 检查备份是否成功
  if(CheckFileSTS(strTmpLFileName,"没有出现警告。") == FALSE)
  {
    if(CheckFileSTS(strTmpLFileName,"Export terminated successfully without warnings.") == FALSE) 
    {
      m_logfile->Write("%s\n",strCMD); return FALSE;
    }
  }

  // 压缩备份文件和日志文件
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,2000,"gzip -f %s 1>/dev/null 2>/dev/null",strTmpDFileName);
  system(strCMD);

  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,2000,"gzip -f %s 1>/dev/null 2>/dev/null",strTmpLFileName);
  system(strCMD);

  // 把备份文件和日志文件转移到STD目录
  if (RENAME(strGZTmpDFileName,strStdDFileName) == FALSE) return FALSE;
  if (RENAME(strGZTmpLFileName,strStdLFileName) == FALSE) return FALSE;

  return TRUE;
}

// 更新T_ALLTABLE表的记录总数、统计时间字段、表的大小、索引的大小
long CALLTABLE::UptCountTime(char *in_DBLinkName)
{
  sqlstatement stmt;
  stmt.connect(m_connidc);

  stmt.prepare("\
    update T_APPTABLE set totalcount=:1,counttime=sysdate,\
      indexbytes=(select :2*sum(column_length+10) from USER_IND_COLUMNS where table_name=T_APPTABLE.tname),\
      tablebytes=(select :3*avg_row_len from USER_TABLES where table_name=T_APPTABLE.tname)\
     where appid=:4 and tname=upper(:5)");
  stmt.bindin(1,&m_stALLTABLE.totalcount);
  stmt.bindin(2,&m_stALLTABLE.totalcount);
  stmt.bindin(3,&m_stALLTABLE.totalcount);
  stmt.bindin(4,&m_appid);
  stmt.bindin(5, m_stALLTABLE.tname,30);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptCountTime update T_APPTABLE failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  // 文档文件数据的表数据大小要采用文件大小合计的方式
  if (m_stALLTABLE.tabletype == 2)
  {
    stmt.prepare("update T_APPTABLE set tablebytes=(select sum(150+filesize) from %s@%s) where appid=:1 and tname=upper(:2)",m_stALLTABLE.tname,in_DBLinkName);
    stmt.bindin(1,&m_appid);
    stmt.bindin(2, m_stALLTABLE.tname,30);

    if (stmt.execute() != 0)
    {
      m_logfile->Write("UptCountTime update T_APPTABLE failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
    }
  }

  return stmt.cda.rc;
}

/*
// 核对表的记录数
long CALLTABLE::CheckTable()
{
  m_stALLTABLE.checkok=2;

  // 判断该表是否有keyid字段
  int ikeyidexist=0;
  sqlstatement stmt;
  stmt.connect(m_connidc);
  stmt.prepare(\
   "select count(*) from USER_TAB_COLUMNS where table_name=upper('%s')\
       and column_name=upper('keyid')",m_stALLTABLE.tname);
  stmt.bindout(1,&ikeyidexist);
  stmt.execute();
  stmt.next();

  // 没有keyid字段，历史表或归档表
  if ( (ikeyidexist == 0) ||
       (MatchFileName(m_stALLTABLE.tname,"*_HIS")  ==TRUE) ||
       (MatchFileName(m_stALLTABLE.tname,"*_ARH_*")==TRUE) )
  {
    sprintf(m_stALLTABLE.srcchecksql,"select count(*) from %s",m_stALLTABLE.tname);
    sprintf(m_stALLTABLE.dstchecksql,"select count(*) from %s@%s",m_stALLTABLE.tname,m_dblinkname);
  }
  else
  {
    UINT umaxkeyid=0;
    stmt.prepare("select max(keyid) from %s@%s",m_stALLTABLE.tname,m_dblinkname);
    stmt.bindout(1,&umaxkeyid);
    stmt.execute();
    stmt.next();
    if (umaxkeyid > 0)
    {
      sprintf(m_stALLTABLE.srcchecksql,"select count(*) from %s where keyid<=%ld",m_stALLTABLE.tname,umaxkeyid);
      sprintf(m_stALLTABLE.dstchecksql,"select count(*) from %s@%s where keyid<=%ld",m_stALLTABLE.tname,m_dblinkname,umaxkeyid);
    }
    else
    {
      sprintf(m_stALLTABLE.srcchecksql,"select count(*) from %s",m_stALLTABLE.tname);
      sprintf(m_stALLTABLE.dstchecksql,"select count(*) from %s@%s",m_stALLTABLE.tname,m_dblinkname);
    }
  }

  long icount1,icount2;
  icount1=icount2=0;

  stmt.prepare(m_stALLTABLE.dstchecksql);
  stmt.bindout(1,&icount2);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("CheckTable execute %s failed.\n%s\n",m_stALLTABLE.dstchecksql,stmt.cda.message);
    return stmt.cda.rc;
  }
  stmt.next();

  stmt.prepare(m_stALLTABLE.srcchecksql);
  stmt.bindout(1,&icount1);
  if (stmt.execute() != 0)
  {
    m_logfile->Write("CheckTable execute %s failed.\n%s\n",m_stALLTABLE.srcchecksql,stmt.cda.message);
    return stmt.cda.rc;
  }
  stmt.next();

  if (icount1==icount2) m_stALLTABLE.checkok=1;

  m_logfile->WriteEx("%ld,%ld,",icount1,icount2);

  return 0;
}

// 更新T_ALLTABLE表的核对结果和核对时间
long CALLTABLE::UptCheckTime()
{
  sqlstatement stmt;
  stmt.connect(m_connidc);

  stmt.prepare("\
    update T_APPTABLE set checkok=:1,checktime=sysdate where appid=:2 and tname=:3");
  stmt.bindin(1,&m_stALLTABLE.checkok);
  stmt.bindin(2,&m_stALLTABLE.appid);
  stmt.bindin(3, m_stALLTABLE.tname,30);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptCheckTime update T_APPTABLE failed.\n%s\n",stmt.cda.message);
  }

  return stmt.cda.rc;
}
*/

// 更新T_ALLTABLE表的备份时间字段
long CALLTABLE::UptBackupTime(char *strTName)
{
  sqlstatement stmt;
  stmt.connect(m_connidc);
  stmt.prepare("update T_ALLTABLE set backuptime=sysdate where tname=upper(:1)");
  stmt.bindin(1, strTName,30);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptBackupTime select %s failed.\n%s\n",m_stALLTABLE.tname,stmt.cda.message);
  }

  return stmt.cda.rc;
}

// 用当前表名创建联合当前表、历史表和归档表的视图
long CrtView(connection *conn,char *strTName)
{
  char strColumnStr[2048];
  memset(strColumnStr,0,sizeof(strColumnStr));

  CTABFIELD TABFIELD;
  if (TABFIELD.GetALLField(conn,strTName) == FALSE) return FALSE;
  strcpy(strColumnStr,TABFIELD.m_allfieldstr);

  sqlstatement stmt;
  stmt.connect(conn);

  char strTempTName[51];

  // 目前，改为只创建连接全部表的视图，不再创建连接当前表和历史表的视图

  /*
  // 判断历史表是否存在，如果存在，就创建连接当前表和历史表的视图
  memset(strTempTName,0,sizeof(strTempTName));
  sprintf(strTempTName,"%s_HIS",strTName);
  if (CheckTExist(conn,strTempTName) == TRUE)
  {
    stmt.prepare("create or replace view V_%s as select %s from %s union select %s from %s WITH READ ONLY",\
                  strTName+2,strColumnStr,strTName,strColumnStr,strTempTName);

    if (stmt.execute() != 0)
    {
      printf("create or replace view V_%s failed.\n%s\n",strTName+2,stmt.cda.message);
      return stmt.cda.rc;
    }
  }
  else
  {
    // 如果历史表不存在，视图就创建在当前表上
    stmt.prepare("create or replace view V_%s as select %s from %s WITH READ ONLY",\
                  strTName+2,strColumnStr,strTName);


    if (stmt.execute() != 0)
    {
      printf("create or replace view V_%s failed.\n%s\n",strTName+2,stmt.cda.message);
      return stmt.cda.rc;
    }
  }
  */

  // 创建连接全部表的视图
  char strSQL[40960],strTemp[4096];

  memset(strSQL,0,sizeof(strSQL));
  sprintf(strSQL,"create or replace view V_%s as select %s from %s",\
                  strTName+2,strColumnStr,strTName);

  // 查询出该当前表名的全部历史表和归档表的表名
  stmt.prepare("select upper(table_name) from USER_TABLES where table_name like upper('%s_HIS') or table_name like upper('%s_ARH_%%') order by length(table_name),table_name ",strTName,strTName);
  stmt.bindout(1,strTempTName,50);

  if (stmt.execute() != 0)
  {
    printf("select USER_TABLES failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }

  while (TRUE)
  {
    memset(strTempTName,0,sizeof(strTempTName));
    memset(strTemp,0,sizeof(strTemp));

    if (stmt.next() != 0) break;

    sprintf(strTemp," union select %s from %s",strColumnStr,strTempTName);
    strcat(strSQL,strTemp);
  }


  stmt.prepare("%s WITH READ ONLY",strSQL);
  if (stmt.execute() != 0)
  {
    printf("create or replace view V_%s failed.\n%s\n",strTName+2,stmt.cda.message);
    return stmt.cda.rc;
  }

  printf("ok.\n");

  return 0;
}

// 判断strtname表是否存在，如果不存在，就用tname2生成它
long CrtByTable(connection *conn,char *strtempletname,char *strtname)
{
  // 如果表存在，就不必创建了，返回
  if ( CheckTExist(conn,strtname) == TRUE) return 0;

  // 如果模板表或目的表不是以"T_"做为前缀，就返回失败，这是为了统一标准，没什么技术原因
  if ( (strncmp(strtempletname,"T_",2) != 0) || (strncmp(strtname,"T_",2) != 0) ) return -1;

  char strtablespace[31];
  memset(strtablespace,0,sizeof(strtablespace));

  // 从USER_TABLES数据字典中获取模板表所在的表空间
  //findbypk(conn,"USER_TABLES","table_name","tablespace_name",strtempletname,strtablespace,30);

  // 如果模板表所在的表空间是空的，表示不存在该表
  if (strlen(strtablespace) == 0) return -1;

  // 创建表
  sqlstatement stmt;
  stmt.connect(conn);
  stmt.prepare("create table %s tablespace %s as select * from %s where rownum=0",strtname,strtablespace,strtempletname);

  if (stmt.execute() != 0) return stmt.cda.rc;

  // 用模板表的索引生成目的表的索引

  char strindexname[31],struniqueness[31];


  sqlstatement selindexs;
  selindexs.connect(conn);
  selindexs.prepare(\
   "select index_name,uniqueness,tablespace_name from USER_INDEXES where table_name=:1 and index_type='NORMAL'");
  selindexs.bindin(1,strtempletname,30);
  selindexs.bindout(1,strindexname,30);
  selindexs.bindout(2,struniqueness,30);
  selindexs.bindout(3,strtablespace,30);

  char strcolumnname[51],strcolumnnamestr[51];

  sqlstatement selfields;
  selfields.connect(conn);
  selfields.prepare(\
   "select column_name from USER_IND_COLUMNS where table_name=:1 and index_name=:2 order by column_position");
  selfields.bindin(1,strtempletname,30);
  selfields.bindin(2,strindexname,30);
  selfields.bindout(1,strcolumnname,50);

  selindexs.execute();

  while (TRUE)
  {
    memset(strindexname,0,sizeof(strindexname));
    memset(struniqueness,0,sizeof(struniqueness));
    memset(strtablespace,0,sizeof(strtablespace));

    if (selindexs.next() != 0) break;

    if (strcmp(struniqueness,"UNIQUE") != 0) memset(struniqueness,0,sizeof(struniqueness));

    selfields.execute();

    memset(strcolumnnamestr,0,sizeof(strcolumnnamestr));

    while (TRUE)
    {
      memset(strcolumnname,0,sizeof(strcolumnname));

      if (selfields.next() != 0) break;

      if (selfields.cda.rpc != 1) strcat(strcolumnnamestr,",");

      strcat(strcolumnnamestr,strcolumnname);
    }

    // 替换索引的名，只替换一次，所以第四个参数一定要用FALSE
    UpdateStr(strindexname,strtempletname+2,strtname+2,FALSE);

    if ( (strncmp(strindexname,"PK_",3)==0) && (strcmp(struniqueness,"UNIQUE")==0) )
    {
      stmt.prepare(\
        "alter table %s add constraint %s primary key (%s) using index tablespace %s",strtname,strindexname,strcolumnnamestr,strtablespace);
    }
    else
    {
      stmt.prepare(\
        "create %s index %s on %s(%s) tablespace %s",struniqueness,strindexname,strtname,strcolumnnamestr,strtablespace);
    }

    if (stmt.execute() != 0) return stmt.cda.rc;
  }

  return 0;
}

// 备份到历史表
long CALLTABLE::BackupToHTable()
{
  if (m_vROWIDYEAR.size() == 0)
  {
    m_logfile->Write("0 rows backup\n"); return 0;
  }

  // 由当前表名组成历史表名
  char strHisTName[31];
  memset(strHisTName,0,sizeof(strHisTName));
  snprintf(strHisTName,31,"%s_HIS",m_stALLTABLE.tname);

  // 如果历史表不存在，就创建历史表
  if (CrtByTable(m_conndst,m_stALLTABLE.tname,strHisTName) != 0)
  {
    m_logfile->Write("CrtByTable(%s,%s) failed.\n",m_stALLTABLE.tname,strHisTName); return FALSE;
  }

  sqlstatement stmtins,stmtdel;
  stmtins.connect(m_conndst);
  stmtins.prepare("\
    insert into %s(%s) select %s from %s where rowid=:1",\
           strHisTName,m_COLUMNSSTR,m_COLUMNSSTR,m_stALLTABLE.tname);
  stmtins.bindin(1,m_stROWIDYEAR.rowid,30);

  stmtdel.connect(m_conndst);
  stmtdel.prepare("delete from %s where rowid=:1",m_stALLTABLE.tname);
  stmtdel.bindin(1,m_stROWIDYEAR.rowid,30);

  UINT ivSize=m_vROWIDYEAR.size();

  int iCount=0;

  for (UINT ii=0;ii<ivSize;ii++)
  {
    memcpy(&m_stROWIDYEAR,&m_vROWIDYEAR[ii],sizeof(m_stROWIDYEAR));

    if (stmtins.execute() != 0)
    {
      if (stmtins.cda.rc != 1)
      {
        m_logfile->Write("BackupToHIS insert %s failed.\n%s\n",strHisTName,stmtins.cda.message);
        return stmtins.cda.rc;
      }
    }

    if (stmtdel.execute() != 0)
    {
      m_logfile->Write("BackupToHIS delete %s failed.\n%s\n",m_stALLTABLE.tname,stmtdel.cda.message);
      return stmtdel.cda.rc;
    }

    if (iCount++>5000) { iCount=0; m_ProgramActive->WriteToFile(); }
  }

  m_logfile->Write("%ld rows backup to %s\n",ivSize,strHisTName);

  m_stALLTABLE.totalcount=ivSize;

  return 0;
}


// 只删除不备份
long CALLTABLE::DeleteTable()
{
  if (m_vROWIDYEAR.size() == 0)
  {
    m_logfile->Write("0 rows delete\n"); return 0;
  }

  char strrowidn[60][31];

  sqlstatement stmt;
  stmt.connect(m_conndst);
  stmt.prepare("delete from %s where rowid in (\
                 :1, :2, :3, :4, :5, :6, :7, :8, :9,:10,:11,:12,:13,:14,:15,:16,:17,:18,:19,:20,\
                :21,:22,:23,:24,:25,:26,:27,:28,:29,:30,:31,:32,:33,:34,:35,:36,:37,:38,:39,:40,\
                :41,:42,:43,:44,:45,:46,:47,:48,:49,:50,:51,:52,:53,:54,:55,:56,:57,:58,:59,:60)",\
                m_stALLTABLE.tname);
  for (UINT ii=0;ii<60;ii++)
  {
    stmt.bindin(ii+1,strrowidn[ii],30);
  }

  UINT ivSize=m_vROWIDYEAR.size();

  int iExecCount=0;
  int iCheckCount=0;

  memset(strrowidn,0,sizeof(strrowidn));

  for (UINT ii=0;ii<ivSize;ii++)
  {
    memcpy(&m_stROWIDYEAR,&m_vROWIDYEAR[ii],sizeof(m_stROWIDYEAR));

    strncpy(strrowidn[iExecCount++],m_stROWIDYEAR.rowid,30);


    if (iExecCount==60)
    {
      if (stmt.execute() != 0)
      {
        m_logfile->Write("DeleteTable delete %s failed.\n%s\n",m_stALLTABLE.tname,stmt.cda.message);
        return stmt.cda.rc;
      }

      iExecCount=0;
      memset(strrowidn,0,sizeof(strrowidn));
    }

    if (iCheckCount++>5000) { iCheckCount=0; m_ProgramActive->WriteToFile(); }
  }

  if (iExecCount>0)
  {
    if (stmt.execute() != 0)
    {
      m_logfile->Write("DeleteTable delete %s failed.\n%s\n",m_stALLTABLE.tname,stmt.cda.message);
      return stmt.cda.rc;
    }
  }

  m_logfile->Write("%ld rows delete.\n",ivSize);

  m_stALLTABLE.totalcount=ivSize;

  return 0;
}

CBDSYNCCFG::CBDSYNCCFG()
{ 
  m_conn=0;
  m_logfile=0;
 
  m_vBDSYNCCFG.clear();
} 

CBDSYNCCFG::~CBDSYNCCFG()
{ 

} 
  
void CBDSYNCCFG::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{ 
  m_conn = in_conn;
  m_logfile = in_logfile;
}

// 从T_BDSYNCCFG表中载入需要执行批量同步的表
long CBDSYNCCFG::LoadBSyncTable(int in_appid)
{
  // 获取数据库的时间
  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  //LocalTime(m_conn,strLocalTime,"yyyymmddhh24miss");

  char strTemp[101];
  memset(strTemp,0,sizeof(strTemp));
  sprintf(strTemp,"to_date('%s','yyyymmddhh24miss')",strLocalTime);

  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select tname,orgkeyid,\
           (select bsynctype from T_ALLTABLE where tname=T_BDSYNCCFG.tname) bsynctype,\
           (select trim(bsyncterm) from T_ALLTABLE where tname=T_BDSYNCCFG.tname) bsyncterm,\
           (select count(*) from USER_TAB_COLUMNS where table_name=T_BDSYNCCFG.tname and data_type in ('BLOB','CLOB')) lobfieldcount\
      from T_BDSYNCCFG\
     where appid in (select appid from T_DAPPSERVER where appid=:1 and rsts=1)\
       and rsts=1 and (bsynctime is null or bsynctime<=sysdate-bsynctvl/1440)\
       and tname in (select tname from T_ALLTABLE where bsync=1)\
     order by bsynctime");
  stmt.bindin(1,&in_appid);
  stmt.bindout(1, m_stBDSYNCCFG.tname,30);
  stmt.bindout(2,&m_stBDSYNCCFG.orgkeyid);
  stmt.bindout(3,&m_stBDSYNCCFG.bsynctype);
  stmt.bindout(4, m_stBDSYNCCFG.bsyncterm,500);
  stmt.bindout(5,&m_stBDSYNCCFG.lobfieldcount);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("LoadBSyncTable select T_BDSYNCCFG failed.\n%s\n",stmt.cda.message);
    return stmt.cda.rc;
  }

  while (TRUE)
  {
    memset(&m_stBDSYNCCFG,0,sizeof(m_stBDSYNCCFG));

    if (stmt.next() != 0) break;

    // 把同步条件中的sysdate替换为当前的时间
    UpdateStr(m_stBDSYNCCFG.bsyncterm,"sysdate",strTemp,TRUE);

    m_vBDSYNCCFG.push_back(m_stBDSYNCCFG);
  }

  return 0;
}

// 更新已同步表的执行时间为当前时间
long CBDSYNCCFG::UptBSyncTable(int in_appid,char *in_tname)
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    update T_BDSYNCCFG set bsynctime=sysdate where appid=:1 and tname=:2");
  stmt.bindin(1,&in_appid);
  stmt.bindin(2, in_tname,30);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptBSyncTable update T_BDSYNCCFG failed.\n%s\n",stmt.cda.message);
  }

  return stmt.cda.rc;
}

// 更新已同步表的已同步记录的标志
long CBDSYNCCFG::UptBSyncTable(int in_appid,char *in_tname,UINT in_orgkeyid)
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    update T_BDSYNCCFG set orgkeyid=:1 where appid=:2 and tname=:3");
  stmt.bindin(1,&in_orgkeyid);
  stmt.bindin(2,&in_appid);
  stmt.bindin(3, in_tname,30);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("UptBSyncTable update T_BDSYNCCFG failed.\n%s\n",stmt.cda.message);
  }

  return stmt.cda.rc;
}


CALARMLOG::CALARMLOG()
{
  m_conn=0;
  m_logfile=0;

  memset(&m_stALARMLOG,0,sizeof(m_stALARMLOG));

  m_vALARMLOG.clear();
}

void CALARMLOG::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn = in_conn;
  m_logfile = in_logfile;
}

CALARMLOG::~CALARMLOG()
{
}

BOOL CALARMLOG::ReadXMLFile(char *in_FileName)
{
  memset(&m_stALARMLOG,0,sizeof(m_stALARMLOG));

  FILE *fp=0;

  if ( (fp=FOPEN(in_FileName,"r")) == 0) return FALSE;

  char strBuffer[4096];

  if (FGETS(strBuffer,4000,fp,"<endl/>") == FALSE) { fclose(fp); return FALSE; }
  
  GetXMLBuffer(strBuffer,"crttime",m_stALARMLOG.leasttime,14);
  GetXMLBuffer(strBuffer,"progname",m_stALARMLOG.progname,100);
  GetXMLBuffer(strBuffer,"alarmtext",m_stALARMLOG.alarmtext,1000);

  DeleteRChar(m_stALARMLOG.alarmtext,'\n');
  
  fclose(fp);

  return TRUE;
}

long CALARMLOG::UptAlarmLog()
{
  sqlstatement stmtins,stmtupt;

  stmtins.connect(m_conn);
  stmtins.prepare("\
    insert into T_ALARMLOG(logid,progname,alarmtimes,begintime,leasttime,alarmtext,readbz,noticebz)\
    values(SEQ_ALARMLOG.nextval,:1,1,to_date(:2,'yyyymmddhh24miss'),to_date(:3,'yyyymmddhh24miss'),:4,1,1)");
  stmtins.bindin(1,m_stALARMLOG.progname,100);
  stmtins.bindin(2,m_stALARMLOG.leasttime,14);
  stmtins.bindin(3,m_stALARMLOG.leasttime,14);
  stmtins.bindin(4,m_stALARMLOG.alarmtext,1000);

  if (strlen(m_stALARMLOG.progname) == 0) return 0;
  if (strlen(m_stALARMLOG.leasttime) == 0) return 0;
  if (strlen(m_stALARMLOG.alarmtext) == 0) return 0;

  if (stmtins.execute() == 0) return 0;

  if (stmtins.cda.rc > 1)
  {
    m_logfile->Write("insert T_ALARMLOG failed.\n%s\n",stmtins.cda.message); return stmtins.cda.rc;
  }

  // 以下是stmtins.cda.rc==1的情况
  stmtupt.connect(m_conn);
  stmtupt.prepare("update T_ALARMLOG set alarmtimes=alarmtimes+1,leasttime=to_date(:1,'yyyymmddhh24miss'),readbz=1,noticebz=1 where progname=:2 and alarmtext=:3");
  stmtupt.bindin(1,m_stALARMLOG.leasttime,14);
  stmtupt.bindin(2,m_stALARMLOG.progname,100);
  stmtupt.bindin(3,m_stALARMLOG.alarmtext,1000);

  if (stmtupt.execute() != 0)
  {
    m_logfile->Write("update T_ALARMLOG failed.\n%s\n",stmtupt.cda.message); return stmtupt.cda.rc;
  }

  return 0;
}

// 删除表中三天之前的记录
long CALARMLOG::DelAlarmLog()
{
  sqlstatement stmtdel;
  stmtdel.connect(m_conn);
  stmtdel.prepare("delete T_ALARMLOG where leasttime<sysdate-3 and rownum<100");

  if (stmtdel.execute() != 0)
  {
    m_logfile->Write("delete T_ALARMLOG failed.\n%s\n",stmtdel.cda.message); return stmtdel.cda.rc;
  }

  return 0;
}

CIDCCFG::CIDCCFG()
{
  initdata();
}

void CIDCCFG::initdata()
{
  memset(m_idcconnstr,0,sizeof(m_idcconnstr));          // 数据中心应用数据库的连接串

  memset(m_idcusername,0,sizeof(m_idcusername));        // 数据中心数据库的用户名

  memset(m_logpath,0,sizeof(m_logpath));                // 日志文件目录

  memset(m_listpath,0,sizeof(m_listpath));              // 存放二进制文档文件导出列表文件的目录

  memset(m_tmppath,0,sizeof(m_tmppath));                // 存放临时文件的目录

  memset(m_sdatastdpath,0,sizeof(m_sdatastdpath));        // 二维表格数据标准XML文件存放的目录
  memset(m_sdatastdbakpath,0,sizeof(m_sdatastdbakpath));     // 二维表格数据标准XML文件备份的目录
  memset(m_sdatastderrpath,0,sizeof(m_sdatastderrpath));     // 二维表格数据标准XML文件错误文件的备份的目录

  memset(m_wfilestdpath,0,sizeof(m_wfilestdpath));        // 二进制文档文件数据标准命名文件存放的目录
  memset(m_wfilestdbakpath,0,sizeof(m_wfilestdbakpath));     // 二进制文档文件数据标准命名文件备份的目录
  memset(m_wfilestderrpath,0,sizeof(m_wfilestderrpath));     // 二进制文档文件数据标准命名文件备份的目录
}

// 载入短信系统参数
BOOL CIDCCFG::LoadIniFile(char *in_inifile)
{
  initdata();

  CIniFile IniFile;

  // 导入参数文件
  if (IniFile.LoadFile(in_inifile) == FALSE)
  {
    printf("IniFile.LoadFile(%s) failed.\n",in_inifile); return FALSE;
  }

  if (IniFile.GetValue("idcconnstr",m_idcconnstr,100) == FALSE)
  {
    printf("IniFile.GetValue field(idcconnstr) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("idcusername",m_idcusername,100) == FALSE)
  {
    printf("IniFile.GetValue field(idcusername) failed.\n"); return FALSE;
  }

  // 把数据中心的用户名转换为大写
  ToUpper(m_idcusername);

  if (IniFile.GetValue("logpath",m_logpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(logpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("listpath",m_listpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(listpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("tmppath",m_tmppath,200) == FALSE)
  {
    printf("IniFile.GetValue field(tmppath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("sdatastdpath",m_sdatastdpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(sdatastdpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("sdatastdbakpath",m_sdatastdbakpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(sdatastdbakpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("sdatastderrpath",m_sdatastderrpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(sdatastderrpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("wfilestdpath",m_wfilestdpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(wfilestdpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("wfilestdbakpath",m_wfilestdbakpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(wfilestdbakpath) failed.\n"); return FALSE;
  }

  if (IniFile.GetValue("wfilestderrpath",m_wfilestderrpath,200) == FALSE)
  {
    printf("IniFile.GetValue field(wfilestderrpath) failed.\n"); return FALSE;
  }

  return TRUE;
}

CFILELIST::CFILELIST()
{
  memset(m_filename,0,sizeof(m_filename));
  memset(m_ddatetime,0,sizeof(m_ddatetime));
  memset(m_ftypename,0,sizeof(m_ftypename));
  memset(m_pfilename,0,sizeof(m_pfilename));
  memset(m_addatetime,0,sizeof(m_addatetime));
  memset(m_dmintime,0,sizeof(m_dmintime));
  memset(m_tname,0,sizeof(m_tname));
  memset(m_fullfilename,0,sizeof(m_fullfilename));

  m_conn=0; m_logfile=0;
}

CFILELIST::~CFILELIST()
{
  m_vFILELIST.clear();
}

void CFILELIST::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn; m_logfile=in_logfile;
}

// 把全部的气象文件参数载入容器
long CFILELIST::LoadFileCFG()
{
  // 注意，一定要按length(pfilename)降序，保证更精确的配置参数有高的优先级。
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select dtypename,dtypeid,to_char(dmintime,'yyyymmddhh24miss') dmintime,\
           pfilename,upttype,upttlimit,tname,addatetime,length(pfilename) pfilenamelen\
      from T_APPDTYPE where rsts=1 and tname in (select tname from t_ALLTABLE where tabletype=2) order by pfilenamelen desc");
  stmt.bindout(1, m_stFILELIST.ftypename,50);
  stmt.bindout(2,&m_stFILELIST.dtypeid);
  stmt.bindout(3, m_stFILELIST.dmintime,14);
  stmt.bindout(4, m_stFILELIST.pfilename,201);
  stmt.bindout(5,&m_stFILELIST.upttype);
  stmt.bindout(6,&m_stFILELIST.upttlimit);
  stmt.bindout(7, m_stFILELIST.tname,30);
  stmt.bindout(8, m_stFILELIST.addatetime,200);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("LoadFileCFG select T_APPDTYPE,t_ALLTABLE failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
  }
  
  m_vFILELIST.clear();

  while (TRUE)
  {
    memset(&m_stFILELIST,0,sizeof(m_stFILELIST));

    if (stmt.next() != 0) break;

    // 如果pfilename为空，那么一定是参数配置错了
    if (strlen(m_stFILELIST.pfilename) == 0) continue;

    m_vFILELIST.push_back(m_stFILELIST);
  }

  return 0;
}

// 从容器中获取某种类型文件的参数
BOOL CFILELIST::GETFILECFG()
{
  UINT uPOS;

  char pfilenametmp[301],m_filenametmp[301];

  memset(m_filenametmp,0,sizeof(m_filenametmp));
  strcpy(m_filenametmp,m_filename);
  ToUpper(m_filenametmp);

  for (uPOS=0;uPOS<m_vFILELIST.size();uPOS++)
  {
    memset(pfilenametmp,0,sizeof(pfilenametmp));
    strcpy(pfilenametmp,m_vFILELIST[uPOS].pfilename);
    ToUpper(pfilenametmp);

    if ( (strncmp(pfilenametmp,m_filenametmp,strlen(pfilenametmp)) == 0) ||
         (MatchFileName(m_filenametmp,pfilenametmp) == TRUE) )
    {
      m_dtypeid=m_vFILELIST[uPOS].dtypeid;
      strcpy(m_ftypename,m_vFILELIST[uPOS].ftypename);
      strcpy(m_dmintime,m_vFILELIST[uPOS].dmintime);
      strcpy(m_addatetime,m_vFILELIST[uPOS].addatetime);
      strcpy(m_pfilename,m_vFILELIST[uPOS].pfilename);
      m_upttype=m_vFILELIST[uPOS].upttype;
      m_upttlimit=m_vFILELIST[uPOS].upttlimit;
      strcpy(m_tname,m_vFILELIST[uPOS].tname);

      return TRUE;
    }
  }

  return FALSE;
}

// 根据数据类别和文件名，查询表中是否已存在该文件
long CFILELIST::FindFExist()
{
  stmtfexist.connect(m_conn);
  stmtfexist.prepare("select keyid,round((sysdate-ddatetime)*24,0) from %s where filename=:1",m_tname);
  stmtfexist.bindin(1, m_filename,200);
  stmtfexist.bindout(1,&m_keyid);
  stmtfexist.bindout(2,&m_timeexist);

  if (stmtfexist.execute() != 0)
  {
    m_logfile->Write("FindFExist select %s failed.\n%s\n",m_tname,stmtfexist.cda.message); 
  }

  return stmtfexist.cda.rc;
}

long CFILELIST::FindFExistNext()
{
  m_keyid=0;
  m_timeexist=0;

  return stmtfexist.next();
}

long CFILELIST::InsertFileToDBEx()
{
  // 获取基表的序列生成器的值，用于keyid
  char strSequenceName[51];
  memset(strSequenceName,0,sizeof(strSequenceName));
  snprintf(strSequenceName,51,"SEQ_%s",m_tname+2);

  if (FetchSequence(m_conn,strSequenceName,m_keyid) != 0)
  {
    m_logfile->Write("FetchSequence %s failed.\n",strSequenceName); return 3;
  }

  // 更新相关的表
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
      insert into %s(filename,ddatetime,filesize,filecontent,keyid)\
              values(:1,to_date(:2,'yyyymmddhh24miss'),:3,empty_blob(),:4)",m_tname);
  stmt.bindin( 1, m_filename,200);
  stmt.bindin( 2, m_ddatetime,14);
  stmt.bindin( 3,&m_filesize);
  stmt.bindin( 4,&m_keyid);

  m_filesize=FileSize(m_fullfilename);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("InsertFileToDBEx insert %s failed.\n%s\n",m_tname,stmt.cda.message); return 3;
  }

  m_IsInsertFile=TRUE;

  return UpdateFileToDBEx();
}

// 用新的文件内容更新旧的内容
long CFILELIST::UpdateFileToDBEx()
{
  // 更新文件的大小和更新时间字段
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("update %s set filesize=:1,filecontent=empty_blob(),upttime=sysdate where keyid=:2",m_tname);
  stmt.bindin(1,&m_filesize);
  stmt.bindin(2,&m_keyid);

  m_filesize=FileSize(m_fullfilename);
  
  if (stmt.execute() != 0) 
  {
    m_logfile->Write("UpdateFileToDBEx update %s failed.\n%s\n",m_tname,stmt.cda.message); return 3;
  }

  // 更新filecontent字段的内容
  stmt.prepare("select filecontent from %s where keyid=:1 for update",m_tname);
  stmt.bindin(1,&m_keyid);
  //stmt.bindblob(1);
  if (stmt.execute() != 0) 
  {
    m_logfile->Write("UpdateFileToDBEx execute %s failed.\n%s\n",m_tname,stmt.cda.message); return 3;
  }

  if (stmt.next() != 0)
  {
    m_logfile->Write("UpdateFileToDBEx next %s failed.\n%s\n",m_tname,stmt.cda.message); return 3;
  }

  // 把文件写入BLOB字段
  /*
  if (stmt.filetolob(m_fullfilename) != 0)
  {
    m_logfile->Write("UpdateFileToDBEx filetolob %s failed.\n%s\n",m_tname,stmt.cda.message); return 3;
  }
  */

  return 0;
}

// 获取序列生成器的值
long FetchSequence(connection *conn,char *SequenceName,UINT &uValue)
{
  sqlstatement stmt;
  stmt.connect(conn);
  if (strcmp(conn->m_dbtype,"oracle")==0)
  {
    stmt.prepare("select %s.nextval from dual",SequenceName);
  }
  else
  {
    stmt.prepare("select %s.nextval",SequenceName);
  }
  stmt.bindout(1,&uValue);
  stmt.execute();

  return stmt.next();
}

// 判断数据时间是否合法，即是否在dmintime日到当前时间之后的10天
BOOL CheckDDateTime(char *in_DDateTime,char *in_DMinTime)
{
  char strDDateTime[20];
  memset(strDDateTime,0,sizeof(strDDateTime));

  PickNumber(in_DDateTime,strDDateTime,FALSE,FALSE);

  // 如果数据的时间小于dmintime，认为是非法的
  if (strcmp(strDDateTime,in_DMinTime) < 0) return FALSE;

  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss",20*24*60*60);

  // 如果数据的时间大于当前时间加10天，认为是非法的
  if (strncmp(strDDateTime,strLocalTime,14) > 0) return FALSE;
  
  return TRUE;
}


CDMONCFG::CDMONCFG()
{
  m_conn=0;
  m_logfile=0;
}

CDMONCFG::~CDMONCFG()
{
}

void CDMONCFG::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;
  m_logfile=in_logfile;
};


// 从T_DMONCFG表中加载需要统计的记录，存放在m_vDMONCFG中
BOOL CDMONCFG::LoadDMONCFG(const char *strWhere)
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
   select taskid,taskname,tname,trim(strwhere),analytype,analyunit,spanunit,exectvl,\
          to_char(exectime,'yyyymmddhh24miss'),alarmbz,dcount,delaytime,alarmtvl,\
          to_char(alarmtime,'yyyymmddhh24miss'),alarmsts,alarminfo,alarmtimes,alarmedtimes,\
          (sysdate-alarmtime)*1440 alarmedtvl\
     from T_DMONCFG %s order by exectime",strWhere);
  stmt.bindout( 1,&m_stDMONCFG.taskid);
  stmt.bindout( 2, m_stDMONCFG.taskname,100);
  stmt.bindout( 3, m_stDMONCFG.tname,50);
  stmt.bindout( 4, m_stDMONCFG.strwhere,500);
  stmt.bindout( 5,&m_stDMONCFG.analytype);
  stmt.bindout( 6,&m_stDMONCFG.analyunit);
  stmt.bindout( 7,&m_stDMONCFG.spanunit);
  stmt.bindout( 8,&m_stDMONCFG.exectvl);
  stmt.bindout( 9, m_stDMONCFG.exectime,14);
  stmt.bindout(10,&m_stDMONCFG.alarmbz);
  stmt.bindout(11,&m_stDMONCFG.dcount);
  stmt.bindout(12,&m_stDMONCFG.delaytime);
  stmt.bindout(13,&m_stDMONCFG.alarmtvl);
  stmt.bindout(14, m_stDMONCFG.alarmtime,14);
  stmt.bindout(15,&m_stDMONCFG.alarmsts);
  stmt.bindout(16, m_stDMONCFG.alarminfo,500);
  stmt.bindout(17,&m_stDMONCFG.alarmtimes);
  stmt.bindout(18,&m_stDMONCFG.alarmedtimes);
  stmt.bindout(19,&m_stDMONCFG.alarmedtvl);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("select T_DMONCFG failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  while (TRUE)
  {
    memset(&m_stDMONCFG,0,sizeof(m_stDMONCFG));
   
    if (stmt.next() != 0) break;

    m_vDMONCFG.push_back(m_stDMONCFG);
  }

  return TRUE;
}

// 读取m_vDMONCFG中全部的参数，分析告警 
BOOL CDMONCFG::MONTable()
{
  int  totalcount=0;

  sqlstatement stmtsel;
  stmtsel.connect(m_conn);
  stmtsel.prepare("select totalcount from T_DMONITEM where taskid=:1 and ddatetime>=sysdate-:2/1440 and totalcount>=:3");
  stmtsel.bindin(1,&m_stDMONCFG.taskid);
  stmtsel.bindin(2,&m_stDMONCFG.delaytime);
  stmtsel.bindin(3,&m_stDMONCFG.dcount);
  stmtsel.bindout(1,&totalcount);

  for (UINT ii=0;ii<m_vDMONCFG.size();ii++)
  {
    memset(&m_stDMONCFG,0,sizeof(m_stDMONCFG));
    memcpy(&m_stDMONCFG,&m_vDMONCFG[ii],sizeof(m_stDMONCFG));

    m_logfile->Write("taskid=%ld,taskname=%s,alarmbz=%d,dcount=%d,delaytime=%d,alarmtvl=%d.\n",\
                      m_stDMONCFG.taskid,m_stDMONCFG.taskname,m_stDMONCFG.alarmbz,m_stDMONCFG.dcount,\
                      m_stDMONCFG.delaytime,m_stDMONCFG.alarmtvl);

    if (stmtsel.execute() != 0)
    {
      m_logfile->Write("select %s failed.\n%s\n",stmtsel.cda.message); return FALSE;
    }

    totalcount=0;

    stmtsel.next();

    if (totalcount>=m_stDMONCFG.dcount)
    {
      // 告警解除
      sqlstatement stmt;
      stmt.connect(m_conn);
      stmt.prepare("update T_DMONCFG set alarmtime=null,alarmsts=1,alarminfo=null,alarmedtimes=0 where taskid=:1");
      stmt.bindin(1,&m_stDMONCFG.taskid);
      stmt.execute();
      m_conn->commitwork();
      continue;
    }

    if ( (m_stDMONCFG.alarmbz != 1) || (m_stDMONCFG.alarmedtimes>=m_stDMONCFG.alarmtimes) ) continue;
  
    // 数据缺失，分析告警
    if ( (strlen(m_stDMONCFG.alarmtime)==0) || (m_stDMONCFG.alarmedtvl>=m_stDMONCFG.alarmtvl) )
    {
      char strLocalTime[21];
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi");
      memset(m_stDMONCFG.alarminfo,0,sizeof(m_stDMONCFG.alarminfo));
      sprintf(m_stDMONCFG.alarminfo,"数据延迟告警，任务：%s，记录阀值：%d，时间阀值：%d，告警时间：%s。",m_stDMONCFG.taskname,m_stDMONCFG.dcount,m_stDMONCFG.delaytime,strLocalTime);

      sqlstatement stmt;
      stmt.connect(m_conn);
      stmt.prepare("update T_DMONCFG set alarmtime=sysdate,alarmsts=2,alarminfo=:1,alarmedtimes=alarmedtimes+1 where taskid=:2");
      stmt.bindin(1, m_stDMONCFG.alarminfo,500);
      stmt.bindin(2,&m_stDMONCFG.taskid);
      stmt.execute();
      stmt.prepare("insert into T_DMONLOG(logid,taskid,alarminfo,ddatetime) values(SEQ_DMONLOG.nextval,:1,:2,sysdate)");
      stmt.bindin(1,&m_stDMONCFG.taskid);
      stmt.bindin(2, m_stDMONCFG.alarminfo,500);
      stmt.execute();
      m_conn->commitwork();

      m_logfile->Write("%s\n",m_stDMONCFG.alarminfo);

      m_logfile->WriteAlarmFile();
    }
  }

  return TRUE;
}

// 读取m_vDMONCFG中全部的参数，统计数据量
BOOL CDMONCFG::CountTable()
{
  sqlstatement stmtuptcfg,stmtsel,stmtupt,stmtins;

  stmtuptcfg.connect(m_conn);
  stmtsel.connect(m_conn);
  stmtupt.connect(m_conn);
  stmtins.connect(m_conn);

  stmtuptcfg.prepare("update T_DMONCFG set exectime=sysdate where taskid=:1");
  stmtuptcfg.bindin(1,&m_stDMONCFG.taskid);

  int totalcount,timedelay;

  for (UINT ii=0;ii<m_vDMONCFG.size();ii++)
  {
    memset(&m_stDMONCFG,0,sizeof(m_stDMONCFG));
    memcpy(&m_stDMONCFG,&m_vDMONCFG[ii],sizeof(m_stDMONCFG));

    m_logfile->Write("taskid=%ld,taskname=%s,analytype=%d,analyunit=%d...",m_stDMONCFG.taskid,m_stDMONCFG.taskname,m_stDMONCFG.analytype,m_stDMONCFG.analyunit);

    // 分配时间粒度
    TimeSpan.defvector(m_stDMONCFG.analyunit,m_stDMONCFG.spanunit);

    char strSQL[2048];
    memset(strSQL,0,sizeof(strSQL));
    if (m_stDMONCFG.analytype==1) // 区间内
      snprintf(strSQL,2000,"select count(*),avg(crttime-to_date(:1,'yyyymmddhh24miss'))*1440 from %s where ddatetime>=to_date(:2,'yyyymmddhh24miss') and ddatetime<to_date(:3,'yyyymmddhh24miss') ",m_stDMONCFG.tname);
    else  // 时间点
      snprintf(strSQL,2000,"select count(*),avg(crttime-to_date(:1,'yyyymmddhh24miss'))*1440 from %s where ddatetime =to_date(:2,'yyyymmddhh24miss') ",m_stDMONCFG.tname);

    if (strlen(m_stDMONCFG.strwhere) > 0)
    {
      strcat(strSQL," and "); strcat(strSQL,m_stDMONCFG.strwhere);
    }
    stmtsel.prepare(strSQL);
    stmtsel.bindin(1,TimeSpan.m_sttimespan.begintime,14);
    stmtsel.bindin(2,TimeSpan.m_sttimespan.begintime,14);
    if (m_stDMONCFG.analytype==1) stmtsel.bindin(3,TimeSpan.m_sttimespan.endtime,14);
    stmtsel.bindout(1,&totalcount);
    stmtsel.bindout(2,&timedelay);

    stmtupt.prepare("update T_DMONITEM set totalcount=:1,timedelay=:2,crttime=sysdate where taskid=:3 and ddatetime=to_date(:4,'yyyymmddhh24miss')");
    stmtupt.bindin(1,&totalcount);
    stmtupt.bindin(2,&timedelay);
    stmtupt.bindin(3,&m_stDMONCFG.taskid);
    stmtupt.bindin(4, TimeSpan.m_sttimespan.begintime,14);

    stmtins.prepare("\
      insert into T_DMONITEM(totalcount,timedelay,crttime,taskid,ddatetime)\
                      values(:1,:2,sysdate,:3,to_date(:4,'yyyymmddhh24miss'))");
    stmtins.bindin(1,&totalcount);
    stmtins.bindin(2,&timedelay);
    stmtins.bindin(3,&m_stDMONCFG.taskid);
    stmtins.bindin(4, TimeSpan.m_sttimespan.begintime,14);

    // 统计每个时间粒度的数据量
    for (UINT jj=0;jj<TimeSpan.m_vtimespan.size();jj++)
    {
      totalcount=timedelay=0;
      memcpy(&TimeSpan.m_sttimespan,&TimeSpan.m_vtimespan[jj],sizeof(struct st_timespan));

      if (stmtsel.execute() != 0)
      {
        m_logfile->Write("select %s failed.\n%s\n",m_stDMONCFG.tname,stmtsel.cda.message); return FALSE;
      }

      stmtsel.next();

      if (stmtupt.execute() != 0)
      {
        m_logfile->Write("update T_DMONITEM failed.\n%s\n",stmtupt.cda.message); return FALSE;
      }
      
      if (stmtupt.cda.rpc==0)
      {
        if (stmtins.execute() != 0)
        {
          m_logfile->Write("insert T_DMONITEM failed.\n%s\n",stmtins.cda.message); return FALSE;
        }
      }
    }

    if (stmtuptcfg.execute() != 0)
    {
      m_logfile->Write("update T_DMONCFG failed.\n%s\n",stmtuptcfg.cda.message); return FALSE;
    }

    m_logfile->WriteEx("ok.\n");

    m_conn->commitwork();
  }

  return TRUE;
}

// 分配一个时间粒度表，从spanunit分钟到当前时间
// unit为时间粒度，1-五分钟；2-六分钟；3-十分钟；4-小时；5-天。
void CTimeSpan::defvector(int unit,int spanunit)
{
  m_vtimespan.clear();

  int  timetvl=0;
  char strStartTime[21],strEndTime[21];

  memset(strStartTime,0,sizeof(strStartTime));
  memset(strEndTime,0,sizeof(strEndTime));

  LocalTime(strStartTime,"yyyymmddhh24miss");

  // 获取当前时间点所处的粒度的开始时间和结束时间

  // 1-五分钟
  if (unit==1)
  {
    timetvl=5;

    strStartTime[12]=0;
    
    int imin=atoi(strStartTime+10); 

    if ( (imin>= 0) && (imin< 5) ) imin= 0;
    if ( (imin>= 5) && (imin<10) ) imin= 5;
    if ( (imin>=10) && (imin<15) ) imin=10;
    if ( (imin>=15) && (imin<20) ) imin=15;
    if ( (imin>=20) && (imin<25) ) imin=20;
    if ( (imin>=25) && (imin<30) ) imin=25;
    if ( (imin>=30) && (imin<35) ) imin=30;
    if ( (imin>=35) && (imin<40) ) imin=35;
    if ( (imin>=40) && (imin<45) ) imin=40;
    if ( (imin>=45) && (imin<50) ) imin=45;
    if ( (imin>=50) && (imin<55) ) imin=50;
    if ( (imin>=55) && (imin<60) ) imin=55;

    sprintf(strStartTime+10,"%02d00",imin);
  }

  // 2-六分钟
  if (unit==2)
  {
    timetvl=6;

    strStartTime[12]=0;
    
    int imin=atoi(strStartTime+10); 

    if ( (imin>= 0) && (imin< 6) ) imin= 0;
    if ( (imin>= 6) && (imin<12) ) imin= 6;
    if ( (imin>=12) && (imin<18) ) imin=12;
    if ( (imin>=18) && (imin<24) ) imin=18;
    if ( (imin>=24) && (imin<30) ) imin=24;
    if ( (imin>=30) && (imin<36) ) imin=30;
    if ( (imin>=36) && (imin<42) ) imin=36;
    if ( (imin>=42) && (imin<48) ) imin=42;
    if ( (imin>=48) && (imin<54) ) imin=48;
    if ( (imin>=54) && (imin<60) ) imin=54;

    sprintf(strStartTime+10,"%02d00",imin);
  }

  // 3-十分钟
  if (unit==3)
  {
    timetvl=10;

    strStartTime[12]=0;
    
    int imin=atoi(strStartTime+10); 

    if ( (imin>= 0) && (imin<10) ) imin= 0;
    if ( (imin>=10) && (imin<20) ) imin=10;
    if ( (imin>=20) && (imin<30) ) imin=20;
    if ( (imin>=30) && (imin<40) ) imin=30;
    if ( (imin>=40) && (imin<50) ) imin=40;
    if ( (imin>=50) && (imin<60) ) imin=50;

    sprintf(strStartTime+10,"%02d00",imin);
  }

  // 4-小时
  if (unit==4)
  {
    timetvl=60;

    strStartTime[10]=0;
    strncat(strStartTime,"0000",4);
  }

  // 5-天
  if (unit==5)
  {
    timetvl=1440;

    strStartTime[8]=0;
    strncat(strStartTime,"000000",6);
  }

  // 获取当前时间点所处的粒度的结束时间
  AddTime(strStartTime,strEndTime,timetvl*60,"yyyymmddhh24miss");

  for (int ii=0;ii<spanunit;ii++)
  {
    memset(&m_sttimespan,0,sizeof(m_sttimespan));

    AddTime(strStartTime,m_sttimespan.begintime,0-ii*timetvl*60,"yyyymmddhh24miss");
    AddTime(strEndTime,m_sttimespan.endtime,0-ii*timetvl*60,"yyyymmddhh24miss");

    m_vtimespan.push_back(m_sttimespan);
  }
}

// 杀死与同步目的表有关的会话，清理出锁
BOOL KillLocked(connection *in_conn,char *in_tname)
{
  int isid,iserial;
  sqlstatement stmt;
  stmt.connect(in_conn);
  stmt.prepare("select s.sid,s.serial# from V$LOCKED_OBJECT L, DBA_OBJECTS O, V$SESSION S\
     where L.object_id = O.object_id\
       and L.session_id =S.sid and object_name=:1");
  stmt.bindin(1,in_tname,30);
  stmt.bindout(1,&isid);
  stmt.bindout(2,&iserial);

  if (stmt.execute() != 0)
  {
    return FALSE;
  }

  BOOL bKilledSession=FALSE;

 sqlstatement stmtkill;
  stmtkill.connect(in_conn);
  while (TRUE)
  {
    isid=iserial=0;
    if (stmt.next() != 0) break;

    stmtkill.prepare("alter system kill session '%d,%d' immediate",isid,iserial);

    stmtkill.execute();

    bKilledSession=TRUE;
  }

  if (bKilledSession==TRUE) sleep(30);

  return TRUE;
}

CEXPDTASK::CEXPDTASK()
{
  m_conn=0;
  m_logfile=0;

  memset(&m_stEXPDTASK,0,sizeof(m_stEXPDTASK));
}

CEXPDTASK::~CEXPDTASK()
{
}

void CEXPDTASK::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;
  m_logfile=in_logfile;
}

// 根据taskid从任务表中获取任务的详细信息，结果存放在m_stEXPDTASK中。
long CEXPDTASK::GetTaskByID(UINT in_tasktype)
{
  selstmt.connect(m_conn);
  selstmt.prepare(\
    "select taskid,taskname,tnsname,trim(selectsql),trim(lower(fieldstr)),\
            trim(fieldlen),exptype,position,trim(firstsql),trim(bfilename),trim(efilename),outpath\
       from T_EXPDTASK where tasktype=:1 and rsts=1 and (exptime<=sysdate or exptime is null)");
  selstmt.bindin(1,&in_tasktype);
  selstmt.bindout( 1,&m_stEXPDTASK.taskid);
  selstmt.bindout( 2, m_stEXPDTASK.taskname,100);
  selstmt.bindout( 3, m_stEXPDTASK.tnsname,100);
  selstmt.bindout( 4, m_stEXPDTASK.selectsql,4000);
  selstmt.bindout( 5, m_stEXPDTASK.fieldstr,4000);
  selstmt.bindout( 6, m_stEXPDTASK.fieldlen,4000);
  selstmt.bindout( 7,&m_stEXPDTASK.exptype);
  selstmt.bindout( 8,&m_stEXPDTASK.position);
  selstmt.bindout( 9, m_stEXPDTASK.firstsql,4000);
  selstmt.bindout(10, m_stEXPDTASK.bfilename,30);
  selstmt.bindout(11, m_stEXPDTASK.efilename,30);
  selstmt.bindout(12, m_stEXPDTASK.outpath,200);

  if (selstmt.execute() != 0)
  {
    m_logfile->Write("GetTaskByID select T_EXPDTASK failed.\n%s\n",selstmt.cda.message);
  }

  return selstmt.cda.rc;
}


long CEXPDTASK::GetTaskByIDNext()
{
  memset(&m_stEXPDTASK,0,sizeof(m_stEXPDTASK));

  selstmt.next();

  if (selstmt.cda.rc > 0) return selstmt.cda.rc;

  // 删除SQL语句右达的分号
  DeleteRChar(m_stEXPDTASK.firstsql ,';');
  DeleteRChar(m_stEXPDTASK.selectsql,';');

  return 0;
}

// 更新任务的导出时间
long CEXPDTASK::UptExpTime()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("update T_EXPDTASK set exptime=sysdate+exptvl/1440,position=:1 where taskid=:2");
  stmt.bindin(1,&m_stEXPDTASK.position);
  stmt.bindin(2,&m_stEXPDTASK.taskid);

  return stmt.execute();
}

CRealMon::CRealMon()
{
  m_vCOLLECTLOG.clear();
  m_vPROCESSLOG.clear();
  m_vTODBLOG.clear();  
  m_vCALCLOG.clear();  
  m_vTRANSFERLOG.clear();

  memset(&m_stCOLLECTLOG,0,sizeof(m_stCOLLECTLOG));
  memset(&m_stPROCESSLOG,0,sizeof(m_stPROCESSLOG));
  memset(&m_stTODBLOG,0,sizeof(m_stTODBLOG));
  memset(&m_stPROACTLOG,0,sizeof(m_stPROACTLOG));
  memset(&m_stCALCLOG,0,sizeof(m_stCALCLOG));
  memset(&m_stTRANSFERLOG,0,sizeof(m_stTRANSFERLOG));
  uFileSeq = 0;
  bIsFirstTime=TRUE;
}

CRealMon::~CRealMon()
{
}

BOOL CRealMon::WriteToCollectLog()
{
  if (m_vCOLLECTLOG.size()==0) return TRUE;

  memset(strCollFileName,0,sizeof(strCollFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");

  if (uFileSeq++ > 100) uFileSeq =0;
  snprintf(strCollFileName,300,"/tmp/htidc/monfile/COLLECTLOG_%s_%d_%d.xml",strLocalTime,getpid(),uFileSeq);

  if (CollFile.OpenForRename(strCollFileName,"w+") == FALSE) return FALSE;

  CollFile.Fprintf("<data>\n");

  for (UINT ii=0;ii<m_vCOLLECTLOG.size();ii++)
  {
    CollFile.Fprintf(\
		     "<indexid>%s</indexid>"
		     "<serverip>%s</serverip>"
		     "<programname>%s</programname>"
		     "<colltype>%s</colltype>"
		     "<colltime>%s</colltime>"
		     "<remoteip>%s</remoteip>"
		     "<filename>%s</filename>"
		     "<filesize>%s</filesize>"
		     "<filetime>%s</filetime><endl/>\n",
		     m_vCOLLECTLOG[ii].indexid,\
		     m_vCOLLECTLOG[ii].serverip,\
		     m_vCOLLECTLOG[ii].programname,\
		     m_vCOLLECTLOG[ii].colltype,\
		     m_vCOLLECTLOG[ii].colltime,\
		     m_vCOLLECTLOG[ii].remoteip,\
		     m_vCOLLECTLOG[ii].filename,\
		     m_vCOLLECTLOG[ii].filesize,\
		     m_vCOLLECTLOG[ii].filetime);
  }
  
  CollFile.Fprintf("</data>\n");

  CollFile.CloseAndRename();

  return TRUE;
}

BOOL CRealMon::WriteToProcessLog()
{
  if (m_vPROCESSLOG.size()==0) return TRUE;

  memset(strProcFileName,0,sizeof(strProcFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");

  if (uFileSeq++ > 100) uFileSeq =0;
  snprintf(strProcFileName,300,"/tmp/htidc/monfile/PROCESSLOG_%s_%d_%d.xml",strLocalTime,getpid(),uFileSeq);

  if (ProcFile.OpenForRename(strProcFileName,"w+") == FALSE) return FALSE;

  ProcFile.Fprintf("<data>\n");

  for (UINT ii=0;ii<m_vPROCESSLOG.size();ii++)
  {
    ProcFile.Fprintf(\
                      "<indexid>%s</indexid>"
		      "<serverip>%s</serverip>"
		      "<programname>%s</programname>"
		      "<ddatetime>%s</ddatetime>"
		      "<srcfilename>%s</srcfilename>"
		      "<srcfiletime>%s</srcfiletime>"
		      "<srcfilesize>%s</srcfilesize>"
		      "<stdfilename>%s</stdfilename>"
		      "<stdfiletime>%s</stdfiletime>"
		      "<stdfilesize>%s</stdfilesize>"
		      "<count>%s</count><endl/>\n",
		      m_vPROCESSLOG[ii].indexid,\
		      m_vPROCESSLOG[ii].serverip,\
		      m_vPROCESSLOG[ii].programname,\
		      m_vPROCESSLOG[ii].ddatetime,\
		      m_vPROCESSLOG[ii].srcfilename,\
		      m_vPROCESSLOG[ii].srcfiletime,\
		      m_vPROCESSLOG[ii].srcfilesize,\
		      m_vPROCESSLOG[ii].stdfilename,\
		      m_vPROCESSLOG[ii].stdfiletime,\
		      m_vPROCESSLOG[ii].stdfilesize,\
		      m_vPROCESSLOG[ii].count);
  }

  ProcFile.Fprintf("</data>\n");

  ProcFile.CloseAndRename();

  return TRUE;
}

BOOL CRealMon::WriteToDbLog()
{
  if (m_vTODBLOG.size()==0) return TRUE;

  memset(strTodbFileName,0,sizeof(strTodbFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");

  if (uFileSeq++ > 100) uFileSeq =0;
  snprintf(strTodbFileName,300,"/tmp/htidc/monfile/TODBLOG_%s_%d_%d.xml",strLocalTime,getpid(),uFileSeq);

  if (TodbFile.OpenForRename(strTodbFileName,"w+") == FALSE) return FALSE;

  TodbFile.Fprintf("<data>\n");

  for (UINT ii=0;ii<m_vTODBLOG.size();ii++)
  {
    TodbFile.Fprintf(\
		      "<indexid>%s</indexid>"
		      "<serverip>%s</serverip>"
		      "<programname>%s</programname>"
		      "<ddatetime>%s</ddatetime>"
		      "<filetime>%s</filetime>"
		      "<filename>%s</filename>"
		      "<filesize>%s</filesize>"
		      "<tname>%s</tname>"
		      "<total>%s</total>"
		      "<insrows>%s</insrows>"
		      "<uptrows>%s</uptrows>"
		      "<disrows>%s</disrows><endl/>\n",
		      m_vTODBLOG[ii].indexid,\
		      m_vTODBLOG[ii].serverip,\
		      m_vTODBLOG[ii].programname,\
		      m_vTODBLOG[ii].ddatetime,\
		      m_vTODBLOG[ii].filetime,\
		      m_vTODBLOG[ii].filename,\
		      m_vTODBLOG[ii].filesize,\
		      m_vTODBLOG[ii].tname,\
		      m_vTODBLOG[ii].total,\
		      m_vTODBLOG[ii].insrows,\
		      m_vTODBLOG[ii].uptrows,\
		      m_vTODBLOG[ii].disrows);
  }
  
  TodbFile.Fprintf("</data>\n");

  TodbFile.CloseAndRename();

  return TRUE;
}

BOOL CRealMon::WriteToCalcLog()
{
  if (m_vCALCLOG.size()==0) return TRUE;

  memset(strCalcFileName,0,sizeof(strCalcFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");

  if (uFileSeq++ > 100) uFileSeq =0;
  snprintf(strCalcFileName,300,"/tmp/htidc/monfile/CALCLOG_%s_%d_%d.xml",strLocalTime,getpid(),uFileSeq);

  if (TodbFile.OpenForRename(strCalcFileName,"w+") == FALSE) return FALSE;

  TodbFile.Fprintf("<data>\n");

  for (UINT ii=0;ii<m_vCALCLOG.size();ii++)
  {
    TodbFile.Fprintf(\
		      "<indexid>%s</indexid>"
		      "<serverip>%s</serverip>"
		      "<programname>%s</programname>"
		      "<ddatetime>%s</ddatetime>"
		      "<srctname>%s</srctname>"
		      "<stdtname>%s</stdtname>"
		      "<insrows>%s</insrows>"
		      "<uptrows>%s</uptrows><endl/>\n",
		      m_vCALCLOG[ii].indexid,\
		      m_vCALCLOG[ii].serverip,\
		      m_vCALCLOG[ii].programname,\
		      m_vCALCLOG[ii].ddatetime,\
		      m_vCALCLOG[ii].srctname,\
		      m_vCALCLOG[ii].stdtname,\
		      m_vCALCLOG[ii].insrows,\
		      m_vCALCLOG[ii].uptrows);
  }
  
  TodbFile.Fprintf("</data>\n");

  TodbFile.CloseAndRename();

  return TRUE;
}

BOOL CRealMon::WriteToProActLog(const char *in_Indexid,const char *in_ProgramName,const int in_MaxTimeOut)
{
  // 如果流程编号为空，那就不生成文件。
  if (strlen(in_Indexid) == 0) return TRUE;

  // 默认是第一次
  // 如果是第一次，那就生成心跳文件，否则要10秒才写一次，避免太频繁。
  if (bIsFirstTime == FALSE && m_Timer.Elapsed() < 10) return TRUE;

  // 开始计时
  m_Timer.Beginning();

  memset(strActiFileName,0,sizeof(strActiFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");

  if (uFileSeq++ > 1000) uFileSeq =0;
  snprintf(strActiFileName,300,"/tmp/htidc/monfile/PROACTLOG_%s_%s_%s_%d.xml",in_Indexid,in_ProgramName,strLocalTime,uFileSeq);
  
  memset(&m_stPROACTLOG,0,sizeof(m_stPROACTLOG));
  
  getLocalIP(m_stPROACTLOG.serverip);

  if (ActiFile.OpenForRename(strActiFileName,"w+") == FALSE) return FALSE;

  ActiFile.Fprintf("<data>\n");

  ActiFile.Fprintf("<indexid>%s</indexid><serverip>%s</serverip><programname>%s</programname><pid>%d</pid><maxtimeout>%d</maxtimeout><latestactivetime>%s</latestactivetime><endl/>\n",in_Indexid,m_stPROACTLOG.serverip,in_ProgramName,getpid(),in_MaxTimeOut,strLocalTime);
  
  ActiFile.Fprintf("</data>\n");

  ActiFile.CloseAndRename();

  // 设置为不是第一次
  bIsFirstTime = FALSE;

  return TRUE;
}

BOOL CRealMon::WriteToTransferLog()
{
  if (m_vTRANSFERLOG.size()==0) return TRUE;

  memset(strTranFileName,0,sizeof(strTranFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");

  if (uFileSeq++ > 100) uFileSeq =0;
  snprintf(strTranFileName,300,"/tmp/htidc/monfile/TRANSFERLOG_%s_%d_%d.xml",strLocalTime,getpid(),uFileSeq);

  if (TranFile.OpenForRename(strTranFileName,"w+") == FALSE) return FALSE;

  TranFile.Fprintf("<data>\n");

  for (UINT ii=0;ii<m_vTRANSFERLOG.size();ii++)
  {
    TranFile.Fprintf(\
		     "<indexid>%s</indexid>"
		     "<serverip>%s</serverip>"
		     "<programname>%s</programname>"
		     "<transfertype>%s</transfertype>"
		     "<transfertime>%s</transfertime>"
		     "<remoteip>%s</remoteip>"
		     "<localfilename>%s</localfilename>"
		     "<remotefilename>%s</remotefilename>"
		     "<filesize>%s</filesize>"
		     "<filetime>%s</filetime><endl/>\n",
		     m_vTRANSFERLOG[ii].indexid,\
		     m_vTRANSFERLOG[ii].serverip,\
		     m_vTRANSFERLOG[ii].programname,\
		     m_vTRANSFERLOG[ii].transfertype,\
		     m_vTRANSFERLOG[ii].transfertime,\
		     m_vTRANSFERLOG[ii].remoteip,\
		     m_vTRANSFERLOG[ii].localfilename,\
		     m_vTRANSFERLOG[ii].remotefilename,\
		     m_vTRANSFERLOG[ii].filesize,\
		     m_vTRANSFERLOG[ii].filetime);
  }
  
  TranFile.Fprintf("</data>\n");

  TranFile.CloseAndRename();

  return TRUE;
}
