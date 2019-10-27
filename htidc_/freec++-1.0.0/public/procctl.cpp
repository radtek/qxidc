#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc,char *argv[])
{
  if ( (argc < 3) || (argc > 10) )
  {
    printf("\n");
    printf("Using:./procctl tvltime program argvs......\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /bin/tar zcvf /home/sendtmp/zqczlog.tgz /tmp/htidc/log\n\n");
    printf("        /htidc/htidc/bin/procctl 300 /bin/mv /home/sendtmp/zqczlog.tgz /home/send/zqczlog.tgz\n\n");
 
    printf("���Ǻ�̨����������ȳ��򣬿��Ե���ȫ���ĺ�̨����\n");
    printf("tvltime�������ڣ������ȵĳ������н�������tvltime���ᱻprocctl����������\n");
    printf("program�����ȵĳ�����������ʹ��ȫ·����\n");
    printf("argvs�����ȵĳ���Ĳ��������7����������procctl�Ĳ�������������10����\n");
    printf("ע�⣬procctl���ᱻkillɱ������������kill -9ǿ��ɱ����\n\n\n");
 
    return -1;
  }

  // �ر�ȫ����ϵͳ�ź�
  for(int i=0;i<50;i++)
  {
    signal(i,SIG_IGN); close(i);
  }

  // �����ն˿���
  setpgrp();

  // �����ӽ��̣��������˳�
  if (fork() != 0) exit(0);

  // ����SIGCHLD�ź�
  signal(SIGCHLD,SIG_DFL);

  int status;

  while (1)
  {
    if (fork() == 0)
    {
      if (argc == 3)
      {
        execl(argv[2],argv[2],(char *)0);
      }

      if (argc == 4)
      {
        execl(argv[2],argv[2],argv[3],(char *)0);
      }

      if (argc == 5)
      {
        execl(argv[2],argv[2],argv[3],argv[4],(char *)0);
      }

      if (argc == 6)
      {
        execl(argv[2],argv[2],argv[3],argv[4],argv[5],(char *)0);
      }
      if (argc == 7)
      {
        execl(argv[2],argv[2],argv[3],argv[4],argv[5],argv[6],(char *)0);
      }

      if (argc == 8)
      {
        execl(argv[2],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],(char *)0);
      }

      if (argc == 9)
      {
        execl(argv[2],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],(char *)0);
      }

      if (argc == 10)
      {
        execl(argv[2],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9],(char *)0);
      }
   
      exit(0);
    }
    else
    {
      wait(&status); sleep(atoi(argv[1]));
    }
  }

  exit(0);
}
