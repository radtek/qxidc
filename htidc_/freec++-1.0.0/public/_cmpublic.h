/*
*   ���Ǹ����õ�ͷ�ļ�,�����˳��õ�ϵͳͷ�ļ�,
*   �ڲ�ͬ��UNIXϵͳ��ͷ�ļ������в�ͬ.   
*/

#ifndef _cmpublic_H
#define _cmpublic_H

#include <stdio.h>
#include <utime.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <locale.h>
#include <dirent.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>
#include <deque>
#include <algorithm>

// ����C++��׼ģ���������ռ�
using namespace std;

// �����Ƕ��岼���������͵ļ�����,���е�UNIX��,��һ��֧�ֲ�������,
// �����������Զ���
#ifndef BOOL
  #define BOOL unsigned char 
#endif

#ifndef bool
  #define bool unsigned char 
#endif

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef true
  #define true 1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#ifndef false
  #define false 0
#endif

#ifndef INT
  #define INT long
#endif

#ifndef UINT
  #define UINT unsigned long
#endif

#endif

