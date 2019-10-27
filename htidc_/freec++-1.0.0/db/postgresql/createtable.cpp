//
// ��������ʾ����һ�������ڴ����Ʒ��Ϣ��
//

#include "_postgresql.h"

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

  // ׼���������SQL����Ʒ����Ʒ���id����Ʒ����name���۸�sal
  // ���ʱ��btime����Ʒ˵��memo����ƷͼƬpic
  // prepare��������Ҫ�жϷ���ֵ
  stmt.prepare("\
    create table goods(id    int,\
                       name  varchar(30),\
                       sal   numeric(8,2),\
                       btime timestamp,\
                       memo  text,\
                       pic   bytea,\
                       primary key (id))");
  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // �ύ����ע�⣬��postgresql���ݿ��У�������ҲҪ�ύ���񣬵�ɾ����ȴ���ã����
  conn.commit();

  printf("create table goods ok.\n");

  return 0;
}

