#include "idcapp.h"

void CallQuit(int sig);

char strSRCConnStr[201]; // ����Դ�����Ӳ���
char strDSTConnStr[201]; // Ŀ�����ݿ����Ӳ���

CIDCCFG IDCCFG;

connection     connsrc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;

char strSyncName[31];
char strDBLink[31];
char strTNameStr[4096];

CSYNCTABLE   SYNCTABLE;

// �������Դ��Դ���Ƿ���ڣ�ͬ������־���Ƿ���ڣ�ͬ������־��������Ƿ���ڣ���������Դ��Ĵ������Ƿ���ڡ�
// �������Դ�����ڣ�����ʧ�ܣ������־�����л򴥷��������ڣ��򴴽�����
BOOL CheckTABSEQTR();

int main(int argc,char *argv[])
{
  if (argc != 8)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/synctable inifile src:username/password@tnsname dst:username/password@tnsname dblink tablenames syncname logname\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable /htidc/sqxt/ini/sqxt.xml idc/oracle@EJETDB_172.22.11.112 idc/oracle@EJETDB_10.12.12.61 EJETDB_172_22_11_112 T_GD_AO_201010,T_GD_AO_201011,T_OBTDATA_201010,T_OBTDATA_201011,T_SW_OCEAN,T_SW_SURF,T_ASIAN_CITY_7DAYS,T_ASIAN_GAMES,T_ASIAN_GAMES_SG,T_ASIAN_GAMES_SW _12_61 _11.112.log\n\n");
 
    printf("����һ�����߳��������԰�ĳORACLE���ݿ��еı������ͬ������һ��ORACLE���ݿ⡣\n");
    printf("������Դ�����ݿ��û��У��������Զ�����ͬ����־����ڱ�ÿ����ͬ���ı��ϴ���һ����������\n");
    printf("src:username/password@tnsname������Դ���ݿ������ַ�����\n");
    printf("dst:username/password@tnsname��Ŀ�����ݿ������ַ�����\n");
    printf("dblink��Ŀ�����ݿ��е�����Դ���ݿ��������·����ҪDBA�ֹ�������\n");
    printf("tablenames����Ҫͬ���ı������ö��ŷָ�����ע�⣬tablenames��Դ����������Դ�����������T_��ͷ��\n");
    printf("Ŀ�ı���������Դ�����֮ǰ����T_�����Դ���������T_��ͷ�����ˣ�Ŀ�ı���ҪDBA�ֹ�������\n");
    printf("syncname��ͬ����־�����Ҫ��ͬһ����Դ�������Ŀ�����ݿ�ͬ��������ʹ�ò�ͬ�ı�־��\n");
    printf("logname�Ǹó�������ʱ��������־�ļ����ĺ�׺��\n");
    printf("�����������쳣ʱ���ᷢ��LOGINDB��SYNCTABLE�澯��\n\n");

    printf("ע�⣺\n");
    printf("1��������Ҫ������Դ���Ŀ�ı�Ľṹ��ȫ��ͬ������sync_rowid��crttime��keyid�ֶΣ������ң�Ŀ�ı�\n");
    printf("   һ��Ҫ��sync_rowid��crttime��keyid�ֶΣ�Դ����û���������ֶ�����ν��\n\n");

    printf("2��Ŀ�ı��sync_rowid�ֶ�һ��Ҫ���������������Ψһ����������crttime�ֶβ���Ҫ��\n\n");

    printf("3��Ŀ�ı��keyid�ֶ�һ��Ҫ����Ψһ�����������Ŀ�����ݿ��SEQ_����.nextvalȡֵ��\n\n");

    printf("4���ó������ú�ֻͬ������Դ���б�����������ʱ���֮��䶯�ļ�¼��ԭ�еļ�¼����ͬ�������ϣ��\n");
    printf("   ����Դ������ǰ�ļ�¼Ҳ��ͬ������Ҫ�ֹ���Դ���rowid���뵽ͬ����־���У��ɲο�syncscript.txt��\n\n");

    printf("5����������ͣ�ú�DBAһ��Ҫ�ֹ�ɾ������Դ�û��е�ͬ����־��T_SYNCLOG_ͬ����־����\n\n");

    printf("6����������ͣ�ú�DBAһ��Ҫ�ֹ�ɾ������Դ�û��е�ͬ�����ϵĴ�������TR_����_ͬ����־����\n\n");

    printf("7��/htidc/sqxt/sql/syncscript.txt��ʡ����̨����ͬ�������ӣ������������̣��ɲο���\n\n");

    printf("8��ע�ⲻҪ���û�ɾ���˱��˴����Ĵ������������鷳���ˡ�\n\n");

    return -1;
  }

  memset(strSRCConnStr,0,sizeof(strSRCConnStr));
  memset(strDSTConnStr,0,sizeof(strDSTConnStr));
  memset(strDBLink,0,sizeof(strDBLink));
  memset(strTNameStr,0,sizeof(strTNameStr));
  memset(strSyncName,0,sizeof(strSyncName));

  strcpy(strSRCConnStr,argv[2]);
  strcpy(strDSTConnStr,argv[3]);
  strcpy(strDBLink,argv[4]);
  strcpy(strTNameStr,argv[5]);
  strcpy(strSyncName,argv[6]);

  ToUpper(strTNameStr);

  // �Ӳ����ļ��м���ȫ���Ĳ���
  if (IDCCFG.LoadIniFile(argv[1]) == FALSE)
  {
    printf("IDCCFG.LoadIniFile(%s) failed.\n",argv[1]); return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; 
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,200,"%s/synctable%s",IDCCFG.m_logpath,argv[7]);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("synctable");

  // ע�⣬����ʱ��500��
  ProgramActive.SetProgramInfo(&logfile,"synctable",500);

  strcpy(SYNCTABLE.m_dblink,strDBLink);

  // ��������Դ���ݿ�
  if (connsrc.connecttodb(strSRCConnStr) != 0)
  {
    logfile.Write("connsrc.connecttodb(%s) failed\n",strSRCConnStr); 

    CallQuit(-1);
  }

  // ����Ŀ�����ݿ�
  if (conndst.connecttodb(strDSTConnStr) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed\n",strDSTConnStr); 

    CallQuit(-1);
  }

  // ע�⣬����ʱ֮ǰ��500�룬���ڸ�Ϊ180�룬��Ϊ���������ݿ�ʧ��ʱ����Ҫ�ܳ�ʱ�䣬�����ʱ����Ϊ180�룬
  // ���ܵ��¸澯�޷�����
  ProgramActive.SetProgramInfo(&logfile,"synctable",180);

  SYNCTABLE.BindConnLog(&conndst,&connsrc,&logfile);

  // �������Դ��Դ���Ƿ���ڣ�ͬ������־���Ƿ���ڣ�ͬ������־��������Ƿ���ڣ���������Դ��Ĵ������Ƿ���ڡ�
  // �������Դ�����ڣ�����ʧ�ܣ������־�����л򴥷��������ڣ��򴴽�����
  if (CheckTABSEQTR() == FALSE)
  {
    CallQuit(-1);
  }

  // ����SYNCTABLE.LoadTInfo��ʱ�����ͬ����־��ļ�¼̫�࣬��Ҫ��ʱ���Խ�������������Ϊ1200��
  ProgramActive.SetProgramInfo(&logfile,"synctable",1200);

  // ������Դ���ݿ�������ֵ��������ͬ���ı�Ľṹ����Сд���
  if (SYNCTABLE.LoadTInfo(strTNameStr) != 0)
  {
    CallQuit(-1);
  }

  // �Ļ�180��
  ProgramActive.SetProgramInfo(&logfile,"synctable",180);

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (SYNCTABLE.LoadSyncLog(strSyncName) != 0)
    {
      CallQuit(-1);
    }

    ProgramActive.WriteToFile();

    if (SYNCTABLE.SyncDATA(&ProgramActive,strSyncName) != 0)
    {
      CallQuit(-1);
    }

    ProgramActive.WriteToFile();

    sleep(5);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  connsrc.rollbackwork(); conndst.rollbackwork(); 

  //connalm.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable exit.\n");

  exit(0);
}

