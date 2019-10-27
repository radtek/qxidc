#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;

char strEXPFilePath[201];

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./htidc/htidc/bin/backuptables logfilepath connstr expfilepath\n"); 

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/backuptables /log/ssqx/backuptables.log qxidc/pwdidc@EJETDB_221.179.6.136 /qxdata/ssqx/dmp\n\n");

    printf("�˳��������Զ������������������ݿ⣨�������ݿⲻ��Ҫ���ݣ��ı��������£�\n"); 
    printf("1��ˢ�������ֵ��T_ALLTABLE�������������ֵ��T_SEQANDTABLE�ļ�¼��\n"); 
    printf("2����ȡT_ALLTABLE���еļ�¼��������ifbackup=1 and (sysdate-backuptime>backuptvl or backuptime is null)��\n");
    printf("3���жϱ����Ƿ���T_��ͷ������Ǿͱ����������ǾͲ����ݡ�\n");
    printf("4������ʱ����DMPDAT_DDATETIME_TNAME.dmp�ļ���DMPLOG_DDATETIME_TNAME.log�����ļ���ѹ����\n");
    printf("5���������ɵ������ļ�����־�ļ������expfilepathĿ¼�С�\n\n\n");

    return -1;
  }

  memset(strEXPFilePath,0,sizeof(strEXPFilePath));

  strcpy(strEXPFilePath,argv[3]);

  MKDIR(strEXPFilePath);

  
  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("backuptables");

  ProgramActive.SetProgramInfo(&logfile,"backuptables",60);

  ALLTABLE.BindConnLog(&conn,0,&logfile,&ProgramActive);

  if (conn.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // ע�⣬����ʱ��7200��
  ProgramActive.SetProgramInfo(&logfile,"backuptables",7200);

  // ��TAB�����ֵ��д��ڣ�  ��T_ALLTABLE���в����ڵļ�¼����T_ALLTABLE��
  // ��TAB�����ֵ��в����ڣ���T_ALLTABLE���д��ڵļ�¼ɾ����
  // ��USER_SEQUENCES�����ֵ��д��ڣ�  ��T_SEQANDTABLE���в����ڵļ�¼����T_SEQANDTABLE
  // ��USER_SEQUENCES�����ֵ��в����ڣ���T_SEQANDTABLE���д��ڵļ�¼ɾ����
  if (ALLTABLE.UpdateALLTABLEAndSEQ() != 0)
  {
    logfile.Write("ALLTABLE.UpdateALLTABLEAndSEQ failed.\n"); CallQuit(-1);
  }

  // ��ȡȫ����Ҫ���ݵļ�¼��������m_vALLTABLE��
  if (ALLTABLE.LoadEXPRecord() != 0)
  {
    logfile.Write("ALLTABLE.LoadEXPRecord failed.\n"); CallQuit(-1);
  }

  for (UINT ii=0; ii<ALLTABLE.m_vALLTABLE.size(); ii++)
  {
    ProgramActive.WriteToFile();

    // ���ݵı�ı�����������"T_"��Ϊǰ׺
    if (strncmp(ALLTABLE.m_vALLTABLE[ii].tname,"T_",2) != 0) continue;

    logfile.Write("backup %s ...",ALLTABLE.m_vALLTABLE[ii].tname);

    if (ALLTABLE.ExpTable(strEXPFilePath,strEXPFilePath,argv[2],ALLTABLE.m_vALLTABLE[ii].tname) == FALSE)
    {
      logfile.WriteEx("failed\n"); 
    }
    else
    {
      logfile.WriteEx("ok\n");
    }

    // ����T_ALLTABLE��ı���ʱ���ֶ�
    if (ALLTABLE.UptBackupTime(ALLTABLE.m_vALLTABLE[ii].tname) != 0)
    {
      logfile.Write("ALLTABLE.UptBackupTime failed.\n"); CallQuit(-1);
    }
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("backuptables exit.\n");

  exit(0);
}

