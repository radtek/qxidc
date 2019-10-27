#include "_shqx.h"

CSURFDATA::CSURFDATA(connection *conn,CLogFile *logfile)
{
  initdata();

  m_conn=conn; m_logfile=logfile;
}

void CSURFDATA::initdata()
{
  totalcount=insertcount=updatecount=invalidcount=0;
  m_conn=0; m_logfile=0;
  memset(&m_stsurfdata,0,sizeof(struct st_surfdata));
}

CSURFDATA::~CSURFDATA()
{
}

// ���ö��ŷָ��ļ�¼��ֵ�m_stsurfdata�ṹ�С�
bool CSURFDATA::SplitBuffer(const char *strBuffer)
{
  totalcount++;

  memset(&m_stsurfdata,0,sizeof(struct st_surfdata));

  CCmdStr CmdStr;

  CmdStr.SplitToCmd(strBuffer,",",true);
  if (CmdStr.CmdCount()!=9) { invalidcount++; return false; }

  CmdStr.GetValue(0,m_stsurfdata.obtid,5);      // վ�����
  CmdStr.GetValue(1,m_stsurfdata.ddatetime,19); // ����ʱ�䣺��ʽyyyy-mm-dd hh:mi:ss��
  double dtmp=0;
  CmdStr.GetValue(2,&dtmp); m_stsurfdata.t=(int)(dtmp*10);  // ���£���λ��0.1���϶�
  CmdStr.GetValue(3,&dtmp); m_stsurfdata.p=(int)(dtmp*10);  // ��ѹ��0.1����
  CmdStr.GetValue(4,&m_stsurfdata.u);  // ���ʪ�ȣ�0-100֮���ֵ��
  CmdStr.GetValue(5,&m_stsurfdata.wd); // ����0-360֮���ֵ��
  CmdStr.GetValue(6,&dtmp); m_stsurfdata.wf=(int)(dtmp*10);  // ���٣���λ0.1m/s
  CmdStr.GetValue(7,&dtmp); m_stsurfdata.r=(int)(dtmp*10);   // ��������0.1mm
  CmdStr.GetValue(8,&dtmp); m_stsurfdata.vis=(int)(dtmp*10); // �ܼ��ȣ�0.1��

  return true;
}

// ��xml��ʽ�ļ�¼��ֵ�m_stsurfdata�ṹ�С�
bool CSURFDATA::SplitBuffer1(const char *strBuffer)
{
  totalcount++;

  memset(&m_stsurfdata,0,sizeof(struct st_surfdata));

  GetXMLBuffer(strBuffer,"obtid",m_stsurfdata.obtid,5);      // վ�����
  GetXMLBuffer(strBuffer,"ddatetime",m_stsurfdata.ddatetime,19); // ����ʱ�䣺��ʽyyyy-mm-dd hh:mi:ss��
  double dtmp=0;
  GetXMLBuffer(strBuffer,"t",&dtmp); m_stsurfdata.t=(int)(dtmp*10);  // ���£���λ��0.1���϶�
  GetXMLBuffer(strBuffer,"p",&dtmp); m_stsurfdata.p=(int)(dtmp*10);  // ��ѹ��0.1����
  GetXMLBuffer(strBuffer,"u",&m_stsurfdata.u);  // ���ʪ�ȣ�0-100֮���ֵ��
  GetXMLBuffer(strBuffer,"wd",&m_stsurfdata.wd);  // ����0-360֮���ֵ��
  GetXMLBuffer(strBuffer,"wf",&dtmp); m_stsurfdata.wf=(int)(dtmp*10);  // ���٣���λ0.1m/s
  GetXMLBuffer(strBuffer,"r",&dtmp); m_stsurfdata.r=(int)(dtmp*10);   // ��������0.1mm
  GetXMLBuffer(strBuffer,"vis",&dtmp);  m_stsurfdata.vis=(int)(dtmp*10); // �ܼ��ȣ�0.1��

  return true;
}

