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
 
    printf("����һ�����߳���ϵͳ����Ա����������ͼ��\n");
    printf("V_tname���ӵ�ǰ�����ʷ��V_tname_ALL���ӵ�ǰ����ʷ��͹鵵��\n");
    printf("����tname�Ƿ������ʷ��Ĺ鵵�����ᴴ������������ͼ��\n\n\n");
 
    return -1;
  }
 

  memset(connstr,0,sizeof(connstr));
  memset(tname,0,sizeof(tname));

  strcpy(connstr,argv[1]);
  strcpy(tname,argv[2]);

  // �������ݿ�
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  CrtView(&conn,tname);

  return 0;
}

