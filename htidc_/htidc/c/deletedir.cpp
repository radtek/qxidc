#include "_public.h"

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if ( (argc != 3) && (argc != 4) )
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/deletefiles pathname dayout [matchstr]\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /tmp/htidc 5\n\n");
    printf("        /htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /tmp/htidc 5 *.GIF\n\n");

    printf("����һ�����߳�����������pathnameĿ¼dayout��֮ǰ��Ŀ¼\n");
    printf("����ļ����޸�ʱ����dayout֮ǰ�ͻᱻ������dayout������С����\n");
    printf("matchstr��һ����ѡ������ָ����ɾ�����ļ�����ƥ�䷽ʽ��\n");
    printf("������д��־�ļ���Ҳ��������Ļ������κ���Ϣ��\n");
    printf("����������ֹ����У�Ҳ������procctl���ȡ�\n\n\n");
 
    return -1;
  }

  char   strPathName[201];
  double dDayOut=0;

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  dDayOut=atof(argv[2]);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); 
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strTimeOut[21];

  LocalTime(strTimeOut,"yyyymmdd",0-(int)(dDayOut*24*60*60));

  CDir Dir;

  // ��Ŀ¼����ȡ�ļ�������������Ŀ¼
  if (Dir.OpenDirNoSort(strPathName,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",strPathName); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    if (strcmp(Dir.m_ModifyTime,strTimeOut) > 0) continue;

    if (argc == 4) if (MatchFileName(Dir.m_FileName,argv[3])==FALSE) continue;

    REMOVE(Dir.m_FullFileName);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  exit(0);
}
