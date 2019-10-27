/*
 *  本程序用于处理全国气象站点观测的分钟数据，并保存到数据库的T_SURFDATA表中。
*/
#include "_public.h"
#include "_ooci.h"
#include "_shqx.h"


bool _psurfdata()
{
CLogFile logfile;

 exit(0);
  return true;
}

connection conn;

void EXIT(int sig);

int main(int argc,char *argv[])
{
  // 析构函数是否被调用的问题
  // 当程序调用exit函数退出时候，局部对象的析构函数不会调用。
  
  _psurfdata();
 
  exit(-1);
}

void EXIT(int sig)
{
  //logfile.Write("程序退出，sig=%d\n\n",sig);

  exit(0);
}
