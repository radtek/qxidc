/*
 *  ���������ڴ���ȫ������վ��۲�ķ������ݣ������浽���ݿ��T_SURFDATA���С�
 *  ��psurfdata.cpp��ͬ��������ֻ֧��xml��ʽ��
*/
#include "_public.h"
#include "_ooci.h"
#include "_shqx.h"

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
    printf("��psurfdata.cpp��ͬ��������ֻ֧��xml��ʽ��\n");
    printf("/htidc/shqx/bin/psurfdata1 �����ļ���ŵ�Ŀ¼ ��־�ļ��� ���ݿ����Ӳ��� ��������ʱ����\n");
    printf("���磺/htidc/shqx/bin/psurfdata1 /data/shqx/sdata/surfdata /log/shqx/psurfdata1.log shqx/pwdidc@snorcl11g_198 10\n");
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

    // ɨ�������ļ���ŵ�Ŀ¼��ƥ��"SURF_ZH_*.xml"
    if (Dir.OpenDir(argv[1],"SURF_ZH_*.xml",1000,true,true)==false)
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

  CSURFDATA SURFDATA(&conn,&logfile);

  // ��ȡ�ļ��е�ÿһ�м�¼
  // д�����ݿ�ı���
  char strBuffer[301];

  while (true)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    if (File.FFGETS(strBuffer,300,"<endl/>")==false) break;

logfile.Write("str=%s=\n",strBuffer);

    // logfile.Write("%s\n",strBuffer);
    
    if (SURFDATA.SplitBuffer1(strBuffer)==false) { logfile.Write("%s\n",strBuffer); continue; }

    long rc=SURFDATA.InsertTable();

    // ֻҪ�������ݿ�session�Ĵ��󣬳���ͼ�����
    if ( (rc>=3113) && (rc<=3115) ) return false;

    if (rc != 0) { logfile.Write("%s\n",strBuffer); continue; }
  }

  // �ύ����
  conn.commit();

  // �ر��ļ�ָ�룬��ɾ���ļ�
  File.CloseAndRemove();

  logfile.WriteEx("�ɹ�(total=%d,insert=%d,update=%d,invalid=%d)��\n",SURFDATA.totalcount,SURFDATA.insertcount,SURFDATA.updatecount,SURFDATA.invalidcount);

  return true;
}


