#include "_public.h"
#include "_ooci.h"

struct st_bank
{
  int    msgid;         // ��Ϣ����
  char   oper_id[31];   // ����Ա���
  char   password[31];  // ����Ա����
  char   userid[21];    // �ͻ������ʺ�
  char   cardid[19];    // �ͻ����֤����
  char   username[31];  // �ͻ�����
  char   tel[31];       // �绰
  double ye;            // �ʻ����
  int    opertype;      // ҵ������
  double je;            // �������
  int    rsts;          // ״̬
  char   crttime[20];

  char   userid1[21];    // �ͻ������ʺţ�ת���ʺ�
};

// ����Ա��¼
int msgid_1(connection *conn,struct st_bank *stbank,CLogFile *logfile);


// ����ҵ��
int msgid_2(connection *conn,struct st_bank *stbank,CLogFile *logfile);

// �ͻ������֤
int msgid_3(connection *conn,struct st_bank *stbank,CLogFile *logfile);

// ���ҵ��
int msgid_4(connection *conn,struct st_bank *stbank,CLogFile *logfile);

// ȡ��ҵ��
int msgid_5(connection *conn,struct st_bank *stbank,CLogFile *logfile);

// ת��ҵ��0-�ɹ���-1-���㣻-2-ת���ʻ��Ƿ���>0-ϵͳ����
int msgid_6(connection *conn,struct st_bank *stbank,CLogFile *logfile);

// �����
int msgid_7(connection *conn,struct st_bank *stbank,CLogFile *logfile);

// ����ˮ
int msgid_8(connection *conn,struct st_bank *stbank,CLogFile *logfile,vector<struct st_bank> &vbank);

