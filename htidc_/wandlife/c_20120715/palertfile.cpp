#include "_public.h"
#include "_oracle.h"
#include "wandlife.h"

void CallQuit(int sig);

connection conn;
CLogFile   logfile;
CALERTINFO ALERTINFO;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:/htidc/wandlife/bin/palertfile logfile username/passwd@tnsname alertfilename\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/wandlife/bin/palertfile /log/wandlife/palertfile.log wandlife/pwdidc /qxdata/wandlife/sdata/alert/alert.xml\n\n");

    return -1;
  }
 
  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹ�˽���
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); 
  signal(SIGTERM,CallQuit);   // ��ctl+c
  signal(SIGINT,CallQuit);    // kill �� killall 

  CProgramActive ProgramActive;

  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("palertfile");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"palertfile",180);

  // Դ�ļ�Ϊutf-8���룬ת��Ϊgb18030
  char strgbalertfile[301],strcmd[1024];
  memset(strgbalertfile,0,sizeof(strgbalertfile));
  strcpy(strgbalertfile,argv[3]);
  UpdateStr(strgbalertfile,"alert.xml","alert_gb18030.xml");
  memset(strcmd,0,sizeof(strcmd));
  snprintf(strcmd,1000,"iconv -f utf-8 -t gb18030 %s>%s",argv[3],strgbalertfile);
  system(strcmd);
  
  // ��Ҫ������ļ������ļ������ݶ���������
  CIniFile IniFile;
  if (IniFile.LoadFile(strgbalertfile) == FALSE) return 0;

  if (conn.connecttodb(argv[2])!=0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); return -1;
  }

  logfile.Write("��ʼ����%s.\n",strgbalertfile);

  ALERTINFO.BindConnLog(&conn,&logfile);

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(IniFile.m_XMLBuffer,"</item>");

  char *itempos=0;
  char strBuffer[2048];
  for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
  {
    itempos=0;
    memset(strBuffer,0,sizeof(strBuffer));

    CmdStr.GetValue(ii,strBuffer,2000);

    itempos=strstr(strBuffer,"<item>");

    if (itempos==0) continue;

    ALERTINFO.SplitBuffer(itempos);
  }

  ALERTINFO.UptTable();

  logfile.Write("�������.\n\n");

  REMOVE(argv[3]);

  REMOVE(strgbalertfile);
  
  return 0;
}

void CallQuit(int sig)
{
  signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
 
  logfile.Write("palertfile exit.\n");
 
  exit(0);
}

