//
// ��������ʾ����CLogFile��д��־
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./demo3 logfilename\n\n");

    printf("Example:./demo3 demo3.log\n\n");

    printf("��������ʾ����CLogFile��д�������е���־����������־�ļ�����\n\n");

    return -1;
  }

  CLogFile logfile;

  // ������־�ļ�
  if (logfile.Open(argv[1],"a+")==FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  // ����־�ļ���д��2000000�����ݡ�
  for (int ii=0;ii<2000000;ii++)
  {
    logfile.Write("%010d�����ǵ�%010d����־��������ѭ����%10d�Ρ�\n",ii+1,ii+1,ii+1);
  }

  return 0;
}

