#include <math.h>
#include "idcapp.h"
 
void CallQuit(int sig);
 
UINT uAPPID=0;
UINT uMaxListID=0;
UINT uTotalCount=0;
char strDstConnStr[201];  // Զ�����ݿ����Ӳ���

connection     conndst,connidc;
CLogFile       logfile;
CProgramActive ProgramActive;
CBDSYNCCFG     BDSYNCCFG;
UINT           OrgKeyID;

BOOL _bissuedata(struct st_BDSYNCCFG stBDSYNCCFG);

#define MAXCOUNT 100
UINT    uMaxCount;

// ɾ��Զ�����ݿ��û�������
BOOL DropFKey(char *strDstConnStr);

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./bissuedata logfilename connstr appid\n");
 
    printf("Example:/htidc/htidc/c/procctl 30 /htidc/htidc/c/bissuedata /log/ssqx/bissuedata_2.log qxidc/pwdidc@EJETDB_221.179.6.136  2\n");
    printf("        /htidc/htidc/c/procctl 30 /htidc/htidc/c/bissuedata /log/ssqx/bissuedata_3.log qxidc/pwdidc@EJETDB_221.179.6.136  3\n\n");
 
    printf("����ͬ���ķ�����򣬰����ݶ�ʱ�����ķַ�����ҵ��Ӧ�õ����ݿ��С�\n");
    printf("appid�ǽ������ݵ����ݿ��ID���������������ĵ�T_DAPPSERVER���ж��塣\n");
    printf("�����Ҫ�����Ŀ�����ݿ�ͬ�����ݣ��ͱ����������bissuedata����\n");
    printf("ע�⣺1.appid������T_DAPPSERVER�д��ڡ�\n");
    printf("      2.��Ҫͬ���ı�Ҫ��T_ALLTABLE�������ú�����ͬ����\n");
    printf("      3.��Ҫͬ���ı�Ҫ��T_BDSYNCCFG��\n\n\n");
 
    return -1;
  }

  uAPPID = atoi(argv[3]);

  memset(strDstConnStr,0,sizeof(strDstConnStr));
 
  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);
 
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }
 
  logfile.SetAlarmOpt("bissuedata");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"bissuedata",180);
 
  // ���������������ݿ⣬���ڶ�ȡͬ�������͸���ͬ��ʱ���Լ�����澯
  if (connidc.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  BDSYNCCFG.BindConnLog (&connidc,&logfile);

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
 
  // ����Զ�����ݿ⣬��������ͬ��
  if (conndst.connecttodb(strDstConnStr,FALSE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed\n",strDstConnStr); CallQuit(-1);
  }

  // ɾ��Զ�����ݿ��û�������
  DropFKey(strDstConnStr);
 
  // ��T_BDSYNCCFG����������Ҫִ������ͬ���ı�
  if (BDSYNCCFG.LoadBSyncTable(uAPPID) != 0)
  {
    logfile.Write("BDSYNCCFG.LoadBSyncTable failed.\n"); CallQuit(-1);
  } 

  // logfile.Write( "Info: get %d record\n", BDSYNCCFG.m_vBDSYNCCFG.size() );
  for (UINT ii=0;ii<BDSYNCCFG.m_vBDSYNCCFG.size();ii++)
  {
    logfile.Write("copy %-20s ... ",\
                   BDSYNCCFG.m_vBDSYNCCFG[ii].tname);

    if ( (BDSYNCCFG.m_vBDSYNCCFG[ii].bsynctype!=1) && (BDSYNCCFG.m_vBDSYNCCFG[ii].bsynctype!=2) )
    {
      logfile.WriteEx("failed.bsynctype is invalid.\n");
    }

    // ������ͬ�����ִ��ʱ��Ϊ��ǰʱ��
    if (BDSYNCCFG.UptBSyncTable(uAPPID,BDSYNCCFG.m_vBDSYNCCFG[ii].tname) != 0)
    {
      logfile.Write("BDSYNCCFG.UptBSyncTable failed.\n"); CallQuit(-1);
    }   
  
    if (_bissuedata(BDSYNCCFG.m_vBDSYNCCFG[ii]) == FALSE) { logfile.WriteEx("failed.\n"); continue; }

    // ���ͬ���������������£��͸�����ͬ�������ͬ����¼��λ��
    if (BDSYNCCFG.m_vBDSYNCCFG[ii].bsynctype == 2)
    {
      if (BDSYNCCFG.UptBSyncTable(uAPPID,BDSYNCCFG.m_vBDSYNCCFG[ii].tname,OrgKeyID) != 0)
      {
        logfile.Write("BDSYNCCFG.UptBSyncTable failed.\n"); CallQuit(-1);
      }   
    }

    ProgramActive.WriteToFile();
  }

  return 0;
}

BOOL _bissuedata(struct st_BDSYNCCFG stBDSYNCCFG)
{
  char strColumnStr[2048];
  memset(strColumnStr,0,sizeof(strColumnStr));

  // ��ȡ���ȫ��������Ϣ
  CTABFIELD TABFIELD;
  TABFIELD.GetALLField(&conndst,stBDSYNCCFG.tname);
  strcpy(strColumnStr,TABFIELD.m_allfieldstr);

  // ��m_stDSYNCCFG.columnsstr�е�crttime��sync_rowid�ֶ��޳�
  UpdateStr(strColumnStr,",crttime",""); UpdateStr(strColumnStr,"crttime,","");
  UpdateStr(strColumnStr,",sync_rowid",""); UpdateStr(strColumnStr,"sync_rowid,","");

  // �жϱ��Ƿ���ڣ����û������Ϣ����Ϳ϶�������
  if (strlen(strColumnStr) == 0) 
  {
    logfile.Write("table %s no exist.\n",stBDSYNCCFG.tname); return FALSE;
  }

  CTimer Timer;

  // ���ͬ��������ˢ�£�����ͬ�������Ӿ�Ϊ�գ���һ����ͬ���ñ�ȫ���ļ�¼��
  // ���ֱ�ļ�¼һ�㲻��̫�࣬Ϊ�˱�֤����һ���Ժ�Ч�ʣ���һ��SQL���̸㶨����
  // ���ң�����ͬ������Ҫkeyid�ֶΡ�
  if ( (stBDSYNCCFG.bsynctype == 1) && (strlen(stBDSYNCCFG.bsyncterm) == 0) )
  {
    sqlstatement stmt;
    stmt.connect(&conndst);
    stmt.prepare("\
      BEGIN\
         delete from %s;\
         insert into %s(%s) select %s from %s@HTIDC;\
      END;",stBDSYNCCFG.tname,stBDSYNCCFG.tname,strColumnStr,strColumnStr,stBDSYNCCFG.tname);
    if (stmt.execute() != 0)
    {
      logfile.Write("_bissuedata insert %s failed.\n%s\n%s\n",stBDSYNCCFG.tname,stmt.cda.message, stmt.m_sql);
      logfile.Write("user:%s\npassword:%s\ntnsname:%s\n", conndst.env.user,conndst.env.pass,conndst.env.tnsname );
    }
         
    logfile.WriteEx("ok,table refreshed(%ld).\n",Timer.Elapsed());

    conndst.commitwork();

    return TRUE;
  }

  char strterm[301];
  memset(strterm,0,sizeof(strterm));

  // ����ͬ�������Ӿ�
  if (stBDSYNCCFG.bsynctype == 1)
  {
    OrgKeyID=0; 
    strcpy(strterm,stBDSYNCCFG.bsyncterm);
  }
  else
  {
    OrgKeyID=stBDSYNCCFG.orgkeyid;
    sprintf(strterm," where keyid>%lu and keyid>nvl((select max(keyid)-100000 from %s@HTIDC),0) order by keyid",\
            stBDSYNCCFG.orgkeyid,stBDSYNCCFG.tname);
  }

  UINT ccount=0;
  UINT keyid,keyidn[MAXCOUNT];

  if (stBDSYNCCFG.lobfieldcount == 0) uMaxCount=MAXCOUNT;
  if (stBDSYNCCFG.lobfieldcount >  0) uMaxCount=1;

  sqlstatement selidc,delstmt,insstmt;

  selidc.connect(&conndst);
  selidc.prepare("select keyid from %s@HTIDC %s",stBDSYNCCFG.tname,strterm);
  selidc.bindout(1,&keyid);
  
  if (selidc.execute() != 0)
  {
    logfile.Write("_bissuedata select keyid from %s@HTIDC failed.\n%s\n",stBDSYNCCFG.tname,selidc.cda.message); 
    logfile.Write("%s\nuser:%s\npassword:%s\ntnsname:%s\n",
                   selidc.m_sql, conndst.env.user,conndst.env.pass,conndst.env.tnsname );
    return FALSE;
  }

  UINT ii=0;
  char strtemp[11],strInStr[4096];

  memset(strInStr,0,sizeof(strInStr));

  for (ii=0; ii<uMaxCount; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%lu",ii+1);
    if (ii >0) sprintf(strtemp,",:%lu",ii+1);
    strcat(strInStr,strtemp);
  }

  insstmt.connect(&conndst);
  insstmt.prepare("\
    BEGIN\
      delete from %s where keyid in (%s);\
      insert into %s(%s) select %s from %s@HTIDC where keyid in (%s);\
    END;",stBDSYNCCFG.tname,strInStr,stBDSYNCCFG.tname,strColumnStr,strColumnStr,stBDSYNCCFG.tname,strInStr);\
  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1,&keyidn[ii]);
  }

  // ÿuMaxCount����¼�Ͳ���һ��
  while (TRUE)
  {
    keyid=0;

    if (selidc.next() != 0) break;

    keyidn[ccount]=keyid;

    if (stBDSYNCCFG.bsynctype == 2) OrgKeyID=keyid;

    ccount++;

    if (ccount == uMaxCount)
    {
      if (insstmt.execute() != 0) 
      {
        if (insstmt.cda.rc != 1)
        {
          logfile.Write("_bissuedata insert %s failed.\n%s\n",stBDSYNCCFG.tname,insstmt.cda.message);
          logfile.Write("%s\nuser:%s\npassword:%s\ntnsname:%s\n",
                     delstmt.m_sql, conndst.env.user,conndst.env.pass,conndst.env.tnsname );
          return FALSE;
        }
      }

      ProgramActive.WriteToFile();

      conndst.commitwork();

      memset(&keyidn,0,sizeof(keyidn));

      ccount=0;
    }
  }

  // ������ѭ�������ʱ���������uMaxCount���������ﴦ��
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      BEGIN\
        delete from %s where keyid=:1;\
        insert into %s(%s) select %s from %s@HTIDC where keyid=:2;\
      END;",stBDSYNCCFG.tname,stBDSYNCCFG.tname,strColumnStr,strColumnStr,stBDSYNCCFG.tname);
    insstmt.bindin(1,&keyidn[ii]);
    insstmt.bindin(2,&keyidn[ii]);
    if (insstmt.execute() != 0) 
    {
      if (insstmt.cda.rc != 1)
      {
        logfile.Write("_bissuedata insert %s failed.\n%s\n",stBDSYNCCFG.tname,insstmt.cda.message); return FALSE;
      }
    }

    conndst.commitwork();
  }

  if (stBDSYNCCFG.bsynctype == 1) 
  {
    logfile.WriteEx("ok,%ld rows refreshed(%ld).\n",selidc.cda.rpc,Timer.Elapsed());
  }
  else
  {
    logfile.WriteEx("ok,%ld rows inserted(%ld).\n",selidc.cda.rpc,Timer.Elapsed());
  }

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  printf("catching the signal(%d).\n",sig);

  printf("tablecp exit.\n");

  exit(0);
}


// ɾ��Զ�����ݿ��û�������
BOOL DropFKey(char *strDstConnStr)
{
  char strIDCUserName[51];
  memset(strIDCUserName,0,sizeof(strIDCUserName));
  strcpy(strIDCUserName,strDstConnStr);
  char *pos=strstr(strIDCUserName,"/");
  if (pos<=0) CallQuit(-1);
  pos[0]=0;
  ToUpper(strIDCUserName);

  char strSQL[1024];
  sqlstatement stmtsel,stmtdel;

  stmtsel.connect(&conndst);
  stmtsel.prepare("select 'alter table '||table_name||' drop constraint '||constraint_name from USER_CONSTRAINTS where OWNER=:1 and constraint_type='R' and substr(table_name,1,2)='T_'");
  stmtsel.bindin(1,strIDCUserName,20);
  stmtsel.bindout(1,strSQL,300);
  stmtsel.execute();

  while (TRUE)
  {
    memset(strSQL,0,sizeof(strSQL));
    if (stmtsel.next() != 0) break;

    stmtdel.connect(&conndst);
    stmtdel.prepare(strSQL);
    stmtdel.execute();

    logfile.Write("%s\n",strSQL);
  }

  return TRUE;
}

