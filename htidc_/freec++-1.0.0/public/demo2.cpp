//
// ��������ʾ����CDir���ȡĿ¼����Ŀ¼�е��ļ���
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:./demo2 pathname match\n\n");

    printf("Example:./demo2 /tmp \"*.txt,*.cpp\"\n\n");

    printf("��������ʾ����CDir���ȡĿ¼�е��ļ���������1��Ŀ¼��������2���ļ���ƥ��Ĺ���\n\n");

    return -1;
  }

  CDir Dir;

  // ��Ŀ¼����ȡ��Ŀ¼��ȫ����*.txt�ļ���������Ŀ¼��
  if (Dir.OpenDir(argv[1],argv[2],10000,TRUE,TRUE)==FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",argv[1]); return -1;
  }

  // �г�ȫ�����ļ�����������Ϣ
  while (TRUE)
  {
    if (Dir.ReadDir()==FALSE) break;

    printf("file=%s,size=%ld,mtime=%s\n",Dir.m_FullFileName,Dir.m_FileSize,Dir.m_ModifyTime);
  }

  return 0;
}

