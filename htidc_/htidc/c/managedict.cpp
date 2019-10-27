#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;

// ɾ��ϵͳ�����ֵ��в����ڣ���T_ALLTABLE���д��ڵı�
long DeleteALLTABLE();
// �������ֵ��д��ڣ���T_ALLTABLE���в����ڵļ�¼����T_ALLTABLE����
long InsertALLTABLE();
// ��T_ALLTABLE���д��ڣ���T_APPTABLE���в����ڵļ�¼����T_APPTABLE����
long InsertAPPTABLE();
// ��T_ALLTABLE���д��ڣ���T_DSYNCCFG���в����ڵļ�¼����T_DSYNCCFG����
long InsertDSYNCCFG();
// ��T_ALLTABLE���д��ڣ���T_BDSYNCCFG���в����ڵļ�¼����T_BDSYNCCFG����
long InsertBDSYNCCFG();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/managedict logfilename connstr \n");

    printf("Example:/htidc/htidc/bin/procctl 120 /htidc/htidc/bin/managedict /log/ssqx/managedict.log qxidc/pwdidc@EJETDB_221.179.6.136\n\n");

    printf("�����������Զ�ά��T_ALLTABLE��T_APPTABLE��T_DSYNCCFG��T_BDSYNCCFG�ֵ��ļ�¼��\n");
    printf( "�������ֵ��д��ڣ���T_ALLTABLE���в����ڵļ�¼����T_ALLTABLE����\n" );
    printf( "��T_ALLTABLE���д��ڣ���T_APPTABLE���в����ڵļ�¼����T_APPTABLE����\n" );
    printf( "��T_ALLTABLE���д��ڣ���T_DSYNCCFG���в����ڵļ�¼����T_DSYNCCFG����\n" );
    printf( "��T_ALLTABLE���д��ڣ���T_BDSYNCCFG���в����ڵļ�¼����T_BDSYNCCFG����\n" );
    printf("���������������ĵ�һ���������ܳ�����procctl���ȡ�\n\n\n");
    return -1;
  }


  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("managedict");

  // ע�⣬�������ݿ�ĳ�ʱʱ����500��
  ProgramActive.SetProgramInfo(&logfile,"managedict",500);

  if (conn.connecttodb(argv[2]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // ע�⣬����ʱ֮ǰ��500�룬���ڸ�Ϊ180�룬��Ϊ���������ݿ�ʧ��ʱ����Ҫ�ܳ�ʱ�䣬�����ʱ����Ϊ180�룬
  // ���ܵ��¸澯�޷�����
  ProgramActive.SetProgramInfo(&logfile,"managedict",180);

  logfile.Write("begin ...");

  // ɾ��ϵͳ�����ֵ��в����ڣ���T_ALLTABLE���д��ڵı�
  if (DeleteALLTABLE() != 0) { logfile.Write("DeleteALLTABLE failed.\n"); CallQuit(-1); }
   
  // �������ֵ��д��ڣ���T_ALLTABLE���в����ڵļ�¼����T_ALLTABLE����
  if (InsertALLTABLE() != 0) { logfile.Write("InsertALLTABLE failed.\n"); CallQuit(-1); }

  // ��T_ALLTABLE���д��ڣ���T_APPTABLE���в����ڵļ�¼����T_APPTABLE����
  if (InsertAPPTABLE() != 0) { logfile.Write("InsertAPPTABLE failed.\n"); CallQuit(-1); }

  // ��T_ALLTABLE���д��ڣ���T_DSYNCCFG���в����ڵļ�¼����T_DSYNCCFG����
  if (InsertDSYNCCFG() != 0) { logfile.Write("InsertDSYNCCFG failed.\n"); CallQuit(-1); }

  // ��T_ALLTABLE���д��ڣ���T_BDSYNCCFG���в����ڵļ�¼����T_BDSYNCCFG����
  if (InsertBDSYNCCFG() != 0) { logfile.Write("InsertBDSYNCCFG failed.\n"); CallQuit(-1); }

  conn.commitwork();

  logfile.WriteEx("end.\n\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("managedict exit.\n");

  exit(0);
}

// ɾ��ϵͳ�����ֵ��в����ڣ���T_ALLTABLE���д��ڵı�
long DeleteALLTABLE()
{
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    BEGIN\
      delete T_DSYNCCFG\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
      delete T_BDSYNCCFG\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
      delete T_APPTABLE\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
      delete T_ALLTABLE\
       where tname not in (select upper(table_name) from USER_TABLES\
                            where substr(table_name,1,2)='T_');\
    END;");
  if (stmt.execute() != 0)
  {
     logfile.Write("DeleteALLTABLE T_ALLTABLE failed.\n%s\n",stmt.cda.message);
  }
    
  return stmt.cda.rc;
}

// �������ֵ��д��ڣ���T_ALLTABLE���в����ڵļ�¼����T_ALLTABLE����
long InsertALLTABLE()
{
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("\
    insert into T_ALLTABLE(tname,tcname,keyid)\
     select upper(table_name),upper(table_name),SEQ_ALLTABLE.nextval from USER_TABLES\
      where substr(table_name,1,2)='T_' and table_name not in (select upper(tname) from T_ALLTABLE)");
  if (stmt.execute() != 0)
  {
     logfile.Write("InsertALLTABLE T_ALLTABLE failed.\n%s\n",stmt.cda.message);
  }
  
  return stmt.cda.rc;    
}

// ��T_ALLTABLE���д��ڣ���T_APPTABLE���в����ڵļ�¼����T_APPTABLE����
long InsertAPPTABLE()
{
  int iappid=0;
  sqlstatement stmtsel,stmtins;
  stmtsel.connect(&conn);
  stmtsel.prepare("select appid from T_DAPPSERVER where servertype in (1,2) order by appid");
  stmtsel.bindout(1,&iappid);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("InsertAPPTABLE select T_DAPPSERVER failed.\n%s\n",stmtsel.cda.message); 
    return stmtsel.cda.rc;
  }

  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_APPTABLE(appid,tname,hdataptype,keyid)\
       select :1,tname,3,SEQ_APPTABLE.nextval from T_ALLTABLE\
        where tname not in (select tname from T_APPTABLE where appid=:2)");
  stmtins.bindin(1,&iappid);
  stmtins.bindin(2,&iappid);
 
  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  
    if (stmtins.execute() != 0)
    {
      logfile.Write("InsertAPPTABLE insert T_APPTABLE failed.\n%s\n",stmtins.cda.message); 
      return stmtins.cda.rc;
    }
  }

  return 0;    
}