// �������Դ��Դ���Ƿ���ڣ�ͬ������־���Ƿ���ڣ�ͬ������־��������Ƿ���ڣ���������Դ��Ĵ������Ƿ���ڡ�
// �������Դ�����ڣ�����ʧ�ܣ������־�����л򴥷��������ڣ��򴴽�����
BOOL CheckTABSEQTR()
{
  char strDstTName[51],strTRName[51],strTmpName[31];

  sqlstatement stmt;
  stmt.connect(&connsrc);

  // ��������Ƿ����
  memset(strTmpName,0,sizeof(strTmpName));
  snprintf(strTmpName,30,"SEQ_SYNCLOG%s",strSyncName);
  if (CheckSEQExist(&connsrc,strTmpName) == FALSE) 
  {
    stmt.prepare("CREATE SEQUENCE %s increment by 1 minvalue 1 nocycle",strTmpName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE SEQUENCE %s failed.\n%s\n",strTmpName,stmt.cda.message); return FALSE;
    }
    logfile.Write("CREATE SEQUENCE %s ok.\n",strTmpName);
  }

  // �����־��T_SYNCLOG_SyncName�Ƿ����
  memset(strTmpName,0,sizeof(strTmpName));
  snprintf(strTmpName,30,"T_SYNCLOG%s",strSyncName);
  if (CheckTExist(&connsrc,strTmpName) == FALSE) 
  {
    stmt.prepare("CREATE TABLE %s(keyid number(18),tname varchar2(50),ctype number(1),sync_rowid rowid,crttime date default sysdate)",strTmpName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE TABLE %s failed.\n%s\n",strTmpName,stmt.cda.message); return FALSE;
    }

    stmt.prepare("CREATE UNIQUE INDEX IDX_SYNCLOG%s_1 on T_SYNCLOG%s(keyid)",strSyncName,strSyncName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE UNIQUE INDEX IDX_SYNCLOG%s_1 failed.\n%s\n",strSyncName,stmt.cda.message); return FALSE;
    }

    stmt.prepare("comment on table T_SYNCLOG%s is \
                  '�˱���������ͬ������ֹ�κ��˶Ըñ����κβ������������ʣ��������ݿ����Ա��ͨ��'",strSyncName);
    stmt.execute();

    stmt.prepare("CREATE INDEX IDX_SYNCLOG%s_2 on T_SYNCLOG%s(tname)",strSyncName,strSyncName);
    if (stmt.execute() != 0)
    {
      logfile.Write("CREATE INDEX IDX_SYNCLOG%s_2 failed.\n%s\n",strSyncName,stmt.cda.message); return FALSE;
    }

    logfile.Write("CREATE TABLE T_SYNCLOG%s ok.\n",strSyncName);
  }

   // �������Դ���Ŀ�����ݿ�ı��Ƿ����
  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strTNameStr,",");

  for (UINT ii=0;ii<CmdStr.m_vCmdStr.size();ii++)
  { 
    // �������Դ���Ƿ����
    if (CheckTExist(&connsrc,(char *)CmdStr.m_vCmdStr[ii].c_str()) == FALSE) 
    {
      logfile.Write("����Դ���ݿ�ı����ڣ�%s��.\n",CmdStr.m_vCmdStr[ii].c_str()); return FALSE;
    }

    // ���Ŀ�����ݿ���Ƿ���ڣ�Ŀ�ı���Դ����ǰ�ӡ�T_�������Դ���Ѿ��ǡ�T_����ͷ���Ͳ�����
    memset(strDstTName,0,sizeof(strDstTName));
    if (strncmp(CmdStr.m_vCmdStr[ii].c_str(),"T_",2) == 0)
      strcpy(strDstTName,CmdStr.m_vCmdStr[ii].c_str());
    else
      sprintf(strDstTName,"T_%s",CmdStr.m_vCmdStr[ii].c_str());
    
    if (CheckTExist(&conndst,strDstTName) == FALSE) 
    {
      logfile.Write("Ŀ�����ݿ�ı����ڣ�%s��.\n",strDstTName); return FALSE;
    }

    // �������Դ��Ĵ������Ƿ���ڣ���������ڣ��ʹ�������
    memset(strTRName,0,sizeof(strTRName));
    if (strncmp(CmdStr.m_vCmdStr[ii].c_str(),"T_",2) == 0)
      sprintf(strTRName,"TR_%s%s",CmdStr.m_vCmdStr[ii].c_str()+2,strSyncName);
    else
      sprintf(strTRName,"TR_%s%s",CmdStr.m_vCmdStr[ii].c_str(),strSyncName);
    if (CheckTRExist(&connsrc,strTRName) == FALSE) 
    {
      stmt.prepare("\
        CREATE OR REPLACE TRIGGER %s\
         AFTER INSERT OR UPDATE OR DELETE\
            ON %s FOR EACH ROW\
        BEGIN\
          if inserting then\
            insert into T_SYNCLOG%s(keyid,tname,ctype,sync_rowid) values(SEQ_SYNCLOG%s.nextval,'%s',1,:new.rowid);\
          end if;\
          if updating then\
            insert into T_SYNCLOG%s(keyid,tname,ctype,sync_rowid) values(SEQ_SYNCLOG%s.nextval,'%s',2,:new.rowid);\
          end if;\
          if deleting then\
            insert into T_SYNCLOG%s(keyid,tname,ctype,sync_rowid) values(SEQ_SYNCLOG%s.nextval,'%s',3,:old.rowid);\
          end if;\
        END;",strTRName,CmdStr.m_vCmdStr[ii].c_str(),strSyncName,strSyncName,CmdStr.m_vCmdStr[ii].c_str(),strSyncName,strSyncName,CmdStr.m_vCmdStr[ii].c_str(),strSyncName,strSyncName,CmdStr.m_vCmdStr[ii].c_str());
      if (stmt.execute() != 0)
      {
        logfile.Write("CREATE TRIGGER %s.\n%s\n",strTRName,stmt.cda.message); return FALSE;
      }

      logfile.Write("CREATE TRIGGER %s ok.\n",strTRName);
    }
  }

  return TRUE;
}

