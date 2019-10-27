//
// ��������ʾ��ΰ�Oracle��BLOB�ֶε�������ȡ�������ļ��С�
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
  stmt.prepare("select pic from goods where id=1");
  stmt.bindblob();

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  // ��ȡһ����¼��һ��Ҫ�жϷ���ֵ��0-�ɹ���1403-�޼�¼������-ʧ�ܡ�
  if (stmt.next() != 0) return 0;

  // ��BLOB�ֶ��е�����д������ļ���һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.lobtofile((char *)"pic_out.jpg") != 0)
  {
    printf("stmt.lobtofile() failed.\n%s\n",stmt.m_cda.message); return -1;
  }

  return 0;
}