// ��m_stsurfdata�ṹ�е�ֵ���µ�T_SURFDATA���С�
long CSURFDATA::InsertTable()
{
  if (stmtsel.m_state==0)
  {
    stmtsel.connect(m_conn);
    stmtsel.prepare("select count(*) from T_SURFDATA where obtid=:1 and ddatetime=to_date(:2,'yyyy-mm-dd hh24:mi:ss')");
    stmtsel.bindin( 1, m_stsurfdata.obtid,5);
    stmtsel.bindin( 2, m_stsurfdata.ddatetime,19);
    stmtsel.bindout(1,&iccount);
  }

  if (stmtins.m_state==0)
  {
    stmtins.connect(m_conn);
    stmtins.prepare("insert into T_SURFDATA(obtid,ddatetime,t,p,u,wd,wf,r,vis,crttime,keyid) values(:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),:3,:4,:5,:6,:7,:8,:9,sysdate,SEQ_SURFDATA.nextval)");
    stmtins.bindin( 1, m_stsurfdata.obtid,5);
    stmtins.bindin( 2, m_stsurfdata.ddatetime,19);
    stmtins.bindin( 3,&m_stsurfdata.t);
    stmtins.bindin( 4,&m_stsurfdata.p);
    stmtins.bindin( 5,&m_stsurfdata.u);
    stmtins.bindin( 6,&m_stsurfdata.wd);
    stmtins.bindin( 7,&m_stsurfdata.wf);
    stmtins.bindin( 8,&m_stsurfdata.r);
    stmtins.bindin( 9,&m_stsurfdata.vis);
  }

  if (stmtupt.m_state==0)
  {
    stmtupt.connect(m_conn);
    stmtupt.prepare("update T_SURFDATA set t=:1,p=:2,u=:3,wd=:4,wf=:5,r=:6,vis=:7 where obtid=:8 and ddatetime=to_date(:2,'yyyy-mm-dd hh24:mi:ss')");
    stmtupt.bindin( 1,&m_stsurfdata.t);
    stmtupt.bindin( 2,&m_stsurfdata.p);
    stmtupt.bindin( 3,&m_stsurfdata.u);
    stmtupt.bindin( 4,&m_stsurfdata.wd);
    stmtupt.bindin( 5,&m_stsurfdata.wf);
    stmtupt.bindin( 6,&m_stsurfdata.r);
    stmtupt.bindin( 7,&m_stsurfdata.vis);
    stmtupt.bindin( 8, m_stsurfdata.obtid,5);
    stmtupt.bindin( 9, m_stsurfdata.ddatetime,19);
  }

  if (stmtsel.execute() != 0)
  {
    invalidcount++; 
    m_logfile->Write("stmtsel.execute() failed.\n%s\n%s\n",stmtsel.m_sql,stmtsel.m_cda.message); 
    return stmtsel.m_cda.rc;
  }

  iccount=0;
  stmtsel.next();

  if (iccount>0) 
  {
    // ִ�и��µ�SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
    if (stmtupt.execute() != 0)
    {
      invalidcount++; 
      m_logfile->Write("stmtupt.execute() failed.\n%s\n%s\n",stmtupt.m_sql,stmtupt.m_cda.message);
      return stmtupt.m_cda.rc;
    }
    updatecount++;
  }
  else
  {
    // ִ�в����SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
    if (stmtins.execute() != 0)
    {
      invalidcount++; 
      m_logfile->Write("stmtins.execute() failed.\n%s\n%s\n",stmtins.m_sql,stmtins.m_cda.message);
      return stmtins.m_cda.rc;
    }
    insertcount++;
  }

  return 0;
}

CSIGNALLOG::CSIGNALLOG(connection *conn,CLogFile *logfile)
{
  initdata();

  m_conn=conn; m_logfile=logfile;
}

void CSIGNALLOG::initdata()
{
  totalcount=insertcount=updatecount=invalidcount=0;
  m_conn=0; m_logfile=0;
  memset(&m_stsignallog,0,sizeof(struct st_signallog));
  vsignallog.clear();
}

CSIGNALLOG::~CSIGNALLOG()
{
}

