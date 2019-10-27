#include "idcapp.h"

void CallQuit(int sig);

CLogFile   logfile;
connection conn;

// ����ͬ����־������������
BOOL CrtLogTable();

// ����������
BOOL CrtTrigger();

// ������Դ���еļ�¼���µ���־����
BOOL InitData();

int main(int argc,char *argv[])
{
  // ����־�ļ�
  if (logfile.Open("/tmp/synctest.log","a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n","/tmp/synctest.log"); return -1;
  }

  // �������ݿ�
  if (conn.connecttodb("szidc/pwdidc",FALSE) != 0)
  {
    logfile.Write("connect database %s failed.\n","szidc/pwdidc"); return -1;
  }

  int ii=0;
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("insert into t_allawsdata(ddatetime,ossmocode,checksts,keyid) values(sysdate+:1/86400,'aaa',1,:2)");
  stmt.bindin(1,&ii);
  stmt.bindin(2,&ii);
  
  logfile.Write("begin\n");
  for (ii=0;ii<300000;ii++)
  {
    if (stmt.execute() != 0)
    {
      logfile.Write("insert t_allawsdata failed.\n%s\n",stmt.cda.message);
    }
  }
  logfile.Write("ok\n");

  conn.commitwork();

  exit(0);
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("crtsynctrigger exit.\n");

  exit(0);
}

