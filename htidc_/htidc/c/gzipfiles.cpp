#include "_public.h"

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  char   strPathName[201];
  double dDayOut=0;

  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/gzipfiles pathname dayout\n\n");

    printf("Example:/htidc/htidc/bin/gzipfiles /htidc/data 0.01\n\n");

    printf("����һ�����߳�������ѹ��pathnameĿ¼dayout��֮ǰ��ȫ���ļ������һᴦ��pathname�ĸ�����Ŀ¼��\n");
    printf("����ļ����޸�ʱ����dayout֮ǰ�ͻᱻѹ����dayout������С����\n");
    printf("������д��־�ļ���Ҳ�����ڿ�������κ���Ϣ��\n");
    printf("����������ֹ����У�Ҳ������procctl���ȡ�\n");
    printf("���������/usr/bin/gzip����ѹ���ļ���\n\n\n");
 
    return -1;
  }

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  dDayOut=atof(argv[2]);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); 
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strTimeOut[21];

  LocalTime(strTimeOut,"yyyy-mm-dd hh24:mi:ss",0-(int)(dDayOut*24*60*60));

  CDir Dir;
 
  // ��Ŀ¼����ȡ�ļ�������������Ŀ¼
  if (Dir.OpenDir(strPathName,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",strPathName); exit(-1);
  }

  char strCmd[1024];

  while (Dir.ReadDir() == TRUE)
  {
    if ( (strcmp(Dir.m_ModifyTime,strTimeOut) < 0) && (MatchFileName(Dir.m_FileName,"*.gz") == FALSE) )
    {
      memset(strCmd,0,sizeof(strCmd));
      sprintf(strCmd,"/usr/bin/gzip -f %s 1>/dev/null 2>/dev/null",Dir.m_FullFileName);
      system(strCmd);
    }
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  exit(0);
}

