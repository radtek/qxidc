#include "idcapp.h"

connection conn;
CLogFile  logfile;
CProgramActive ProgramActive;

void EXIT(int sig);

int main(int argc,char *argv[])
{
  if (argc != 3) 
  {
    printf("\nUsing:/htidc/htidc/bin/killidlesession username/password@tnsname timeout\n\n"); 

    printf("ÕâÊÇÒ»¸ö¹¤¾ß³ÌĞò£¬ÓÃÓÚÇåÀíÊı¾İ¿âÖĞ¿ÕÏĞµÄ»á»°¡£\n");
    printf("±¾³ÌĞòÖ»Ö§³Öoracle 11g£¬ÒòÎªÔÚoracle 10gÖĞ£¬V$SESSIONÊÓÍ¼Ã»ÓĞprev_exec_start×Ö¶Î¡£\n");
    printf("username/password@tnsnameÊÇ½ÓÊÕÊı¾İµÄÊı¾İ¿âÁ¬½Ó²ÎÊı¡£\n");
    printf("timeout³¬Ê±Ê±¼ä£¬µ¥Î»ÊÇ·ÖÖÓ¡£\n");

    return -1;
  }

  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open("/tmp/htidc/log/killidlesession.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/killidlesession.log) failed.\n"); return -1;
  }

  // ´ò¿ª¸æ¾¯
  logfile.SetAlarmOpt("killidlesession");

  // ×¢Òâ£¬³ÌĞò³¬Ê±ÊÇ180Ãë
  ProgramActive.SetProgramInfo(&logfile,"killidlesession",180);

  // Á¬½ÓÊı¾İ¿â
  if (conn.connecttodb(argv[1],TRUE) != 0)
  {
    printf("connect database %s failed.\n",argv[1]); return -1;
  }

  int sid,serial;
  char logon_time[21],prev_exec_start[21];

  sqlstatement stmtsel;
  stmtsel.connect(&conn);
  stmtsel.prepare("select sid,serial#,to_char(logon_time,'yyyy-mm-dd hh24:mi:ss'),to_char(prev_exec_start,'yyyy-mm-dd hh24:mi:ss') from V$SESSION where status in ('KILLED','INACTIVE') and wait_class='Idle' and prev_exec_start<sysdate-%s/1440 order by prev_exec_start",argv[2]);
  stmtsel.bindout(1,&sid);
  stmtsel.bindout(2,&serial);
  stmtsel.bindout(3, logon_time,19);
  stmtsel.bindout(4, prev_exec_start,19);

  sqlstatement stmtkill1;
  stmtkill1.connect(&conn);

  sqlstatement stmtkill2;
  stmtkill2.connect(&conn);

  if (stmtsel.execute() != 0)
  {
    logfile.Write("select V$SESSION failed.%s\n",stmtsel.cda.message); EXIT(-1);
  }
  
  while (TRUE)
  {
    sid=serial=0;
    memset(logon_time,0,sizeof(logon_time));
    memset(prev_exec_start,0,sizeof(prev_exec_start));

    if (stmtsel.next() != 0) break;

    logfile.Write("session(sid=%d,serial=%d,logon_time=%s,prev_exec_start=%s\n",sid,serial,logon_time,prev_exec_start);

    // ¾­ÎÒ¹Û²â£¬Ö»ĞèÒªµÚÒ»ÖÖ·½Ê½¾Í¿ÉÒÔÉ±µôÁË£,ÔÙÔËĞĞµÚ¶şÖÖ·½Ê½»áË«ÖØ±£ÏÕ£¬µ«ÊÇµÚ¶ş¸öÓĞ¿ÉÄÜ±¨´í£¬ÒòÎªÇ°ÃæÒÑ¾­É±µô»á»°ÁË¡£
    stmtkill1.prepare("alter system kill session '%d,%d' immediate",sid,serial);
    if (stmtkill1.execute() != 0)
    {
      logfile.Write("alter system kill session '%d,%d' immediate failed.%s\n",sid,serial,stmtkill1.cda.message);
    }
    else 
    {
      logfile.Write("kill session '%d,%d' ok.\n",sid,serial);
    }

    stmtkill2.prepare("alter system disconnect session '%d,%d' immediate",sid,serial);
    if (stmtkill2.execute() != 0)
    {
      logfile.Write("alter system disconnect session '%d,%d' immediate failed.%s\n",sid,serial,stmtkill2.cda.message);
    }
    else
    {
      logfile.Write("disconnect session '%d,%d' ok.\n",sid,serial);
    }
  }

  if (stmtsel.cda.rpc>0) logfile.WriteEx("\n");

  return 0;
}

void EXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  printf("catching the signal(%d).\n",sig);

  printf("killidlesession exit.\n");

  exit(0);
}


