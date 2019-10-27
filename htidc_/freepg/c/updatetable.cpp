//
// ��������ʾ������Ʒ��������
//

#include "_postgresql.h"

int main(int argc,char *argv[])
{
  // ���ݿ����ӳ�
  connection conn;
  
  // SQL���Բ�����
  sqlstatement stmt;

  // �������ݿ⣬����ֵ0-�ɹ�������-ʧ��
  // ʧ�ܴ�����conn.m_cda.rc�У�ʧ��������conn.m_cda.message�С�
  if (conn.connecttodb("host=10.151.64.150 user=postgres password=oracle dbname=gxpt port=5432","gbk")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); exit(-1);
  }

  // Ϊsqlstatementָ�����ݿ����ӳأ�����Ҫ�жϷ���ֵ
  stmt.connect(&conn);

  int iminid,imaxid;
  char strbtime[20];

  // ׼���������ݵ�SQL������Ҫ�жϷ���ֵ
  stmt.prepare("\
    update goods set btime=to_date(:1,'yyyy-mm-dd hh24:mi:ss') where id>:2 and id<:3");
  // ΪSQL������������ĵ�ַ
  stmt.bindin(1, strbtime,19);
  stmt.bindin(2,&iminid);
  stmt.bindin(3,&imaxid);

  // �ֹ�ָ��id�ķ�ΧΪ1��5��btimeΪ2017-12-20 09:45:30��ִ��һ�θ���
  iminid=1;
  imaxid=5;
  memset(strbtime,0,sizeof(strbtime));
  strcpy(strbtime,"2017-12-20 09:45:30");

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); 
    exit(-1);
  }

  // ��ע�⣬stmt.m_cda.rpc�����ǳ���Ҫ����������SQL��ִ�к�Ӱ��ļ�¼����
  printf("���θ�����goods��%ld����¼��\n",stmt.m_cda.rpc);

  // �ύ����
  conn.commitwork();

  exit(0);
}

