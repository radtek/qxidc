/*
 *  ���������ڴ���ȫ������վ��۲�ķ������ݣ������浽���ݿ��T_SURFDATA���С�
 *  �����û���Ż�ǰ�ĳ���
*/
#include "_public.h"
#include "_ooci.h"

// ȫ������վ����ӹ۲����ݽṹ
struct st_surfdata
{
  char obtid[11];      // վ�����
  char ddatetime[21];  // ����ʱ�䣺��ʽyyyy-mm-dd hh:mi:ss��
  int  t;              // ���£���λ��0.1���϶�
  int  p;              // ��ѹ��0.1����
  int  u;              // ���ʪ�ȣ�0-100֮���ֵ��
  int  wd;             // ����0-360֮���ֵ��
  int  wf;             // ���٣���λ0.1m/s
  int  r;              // ��������0.1mm
  int  vis;            // �ܼ��ȣ�0.1��
};

CLogFile logfile;

CDir Dir;

// ���������ļ�
bool _psurfdata();

connection conn;

void EXIT(int sig);

int main(int argc,char *argv[])
{
  if (argc!=5)
  {
    printf("\n���������ڴ���ȫ������վ��۲�ķ������ݣ������浽���ݿ��T_SURFDATA���С�\n");
    printf("/htidc/shqx/bin/psurfdata �����ļ���ŵ�Ŀ¼ ��־�ļ��� ���ݿ����Ӳ��� ��������ʱ����\n");
    printf("���磺/htidc/shqx/bin/psurfdata /data/shqx/sdata/surfdata /log/shqx/psurfdata.log shqx/pwdidc@snorcl11g_198 10\n");
    return -1;
  }

  // �ر�ȫ�����źź��������
  CloseIOAndSignal();

  // ��������˳����ź�
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[2],"a+")==false)
  {
    printf("����־�ļ�ʧ�ܣ�%s����\n",argv[2]); return -1;
  }

  logfile.Write("����������\n");

  while (true)
  {
    // logfile.Write("��ʼɨ��Ŀ¼��\n");

    // ɨ�������ļ���ŵ�Ŀ¼
    if (Dir.OpenDir(argv[1],"SURF_ZH_*.txt",1000,true,true)==false)
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",argv[1]); sleep(atoi(argv[4])); continue;
    }

    // �������Ŀ¼�е������ļ�
    while (true)
    {
      if (Dir.ReadDir()==false) break;
  
      if (conn.m_state==0)
      {
        if (conn.connecttodb(argv[3],"Simplified Chinese_China.ZHS16GBK")!=0)
        {
          logfile.Write("connect database(%s) failed.\n%s\n",argv[3],conn.m_cda.message); break;
        }
        // logfile.Write("�������ݿ�ɹ���\n");
      }
  
      logfile.Write("��ʼ�����ļ�%s...",Dir.m_FileName);
  
      // ���������ļ�
      if (_psurfdata()==false) 
      {
        logfile.WriteEx("ʧ�ܡ�\n"); break;
      }
      
      logfile.WriteEx("�ɹ���\n");
    }

    // �Ͽ������ݿ������
    if (conn.m_state==1) conn.disconnect(); 

    sleep(atoi(argv[4]));
  }

  return 0;
}

void EXIT(int sig)
{
  logfile.Write("�����˳���sig=%d\n\n",sig);

  exit(0);
}
     

