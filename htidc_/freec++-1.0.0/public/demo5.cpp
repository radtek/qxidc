//
// ��������ʾ����ʱ���������
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));

  // ��ȡ��ǰʱ��
  LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss");
  printf("%s\n",strLocalTime);

  // �ѵ�ǰʱ����ַ�����ʽת��Ϊtime_t
  printf("%ld\n",UTCTime(strLocalTime));

  // ��time_t����ʽ��ȡ��ǰʱ��
  time_t  timer;
  time( &timer );
  printf("%ld\n",timer);

  // �ѵ�ǰʱ���һ��
  AddTime(strLocalTime,strLocalTime,0-24*60*60,"yyyy-mm-dd hh24:mi:ss");
  printf("%s\n",strLocalTime);

  return 0;
}

