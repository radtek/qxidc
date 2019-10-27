#include "wandlife.h"

CALERTINFO::CALERTINFO()
{
  m_conn=0;
  m_logfile=0;

  memset(&m_stALERTINFO,0,sizeof(struct st_ALERTINFO));

  m_vALERTINFO.clear();
}

CALERTINFO::~CALERTINFO()
{
  m_vALERTINFO.clear();
}

void CALERTINFO::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;

  m_logfile=in_logfile;
}

BOOL CALERTINFO::SplitBuffer(char *strBuffer)
{
  memset(&m_stALERTINFO,0,sizeof(struct st_ALERTINFO));
  
  if (GetXMLBuffer(strBuffer,"title",m_stALERTINFO.title,100) == FALSE) return FALSE;
  if (GetXMLBuffer(strBuffer,"link",m_stALERTINFO.link,300) == FALSE) return FALSE;
  if (GetXMLBuffer(strBuffer,"atype",m_stALERTINFO.atype,20) == FALSE) return FALSE;
  if (GetXMLBuffer(strBuffer,"alevel",m_stALERTINFO.alevel,20) == FALSE) return FALSE;
  if (GetXMLBuffer(strBuffer,"astatus",m_stALERTINFO.astatus,20) == FALSE) return FALSE;
  if (GetXMLBuffer(strBuffer,"description",m_stALERTINFO.description,2000) == FALSE) return FALSE;
  if (GetXMLBuffer(strBuffer,"pubDate",m_stALERTINFO.pubdate,19) == FALSE) return FALSE;

  // 解析出发布单位
  char *start,*end;
  start=end=0;

  start=strstr(m_stALERTINFO.link,"file=101");
  if (start==0) return FALSE;
  end=strstr(start,"-");
  if (end==0) return FALSE;

  if (end-start-8>20) return FALSE;

  strncpy(m_stALERTINFO.puborg,start+8,end-start-8);

  UpdateStr(m_stALERTINFO.description,"<![CDATA[","");
  UpdateStr(m_stALERTINFO.description,"]]>","");

  char *pos=0;

  // 如果出现某些字段串，表示内容已结束
  pos=strstr(m_stALERTINFO.description,"防范。"); if (pos>0) pos[6]=0;
  pos=strstr(m_stALERTINFO.description,"防范！"); if (pos>0) pos[6]=0;
  pos=strstr(m_stALERTINFO.description,"防范.");  if (pos>0) pos[5]=0;
  pos=strstr(m_stALERTINFO.description,"防范!");  if (pos>0) pos[5]=0;
  pos=strstr(m_stALERTINFO.description,"防护措施。");  if (pos>0) pos[10]=0;
  pos=strstr(m_stALERTINFO.description,"防护措施！");  if (pos>0) pos[10]=0;
  pos=strstr(m_stALERTINFO.description,"防护措施.");   if (pos>0) pos[9]=0;
  pos=strstr(m_stALERTINFO.description,"防护措施!");   if (pos>0) pos[9]=0;

  pos=strstr(m_stALERTINFO.description,"防御措施：");   if (pos>0) pos[0]=0;
  pos=strstr(m_stALERTINFO.description,"防御措施:");    if (pos>0) pos[0]=0;
  pos=strstr(m_stALERTINFO.description,"防御指南：");   if (pos>0) pos[0]=0;
  pos=strstr(m_stALERTINFO.description,"防御指南:");    if (pos>0) pos[0]=0;

  UpdateStr(m_stALERTINFO.description,"??","-");
  UpdateStr(m_stALERTINFO.description,"?","-");
  UpdateStr(m_stALERTINFO.description,"？？","-");
  UpdateStr(m_stALERTINFO.description,"？","-");
 
  // 处理内容中的特别字符，即温度范围之间的那个特别符号
  char strtemp[5];
  memset(strtemp,0,sizeof(strtemp));
  strtemp[0]=-127;
  strtemp[1]=57;
  strtemp[2]=-92;
  strtemp[3]=49;
  UpdateStr(m_stALERTINFO.description,strtemp,"-");
  
  /*
  if (strncmp(m_stALERTINFO.description,"三明市气象台7月14日",19)==0)
  {
    char *pos=0;
    pos=strstr(m_stALERTINFO.description,"36");
  }
  */

  m_vALERTINFO.push_back(m_stALERTINFO);

  return TRUE;
}


