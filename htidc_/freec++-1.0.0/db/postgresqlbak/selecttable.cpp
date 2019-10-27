//
// ��������ʾ����Ʒ���в�ѯ����
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
  
  // �������ݿ⣬����ֵ0-�ɹ�������-ʧ�ܣ�ʧ�ܴ�����conn.m_cda.rc��
  // ʧ��������conn.m_cda.message�С�
  if (conn.connecttodb("host=118.89.50.198 user=postgres password=pwdidc dbname=postgres port=5432","gbk")!=0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }

  // SQL���Բ�����
  sqlstatement stmt(&conn);

  int iminid,imaxid;

  // ׼����ѯ���ݵ�SQL������Ҫ�жϷ���ֵ
  stmt.prepare("\
    select id,name,sal,to_char(btime,'yyyy-mm-dd hh24:mi:ss')\
      from goods where id>:1 and id<:2");
  // ΪSQL������������ĵ�ַ
  stmt.bindin(1,&iminid);
  stmt.bindin(2,&imaxid);

  // ΪSQL������������ĵ�ַ
  stmt.bindout(1,&stgoods.id);
  stmt.bindout(2, stgoods.name,30);
  stmt.bindout(3,&stgoods.sal);
  stmt.bindout(4, stgoods.btime,19);

  // �ֹ�ָ��id�ķ�ΧΪ1��5��ִ��һ�β�ѯ
  iminid=1;
  imaxid=8;

  // ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
  if (stmt.execute() != 0)
  {
    printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
  }

  while (1)
  {
    // �Ȱѽṹ�������ʼ����Ȼ��Ż�ȡ��¼
    memset(&stgoods,0,sizeof(stgoods));

    // ��ȡһ����¼��һ��Ҫ�жϷ���ֵ��0-�ɹ�������-�޼�¼
    if (stmt.next() !=0) break;
    
    // �ѻ�ȡ���ļ�¼��ֵ��ӡ����
    printf("id=%ld,name=%s,sal=%.02f,btime=%s\n",stgoods.id,stgoods.name,stgoods.sal,stgoods.btime);
  }

  // ��ע�⣬stmt.m_cda.rpc�����ǳ���Ҫ����������SQL��ִ�к�Ӱ��ļ�¼����
  printf("���β�ѯ��goods��%ld����¼��\n",stmt.m_cda.rpc);
  
  return 0;
}

