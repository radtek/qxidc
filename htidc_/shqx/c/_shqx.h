#ifndef _SHQX_H
#define _SHQX_H

#include "_public.h"
#include "_ooci.h"

// ȫ������վ��������ݽṹ
struct st_stcode
{
  char provname[31];   // ʡ����
  char obtid[11];      // վ�����
  char cityname[31];   // ������
  double lat;          // γ��
  double lon;          // ����
  double height;       // ���θ߶�
};

// ȫ������վ����ӹ۲����ݽṹ
struct st_surfdata
{
  char obtid[11];      // վ�����
  char ddatetime[21];  // ����ʱ�䣺��ʽyyyy-mm-dd hh:mi:ss��
  int  t;              // ���£���λ��0.1���϶�
  int  p;              // ��ѹ��0.1����
  int  u;              // ���ʪ�ȣ�0-100֮���ֵ��
  int  wd;             // ����0-360֮���ֵ��
  int  wf;             // ���٣���λ0.1m/s
  int  r;              // ��������0.1mm
  int  vis;            // �ܼ��ȣ�0.1��
};

class CSURFDATA
{
public:
  int totalcount,insertcount,updatecount,invalidcount;  // ��¼�����ݡ�������������������Ч��¼����
  struct st_surfdata m_stsurfdata;

  CSURFDATA(connection *conn,CLogFile *logfile);
 ~CSURFDATA();

  void initdata();  // ���ݳ�ʼ��

  connection *m_conn;
  CLogFile   *m_logfile;

  int iccount;
  sqlstatement stmtsel,stmtins,stmtupt;

  // ���ö��ŷָ��ļ�¼��ֵ�m_stsurfdata�ṹ�С�
  bool SplitBuffer(const char *strBuffer);

  // ��xml��ʽ�ļ�¼��ֵ�m_stsurfdata�ṹ�С�
  bool SplitBuffer1(const char *strBuffer);

  // ��m_stsurfdata�ṹ�е�ֵ���µ�T_SURFDATA���С�
  long InsertTable();
};

struct st_signallog
{
  char obtid[11];
  char ddatetime[20];
  char signalname[2];
  char signalcolor[2];
};

class CSIGNALLOG
{
public:
  int totalcount,insertcount,updatecount,invalidcount;  // ��¼�����ݡ�������������������Ч��¼����
  struct st_signallog m_stsignallog;
  vector<struct st_signallog> vsignallog;   // �������һ���ļ���ȫ����¼

  CSIGNALLOG(connection *conn,CLogFile *logfile);
 ~CSIGNALLOG();

  void initdata();  // ���ݳ�ʼ��

  connection *m_conn;
  CLogFile   *m_logfile;

  int iccount;
  sqlstatement stmtsel,stmtins,stmtupt;

  // �Ѽ�¼��ֵ�m_stsignallog�ṹ�С�
  bool SplitBuffer(const char *strBuffer);

  // ��m_stsignallog�ṹ�е�ֵ���µ�T_SIGNALDATA���С�
  long InsertTable();
};


#endif
