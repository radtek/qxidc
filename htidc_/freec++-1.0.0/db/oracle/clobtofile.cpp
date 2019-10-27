//
// ��������ʾ��ΰ�Oracle��CLOB�ֶε�������ȡ�������ļ��С�
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

  // ����Ҫ�жϷ���ֵ
  stmt.prepare("select memo from goods where id=1");
  stmt.bindclob();

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }
  
  // ��ȡһ����¼��һ��Ҫ�жϷ���ֵ��0-�ɹ���1403-�޼�¼������-ʧ�ܡ�
  if (stmt.next() != 0) return 0;
  
  // ��CLOB�ֶ��е�����д������ļ���һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.lobtofile((char *)"memo_out.txt") != 0)
  {
    printf("stmt.lobtofile() failed.\n%s\n",stmt.m_cda.message); return -1;
  }

  return 0;
}