// ��T_ALLTABLE���д��ڣ���T_DSYNCCFG���в����ڵļ�¼����T_DSYNCCFG����
long InsertDSYNCCFG()
{
  int iappid=0;
  sqlstatement stmtsel,stmtins;
  stmtsel.connect(&conn);
  stmtsel.prepare("select appid from T_DAPPSERVER where servertype=2 order by appid");
  stmtsel.bindout(1,&iappid);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("InsertDSYNCCFG select T_DAPPSERVER failed.\n%s\n",stmtsel.cda.message); 
    return stmtsel.cda.rc;
  }

  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_DSYNCCFG(appid,tname,rsts,keyid)\
       select :1,tname,2,SEQ_DSYNCCFG.nextval from T_ALLTABLE\
        where tname not in (select tname from T_DSYNCCFG where appid=:2)");
  stmtins.bindin(1,&iappid);
  stmtins.bindin(2,&iappid);
 
  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  
    if (stmtins.execute() != 0)
    {
      logfile.Write("InsertDSYNCCFG insert T_DSYNCCFG failed.\n%s\n",stmtins.cda.message); 
      return stmtins.cda.rc;
    }
  }

  return 0;    
}

// ��T_ALLTABLE���д��ڣ���T_BDSYNCCFG���в����ڵļ�¼����T_BDSYNCCFG����
long InsertBDSYNCCFG()
{
  // Ϊ�˼������ڵ��������ģ�servertype in (2,5,6)���µ�ֻ��2
  int iappid=0;
  sqlstatement stmtsel,stmtins;
  stmtsel.connect(&conn);
  stmtsel.prepare("select appid from T_DAPPSERVER where servertype in (2,5,6) order by appid");
  stmtsel.bindout(1,&iappid);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("InsertBDSYNCCFG select T_DAPPSERVER failed.\n%s\n",stmtsel.cda.message); 
    return stmtsel.cda.rc;
  }

  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_BDSYNCCFG(appid,tname,rsts,keyid)\
       select :1,tname,2,SEQ_BDSYNCCFG.nextval from T_ALLTABLE\
        where tname not in (select tname from T_BDSYNCCFG where appid=:2)");
  stmtins.bindin(1,&iappid);
  stmtins.bindin(2,&iappid);
 
  while (TRUE)
  {
    if (stmtsel.next() != 0) break;
  
    if (stmtins.execute() != 0)
    {
      logfile.Write("InsertBDSYNCCFG insert T_BDSYNCCFG failed.\n%s\n",stmtins.cda.message); 
      return stmtins.cda.rc;
    }
  }

  return 0;    
}
