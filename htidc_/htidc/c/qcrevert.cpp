#include "qxidc.h"

connection conn;
CLogFile   logfile;

char strConnStr[101];
char strTableName[51];

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if (argc != 3) 
  {
    printf("\nUsing:./qcrevert username/password@tnsname tablename\n\n"); 
    printf("        ./qcrevert pyidc/pwdidc T_OBTMIND\n\n"); 

    printf("����һ�����߳������ڻ�ԭ�ʿأ��ָ�ԭʼ���ݡ�\n");
    printf("ע�⣬�ʿػ�ԭʱ���ֹ��޸ĺ�������ֶ�ֵ���ᱻ��ԭ��\n");
    printf("�ʿػ�ԭʱ���ʿ�״̬����ԭ���ʿس�ʼ״̬����һ����0��\n");
    printf("�ʿر�����ݻ�ԭһ��Ҫ�ô˳��򣬽�ֹ�ֹ��޸ı���ʿ��ֶΡ�\n\n\n");

    return -1;
  }

  memset(strConnStr,0,sizeof(strConnStr));
  memset(strTableName,0,sizeof(strTableName));

  strcpy(strConnStr,argv[1]);
  strcpy(strTableName,argv[2]);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  logfile.Open("/tmp/qcrevert.log","a+");

  // �������ݿ�
  if (conn.connecttodb(strConnStr,TRUE) != 0)
  {
    printf("conn.connecttodb(%s) failed\n",strConnStr); exit(-1);
  }

  CDATAQC DATAQC;

  DATAQC.BindConnLog(&conn,&logfile);

  DATAQC.RevertTable(strTableName);

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("qcrevert exit.\n");

  exit(0);
}

