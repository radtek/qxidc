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

    printf("����һ�����߳�����ĳ��Ϊģ�壬������һ�ű��±�����ݽṹ��������ԭ����ȫ��ͬ��\n");
    printf("���ǣ�ԭ�����������������һ��Ҫ���������ĵĹ淶������PK_tname������IDX_tname_n��\n\n");

    return -1;
  } 

  memset(connstr,0,sizeof(connstr));
  memset(templetname,0,sizeof(templetname));
  memset(tname,0,sizeof(tname));

  strcpy(connstr,argv[1]);
  strcpy(templetname,argv[2]);
  strcpy(tname,argv[3]);

  // �������ݿ�
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

