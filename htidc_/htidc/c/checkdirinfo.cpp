#include "_public.h"

// �����쳣�жϴ�����
void EXIT(int sig);

CLogFile       logfile;                // ��־�ļ���
CProgramActive ProgramActive;          // ��̨���̼����
CFile          File;
CDir           Dir;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/checkdirinfo checkdirinfo.xml\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/checkdirinfo /htidc/hssms/ini/checkdirinfo.xml\n\n");
    printf("�����ȡcheckdirinfo.xml�����ļ������������Ŀ¼�µ��ļ����Ƿ񳬳���ֵ������������ͷ����澯��\n");
    printf("������5��������һ�Σ��ɵ��ȳ�����ƣ���־�ļ���Ϊ/tmp/htidc/log/checkdirinfo.log��\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // �򿪳���������־
  if (logfile.Open("/tmp/htidc/log/checkdirinfo.log","a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n","/tmp/htidc/log/checkdirinfo.log"); return -1;
  }

  logfile.SetAlarmOpt("checkdirinfo");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"checkdirinfo",180);

  logfile.Write("��ʼ����.\n");

  if (File.OpenForRead(argv[1],"r") == FALSE)
  {
    logfile.Write("File.OpenForRead(%s) failed.\n",argv[2]); EXIT(-1);
  }

  char strBuffer[1024];
  char pathname[301],andchild[11];
  long filecount;

  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));
    memset(pathname,0,sizeof(pathname));
    memset(andchild,0,sizeof(andchild));
    filecount=0;

    if (File.FFGETS(strBuffer,1000) == FALSE) break;

    if (strstr(strBuffer,"#") != 0) continue;

    GetXMLBuffer(strBuffer,"pathname",pathname,300);
    GetXMLBuffer(strBuffer,"andchild",andchild,5);
    GetXMLBuffer(strBuffer,"filecount",&filecount);

    if ( (strlen(pathname)==0) || (strlen(andchild)==0) || (filecount<=0) ) continue;

    BOOL bAndChild=FALSE;
    if ( (strcmp(andchild,"true")==0) || (strcmp(andchild,"TRUE")==0) ) bAndChild=TRUE;

    if (Dir.OpenDir(pathname,bAndChild) == FALSE)
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",pathname); continue;
    }

    logfile.Write("pathname=%s,andchild=%s,filecount=%ld,fileexist=%lu.\n",pathname,andchild,filecount,Dir.m_vFileName.size());

    if (Dir.m_vFileName.size() >= (UINT)filecount)
    {
      logfile.Write("Ŀ¼%s���Ƿ������Ŀ¼��%s���е��ļ���Ϊ%lu�������˼�صķ�ֵ%ld��failed.\n",pathname,andchild,Dir.m_vFileName.size(),filecount);
    }
  }

  logfile.Write("�������.\n\n");

  exit(0);
}

void EXIT(int sig)
{
  if (sig > 0) 
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  logfile.Write("checkdirinfo EXIT.\n");

  exit(0);
}