// ���������ļ�
bool _psurfdata()
{
  // ���ļ�
  CFile File;

  if (File.Open(Dir.m_FullFileName,"r")==false)
  {
    logfile.Write("(File.Open(%s) failed.\n",Dir.m_FullFileName); return false;
  }

  // ��ȡ�ļ��е�ÿһ�м�¼
  // д�����ݿ�ı���
  char strBuffer[301];
  CCmdStr CmdStr;
  struct st_surfdata stsurfdata;

  int iccount=0;
  sqlstatement stmtsel(&conn);
  stmtsel.prepare("select count(*) from T_SURFDATA where obtid=:1 and ddatetime=to_date(:2,'yyyy-mm-dd hh24:mi:ss')");
  stmtsel.bindin( 1, stsurfdata.obtid,5);
  stmtsel.bindin( 2, stsurfdata.ddatetime,19);
  stmtsel.bindout(1,&iccount);

  sqlstatement stmtins(&conn);
  stmtins.prepare("insert into T_SURFDATA(obtid,ddatetime,t,p,u,wd,wf,r,vis,crttime,keyid) values(:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),:3,:4,:5,:6,:7,:8,:9,sysdate,SEQ_SURFDATA.nextval)");
  stmtins.bindin( 1, stsurfdata.obtid,5);
  stmtins.bindin( 2, stsurfdata.ddatetime,19);
  stmtins.bindin( 3,&stsurfdata.t);
  stmtins.bindin( 4,&stsurfdata.p);
  stmtins.bindin( 5,&stsurfdata.u);
  stmtins.bindin( 6,&stsurfdata.wd);
  stmtins.bindin( 7,&stsurfdata.wf);
  stmtins.bindin( 8,&stsurfdata.r);
  stmtins.bindin( 9,&stsurfdata.vis);

  while (true)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    if (File.Fgets(strBuffer,300,true)==false) break;

    // logfile.Write("%s\n",strBuffer);
    CmdStr.SplitToCmd(strBuffer,",",true);
    if (CmdStr.CmdCount()!=9)
    {
      logfile.Write("%s\n",strBuffer); continue;
    }

    memset(&stsurfdata,0,sizeof(struct st_surfdata));
    CmdStr.GetValue(0,stsurfdata.obtid,5);      // վ�����
    CmdStr.GetValue(1,stsurfdata.ddatetime,19); // ����ʱ�䣺��ʽyyyy-mm-dd hh:mi:ss��
    double dtmp=0;
    CmdStr.GetValue(2,&dtmp); stsurfdata.t=(int)(dtmp*10);  // ���£���λ��0.1���϶�
    CmdStr.GetValue(3,&dtmp); stsurfdata.p=(int)(dtmp*10);  // ��ѹ��0.1����
    CmdStr.GetValue(4,&stsurfdata.u);  // ���ʪ�ȣ�0-100֮���ֵ��
    CmdStr.GetValue(5,&stsurfdata.wd); // ����0-360֮���ֵ��
    CmdStr.GetValue(6,&dtmp); stsurfdata.wf=(int)(dtmp*10);  // ���٣���λ0.1m/s
    CmdStr.GetValue(7,&dtmp); stsurfdata.r=(int)(dtmp*10);   // ��������0.1mm
    CmdStr.GetValue(8,&dtmp); stsurfdata.vis=(int)(dtmp*10); // �ܼ��ȣ�0.1��

    if (stmtsel.execute() != 0)
    {
      logfile.Write("stmtsel.execute() failed.\n%s\n%s\n",stmtsel.m_sql,stmtsel.m_cda.message); 
      if ( (stmtsel.m_cda.rc>=3113) && (stmtsel.m_cda.rc<=3115) ) return false;
      continue;
    }

    iccount=0;
    stmtsel.next();

    if (iccount>0) continue;

    // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
    if (stmtins.execute() != 0)
    {
      if (stmtins.m_cda.rc!=1)
      {
        logfile.Write("%s\n",strBuffer);
        logfile.Write("stmtins.execute() failed.\n%s\n%s\n",stmtins.m_sql,stmtins.m_cda.message); 
        if ( (stmtsel.m_cda.rc>=3113) && (stmtsel.m_cda.rc<=3115) ) return false;
      }
    }
  }

  // �ύ����
  conn.commit();

  // �ر��ļ�ָ�룬��ɾ���ļ�
  File.CloseAndRemove();

  return true;
}


