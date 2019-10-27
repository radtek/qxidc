//
// ��������ʾ��ΰѴ����ļ����ı��ļ�д��Oracle��CLOB�ֶ��С�
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

  // Ϊ�˷�����ʾ����goods���еļ�¼ȫɾ�����ٲ���һ�����ڲ��Ե����ݡ�
  // ����Ҫ�жϷ���ֵ
  stmt.prepare("\
    BEGIN\
      delete from goods;\
      insert into goods(id,name,memo) values(1,'��Ʒ����',empty_clob());\
    END;");
  
  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // ʹ���α��goods������ȡidΪ1��memo�ֶ�
  // ע���ˣ�ͬһ��sqlstatement���Զ��ʹ��
  // ���ǣ��������sql�ı��ˣ���Ҫ����prepare��bindin��bindout����
  stmt.prepare("select memo from goods where id=1 for update");
  stmt.bindclob();

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }
  
  // ��ȡһ����¼��һ��Ҫ�жϷ���ֵ��0-�ɹ���1403-�޼�¼������-ʧ�ܡ�
  if (stmt.next() != 0) return 0;
  
  // �Ѵ����ļ�memo_in.txt������д��CLOB�ֶ�
  if (stmt.filetolob((char *)"memo_in.txt") != 0)
  {
    printf("stmt.filetolob() failed.\n%s\n",stmt.m_cda.message); return -1;
  }

  // �ύ����
  conn.commit();

  return 0;
}

