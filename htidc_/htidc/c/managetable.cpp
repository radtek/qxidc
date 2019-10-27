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
    printf("Using:/htidc/htidc/bin/managetable logfilename connstr appid\n\n");

    printf("Example:/htidc/htidc/bin/procctl 14400 /htidc/htidc/bin/managetable /log/ssqx/managetable_1.log qxidc/pwdidc@EJETDB_221.179.6.136 1\n");
    printf("        /htidc/htidc/bin/procctl 14400 /htidc/htidc/bin/managetable /log/ssqx/managetable_2.log qxidc/pwdidc@EJETDB_221.179.6.136 5\n\n");

    printf("�����������ļ�Ⱥ������ȫ�������ݵı��ݡ��鵵������\n");
    printf("���ݹ���Ĳ����������ֵ��T_APPTABLE�е�hdataptype��hdatapcfg������\n");
    printf("appid�Ǵ���������ݿ⣬�������Ǽ�Ⱥ�е����ݿ�֮һ��\n");
    printf("�����Ҫ�Զ����Ⱥ���ݿ���й����ͱ������ж��managetable����\n\n\n");
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
  logfile.SetAlarmOpt("managetable");

  // ע�⣬����ʱ��60��
  ProgramActive.SetProgramInfo(&logfile,"managetable",60);

  // �����������ĵļ�Ⱥ���ݿ�
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

  // ���Ӵ��������ݵ����ݿ�
  if (conndst.connecttodb(strDstConnStr,TRUE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed.\n",strDstConnStr); CallQuit(-1);
  }

  // ע�⣬�������ݿ�󣬳���ʱ��Ϊ3600��
  ProgramActive.SetProgramInfo(&logfile,"managetable",3600);

  ALLTABLE.BindConnLog(&connidc,&conndst,&logfile,&ProgramActive);

  CTABFIELD TABFIELD;

  // ���ü�Ⱥ��������ID
  ALLTABLE.m_appid=uAPPID;

  // ��T_APPTABLE��������ĳ��Ⱥ��������Ҫ����ı�ļ�¼���������ݹ���
  if (ALLTABLE.LoadForManager() != 0)
  {
    logfile.Write("ALLTABLE.LoadForManager failed.\n"); CallQuit(-1);
  }

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (ALLTABLE.LoadAllRecordNext() != 0) break;

    // ��ȡ���ȫ��������Ϣ
    if (TABFIELD.GetALLField(&conndst,ALLTABLE.m_stALLTABLE.tname) == FALSE)
    {
      logfile.Write("TABFIELD.GetALLField failed.\n"); CallQuit(-1);
    }

    memset(ALLTABLE.m_COLUMNSSTR,0,sizeof(ALLTABLE.m_COLUMNSSTR));
    strcpy(ALLTABLE.m_COLUMNSSTR,TABFIELD.m_allfieldstr);

    logfile.Write("%s\n",ALLTABLE.m_stALLTABLE.tname);

    while (TRUE)
    {
      ProgramActive.WriteToFile();

      // �ѻ����ݱ�ļ�¼װ������
      if (ALLTABLE.LoadALLROWID() != 0)
      {
        logfile.Write("ALLTABLE.LoadALLROWID failed.\n"); CallQuit(-1);
      }

      ProgramActive.WriteToFile();

      // �鵵
      if (ALLTABLE.m_stALLTABLE.hdataptype == 0)
      {
        if (ALLTABLE.BackupToATable() != 0)
        {
          logfile.Write("ALLTABLE.BackupToATable failed.\n"); CallQuit(-1);
        }
      }
   
      // ����
      if (ALLTABLE.m_stALLTABLE.hdataptype == 1)
      {
        if (ALLTABLE.BackupToHTable() != 0)
        {
          logfile.Write("ALLTABLE.BackupToHTable failed.\n"); CallQuit(-1);
        }
      }

      // ɾ��
      if (ALLTABLE.m_stALLTABLE.hdataptype == 2)
      {
        if (ALLTABLE.DeleteTable() != 0)
        {
          logfile.Write("ALLTABLE.DeleteTable failed.\n"); CallQuit(-1);
        }
      }

      // ���������ļ�¼С��50000����ʾ�ñ���Ҫ����������ѱ������꣬���˳������ѭ��
      if (ALLTABLE.m_stALLTABLE.totalcount<50000 ) break;
    }

    logfile.WriteEx("\n");
  }

  logfile.Write("ok.\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("managetable exit.\n");

  exit(0);
}

