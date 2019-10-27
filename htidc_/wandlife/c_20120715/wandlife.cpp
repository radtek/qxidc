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
    m_logfile->Write("delete T_ALERTINFO failed.\n%s\n",stmtdel.cda.message);
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

/*
  // 再获取
  m_vALERTINFO.clear();
  stmtsel.execute();
  while (TRUE)
  {
    memset(&m_stALERTINFO,0,sizeof(struct st_ALERTINFO));

    if (stmtsel.next() != 0) break;

    m_vALERTINFO.push_back(m_stALERTINFO);
  }

  // 再删除
  stmtdel.execute();

  // 再插入
  for (UINT ii=0;ii<m_vALERTINFO.size();ii++)
  {
    memcpy(&m_stALERTINFO,&m_vALERTINFO[ii],sizeof(struct st_ALERTINFO));

    // 就算某一行出错，也不可退出
    if (stmtins.execute() != 0)
    {
      m_logfile->Write("insert into T_ALERTINFO failed.\n%s\n",stmtins.cda.message);
    }
  }
*/

  m_conn->commitwork();

  return TRUE;
}

