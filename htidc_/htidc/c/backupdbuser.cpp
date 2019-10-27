#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;

CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 7)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/backupdbuser hostname username/password@tnsname backuppath timeout starttime rows\n");  
 
    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/backupdbuser 59.33.252.244 scott/tiger /tmp/htidc/bak 10 Y\n\n");

    printf("�˳������exp����ָ�����ݿ���û���ȫ�����󣬰����˱���ͼ�������������еȡ�\n");
    printf("hostnameָ��Ҫ���ݵķ����������ƣ�һ����ϵͳ����IP�����ݽ�����ļ�Ϊhostname_yyyymmdd.*.gz��\n");
    printf("username/password@tnsname������Ҫ���ݵ����ݿ���û��������롣\n");
    printf("backuppath���ݺ��dmp�ļ���log�ļ���ŵ�Ŀ¼��\n");
    printf("timeoutִָ�б���ָ��ĳ�ʱʱ�䣬��λ�����ӡ�\n");
    printf("starttimeָ����ÿ��������ʱ�䣬ֻ֧��һ��ʱ�䣬ȡֵ��00,01,02,03,04......22,23��\n");
    printf("rows�Ƿ񱸷ݳ����ݣ�ȡֵY��N��\n");
    printf("ע�⣬������һ��Ҫ3600������һ�Σ����ò���ϵͳ��exp����������־�ļ�Ϊ/tmp/htidc/log/backupdbuser_username.log��\n\n\n");
 
    return -1;
  }

  char strPUserName[201];   // �û���������
  char strUserName[51];     // ��ʱ����û���
  char strBackupPath[301];  // ���ݺ��dmp�ļ���log�ļ���ŵ�Ŀ¼
  char strLocalTime[21];    // ϵͳʱ��
  char strFileName[201];    // ��ű����ļ����ļ���
  char strLogName[201];     // ��ű�����־����־�ļ�
  char strFullLogName[201]; // ��ű�����־����־ȫ·��
  char strExp[501];         // exp���
  char strGzip[501];        // gzipѹ�����
  int  iTimeOut=0;          // ��ʱʱ����
  char *p=0;

  memset(strPUserName,0,sizeof(strPUserName));
  memset(strFileName,0,sizeof(strFileName));
  memset(strFullLogName,0,sizeof(strFullLogName));
  memset(strGzip,0,sizeof(strGzip));

  strcpy(strPUserName,argv[2]);
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

  // ��ȡϵͳʱ��
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmdd");

  // ��ȡ��ŵ��ļ�������־�ļ���
  memset(strUserName,0,sizeof(strUserName));
  p=strstr(strPUserName,"/");
  strncpy(strUserName,strPUserName,strlen(strPUserName)-strlen(p));
  sprintf(strFileName,"%s_%s_%s.dmp",argv[1],strUserName,strLocalTime);
  sprintf(strLogName,"%s_%s_%s.log",argv[1],strUserName,strLocalTime);
  
  // ��ȡ��־�ļ�ȫ·��
  sprintf(strFullLogName,"%s/%s",strBackupPath,strLogName);

  // ƴ��expִ�����
  sprintf(strExp,"exp %s file=%s/%s log=%s rows=%s 1>/dev/null 2>/dev/null",strPUserName,strBackupPath,strFileName,strFullLogName,argv[6]);

  // ����־�ļ�
  char strLogfile[201];   
  memset(strLogfile,0,sizeof(strLogfile)); 
  sprintf(strLogfile,"/tmp/htidc/log/backupdbuser_%s.log",strUserName);
  if (logfile.Open(strLogfile,"a+") == FALSE) 
  {
    printf("logfile.Open backupdbuser_%s.log filed",strUserName); CallQuit (-1);
  }

  logfile.SetAlarmOpt("backupdbuser");

  // ע�⣬����ʱ��iTimeOut��
  ProgramActive.SetProgramInfo(&logfile,"backupdbuser",iTimeOut);

  // ��ʼ����
  logfile.Write("backup %s/%s ...",strBackupPath,strFileName);

  // ִ�б���exp���
  system(strExp);

  // �򿪱��ݵ���־�ļ�
  if(CheckFileSTS(strFullLogName,"û�г��־��档") == FALSE)
  {
    if(CheckFileSTS(strFullLogName,"successfully without warnings.") == FALSE)
    {
      logfile.WriteEx("failed.\n"); logfile.Write("%s\n",strExp); CallQuit (-1); 
    }
  }

  char strDmpFileName[301];
  memset(strDmpFileName,0,sizeof(strDmpFileName));
  snprintf(strDmpFileName,300,"%s/%s",strBackupPath,strFileName);

  // ִ��ѹ������
  sprintf(strGzip,"/usr/bin/gzip -c %s > %s.tmp 2>/dev/null",strDmpFileName,strDmpFileName);

  system(strGzip);

  REMOVE(strDmpFileName);
  
  strncat(strDmpFileName,".tmp",4);

  char strDmpFileNameGZ[301];
  memset(strDmpFileNameGZ,0,sizeof(strDmpFileNameGZ));
  snprintf(strDmpFileNameGZ,300,"%s/%s.gz",strBackupPath,strFileName);

  // ���ļ�����
  if (RENAME(strDmpFileName,strDmpFileNameGZ) == FALSE)
  {
    logfile.Write("RENAME %s to %s failed.\n",strDmpFileName,strDmpFileNameGZ); return -1;
  }

  // ��ɵ���ѹ��
  logfile.WriteEx("ok.\n"); 

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);
 
  logfile.Write("catching the signal(%d).\n",sig);
 
  logfile.Write("backupdbuser exit.\n");
 
  exit(0);
}
