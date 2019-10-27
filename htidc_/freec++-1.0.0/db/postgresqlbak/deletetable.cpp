//
// ��������ʾɾ����Ʒ��������
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

  int iminid,imaxid;

  // ׼��ɾ�����ݵ�SQL������Ҫ�жϷ���ֵ
  stmt.prepare("delete from goods where id>:1 and id<:2");
  // ΪSQL������������ĵ�ַ
  stmt.bindin(1,&iminid);
  stmt.bindin(2,&imaxid);

  // �ֹ�ָ��id�ķ�ΧΪ1��5
  iminid=1;
  imaxid=5;

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // ��ע�⣬stmt.m_cda.rpc�����ǳ���Ҫ����������SQL��ִ�к�Ӱ��ļ�¼����
  printf("���δ�goods����ɾ����%ld����¼��\n",stmt.m_cda.rpc); 

  // �ύ����
  conn.commitwork();

  return 0;
}