BOOL CALERTINFO::UptTable()
{
  sqlstatement stmtdel,stmtins,stmtsel;
  stmtdel.connect(m_conn);
  stmtdel.prepare("delete from T_ALERTINFO");

  stmtsel.connect(m_conn);
  stmtsel.prepare("\
    select puborg,title,link,atype,alevel,astatus,description,pubstd,prem,to_char(pubdate,'yyyy-mm-dd hh24:mi:ss') from T_ALERTINFO");
  stmtsel.bindin( 1,m_stALERTINFO.puborg,20);
  stmtsel.bindin( 2,m_stALERTINFO.title,100);
  stmtsel.bindin( 3,m_stALERTINFO.link,300);
  stmtsel.bindin( 4,m_stALERTINFO.atype,20);
  stmtsel.bindin( 5,m_stALERTINFO.alevel,20);
  stmtsel.bindin( 6,m_stALERTINFO.astatus,20);
  stmtsel.bindin( 7,m_stALERTINFO.description,2000);
  stmtsel.bindin( 8,m_stALERTINFO.pubstd,2000);
  stmtsel.bindin( 9,m_stALERTINFO.prem,2000);
  stmtsel.bindin(10,m_stALERTINFO.pubdate,20);

  stmtins.connect(m_conn);
  stmtins.prepare("\
    insert into T_ALERTINFO(puborg,title,link,atype,alevel,astatus,description,pubstd,prem,pubdate)\
                     values(:1,:2,:3,:4,:5,:6,:7,:8,:9,to_date(:10,'yyyy-mm-dd hh24:mi:ss'))");
  stmtins.bindin( 1,m_stALERTINFO.puborg,20);
  stmtins.bindin( 2,m_stALERTINFO.title,100);
  stmtins.bindin( 3,m_stALERTINFO.link,300);
  stmtins.bindin( 4,m_stALERTINFO.atype,20);
  stmtins.bindin( 5,m_stALERTINFO.alevel,20);
  stmtins.bindin( 6,m_stALERTINFO.astatus,20);
  stmtins.bindin( 7,m_stALERTINFO.description,2000);
  stmtins.bindin( 8,m_stALERTINFO.pubstd,2000);
  stmtins.bindin( 9,m_stALERTINFO.prem,2000);
  stmtins.bindin(10,m_stALERTINFO.pubdate,20);

  // 先删除
  if (stmtdel.execute() != 0)
  {
    m_logfile->Write("delete T_ALERTINFO failed.\n%s\n",stmtdel.cda.message); return FALSE;
  }

  // 再插入
  for (UINT ii=0;ii<m_vALERTINFO.size();ii++)
  {
    memcpy(&m_stALERTINFO,&m_vALERTINFO[ii],sizeof(struct st_ALERTINFO));

    m_logfile->WriteEx("title=%s\n",m_stALERTINFO.title);
    m_logfile->WriteEx("descr=%s\n",m_stALERTINFO.description);

    // 就算某一行出错，也不可退出
    if (stmtins.execute() != 0)
    {
      m_logfile->Write("insert into T_ALERTINFO failed.\n%s\n",stmtins.cda.message);
    }
  }

  m_conn->commitwork();

  return TRUE;
}

