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

  // ������������λ
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

  // �������ĳЩ�ֶδ�����ʾ�����ѽ���
  pos=strstr(m_stALERTINFO.description,"������"); if (pos>0) pos[6]=0;
  pos=strstr(m_stALERTINFO.description,"������"); if (pos>0) pos[6]=0;
  pos=strstr(m_stALERTINFO.description,"����.");  if (pos>0) pos[5]=0;
  pos=strstr(m_stALERTINFO.description,"����!");  if (pos>0) pos[5]=0;
  pos=strstr(m_stALERTINFO.description,"������ʩ��");  if (pos>0) pos[10]=0;
  pos=strstr(m_stALERTINFO.description,"������ʩ��");  if (pos>0) pos[10]=0;
  pos=strstr(m_stALERTINFO.description,"������ʩ.");   if (pos>0) pos[9]=0;
  pos=strstr(m_stALERTINFO.description,"������ʩ!");   if (pos>0) pos[9]=0;

  pos=strstr(m_stALERTINFO.description,"������ʩ��");   if (pos>0) pos[0]=0;
  pos=strstr(m_stALERTINFO.description,"������ʩ:");    if (pos>0) pos[0]=0;
  pos=strstr(m_stALERTINFO.description,"����ָ�ϣ�");   if (pos>0) pos[0]=0;
  pos=strstr(m_stALERTINFO.description,"����ָ��:");    if (pos>0) pos[0]=0;

  UpdateStr(m_stALERTINFO.description,"??","-");
  UpdateStr(m_stALERTINFO.description,"?","-");
  UpdateStr(m_stALERTINFO.description,"����","-");
  UpdateStr(m_stALERTINFO.description,"��","-");
 
  // ���������е��ر��ַ������¶ȷ�Χ֮����Ǹ��ر����
  char strtemp[5];
  memset(strtemp,0,sizeof(strtemp));
  strtemp[0]=-127;
  strtemp[1]=57;
  strtemp[2]=-92;
  strtemp[3]=49;
  UpdateStr(m_stALERTINFO.description,strtemp,"-");
  
  /*
  if (strncmp(m_stALERTINFO.description,"����������̨7��14��",19)==0)
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

  // ��ɾ��
  if (stmtdel.execute() != 0)
  {
    m_logfile->Write("delete T_ALERTINFO failed.\n%s\n",stmtdel.cda.message);
  }

  // �ٲ���
  for (UINT ii=0;ii<m_vALERTINFO.size();ii++)
  {
    memcpy(&m_stALERTINFO,&m_vALERTINFO[ii],sizeof(struct st_ALERTINFO));

    m_logfile->WriteEx("title=%s\n",m_stALERTINFO.title);
    m_logfile->WriteEx("descr=%s\n",m_stALERTINFO.description);

    // ����ĳһ�г���Ҳ�����˳�
    if (stmtins.execute() != 0)
    {
      m_logfile->Write("insert into T_ALERTINFO failed.\n%s\n",stmtins.cda.message);
    }
  }

/*
  // �ٻ�ȡ
  m_vALERTINFO.clear();
  stmtsel.execute();
  while (TRUE)
  {
    memset(&m_stALERTINFO,0,sizeof(struct st_ALERTINFO));

    if (stmtsel.next() != 0) break;

    m_vALERTINFO.push_back(m_stALERTINFO);
  }

  // ��ɾ��
  stmtdel.execute();

  // �ٲ���
  for (UINT ii=0;ii<m_vALERTINFO.size();ii++)
  {
    memcpy(&m_stALERTINFO,&m_vALERTINFO[ii],sizeof(struct st_ALERTINFO));

    // ����ĳһ�г���Ҳ�����˳�
    if (stmtins.execute() != 0)
    {
      m_logfile->Write("insert into T_ALERTINFO failed.\n%s\n",stmtins.cda.message);
    }
  }
*/

  m_conn->commitwork();

  return TRUE;
}

