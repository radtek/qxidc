#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;

CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 6)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/backupapp hostname pathandfiles backuppath timeout starttime\n");  
 
    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/backupapp 59.33.252.244 \"/etc/rc.d/rc.local /etc/profile.d/sysenv.sh $ORACLE_HOME/network/admin /www/Tomcat/webapps /home/oracle/esanew\" /tmp/htidc/bak 10 02\n\n");

    printf("�˳������ڱ���ĳ��������ȫ����Ӧ�ó��������ű����������ýű���\n");
    printf("hostnameָ��Ҫ���ݵķ����������ƣ�һ����ϵͳ����IP�����ݽ�����ļ�Ϊhostname_yyyymmdd.tgz��\n");
    printf("pathandfilesָ��Ҫ���ݵ��ļ���Ŀ¼���б���Ҫ�������¼��ࣺ\n");
    printf("/etc/rc.d/rc.local /etc/profile.d/sysenv.sh oracle��tns�����ļ� tomcat��webappsĿ¼ ����Ӧ�ó���\n");
    printf("backuppathָ���ݺ��*.tgz�ļ���ŵ�Ŀ¼��\n");
    printf("timeoutִָ�б���ָ��ĳ�ʱʱ�䣬��λ�����ӡ�\n");
    printf("starttimeָ����ÿ��������ʱ�䣬ֻ֧��һ��ʱ�䣬ȡֵ��00,01,02,03,04......22,23��\n");
    printf("ע�⣬������һ��Ҫ3600������һ�Σ����ò���ϵͳ��tar����������־�ļ�Ϊ/tmp/htidc/log/backupapp.log��\n\n\n");
 
    return -1;
  }

  char strHostName[201];      // 
  char strPathAndFiles[1024];     // ��ʱ����û���
  char strBackupPath[301];    // ���ݺ��dmp�ļ���log�ļ���ŵ�Ŀ¼
  char strLocalTime[21];      // ϵͳʱ��
  char strTgzFileName[201];      // ��ű����ļ����ļ���
  char strCmdTar[2048];          // tarѹ�����
  int  iTimeOut=0;            // ��ʱʱ����

  memset(strHostName,0,sizeof(strHostName));
  memset(strPathAndFiles,0,sizeof(strPathAndFiles));
  memset(strBackupPath,0,sizeof(strBackupPath));
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strTgzFileName,0,sizeof(strTgzFileName));
  memset(strCmdTar,0,sizeof(strCmdTar));

  strcpy(strHostName,argv[1]);
  strcpy(strPathAndFiles,argv[2]);
  strcpy(strBackupPath,argv[3]);

  // �ѳ�ʱ��ʱ�䵥λתΪ��
  iTimeOut=atoi(argv[4])*60;

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);  

  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss");
  if (strncmp(strLocalTime+8,argv[5],2) != 0) return 0;

  logfile.Open("/tmp/htidc/log/backupapp.log","a+");

  logfile.SetAlarmOpt("backupapp");

  // ��ȡϵͳʱ��
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd");

  // ƴ��tarִ�����
  snprintf(strCmdTar,2000,"tar zcvf %s/%s_%s.tgz %s 1>/dev/null 2>/dev/null",strBackupPath,strHostName,strLocalTime,strPathAndFiles);

  // ע�⣬����ʱ��iTimeOut��
  ProgramActive.SetProgramInfo(&logfile,"backupapp",iTimeOut);

  // ��ʼ����
  logfile.Write("%s ...",strCmdTar);

  // ִ�б���tgz���
  system(strCmdTar);

  // ��ɵ���ѹ��
  logfile.WriteEx("ok\n"); 

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
 
  logfile.Write("backupapp exit.\n");
 
  exit(0);
}
