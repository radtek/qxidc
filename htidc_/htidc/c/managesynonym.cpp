#include "idcapp.h"

void CallQuit(int sig);

char strDstConnStr[201]; // ���ݿ����Ӳ���

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;
UINT           uAPPID=0;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./managesynonym logfilename connstr appid\n\n");

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/managesynonym /log/ssqx/managesynonym_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/managesynonym /log/ssqx/managesynonym_2.log qxidc/pwdidc@EJETDB_221.179.6.136 2\n\n");

    printf("�����������ļ�Ⱥ���ݿ�ȫ����ͬ��ʣ��κ�qxidc�û��ı���ͼ�����ж��ᱻ��������ͬ��ʡ�\n");
    printf("appid�Ǵ���������ݿ⣬�������Ǽ�Ⱥ�е����ݿ�֮һ��\n");
    printf("�����Ҫ�Զ����Ⱥ���ݿ���й����ͱ������ж��managesynonym����\n");
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
  logfile.SetAlarmOpt("managesynonym");

  // ע�⣬����ʱ��60��
  ProgramActive.SetProgramInfo(&logfile,"managesynonym",60);

  char strIDCUserName[51];
  memset(strIDCUserName,0,sizeof(strIDCUserName));
  strcpy(strIDCUserName,argv[2]);
  char *pos=strstr(strIDCUserName,"/");
  if (pos<=0) CallQuit(-1);
  pos[0]=0;
  ToUpper(strIDCUserName);

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

  // ע�⣬�������ݿ�󣬳���ʱ��Ϊ180��
  ProgramActive.SetProgramInfo(&logfile,"managesynonym",180);

  char strtable_name[51];  // qxidc�û��ı���ͼ��������

  sqlstatement seltname;
  seltname.connect(&conndst);
  seltname.prepare("\
    (select table_name    table_name from    USER_TABLES)\
    union\
    (select view_name     table_name from    USER_VIEWS)\
    union\
    (select Sequence_name table_name from    USER_SEQUENCES)\
    union\
    (select object_name from USER_PROCEDURES where object_name in ('FINDCFIELDNAME','FINDCOMMENT','PATCHEXTVTIME','WDTOWDFW'))");
  seltname.bindout(1,strtable_name,50);

  int icount=0;
  sqlstatement selsynonym;
  selsynonym.connect(&conndst);
  selsynonym.prepare("\
    select count(*) from ALL_SYNONYMS\
     where owner='PUBLIC' and synonym_name=:1 and table_owner='%s' and table_name=synonym_name\
       and db_link is null",strIDCUserName);
  selsynonym.bindin(1,strtable_name,30);
  selsynonym.bindout(1,&icount);

  if (seltname.execute() != 0)
  {
    logfile.Write("select USER_TABLES,USER_VIEWS,USER_SEQUENCES failed.\n%s\n",seltname.cda.message); return seltname.cda.rc;
  }

  sqlstatement stmttmp;
  stmttmp.connect(&conndst);

  while (TRUE)
  {
    icount=0;

    memset(strtable_name,0,sizeof(strtable_name));

    if (seltname.next() != 0) break;

    if (selsynonym.execute() != 0)
    {
      logfile.Write("select ALL_SYNONYMS1 failed.\n%s\n",selsynonym.cda.message); return selsynonym.cda.rc;
    }
    
    selsynonym.next();

    if (icount==0)
    {
      // ��Ȼ�������ֵ���û�в�ѯ���Ըö���������ͬ��ʣ�ͬʱҲִ��ɾ���������б��޻�
      stmttmp.prepare("drop public synonym %s",strtable_name);
      stmttmp.execute();
  
      // Ȼ���ٴ���ͬ���
      stmttmp.prepare("create public synonym %s for %s.%s",strtable_name,strIDCUserName,strtable_name);
      if (stmttmp.execute() != 0)
      {
        logfile.Write("create synonym %s failed.\n%s\n",strtable_name,stmttmp.cda.message); 
        return stmttmp.cda.rc;
      }

      logfile.Write("create public synonym %s ok.\n",strtable_name);
    }
  }

  // ɾ���������ĵ������ֵ��в����ڵ�ͬ���
  char synonym_name[51];
  selsynonym.prepare("\
    select synonym_name from ALL_SYNONYMS\
     where owner='PUBLIC'  and table_owner='%s'\
       and synonym_name not in \
           ((select table_name    table_name from    USER_TABLES)\
             union\
            (select view_name     table_name from    USER_VIEWS)\
             union\
            (select Sequence_name table_name from    USER_SEQUENCES)\
             union\
            (select object_name from USER_PROCEDURES where object_name in ('FINDCFIELDNAME','FINDCOMMENT','PATCHEXTVTIME','WDTOWDFW')))",strIDCUserName);
  selsynonym.bindout(1,synonym_name,30);
  
  if (selsynonym.execute() != 0)
  {
    logfile.Write("select ALL_SYNONYMS2 failed.\n%s\n",selsynonym.cda.message); return selsynonym.cda.rc;
  }
  while (TRUE)
  {
    memset(synonym_name,0,sizeof(synonym_name));
    if (selsynonym.next() != 0) break;

    stmttmp.prepare("drop public synonym %s",synonym_name);
    if (stmttmp.execute() != 0)
    {
      logfile.Write("drop synonym %s failed.\n%s\n",synonym_name,stmttmp.cda.message); 
      return stmttmp.cda.rc;
    }

    logfile.Write("drop public synonym %s ok.\n",synonym_name);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("managesynonym exit.\n");

  exit(0);
}

