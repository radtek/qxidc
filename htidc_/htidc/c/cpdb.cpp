#include "idcapp.h"

UINT keyid;
char connstr[101],tablename[51],fieldname[31],filename[201];
char fieldtype[11];

connection conn;

int main(int argc,char *argv[])
{
  if (argc != 7)
  {
    printf("\n");
    printf("这个程序要改，把keyid改为where的方式。\n");
    printf("Using:/htidc/htidc/bin/cpdb username/password@tnsname tablename fieldname keyid filename blob|clob\n");

    printf("Example:/htidc/htidc/bin/cpdb sqxt/pwdidc T_DAPPSERVER serverpic 1 /tmp/server1.jpg blob\n");
    printf("        /htidc/htidc/bin/cpdb sqxt/pwdidc T_DDEFINE ddetail 4 /tmp/readme.txt blob\n\n");
 
    printf("这是一个工具程序，系统管理员用它把磁盘上的文件拷贝到表的BLOB|CLOB字段中。\n\n\n");
 
    return -1;
  }

  memset(connstr,0,sizeof(connstr));
  memset(tablename,0,sizeof(tablename));
  memset(fieldname,0,sizeof(fieldname));
  keyid=0;
  memset(filename,0,sizeof(filename));
  memset(fieldtype,0,sizeof(fieldtype));

  strcpy(connstr,argv[1]);
  strcpy(tablename,argv[2]);
  strcpy(fieldname,argv[3]);
  keyid=atoi(argv[4]);
  strcpy(filename,argv[5]);
  strcpy(fieldtype,argv[6]);

  // 检查源文件是否存在
  if (access(filename,R_OK) != 0)
  {
    printf("%s no exist.\n",filename); return -1;
  }

  // 连接数据库
  if (conn.connecttodb(connstr,TRUE) != 0)
  {
    printf("connect database %s failed.\n",connstr); return -1;
  }

  long iret;

  if (strcmp(fieldtype,"blob") == 0)
  {
    iret = filetoblob(&conn,tablename,fieldname,keyid,filename);
  }
  else
  {
    iret = filetoclob(&conn,tablename,fieldname,keyid,filename);
  }

  if ( iret == 0)
  {
    printf("%s to %s(%s,keyid=%lu) ok.\n",filename,tablename,fieldname,keyid); 
  }

  if ( iret == 1)
  {
    printf("%s to %s(%s,keyid=%lu) faild,record is not exist.\n",filename,tablename,fieldname,keyid); return -1;
  }

  if ( iret >  1)
  {
    printf("%s to %s(%s,keyid=%lu) failed.%s\n",filename,tablename,fieldname,keyid,conn.lda.message); return -1;
  }

  conn.commitwork();

  return 0;
}

