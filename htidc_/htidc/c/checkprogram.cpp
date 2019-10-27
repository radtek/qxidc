#include "_public.h"

CLogFile       logfile;
CDir           ProcDir;
CProgramActive ProgramActive;

// ��/tmpĿ¼�´���htidc�͸�����Ŀ¼
void mkdirhtidcdirs();

// ����������Ҫһ�����������ӿ���ϵͳ�ĸ�Ŀ¼
int main(int argc,char *argv[])
{
  if (argc != 1)
  {
    printf("\n");
    printf("Using:./htidc/htidc/bin/checkprogram\n"); 

    printf("Example:/htidc/htidc/bin/procctl 60 /htidc/htidc/bin/checkprogram\n\n");

    printf("�˳������ڼ���̨Ӧ�ó����Ƿ�ʱ������ѳ�ʱ����ɱ������\n");
    printf("�˳����������κ��к�̨�������еķ������ϣ���procctl���ȣ�������־�ļ���Ϊ/tmp/htidc/log/checkprogram.log��\n");
    printf("���������ģ�����صĺ�̨�������ɵ�active�ļ������/tmp/htidc/procĿ¼�У���_P_A_��ͷ��\n\n");

    printf("ע�⣬�����κγ��򶼿��Ա�checkprogram��أ�����صĺ���������밴Ҫ��дactive�ļ���\n");
    printf("����active�ļ��ķ�����������ܹ�ͨ��\n\n\n");
 
    return 0;
  }

  // �ر�ȫ�����źź����������������ֻ����killall -9 checkprogram��ֹ��
  CloseIOAndSignal(); 

  // ��/tmpĿ¼�´���htidc�͸�����Ŀ¼
  mkdirhtidcdirs();

  if (logfile.Open("/tmp/htidc/log/checkprogram.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/checkprogram.log) failed.\n"); return -1;
  }

  logfile.SetAlarmOpt("checkprogram");

  ProgramActive.m_logfile=&logfile;

  char strPIDNode[201];

  if (ProcDir.OpenDirNoSort("/tmp/htidc/proc") == FALSE)
  {
    logfile.Write("Dir.Open(/tmp/htidc/proc) FAILED.\n");  return -1;
  }

  while (ProcDir.ReadDir() == TRUE)
  {
    // ������ǽ��̻��Ϣ�ļ�����ɾ������ļ�
    if (MatchFileName(ProcDir.m_FileName,"_P_A_*") == FALSE) 
    {
      logfile.Write("invalied file %s.\n",ProcDir.m_FullFileName);
      REMOVE(ProcDir.m_FullFileName); 
      continue;
    }

    // ��ȡ�����ļ�����Ϣ
    if (ProgramActive.ReadFromFile(ProcDir.m_FullFileName) == FALSE) 
    {
      logfile.Write("ProgramActive.ReadFromFile(%s) FAILED.\n",ProcDir.m_FileName); 
      REMOVE(ProcDir.m_FullFileName); 
      continue;
    }

    // ��ɽ�����/procĿ¼�µ���ʱ�ļ��ڵ����������жϽ����Ƿ񻹴��ڡ�
    memset(strPIDNode,0,sizeof(strPIDNode));
    snprintf(strPIDNode,200,"/proc/%d",ProgramActive.m_PID);

    // ��������Ѳ����ڣ�ֱ��ɾ������ļ���
    if (access(strPIDNode,R_OK) != 0) { REMOVE(ProcDir.m_FullFileName); continue; }

    // �Ѿ���ʱ
    if (ProgramActive.m_Elapsed >= ProgramActive.m_MaxTimeOut)
    {
      logfile.Write("pid=%d,cmd=%s,%d>%d\n",ProgramActive.m_PID,ProgramActive.m_ProgramName,\
                     ProgramActive.m_Elapsed,ProgramActive.m_MaxTimeOut);

      // ��������˳��ź�
      kill(ProgramActive.m_PID,SIGINT);

      // �ȴ�5�룬5�����ǿ��ɱ��
      sleep(5);
    
      kill(ProgramActive.m_PID,SIGKILL); 

      // ɾ�����̻��Ϣ�ļ���
      REMOVE(ProcDir.m_FullFileName);
    }
  }

  return 0;
}


// ��/tmpĿ¼�´���htidc�͸�����Ŀ¼
void mkdirhtidcdirs()
{
  // ���/tmp/htidc�Լ�����Ŀ¼�����ڣ��ʹ�����

  if (access("/tmp/htidc",F_OK) != 0)           mkdir("/tmp/htidc",00777);
  if (access("/tmp/htidc/proc",F_OK) != 0)      mkdir("/tmp/htidc/proc",00777);
  if (access("/tmp/htidc/log",F_OK) != 0)       mkdir("/tmp/htidc/log",00777);
  if (access("/tmp/htidc/bak",F_OK) != 0)       mkdir("/tmp/htidc/bak",00777);
  if (access("/tmp/htidc/tmp",F_OK) != 0)       mkdir("/tmp/htidc/tmp",00777);
  if (access("/tmp/htidc/list",F_OK) != 0)      mkdir("/tmp/htidc/list",00777);
  if (access("/tmp/htidc/ftpput",F_OK) != 0)    mkdir("/tmp/htidc/ftpput",00777);
  if (access("/tmp/htidc/ftpputbak",F_OK) != 0) mkdir("/tmp/htidc/ftpputbak",00777);
  if (access("/tmp/htidc/ftpget",F_OK) != 0)    mkdir("/tmp/htidc/ftpget",00777);
  if (access("/tmp/htidc/alarmxml",F_OK) != 0)  mkdir("/tmp/htidc/alarmxml",00777);

  // һ��Ҫ��Ϊ777Ȩ�ޣ���Ϊ��ЩĿ¼��д�ļ��ĳ���һ����root�û�������
  // ��������û�����û��д��Ȩ�ޣ��ͻᵼ�³���ʧ��
  system("/bin/chmod -R 777 /tmp/htidc");

  return;
}
