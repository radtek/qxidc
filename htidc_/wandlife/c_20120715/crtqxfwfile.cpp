#include "_public.h"
#include "_oracle.h"
#include "wandlife.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

CQXFWFile QXFWFile;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf( "Usage: /htidc/qxfw/bin/crtqxfwfile logfile username/password@tnsname outpath\n" );
    printf( "Example: /htidc/htidc/bin/procctl 300 /htidc/qxfw/bin/crtqxfwfile /log/qxfw/crtqxfwfile.log qxfw/pwdidc /qxfw\n" );

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  // CloseIOAndSignal(); // ���ܵ��øú���������ZIPFiles�������ò��ܳɹ�
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("crtqxfwfile");

  logfile.Write("crtqxfwfile beginning.\n");

  // �������ݿ�֮ǰ����ʱ����Ϊ180��
  ProgramActive.SetProgramInfo(&logfile,"crtqxfwfile",180);

  // �������ݿ�
  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"crtqxfwfile",1200);

  QXFWFile.BindConnLog(&conn,&logfile);

  // ����ȫ����վ�����
  if (QXFWFile.LoadObtCode() == FALSE) CallQuit(-1);

  // ��ȡ��Ҫ��ʾ��λ��
  if (QXFWFile.LoadImpPos() == FALSE) CallQuit(-1);

  while (TRUE)
  {
    // ��m_vOBTCODE�����л�ȡһ��վ�㣬���m_stOBTCODE�ṹ��
    if (QXFWFile.FetchObtCode() == FALSE) break;

    // logfile.Write("obtid=%s\n",QXFWFile.m_stOBTCODE.obtid);

    // �������ļ�������д������
    if (QXFWFile.OpenDFile(argv[3]) == FALSE) CallQuit(-1);

    // ��ȡ��ǰʱ���վ���ʵ�����ݣ�д�������ļ�
    if (QXFWFile.WriteNowData() == FALSE) CallQuit(-1);

    // ��ȡ�ӽ���01ʱ������00ʱȫ������ʱ�������
    if (QXFWFile.WriteTodayData() == FALSE) CallQuit(-1);

    // ��ȡ����Ԥ����Ϣ
    if (QXFWFile.WriteYBData() == FALSE) CallQuit(-1);

    char strDFileName[301];
    memset(strDFileName,0,sizeof(strDFileName));
    strcpy(strDFileName,QXFWFile.m_dfile.m_fullfilename);

    // �ر��Ѵ򿪵������ļ�
    QXFWFile.CloseDFile();

    ZIPFiles(strDFileName,FALSE);

    // ��ȡ�ó������Ҫ��ʾ��Ϣ
    if (QXFWFile.WriteImpMessInfo(argv[3]) == FALSE) CallQuit(-1);
  }

  // �����ѻ�ȡ��Ҫ��ʾ��λ��
  if (QXFWFile.UptImpPos() == FALSE) CallQuit(-1);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("crtqxfwfile exit.\n");

  exit(0);
}

