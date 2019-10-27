#include "_public.h"

CDir           Dir; 
CLogFile       Logfile;
CProgramActive ProgramActive;
CCmdStr        CmdStr;
CFile          File;

char strlogfile[301];
char strsrcfilepath[301];
char strstdfilepath[301];
char strmatchname[301];
char strstarthour[101];

void CallQuit(int sig);

int main( int argc,char *argv[])
{
  if(argc!=2)
  {
    printf("Using: /htidc/htidc/bin/RenameFile xmlbuffer\n");
    printf("Using: /htidc/htidc/bin/RenameFile \"<logfile>/log/gzqx/RenameFile.log</logfile><srcfilepath></srcfilepath><stdfilepath></stdfilepath><matchname>*.GIF</matchname><starthour></starthour>\"\n");

    printf("����һ�����߳���,���ڸ��ļ���\n");
    printf("�кܶ�ķǽṹ�ļ��������ļ����ǹ̶�����Ҫ�����ļ���֮�������\n");
    printf("�ļ�ԭ��Ϊcsnl.html������Ϊ��csnl20151214.html\n");
    printf("������߳���ֻ��Թ��ݳ������Ժ͹������������ļ����д�����Ϊ�ӿͻ������������ݣ��ļ����ǹ̶���,\n");
    printf("�������ϴ������������п���������ģ�������Ҫ�Ƚ��и�ʽת�����鿴�������ݣ�Ȼ�����ж���ʲôʱ������ݡ�\n");
    printf("ֻ�ж��ǲ��ǽ���ģ�������ǽ���ľͲ�����Ҳ��ɾ��\n");
    printf("starthour ��Сʱ���б��ö��Ÿ���������Ϊ��\n");
   
    return -1; 
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001];

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[1],4000);

  memset(strlogfile,0,sizeof(strlogfile));
  memset(strsrcfilepath,0,sizeof(strsrcfilepath));
  memset(strstdfilepath,0,sizeof(strstdfilepath));
  memset(strmatchname,0,sizeof(strmatchname));
  memset(strstarthour,0,sizeof(strstarthour));

  GetXMLBuffer(strXmlBuffer,"logfile",strlogfile,300);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,300);
  GetXMLBuffer(strXmlBuffer,"stdfilepath",strstdfilepath,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"starthour",strstarthour,100);

  // ����־�ļ� 
  if (Logfile.Open(strlogfile,"a+") == FALSE) 
  {   
    printf("Logfile.Open(%s) failed.\n",argv[1]); return -1; 
   }

  if (strlen(strlogfile) == 0)     { Logfile.Write("logfile is null.\n"); return -1; }
  if (strlen(strsrcfilepath) == 0) { Logfile.Write("srcfilepath is null.\n"); return -1; }
  if (strlen(strstdfilepath) == 0) { Logfile.Write("stdfilepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0)   { Logfile.Write("matchname is null.\n"); return -1; }
 
  // ��ȡ��������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬���˳�
  if (strlen(strstarthour) != 0)
  {
     char Time[21];
     memset(Time,0,sizeof(Time));
     LocalTime(Time,"hh24mi");
     Time[2]=0;
     if (strstr(strstarthour,Time) == 0) return 0;
  }
  
  Logfile.SetAlarmOpt("RenameFile");

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&Logfile,"RenameFile",300);
  
  // ��Ҫ������ļ�Ŀ¼
  if (Dir.OpenDir(strsrcfilepath,TRUE) == FALSE)
  {
    Logfile.Write("Dir.OpenDir %s failed.\n",argv[2]); return -1;
  }

  char strcmd[1024];
  char strBuffer[8001];
  char strLocalTime[21];
  char strStdFileName[301];
  char mm[3],dd[3];
  char date[31];

  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(date,0,sizeof(date));

  LocalTime(strLocalTime,"yyyymmdd");
 
  strncpy(mm,strLocalTime+4,2);
  strncpy(dd,strLocalTime+6,2);
  snprintf(date,30,"%s��%s��",mm,dd); 
    
  while(Dir.ReadDir()==TRUE) 
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();
 
    if (MatchFileName(Dir.m_FileName,strmatchname) == FALSE) continue;	
    
    // ��ʼ�����ļ�
    Logfile.Write("process file %s...",Dir.m_FileName);

    memset(strcmd,0,sizeof(strcmd));
    snprintf(strcmd,1000,"iconv -c -f utf-8 -t gb18030 %s -o /tmp/htidc/tmp/gztmpfile.txt",Dir.m_FullFileName);
    system(strcmd);

    // ��ת���ļ�
    if ((File.OpenForRead("/tmp/htidc/tmp/gztmpfile.txt","r")) == FALSE)
    {
       Logfile.Write("OpenForRead(/tmp/htidc/tmp/gztmpfile.txt) failed.\n"); return FALSE;
    }
    while(TRUE)
    {
      memset(strBuffer,0,sizeof(strBuffer));
      if (File.FFGETS(strBuffer,8000) == FALSE) break;

      // ���û���ҵ�����������վ�˵�����ǽ�����ļ�
      if (strstr(strBuffer,date) == 0) continue;

      CmdStr.SplitToCmd(Dir.m_FileName,".");

      memset(strStdFileName,0,sizeof(strStdFileName));
      sprintf(strStdFileName,"%s/%s%s.%s",strstdfilepath,CmdStr.m_vCmdStr[0].c_str(),strLocalTime,CmdStr.m_vCmdStr[1].c_str());

      if (RENAME(Dir.m_FullFileName,strStdFileName) == FALSE) 
      {
         Logfile.Write("failed.RENAME %s to %s failed.\n",Dir.m_FullFileName,strStdFileName);
         return -1;
      }

      Logfile.Write("to %s...ok.\n",strStdFileName);
    }

    REMOVE("/tmp/htidc/tmp/gztmpfile.txt");
  }

  return 0;
}

  void CallQuit(int sig)
  {
  if (sig > 0) signal(sig,SIG_IGN);

  Logfile.Write("catching the signal(%d).\n",sig);

  Logfile.Write("RenameFile exit.\n");

  exit(0);
}

