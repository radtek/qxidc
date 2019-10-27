#include "idcapp.h"

void CallQuit(int sig);

CLogFile   logfile;
connection conn;

// 创建同步日志表、索引、序列
BOOL CrtLogTable();

// 创建触发器
BOOL CrtTrigger();

// 把数据源表中的记录更新到日志表中
BOOL InitData();

int main(int argc,char *argv[])
{
  // 打开日志文件
  if (logfile.Open("/tmp/synctest.log","a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n","/tmp/synctest.log"); return -1;
  }

  // 连接数据库
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

