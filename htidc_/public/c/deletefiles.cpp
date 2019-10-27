#include "_public.h"

void EXIT(int sig);

// ��ʾ����İ���
void _help(char *argv[]);

int main(int argc,char *argv[])
{
  if ( (argc != 3) && (argc != 4) ) { _help(argv); return -1; }

  // �ر�ȫ�����źź��������
  CloseIOAndSignal();

  // ��������˳����ź�
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  char   strPathName[201];
  double dDayOut=0;

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  dDayOut=atof(argv[2]);

  char strTimeOut[21];

  LocalTime(strTimeOut,"yyyy-mm-dd hh24:mi:ss",0-(int)(dDayOut*24*60*60));

  CDir Dir;

  char strMatch[50]; memset(strMatch,0,sizeof(strMatch));
  if (argc==3) strcpy(strMatch,"*");
  else strcpy(strMatch,argv[3]);

  // ��Ŀ¼����ȡ�ļ�������������Ŀ¼
  if (Dir.OpenDir(strPathName,strMatch,10000,true,false) == false)
  {
    printf("Dir.OpenDir(%s) failed.\n",strPathName); return -1;
  }

  char strLocalTime[21];

  while (Dir.ReadDir() == true)
  {
    if (strcmp(Dir.m_ModifyTime,strTimeOut) > 0) continue;

    printf("delete %s ok.\n",Dir.m_FullFileName);

    REMOVE(Dir.m_FullFileName);
  }

  return 0;
}

void EXIT(int sig)
{
  printf("�����˳���sig=%d\n\n",sig);

  exit(0);
}


// ��ʾ����İ���
void _help(char *argv[])
{
  printf("\n");
  printf("Using:/htidc/public/bin/deletefiles pathname dayout [matchstr]\n");
  printf("Sample:/htidc/public/bin/deletefiles /data/shqx/ftp/surfdata 0.1 \"*.TXT,*.CSV\"\n\n");

  printf("���������������ĵĹ�������ģ�飬����ɾ��ָ��Ŀ¼�µ���ʷ�ļ���\n");
  printf("pathname �������Ŀ¼�����������Ŀ¼�µĸ�����Ŀ¼��\n");
  printf("dayout   �ļ�������������λ���죬������С����\n");
  printf("matchstr �������ļ�����ƥ���������һ����ѡ����������ƥ��������͵��ļ����м��ö��ŷָ��������˫���Ű���������\n\n");
}

