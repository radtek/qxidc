//
// ��������ʾ����Ʒ���в���10����¼��
//
#include "_postgresql.h"

// �������ڲ������ݵĽṹ������е��ֶζ�Ӧ
struct st_GOODS
{
  long id;          // ��Ʒ��ţ���long�������Ͷ�Ӧpostgresql��С����int
  char name[31];    // ��Ʒ���ƣ���char��Ӧpostgresql��varchar��ע�⣬�����ֶεĳ�����30��char����ĳ�����31��Ҫ��C���ԵĽ�����
  double sal;       // ��Ʒ�۸���double�������Ͷ�Ӧpostgresql��С����numeric
  char btime[20];   // ���ʱ�䣬��char��Ӧpostgresql��timestamp����ʽ������SQL�����ָ����������ָ��Ϊ'yyyy-mm-dd hh24:mi:ss'
} stgoods;

int main(int argc,char *argv[])
{
  // ���ݿ����ӳ�
  connection conn;

  // �������ݿ⣬����ֵ0-�ɹ�������-ʧ��
  // ʧ�ܴ�����conn.m_cda.rc�У�ʧ��������conn.m_cda.message�С�
  if (conn.connecttodb("host=118.89.50.198 user=postgres password=pwdidc dbname=postgres port=5432","gbk")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }

  // SQL���Բ�����
  sqlstatement stmt(&conn);
  
  // ׼���������ݵ�SQL������Ҫ�жϷ���ֵ
  stmt.prepare("\
    insert into goods(id,name,sal,btime)\
               values(:1,:2,:3,to_timestamp(:4,'yyyy-mm-dd hh24:mi:ss'))");
  // ΪSQL������������ĵ�ַ
  stmt.bindin(1,&stgoods.id);
  stmt.bindin(2, stgoods.name,30);
  stmt.bindin(3,&stgoods.sal);
  stmt.bindin(4, stgoods.btime,19);
  
  // ģ����Ʒ���ݣ�����в���10��������Ϣ
  for (int ii=1;ii<=10;ii++)
  {
    // �ṹ�������ʼ��
    memset(&stgoods,0,sizeof(stgoods));
  
    // Ϊ�ṹ��ı���ָ��ֵ
    stgoods.id=ii;
    sprintf(stgoods.name,"��Ʒ����%02d",ii);
    stgoods.sal=ii*2.11;
    strcpy(stgoods.btime,"2018-03-01 12:25:31");
  
    // ÿ��ָ��������ֵ��ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
    if (stmt.execute() != 0)
    {
printf("id=%d\n",stgoods.id);
      printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
    }
  
    printf("insert ok(id=%d).\n",ii);
  }
  
  printf("insert table goods ok.\n");

  // �ύ���ݿ�����
  conn.commitwork();
  
  return 0;
}  