/*
select count(*) from T_QPEAUTOSTATION@szqx_10_153_97_14;
select count(*) from idcty.QPEAUTOSTATION@ejetdb_10_153_100_3;

select count(*) from T_QPEAUTOSTATIONDAYS@szqx_10_153_97_14;
select count(*) from idcty.QPEAUTOSTATIONDAYS@ejetdb_10_153_100_3;

select count(*) from T_QPEWIND@szqx_10_153_97_14;
select count(*) from idcty.QPEWIND@ejetdb_10_153_100_3;

select count(*) from T_QPEAUTOSTATION@szqx_10_153_97_14;
select count(*) from idcty.QPEAUTOSTATION@ejetdb_10_153_100_3;

select count(*) from T_SERVICE_PRODUCT@szqx_10_153_97_14;
select count(*) from idcty.SERVICE_PRODUCT@ejetdb_10_153_100_3;


select count(*) from T_VENUEGROUP@szqx_10_153_97_14;
select count(*) from forecast.T_VENUEGROUP@ejetdb_10_153_98_3;

select count(*) from T_VENUEINFO@szqx_10_153_97_14;
select count(*) from forecast.T_VENUEINFO@ejetdb_10_153_98_3;


select count(*) from T_VENUEFILE@szqx_10_153_97_14;
select count(*) from forecast.T_VENUEFILE@ejetdb_10_153_98_3;


select count(*) from T_PRODUCETRACE@szqx_10_153_97_14;
select count(*) from forecast.T_PRODUCETRACE@ejetdb_10_153_98_3;


select count(*) from T_BYTHEREHOURFORCAST@szqx_10_153_97_14;
select count(*) from forecast.BYTHEREHOURFORCAST@ejetdb_10_153_98_3;


select count(*) from T_BYHOURPARTONE@szqx_10_153_97_14;
select count(*) from forecast.T_BYHOURPARTONE@ejetdb_10_153_98_3;

select count(*) from T_BYHOURPARTTWO@szqx_10_153_97_14;
select count(*) from forecast.T_BYHOURPARTTWO@ejetdb_10_153_98_3;


select count(*) from T_DBYS_INDEX@szqx_10_153_97_14;
select count(*) from ybdb.DBYS_INDEX@exoa_10_153_98_6;

select count(*) from T_DBYS_DETAILS@szqx_10_153_97_14;
select count(*) from ybdb.DBYS_DETAILS@exoa_10_153_98_6;

select count(*) from T_REPLACEDIC@szqx_10_153_97_14;
select count(*) from ybdb.REPLACEDIC@exoa_10_153_98_6;


select count(*) from T_GDFPICTURE@szqx_10_153_97_14;
select count(*) from idcty.GDFPICTURE@ejetdb_10_153_98_3;

select count(*) from T_GDFPICTURE2@szqx_10_153_97_14;
select count(*) from idcty.GDFPICTURE2@ejetdb_10_153_98_3;


select count(*) from T_SMSTASK@szqx_10_153_97_14;
select count(*) from szalarm.SMSTASK@szalarm_10_153_97_30;

select count(*) from T_SMSUSERGROUP@szqx_10_153_97_14;
select count(*) from szalarm.SMSUSERGROUP@szalarm_10_153_97_30;


*/