/*
COBTCODE::COBTCODE()
{
  m_conn=0;
  m_logfile=0;

  memset(&m_stOBTCODE,0,sizeof(struct st_OBTCODE));

  m_vOBTCODE.clear();
}


COBTCODE::~COBTCODE()
{
}

void COBTCODE::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;

  m_logfile=in_logfile;
}

BOOL COBTCODE::LoadObtCode()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select obtid,obtname,obtname_old,obtlevel,provname,lon,lat,height,w3a,rsts,memo from T_OBTCODE");
  stmt.bindout( 1, m_stOBTCODE.obtid,20);
  stmt.bindout( 2, m_stOBTCODE.obtname,50);
  stmt.bindout( 3, m_stOBTCODE.obtname_old,50);
  stmt.bindout( 4, m_stOBTCODE.obtlevel,20);
  stmt.bindout( 5, m_stOBTCODE.provname,20);
  stmt.bindout( 6,&m_stOBTCODE.lon);
  stmt.bindout( 7,&m_stOBTCODE.lat);
  stmt.bindout( 8,&m_stOBTCODE.height);
  stmt.bindout( 9, m_stOBTCODE.w3a,30);
  stmt.bindout(10,&m_stOBTCODE.rsts);
  stmt.bindout(11, m_stOBTCODE.memo,300);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("select T_OBTCODE failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  m_vOBTCODE.clear();

  while (TRUE)
  {
    memset(&m_stOBTCODE,0,sizeof(struct st_OBTCODE));

    if (stmt.next() != 0) break;

    m_vOBTCODE.push_back(m_stOBTCODE);
  }

  return TRUE;
}

CZHCITYCODE::CZHCITYCODE()
{
  m_conn=0;
  m_logfile=0;

  memset(&m_stZHCITYCODE,0,sizeof(struct st_ZHCITYCODE));

  m_vZHCITYCODE.clear();
}


CZHCITYCODE::~CZHCITYCODE()
{
}

void CZHCITYCODE::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;

  m_logfile=in_logfile;
}

BOOL CZHCITYCODE::LoadCityCode()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    select cityid,cityname,cityname_old,obtid,obtidsk,obtidyb,provname,memo,orderby,rsts\
      from T_ZHCITYCODE where cityid in (select cityid from T_INDEXINFO where pubdate<sysdate-3/24 or pubdate is null or zwx is null or ssd is null or cy is null or ly is null or yd is null or ls is null or ys is null or xc is null)");
  stmt.bindout( 1, m_stZHCITYCODE.cityid,10);
  stmt.bindout( 2, m_stZHCITYCODE.cityname,20);
  stmt.bindout( 3, m_stZHCITYCODE.cityname_old,20);
  stmt.bindout( 4, m_stZHCITYCODE.obtid,20);
  stmt.bindout( 5, m_stZHCITYCODE.obtidsk,20);
  stmt.bindout( 6, m_stZHCITYCODE.obtidyb,20);
  stmt.bindout( 7, m_stZHCITYCODE.provname,20);
  stmt.bindout( 8, m_stZHCITYCODE.memo,300);
  stmt.bindout( 9,&m_stZHCITYCODE.orderby);
  stmt.bindout(10,&m_stZHCITYCODE.rsts);

  if (stmt.execute() != 0)
  {
    m_logfile->Write("select T_ZHCITYCODE failed.\n%s\n",stmt.cda.message); return FALSE;
  }

  m_vZHCITYCODE.clear();

  while (TRUE)
  {
    memset(&m_stZHCITYCODE,0,sizeof(struct st_ZHCITYCODE));

    if (stmt.next() != 0) break;

    m_vZHCITYCODE.push_back(m_stZHCITYCODE);
  }

  return TRUE;
}
*/

CINDEXINFO::CINDEXINFO()
{
  m_conn=0;
  m_logfile=0;

  memset(&m_stINDEXINFO,0,sizeof(struct st_INDEXINFO));

  m_vINDEXINFO.clear();
}


CINDEXINFO::~CINDEXINFO()
{
}

void CINDEXINFO::BindConnLog(connection *in_conn,CLogFile *in_logfile)
{
  m_conn=in_conn;

  m_logfile=in_logfile;
}

