#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CDir           Dir;
CFILELIST      FILELIST;

// ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3-�������ݿ��ʧ��
int _wfiletodb();

int main(int argc,char *argv[])
{
  if (argc != 6)
  {
    printf("\n");
    printf("Using:./wfiletodb logfilename connstr stdfilepath bakfilepath errfilepath\n");

    printf("Example:/htidc/htidc/bin/procctl 5 /htidc/htidc/bin/wfiletodb /log/szqx/wfiletodb_stdsz.log szidc/pwdidc@SZQX_10.153.98.13 /qxdata/szqx/wfile/stdsz /qxdata/szqx/wfile/stdbak /qxdata/szqx/wfile/stderr\n\n");
 
    printf("�������ļ����������򣬸����wfilestdpath������Ŀ¼�µ�XML�ļ���⡣\n");
    printf("wfilestdpathĿ¼�µ��ļ����󣬱�ת�Ƶ�wfilestdpathathĿ¼����deletefiles��ʱ����\n");
    printf("���XML�ļ����ʱ�����˴��󣬷���������ļ��ͻ�ת�Ƶ�wfilestdpathathĿ¼�¡�\n");
    printf("��ⷢ������ʱ��������澯��־��ϵͳ����ԱӦ�ö��ڲ鿴wfilestdpathathĿ¼����û�д����ļ���\n\n\n");
 
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

  logfile.SetAlarmOpt("wfiletodb");

  // ע�⣬����ʱ��180��
  ProgramActive.SetProgramInfo(&logfile,"wfiletodb",300);

  FILELIST.BindConnLog(&conn,&logfile);

  int  iToDBRet=0;

  char strLocalTime[20]; // ��ǰʱ���5��֮ǰ��ʱ��

  Dir.SetDateFMT("yyyymmddhh24miss");

  // �򿪱�׼��ʽ�ļ�Ŀ¼
  if (Dir.OpenDir(argv[3],TRUE) == FALSE)
  {
    logfile.Write("Dir.OpenDir %s failed.\n",argv[3]); CallQuit(-1);
  }

  // ���л�ȡÿ���ļ������
  while (Dir.ReadDir() != 0)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ����ļ���ʱ���ڵ�ǰʱ���ǰ5��֮�ڣ�����ʱ����⣬��ô����Ŀ����Ϊ�˱�֤�����ļ��������ԡ�
    LocalTime(strLocalTime,"yyyymmddhh24miss",0-5);
    if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

    if (conn.state == conn.not_connected)
    {
      // �����������ݿ�Ĵ�����������Ŀ����Ϊ��ȷ�������ļ���Ҫ���ʱ���������ݿ�
      if (conn.connecttodb(argv[2]) != 0)
      {
        logfile.Write("conn.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
      }

      // �����ļ��������
      if (FILELIST.LoadFileCFG() != 0) CallQuit(-1);
    }

    char strSTDBAKFileName[301],strSTDERRFileName[301];
    memset(strSTDBAKFileName,0,sizeof(strSTDBAKFileName));
    memset(strSTDERRFileName,0,sizeof(strSTDERRFileName));
    snprintf(strSTDBAKFileName,300,"%s/%s",argv[4],Dir.m_FileName);
    snprintf(strSTDERRFileName,300,"%s/%s",argv[5],Dir.m_FileName);

    // ��ʼ����ÿ���ļ�
    logfile.Write("Process file %s...",Dir.m_FileName);

    // ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3-�������ݿ��ʧ��
    iToDBRet=_wfiletodb();
  
    if (iToDBRet == 0)
    {
      RENAME(Dir.m_FullFileName,strSTDBAKFileName); logfile.WriteEx("ok.\n"); conn.commitwork(); continue;
    }

    conn.rollbackwork();

    // ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3-�������ݿ��ʧ��
    if (iToDBRet == 1)
    {
      RENAME(Dir.m_FullFileName,strSTDERRFileName); logfile.WriteEx("failed,invalid parameter.\n"); continue;
    }

    // ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3-�������ݿ��ʧ��
    if (iToDBRet == 2)
    {
      RENAME(Dir.m_FullFileName,strSTDERRFileName); logfile.WriteEx("failed.\n");continue;
    }

    // ����������ļ���ɾ��������Ҳ���˳���
    /*
    // ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3��������-�������ݿ��ʧ��
    RENAME(Dir.m_FullFileName,strSTDERRFileName); 

    logfile.WriteEx("failed!,database error.\n"); 

    CallQuit(-1);
    */
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("wfiletodb exit.\n");

  exit(0);
}

// ����ֵ��0-�ɹ���1-��������2-�������ļ���ʱ�䲻��ȷ��3-�������ݿ��ʧ�ܡ�
int _wfiletodb()
{
  // �ļ���
  strcpy(FILELIST.m_filename,Dir.m_FileName);
  strcpy(FILELIST.m_fullfilename,Dir.m_FullFileName);

  // �����ļ������ļ����ͻ�ȡ�����ļ����Ͳ���
  if (FILELIST.GETFILECFG() == FALSE) { logfile.WriteEx("FILELIST.GETFILECFG "); return 1; }

  if (strlen(FILELIST.m_addatetime)!=0)
  {
    DeleteRChar(FILELIST.m_addatetime,';'); // ɾ��SQL�еķֺ�

    // �н�������ʱ��ķ���
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare(FILELIST.m_addatetime);
    stmt.bindin(1,FILELIST.m_filename,100);
    stmt.bindout(1,FILELIST.m_ddatetime,14);
    stmt.execute();
    stmt.next();
  }
  else
  {
    // �ļ����а�����������ʱ������
    char *posdtime=0;

    // ������Ϊ�˼���N��ǰ���ļ����
    if ( (posdtime=strstr(FILELIST.m_filename,"_20")) != 0 ) 
    {
      strncpy(FILELIST.m_ddatetime,posdtime+1,14);
    }
    else
    {
      strcpy(FILELIST.m_ddatetime,Dir.m_ModifyTime);
    }
  }

  // �ж�����ʱ���Ƿ�Ϸ������Ƿ���dmintime�յ���ǰʱ��֮���24��Сʱ
  if (CheckDDateTime(FILELIST.m_ddatetime,FILELIST.m_dmintime) == FALSE) { logfile.WriteEx("invalid date "); return 2; }

  // �������������ļ�������ѯ�����Ƿ��Ѵ��ڸ��ļ�
  if (FILELIST.FindFExist() != 0) { logfile.WriteEx("FILELIST.CheckIfExist "); return 3; }

  FILELIST.m_IsInsertFile=FALSE;

  if (FILELIST.FindFExistNext() != 0)
  {
    // ���ļ������
    logfile.WriteEx("insert %s ",FILELIST.m_tname); 

    return FILELIST.InsertFileToDBEx();
  }
  else
  {
    // �ļ��Ѵ��ڣ��ж��Ƿ���Ҫ���º͸��µ�ʱ��
    if ( (FILELIST.m_upttype != 1) || ((FILELIST.m_timeexist > FILELIST.m_upttlimit) && (FILELIST.m_upttlimit != 0)) ) 
    {
      logfile.WriteEx("no changed "); return 0;
    }

    // �����Ѵ��ڵ��ļ�
    logfile.WriteEx("update %s ",FILELIST.m_tname); 

    return FILELIST.UpdateFileToDBEx();
  }

  return 0;
}


