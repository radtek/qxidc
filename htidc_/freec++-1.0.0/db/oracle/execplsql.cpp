//
// ��������ʾ���ִ��һ��PL/SQL����
// ������˵һ���Ҹ��˵�������ҴӲ���Oracle���ݿ��д���PL/SQL���̣�Ҳ����ʹ�ô�������ԭ�����£�
// 1����Oracle���ݿ��д���PL/SQL���̣�����ĵ��Ժ��鷳��
// 2��ά���������鷳����Ϊ����ԱҪ��ʱ��ȥ�˽����ݿ��еĴ洢���̣�
// 3�����ҷ�װ��oci�У���oracle�Ĳ����Ѿ��Ƿǳ��򵥵����飬û��Ҫȥ���ڴ洢���̣�
//
// ���У�����oci��Ҳ������PL/SQL��䣬Ҳ����Ϊ���ӵ�PL/SQL�����鷳��
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

  // ׼��ɾ����¼��PL/SQL������Ҫ�жϷ���ֵ
  // ��PL/SQL��ɾ��goods���е�ȫ����¼���ٲ���һ����¼
  stmt.prepare("\
    BEGIN\
      delete from goods;\
      insert into goods(id,name,sal,btime)\
                 values(:1,'������Ʒ',55.65,to_date('2018-01-02 13:00:55','yyyy-mm-dd hh24:mi:ss'));\
    END;");
  int id=100;
  stmt.bindin(1,&id);

  // ע�⣬PL/SQL�е�ÿ��SQL��Ҫ�÷ֺŽ�����END֮����һ���ֺš�

  
  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  printf("exec PL/SQL ok.\n");

  // �ύ����
  conn.commit();

  return 0;
}

