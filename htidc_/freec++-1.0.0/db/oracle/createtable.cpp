//
// ��������ʾ����һ�������ڴ����Ʒ��Ϣ��
//

#include "_ooci.h"

int main(int argc,char *argv[])
{
  // ���ݿ����ӳ�
  connection conn;

  // �������ݿ⣬����ֵ0-�ɹ�������-ʧ��
  // ʧ�ܴ�����conn.m_cda.rc�У�ʧ��������conn.m_cda.message�С�
  if (conn.connecttodb("scott/tiger@snorcl11g_198","Simplified Chinese_China.ZHS16GBK") != 0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }
  
  // SQL���Բ�����
  sqlstatement stmt(&conn);

  // ׼���������SQL����Ʒ����Ʒ���id����Ʒ����name���۸�sal
  // ���ʱ��btime����Ʒ˵��memo����ƷͼƬpic
  // prepare��������Ҫ�жϷ���ֵ
  stmt.prepare("\
    create table goods(id    number(10),\
                       name  varchar2(30),\
                       sal   number(10,2),\
                       btime date,\
                       memo  clob,\
                       pic   blob,\
                       primary key (id))");

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  printf("create table goods ok.\n");

  return 0;
}

