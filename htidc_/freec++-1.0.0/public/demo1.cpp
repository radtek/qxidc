//
// ��������ʾ����CXmlFile���ȡ�����ļ�
// �����ļ�����demo1.xml����main��������
// 

#include "_public.h"

// �������ڴ�Ų��������ݽṹ
struct st_args
{
  char connstr[51];    // ����ƽ̨�������ݿ�Ĳ���
  char logpath[301];   // ��־�ļ���ŵ�Ŀ¼
  char smsdata[301];   // ���������ļ���ŵĸ�Ŀ¼
  char cmpphmd[301];   // �ƶ������
  char sgiphmd[301];   // ��ͨ�����
  char smgphmd[301];   // ���ź����
};

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./demo1 xmlfilename\n\n");

    printf("Example:./demo1 demo1.xml\n\n");

    printf("��������ʾ����CXmlFile���ȡ�����ļ���demo1.xml�ǲ����ļ�����\n\n");

    return -1;
  }

  struct st_args stargs;
  memset(&stargs,0,sizeof(struct st_args));

  CXmlFile XmlFile;

  // ��������ļ�
  if (XmlFile.LoadFile(argv[1]) == FALSE)
  {
    printf("XmlFile.LoadFile(%s) failed.\n",argv[1]); return -1;
  }

  // ��ȡÿ������
  XmlFile.GetValue("connstr",stargs.connstr,50);
  XmlFile.GetValue("logpath",stargs.logpath,300);
  XmlFile.GetValue("smsdata",stargs.smsdata,300);
  XmlFile.GetValue("cmpphmd",stargs.cmpphmd,300);
  XmlFile.GetValue("sgiphmd",stargs.sgiphmd,300);
  XmlFile.GetValue("smgphmd",stargs.smgphmd,300);

  // ��ȫ���Ĳ���ֵ��ӡ����
  printf("connstr=%s\n",stargs.connstr);
  printf("logpath=%s\n",stargs.logpath);
  printf("smsdata=%s\n",stargs.smsdata);
  printf("cmpphmd=%s\n",stargs.cmpphmd);
  printf("sgiphmd=%s\n",stargs.sgiphmd);
  printf("smgphmd=%s\n",stargs.smgphmd);

  return 0;
}