BOOL CINDEXINFO::UptIndexInfo()
{
  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("\
    update T_INDEXINFO\
       set\
   pubdate=to_date(:1,'yyyy-mm-dd hh24:mi:ss'),\
   jrrc   =:2,\
   jrrl   =:3,\
   mrrc   =:4,\
   mrrl   =:5,\
   gm     =:6,\
   wrks   =:7,\
   zs     =:8,\
   zwx    =:9,\
   cy     =:10,\
   ssd    =:11,\
   hz     =:12,\
   mf     =:13,\
   xc     =:14,\
   lk     =:15,\
   jt     =:16,\
   ly     =:17,\
   yd     =:18,\
   cl     =:19,\
   dy     =:20,\
   hc     =:21,\
   yh     =:22,\
   gj     =:23,\
   ls     =:24,\
   ys     =:25,\
   ganmao =:26,\
   urlstationname=:27,\
   urlcity=:28,\
   urlprovince=:29\
   where cityid=:30");
  stmt.bindin( 1, m_stINDEXINFO.pubdate,19);
  stmt.bindin( 2, m_stINDEXINFO.jrrc,5);
  stmt.bindin( 3, m_stINDEXINFO.jrrl,5);
  stmt.bindin( 4, m_stINDEXINFO.mrrc,5);
  stmt.bindin( 5, m_stINDEXINFO.mrrl,5);
  stmt.bindin( 6, m_stINDEXINFO.gm,1000);
  stmt.bindin( 7, m_stINDEXINFO.wrks,1000);
  stmt.bindin( 8, m_stINDEXINFO.zs,1000);
  stmt.bindin( 9, m_stINDEXINFO.zwx,1000);
  stmt.bindin(10, m_stINDEXINFO.cy,1000);
  stmt.bindin(11, m_stINDEXINFO.ssd,1000);
  stmt.bindin(12, m_stINDEXINFO.hz,1000);
  stmt.bindin(13, m_stINDEXINFO.mf,1000);
  stmt.bindin(14, m_stINDEXINFO.xc,1000);
  stmt.bindin(15, m_stINDEXINFO.lk,1000);
  stmt.bindin(16, m_stINDEXINFO.jt,1000);
  stmt.bindin(17, m_stINDEXINFO.ly,1000);
  stmt.bindin(18, m_stINDEXINFO.yd,1000);
  stmt.bindin(19, m_stINDEXINFO.cl,1000);
  stmt.bindin(20, m_stINDEXINFO.dy,1000);
  stmt.bindin(21, m_stINDEXINFO.hc,1000);
  stmt.bindin(22, m_stINDEXINFO.yh,1000);
  stmt.bindin(23, m_stINDEXINFO.gj,1000);
  stmt.bindin(24, m_stINDEXINFO.ls,1000);
  stmt.bindin(25, m_stINDEXINFO.ys,1000);
  stmt.bindin(26, m_stINDEXINFO.ganmao,1000);
  stmt.bindin(27, m_stINDEXINFO.urlstationname,20);
  stmt.bindin(28, m_stINDEXINFO.urlcity,20);
  stmt.bindin(29, m_stINDEXINFO.urlprovince,20);
  stmt.bindin(30, m_stINDEXINFO.cityid,20);

  memset(m_stINDEXINFO.pubdate,0,sizeof(m_stINDEXINFO.pubdate));
  LocalTime(m_stINDEXINFO.pubdate,"yyyy-mm-dd hh24:mi:ss");
   
  if (stmt.execute() != 0)
  {
    m_logfile->Write("update T_INDEXINFO failed.\n%s\n",stmt.cda.message); 

    // 只有当数据库会话断开时才返回失败，其它的都认为成功
    if (stmt.cda.rc==3113) return FALSE;
  }

  m_conn->commitwork();

  return TRUE;
}

// 备份T_INDEXINFO表
BOOL CINDEXINFO::BakIndexInfo()
{
  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  strLocalTime[10]=0;

  sqlstatement stmt;
  stmt.connect(m_conn);
  stmt.prepare("create table T_INDEXINFO_%s as select * from T_INDEXINFO",strLocalTime);
  stmt.execute();

  return TRUE;
}
