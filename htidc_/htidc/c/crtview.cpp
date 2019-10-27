#include "idcapp.h"

char connstr[101];
char tname[51];
connection conn;

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/crtview username/password@tnsname tname\n");

    printf("Example:/htidc/htidc/bin/crtview sqxt/pwdidc T_UPDATED_ALARMSIGN\n\n");
 
    printf("这是一个工具程序，系统管理员用它创建视图。\n");
    printf("V_tname连接当前表和历史表，V_tname_ALL连接当前表、历史表和归档表。\n");
    printf("不管tname是否存在历史表的归档表，都会创建以上两个视图。\n\n\n");
 
    return -1;
  }
 

  memset(connstr,0,sizeof(connstr));
  memset(tname,0,sizeof(tname));

  strcpy(connstr,argv[1]);
  strcpy(tname,argv[2]);

  // 连接数据库
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  CrtView(&conn,tname);

  return 0;
}

