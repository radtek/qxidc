//
// ��������ʾ��ʱ����CTimer
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  CTimer Time;

  // ��ʼ��ʱ
  Time.Start();

  sleep(5);      // ˯5��

  usleep(300);   // ˯300΢��

  double tt;

  // ��ʱ������ͬʱҲ����һ��ʱ���ڵĿ�ʼ
  tt=Time.Elapsed();

  printf("%f\n",tt);

  // ��ʱ����
  tt=Time.Elapsed();

  printf("%f\n",tt);

  return 0;
}