// ���ö��ŷָ��ļ�¼��ֵ�m_stsignallog�ṹ�С�
bool CSIGNALLOG::SplitBuffer(const char *strBuffer)
{
  vsignallog.clear();
  memset(&m_stsignallog,0,sizeof(struct st_signallog));

  CCmdStr CmdStr;

  CmdStr.SplitToCmd(strBuffer," ",true);
  if (CmdStr.CmdCount()<3) { invalidcount++; return false; }

  CmdStr.GetValue(0,m_stsignallog.ddatetime,12); // ����ʱ�䣺��ʽyyyy-mm-dd hh:mi:ss��
  CmdStr.GetValue(1,m_stsignallog.obtid,4);      // վ�����
  char strtemp[11];
  for (int ii=3;ii<=CmdStr.CmdCount();ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    CmdStr.GetValue(ii-1,strtemp,5);
    m_stsignallog.signalname[0]=strtemp[0];
    m_stsignallog.signalcolor[0]=strtemp[1];
    vsignallog.push_back(m_stsignallog);
    totalcount++;
  }

  return true;
}

// ��m_stsignallog�ṹ�е�ֵ���µ�T_SIGNALLOG���С�
long CSIGNALLOG::InsertTable()
{
  if (stmtsel.m_state==0)
  {
    stmtsel.connect(m_conn);
    stmtsel.prepare("select count(*) from T_SIGNALLOG where obtid=:1 and ddatetime=to_date(:2,'yyyy-mm-dd hh24:mi:ss') and signalname=:3");
    stmtsel.bindin( 1, m_stsignallog.obtid,4);
    stmtsel.bindin( 2, m_stsignallog.ddatetime,19);
    stmtsel.bindin( 3, m_stsignallog.signalname,1);
    stmtsel.bindout(1,&iccount);
  }

  if (stmtins.m_state==0)
  {
    stmtins.connect(m_conn);
    stmtins.prepare("insert into T_SIGNALLOG(obtid,ddatetime,signalname,signalcolor,crttime,keyid) values(:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),:3,:4,sysdate,SEQ_SIGNALLOG.nextval)");
    stmtins.bindin( 1, m_stsignallog.obtid,4);
    stmtins.bindin( 2, m_stsignallog.ddatetime,19);
    stmtins.bindin( 3, m_stsignallog.signalname,1);
    stmtins.bindin( 4, m_stsignallog.signalcolor,1);
  }

  if (stmtupt.m_state==0)
  {
    stmtupt.connect(m_conn);
    stmtupt.prepare("update T_SIGNALLOG set signalcolor=:1 where obtid=:2 and ddatetime=to_date(:3,'yyyy-mm-dd hh24:mi:ss') and signalname=:4");
    stmtupt.bindin( 1, m_stsignallog.signalcolor,1);
    stmtupt.bindin( 2, m_stsignallog.obtid,4);
    stmtupt.bindin( 3, m_stsignallog.ddatetime,19);
    stmtupt.bindin( 4, m_stsignallog.signalname,1);
  }

  for (int ii=0;ii<vsignallog.size();ii++)
  {
    memcpy(&m_stsignallog,&vsignallog[ii],sizeof(struct st_signallog));

    if (stmtsel.execute() != 0)
    {
      invalidcount++; 
      m_logfile->Write("stmtsel.execute() failed.\n%s\n%s\n",stmtsel.m_sql,stmtsel.m_cda.message); 
      return stmtsel.m_cda.rc;
    }
  
    iccount=0;
    stmtsel.next();
  
    if (iccount>0) 
    {
      // ִ�и��µ�SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
      if (stmtupt.execute() != 0)
      {
        invalidcount++; 
        m_logfile->Write("stmtupt.execute() failed.\n%s\n%s\n",stmtupt.m_sql,stmtupt.m_cda.message);
        return stmtupt.m_cda.rc;
      }
      updatecount++;
    }
    else
    {
      // ִ�в����SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
      if (stmtins.execute() != 0)
      {
        invalidcount++; 
        m_logfile->Write("stmtins.execute() failed.\n%s\n%s\n",stmtins.m_sql,stmtins.m_cda.message);
        return stmtins.m_cda.rc;
      }
      insertcount++;
    }
  }

  return 0;
}

