#include "idcapp.h"

char strConnStr[201]; // 数据库连接参数

connection     conn;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./crtsequence username/password@tnsname\n");

    printf("Example:./crtsequence szidc/pwdidc\n\n");

    printf("这是一个工具程序，用于中心主数据库崩溃后的环境恢复。\n");
    printf("当中心主数据库崩溃时，将启用其它的数据库代替它，但其它的数据库没有创建序列生成器。\n"); 
    printf("此程序用于生成创建序列生成器的脚本，并为序列生成器指定初始值。\n"); 
    printf("程序读取序列生成器字典表T_SEQANDTABLE的参数，再从各数据表中获取已存在记录的最大键\n");
    printf("值，并用该值当成序列的初始值生成脚本。\n\n\n");

    return -1;
  }

  memset(strConnStr,0,sizeof(strConnStr));

  strcpy(strConnStr,argv[1]);

  if (conn.connecttodb(strConnStr,TRUE) != 0)
  {
    printf("connect %s failed.\n",strConnStr); exit(-1);
  }

  char seqname[31],tname[31],fieldname[31],sql[512];
  long value;

  sqlstatement stmt1,stmt2;

  stmt1.connect(&conn);
  stmt2.connect(&conn);

  stmt1.prepare("select seqname,tname,fieldname from T_SEQANDTABLE order by seqname");
  stmt1.bindout(1,seqname,30);
  stmt1.bindout(2,tname,30);
  stmt1.bindout(3,fieldname,30);

  stmt1.execute();

  while (TRUE)
  {
    memset(seqname,0,sizeof(seqname));
    memset(tname,0,sizeof(tname));
    memset(fieldname,0,sizeof(fieldname));
    memset(sql,0,sizeof(sql));
    value=0;

    if (stmt1.next() != 0) break;

    if (strlen(tname)==0)
    {
      printf("--%s,tname is null.\n",seqname); continue;
    }

    if (strlen(fieldname)==0)
    {
      printf("%s,fieldname is null.\n",seqname); continue;
    }

    stmt2.prepare("select max(%s) from %s",fieldname,tname);
    stmt2.bindout(1,&value);

    if (stmt2.execute() != 0) 
    {  
      printf("%s failed.\n%s\n",stmt2.m_sql,stmt2.cda.message); break;
    }

    stmt2.next();

    printf("drop   sequence %s;\n",seqname);
    printf("create sequence %s increment by 1 minvalue %ld nocycle;\n",seqname,value+1);
  }

  return 0;
}


