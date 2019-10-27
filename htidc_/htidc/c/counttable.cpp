#include "idcapp.h"

void CallQuit(int sig);

char strDBLinkName[51]; 
UINT uAPPID;

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./counttable logfilename connstr appid\n");

    printf("Example:/htidc/htidc/bin/procctl 86400 /htidc/htidc/bin/counttable /log/ssqx/counttable_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 86400 /htidc/htidc/bin/counttable /log/ssqx/counttable_2.log qxidc/pwdidc@EJETDB_221.179.6.136 2\n\n");

    printf("�˳��������Զ�ͳ���������ļ�Ⱥ�и����ݿ�ı���Ϣ��\n");
    printf("inifile��ָ�����������������ݿ�����Ӳ�������־�ļ�����ʱ�ļ���Ŀ¼�Ȳ�����\n");
    printf("appid�Ǵ���������ݿ⣬�������Ǽ�Ⱥ�е����ݿ�֮һ��\n");
    printf("�����Ҫ�Զ����Ⱥ���ݿ���й����ͱ������ж��counttable����\n");
    printf("ע�⣺appid������T_DAPPSERVER�д��ڡ�\n\n\n");
 
    return -1;
  }

  uAPPID=atoi(argv[3]);

  memset(strDBLinkName,0,sizeof(strDBLinkName));

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("counttable");

  // ע�⣬�������ݿ�ĳ�ʱʱ����60��
  ProgramActive.SetProgramInfo(&logfile,"counttable",60);

  ALLTABLE.BindConnLog(&conn,0,&logfile,&ProgramActive);

  if (conn.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // ע�⣬����ʱ��21600��
  ProgramActive.SetProgramInfo(&logfile,"counttable",21600);

  // ���������ĵ�T_DAPPSERVER����ȡ������ķ����������ݿ����Ӳ���
  if (findbypk(&conn,"T_DAPPSERVER","appid","dblinkname",uAPPID,strDBLinkName,50) != 0)
  {
    logfile.Write("Call findbypk failed\n"); CallQuit(-1);
  }

  logfile.Write("DBLinkName=%s\n",strDBLinkName);

  // ���ü�Ⱥ��������ID
  ALLTABLE.m_appid=uAPPID;

  // ��T_APPTABLE��������ĳ��Ⱥ��������ȫ����¼
  if (ALLTABLE.LoadForCount() != 0)
  {
    logfile.Write("ALLTABLE.LoadForCount failed.\n"); CallQuit(-1);
  }

  /*
  char strIDCUserName[51];
  memset(strIDCUserName,0,sizeof(strIDCUserName));
  strcpy(strIDCUserName,argv[2]);
  char *pos=strstr(strIDCUserName,"/");
  if (pos<=0) CallQuit(-1);
  pos[0]=0;
  ToUpper(strIDCUserName);
  */

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (ALLTABLE.LoadAllRecordNext() != 0) break;

    // ͳ�Ʊ�ļ�¼��
    if (ALLTABLE.CountTable(strDBLinkName) != 0)
    {
      logfile.Write("ALLTABLE.CountTable FAILED.\n");
    }

    // ����T_ALLTABLE��ļ�¼������ͳ��ʱ���ֶΡ���Ĵ�С�������Ĵ�С
    if (ALLTABLE.UptCountTime(strDBLinkName) != 0)
    {
      logfile.Write("ALLTABLE.UptCountTime failed.\n");
    }

    logfile.Write("tname=%s,%d\n",ALLTABLE.m_stALLTABLE.tname,ALLTABLE.m_stALLTABLE.totalcount);
  }

  logfile.WriteEx("\n\n\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("counttable exit.\n");

  exit(0);
}

