//
// ��������ʾCftp�ͻ����࣬����ftpЭ��ӷ������ϻ�ȡ�ļ�
//

#include "_ftp.h"

int main(int argc,char *argv[])
{
  Cftp ftp;

  // ��¼Զ��FTP������
  if (ftp.login("193.112.167.234:21","freec","qWE12#$(*0776",FTPLIB_PASSIVE) == FALSE)
  {
    printf("ftp.login() failed.\n"); return -1;
  }

  // �ı䱾��Ŀ¼
  chdir("/tmp");

  // ����ftp���������ļ���ŵ�Ŀ¼
  if (ftp.chdir("/home/freec/freec++-1.0.0/public") == FALSE)
  {
    printf("ftp.chdir() failed.\n"); return -1;
  }

  // ��ȡ�Է�Ŀ¼�ļ����б������"/tmp/ftp.list"�ļ���
  if (ftp.nlist("*.cpp","/tmp/ftp.list")== FALSE)
  {
    printf("ftp.nlist() failed.\n"); return -1;
  }

  CFile File;

  File.OpenForRead("/tmp/ftp.list","rt");

  char strFileName[101];

  // ���ж�ȡ�ļ������ݣ������ļ�get������
  while (TRUE)
  {
    // ���ļ��ж�ȡһ��
    memset(strFileName,0,sizeof(strFileName));
    if (File.FFGETS(strFileName,100) == FALSE) break;

    printf("get %s ...",strFileName);

    // ��Զ��ȡ�ļ�
    if (ftp.get(strFileName,strFileName)==FALSE)
    {
      printf("ftp.get(%s) failed.\n",strFileName); break;
    }
    
    printf("ok.\n");
  }

  File.CloseAndRemove();

  ftp.logout();

  return 0;
}

