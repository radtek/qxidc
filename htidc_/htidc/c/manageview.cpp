#include "idcapp.h"

void CallQuit(int sig);

char strDstConnStr[201]; // ���ݿ����Ӳ���

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;
UINT           uAPPID=0;

// ���������ļ�Ⱥ���ݿ��У�ֻ�ܴ���V_TNAME��V_TNAME_ALL��ͼ����������ͼ�ᱻɾ����
long DropOthView();

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./manageview logfilename connstr appid\n\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/manageview /log/ssqx/manageview_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/manageview /log/ssqx/manageview_2.log qxidc/pwdidc@EJETDB_221.179.6.136 2\n\n");

    printf("�����������ļ�Ⱥ���ݿ�ȫ�������ͼ���κα��ᱻ������ͼ��\n");
    printf("V_TNAME�����ϵ�ǰ�����ʷ��V_TNAME_ALL�����ϵ�ǰ����ʷ��͹鵵��\n");
    printf("�����������ʷ���鵵��V_TNAME��V_TNAME_ALL���Ǳ��������ͼ��\n");
    printf("���������ļ�Ⱥ���ݿ��У�ֻ�ܴ���V_TNAME��V_TNAME_ALL��ͼ����������ͼ�ᱻɾ����\n");
    printf("appid�Ǵ���������ݿ⣬�������Ǽ�Ⱥ�е����ݿ�֮һ��\n");
    printf("�����Ҫ�Զ����Ⱥ���ݿ���й����ͱ������ж��manageview����\n\n\n");
    printf("ע�⣺appid������T_DAPPSERVER�������\n\n\n");
 
    return -1;
  }

  uAPPID=atoi(argv[3]);

  memset(strDstConnStr,0,sizeof(strDstConnStr));

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("manageview");

  // ע�⣬����ʱ��60��
  ProgramActive.SetProgramInfo(&logfile,"manageview",60);

  // ���������������ݿ�
  if (connidc.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  long irsts=0;
  // ���������ĵ�T_DAPPSERVER����ȡ������ķ�������״̬����
  if (findbypk(&connidc,"T_DAPPSERVER","appid","rsts",uAPPID,&irsts) != 0)
  {
    logfile.Write("Call findbypk failed\n"); CallQuit(-1);
  } 
  if (irsts!=1)
  {
    logfile.Write("appserver no exist or stopped.\n"); connidc.disconnect(); sleep(30); return 0;
  }

  // ���������ĵ�T_DAPPSERVER����ȡ������ķ����������ݿ����Ӳ���
  if (findbypk(&connidc,"T_DAPPSERVER","appid","tnsname",uAPPID,strDstConnStr,50) != 0)
  {
    logfile.Write("Call findbypk failed\n"); CallQuit(-1);
  }

  logfile.Write("DstConnStr=%s\n",strDstConnStr);

  // �����������ĵļ�Ⱥ���ݿ�
  if (conndst.connecttodb(strDstConnStr,TRUE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed.\n",strDstConnStr); CallQuit(-1);
  }

  // ���������ļ�Ⱥ���ݿ��У�ֻ�ܴ���V_TNAME��V_TNAME_ALL��ͼ����������ͼ�ᱻɾ����
  if (DropOthView() != 0) CallQuit(-1);

  // ע�⣬�������ݿ�󣬳���ʱ��Ϊ180��
  ProgramActive.SetProgramInfo(&logfile,"manageview",180);

  ALLTABLE.BindConnLog(&connidc,&conndst,&logfile,&ProgramActive);

  CTABFIELD TABFIELD;

  // ���ü�Ⱥ��������ID
  ALLTABLE.m_appid=uAPPID;

  // ��T_APPTABLE��������ĳ��Ⱥ������ȫ���ĵ�ǰ�����ڴ�����ͼ
  if (ALLTABLE.LoadForCrtView() != 0)
  {
    logfile.Write("ALLTABLE.LoadForCrtView failed.\n"); CallQuit(-1);
  }

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (ALLTABLE.LoadAllRecordNext() != 0) break;
  
    if (CrtView(&conndst,ALLTABLE.m_stALLTABLE.tname) != 0)
    {
      logfile.Write("create view %s FAILED.\n",ALLTABLE.m_stALLTABLE.tname); continue;
    }

    logfile.Write("create view %s ok.\n",ALLTABLE.m_stALLTABLE.tname);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("manageview exit.\n");

  exit(0);
}


// ���������ļ�Ⱥ���ݿ��У�ֻ�ܴ���V_TNAME��V_TNAME_ALL��V_TNAME_SHARE��ͼ����������ͼ�ᱻɾ����
long DropOthView()
{
  char view_name[31];

  sqlstatement stmtsel;
  stmtsel.connect(&connidc);  // ���������ĺ��Ŀ�
  stmtsel.prepare(\
    "select view_name from USER_VIEWS\
      where view_name not in\
            (select 'V'||substr(tname,2,30) from t_APPTABLE\
              where appid=:1 and tname not like '%%HIS' and tname not like '%%ARH%%')\
        and view_name not in\
            (select 'V'||substr(tname,2,30)||'_ALL' from t_APPTABLE\
              where appid=:2 and tname not like '%%HIS' and tname not like '%%ARH%%')\
        and view_name not in\
            (select 'V'||substr(tname,2,30)||'_SHARE' from t_APPTABLE\
              where appid=:3 and tname not like '%%HIS' and tname not like '%%ARH%%')");
  stmtsel.bindin(1,&uAPPID);
  stmtsel.bindin(2,&uAPPID);
  stmtsel.bindin(3,&uAPPID);
  stmtsel.bindout(1,view_name,30);
  if (stmtsel.execute() != 0)
  {
    logfile.Write("DropOthView select USER_VIEWS failed.\n%s\n",stmtsel.cda.message);
    return stmtsel.cda.rc;
  }

  sqlstatement stmtdel;
  stmtdel.connect(&conndst);  // ����������Ŀ�����ݿ�
  
  while (TRUE)
  {
    memset(view_name,0,sizeof(view_name));

    if (stmtsel.next() != 0) break;

    stmtdel.prepare("drop view %s",view_name);
    stmtdel.execute();

    logfile.Write("%s\n",stmtdel.m_sql);
  }

  return 0;
}
