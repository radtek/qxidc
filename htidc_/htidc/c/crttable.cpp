#include "idcapp.h"

char connstr[101];
char templetname[51];
char tname[51];
connection conn;

int main(int argc,char *argv[])
{
  if (argc != 4)
  { 
    printf("\n");
    printf("Using:./crttable username/password@tnsname templetname tname\n");    

    printf("Example:./crttable szidc/pwdidc T_LOCALOBTMIND T_LOCALOBTMIND_HIS\n\n");

    printf("这是一个工具程序，以某表为模板，创建另一张表，新表的数据结构和索引与原表完全相同。\n");
    printf("但是，原表的主键和索引命名一定要按数据中心的规范，主键PK_tname，索引IDX_tname_n。\n\n");

    return -1;
  } 

  memset(connstr,0,sizeof(connstr));
  memset(templetname,0,sizeof(templetname));
  memset(tname,0,sizeof(tname));

  strcpy(connstr,argv[1]);
  strcpy(templetname,argv[2]);
  strcpy(tname,argv[3]);

  // 连接数据库
  if (conn.connecttodb(connstr) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  ToUpper(tname);
  ToUpper(templetname);

  if (CrtByTable(&conn,templetname,tname) != 0)
  {
    printf("CrtByTable(%s,%s) failed.\n",templetname,tname); return -1;
  }

  printf("ok.\n");

  return 0;
}

