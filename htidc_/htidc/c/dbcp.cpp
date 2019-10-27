#include "_cmpublic.h"
#include "_oracle.h"
#include "_public.h"

void CallQuit(int sig);

char connstr[101],tablename[51],filename[201],topath[101];

int main(int argc,char *argv[])
{
/*
long ll=201112100101001234;
printf("ll=%ld=\n",ll);
exit(0);
*/
/*
  // lon_rd雷达的经度，lat_rd雷达的纬度，h_rd雷达的高度
// 指定格点经度lon_p，纬度lat_p，高度h_p
// angle是雷达站与指定格点连接地心的夹角，斜距range，仰角elev，方位角azim
double angle,range,elev,azim;
angle=range=elev=azim=0;

get_sea(113.20, 22.70, 0 ,
            113.20 , 23.10 , 0  ,
            &angle, &range, &elev, &azim);
printf("angle=%.05f,range=%.05f,elev=%.05f,azim=%.5f\n",angle,range,elev,azim);

angle=angle+5.0;
printf("%f\n",(angle/180.0)*3.1415926*6374131);

get_sea(113.20, 22.70, 0 ,
            113.70 , 22.70 , 0  ,
            &angle, &range, &elev, &azim);
printf("angle=%.05f,range=%.05f,elev=%.05f,azim=%.5f\n",angle,range,elev,azim);
angle=angle+5.0;
printf("%f\n",(angle/180.0)*3.1415926*6374131);

  return 0;
*/

//xxxxxxxxxxxxxxxxxxxxxxxx

  if (argc != 5)
  {
    printf("\n");
    printf("要改，把matchfilename改为where的方式。\n");
    printf("Using:./htidc/htidc/bin/dbcp username/password@tnsname tablename \"matchfilename\" topath\n");

    printf("Example:/htidc/htidc/bin/dbcp sqxt/pwdidc T_RADFILES \"*201003040006*\" /tmp/rad\n\n");
 
    printf("这是一个工具程序，用于把二进制文件从数据库的T_XXXFILES表的BLOB字段中拷贝出来，类似操作系统的cp命令。\n");
    printf("username/password@tnsname是数据库参数，tablename是存放二进制文件的表名，即T_XXXFILES。\n");
    printf("matchfilename指定文件匹配方式，可匹配\"*\"，必须用双引号括起来。\n");
    printf("topath是二进制文件输出的目录。\n");
    printf("本程序一般是手工运行。\n\n\n");
 
    return -1;
  }

  memset(connstr,0,sizeof(connstr));
  memset(tablename,0,sizeof(tablename));
  memset(filename,0,sizeof(filename));
  memset(topath,0,sizeof(topath));

  strcpy(connstr,argv[1]);
  strcpy(tablename,argv[2]);
  strcpy(filename,argv[3]);
  strcpy(topath,argv[4]);

  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  connection conn;

  // 连接数据库
  if (conn.connecttodb(connstr) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  // 把文件名中的“*”用“%”代替
  UpdateStr(filename,"*","%");

  int  filesize;
  char dstfilename[201];

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    select filesize,:1||'/'||filename,filecontent from %s where filename like '%s'",tablename,filename);
  stmt.bindin(1,topath,200);
  stmt.bindout(1,&filesize);
  stmt.bindout(2, dstfilename,200);
  stmt.bindblob(3);

  if (stmt.execute() != 0)
  {
    printf("select from %s failed.\n",tablename);
  }

  int iret=0;

  UINT ifilecount=0;

  while (TRUE)
  {
    filesize=0;
    memset(dstfilename,0,sizeof(dstfilename));

    if (stmt.next() != 0) break;

    if (FileSize(dstfilename) == filesize) continue;

    fprintf(stdout,"copy %s ...",dstfilename);
  
    iret = stmt.lobtofile(dstfilename);

    if (iret == 0) { fprintf(stdout,"ok.\n"); ifilecount++; }

    if (iret != 0) fprintf(stdout,"failed.\n");

    fflush(stdout);
  }

  if (ifilecount == 0)
  {
    printf("no file copied.\n");
  }
  else
  {
    printf("%lu files copied.\n",ifilecount);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  printf("catching the signal(%d).\n",sig);

  printf("dbcp exit.\n");

  exit(0);
}
