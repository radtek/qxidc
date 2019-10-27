#include "_public.h"

pthread_t pthid1,pthid2;

// ��һ���̵߳�������
void *pth1_main(void *arg);

// �ڶ����̵߳�������
void *pth2_main(void *arg);

int ii=10;

int main()
{
  // �����߳�һ
  if ( pthread_create(&pthid1,NULL,pth1_main,(void*)3) != 0)
  {
    printf("pthread_create pthid1 failed.\n"); return -1;
  }
  
  // �����̶߳�
  if ( pthread_create(&pthid2,NULL,pth2_main,(void*)4) != 0)
  {
    printf("pthread_create pthid2 failed.\n"); return -1;
  }

  pthread_join(pthid1,NULL);
  pthread_join(pthid2,NULL);
  
  return 0;
}

// ��һ���̵߳�������
void *pth1_main(void *arg)
{
  long jj = (long)arg;
  printf("���ǵ�һ���̣߳�jj=%ld\n",jj);

  printf("1���ǵ�һ���̣߳�ii=%d\n",ii);
  ii=20;
  sleep(10);
  printf("2���ǵ�һ���̣߳�ii=%d\n",ii);

  pthread_exit(NULL);
}

// �ڶ����̵߳�������
void *pth2_main(void *arg)
{
  long jj = (long)arg;
  printf("���ǵڶ����̣߳�jj=%ld\n",jj);

  printf("1���ǵڶ����̣߳�ii=%d\n",ii);
  sleep(20);
  printf("2���ǵڶ����̣߳�ii=%d\n",ii);

  pthread_exit(NULL);
}

