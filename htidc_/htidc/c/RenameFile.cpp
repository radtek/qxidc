#include "_public.h"

CDir           Dir; 
CLogFile       Logfile;
CProgramActive ProgramActive;
CCmdStr        CmdStr;

char strlogfile[301];
char strsrcfilepath[301];
char strstdfilepath[301];
char strmatchname[301];
char strstarthour[101];
char strfilename[301];

void CallQuit(int sig);

int main( int argc,char *argv[])
{
  if(argc!=2)
  {
    printf("Using: /htidc/htidc/bin/RenameFile xmlbuffer\n");
    printf("Using: /htidc/htidc/bin/RenameFile \"<logfile>/log/gzqx/RenameFile.log</logfile><srcfilepath>/tmp</srcfilepath><stdfilepath>/tmp/11</stdfilepath><matchname>*.GIF</matchname><filename>FY2G_IR1_*.GIF</filename><starthour></starthour>\"\n");
    printf("Using: /htidc/htidc/bin/RenameFile \"<logfile>/log/gzqx/RenameFile.log</logfile><srcfilepath>/tmp</srcfilepath><stdfilepath>/tmp/11</stdfilepath><matchname>*.GIF</matchname><filename>FY2G_IR1_yyymmddhh24miss_*.GIF</filename><starthour></starthour>\"\n");

    printf("����һ�����߳���,���ڸ��ļ���\n");
    printf("�кܶ�ķǽṹ�ļ��������ļ����ǹ̶�����Ҫ�����ļ���֮�������\n");
    printf("srcfilepath ����Դ�ļ����Ŀ¼\n");
    printf("stdfilepath �ļ���������Ŀ¼\n");
    printf("filename �����ļ�����*����ԭ�ļ���,ͬʱ����ƥ��ʱ�䣬Ŀǰ֧��:yyyymmddhh24miss,yyyymmdd,hh24miss\n");
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
  memset(strfilename,0,sizeof(strfilename));

  GetXMLBuffer(strXmlBuffer,"logfile",strlogfile,300);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,300);
  GetXMLBuffer(strXmlBuffer,"stdfilepath",strstdfilepath,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"starthour",strstarthour,100);
  GetXMLBuffer(strXmlBuffer,"filename",strfilename,300);

  // ����־�ļ� 
  if (Logfile.Open(strlogfile,"a+") == FALSE) 
  {   
    printf("Logfile.Open(%s) failed.\n",argv[1]); return -1; 
   }

  if (strlen(strlogfile) == 0)     { Logfile.Write("logfile is null.\n"); return -1;     }
  if (strlen(strsrcfilepath) == 0) { Logfile.Write("srcfilepath is null.\n"); return -1; }
  if (strlen(strstdfilepath) == 0) { Logfile.Write("stdfilepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0)   { Logfile.Write("matchname is null.\n"); return -1;   }
  if (strlen(strfilename) == 0)     { Logfile.Write("filename is null.\n"); return -1;    }
 
  // ��ȡ��������ʱ�䣬�ж��Ƿ�����ó������ʱ���е�Сʱ��ƥ�䣬���˳�
  if (strlen(strstarthour) != 0)
  {
     char strLocalTime[21];
     memset(strLocalTime,0,sizeof(strLocalTime));
     LocalTime(strLocalTime,"hh24");
     if (strstr(strstarthour,strLocalTime) == 0) return 0;
  }
  
  Logfile.SetAlarmOpt("RenameFile");

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&Logfile,"RenameFile",300);
  
  // ��Ҫ������ļ�Ŀ¼
  if (Dir.OpenDir(strsrcfilepath,TRUE) == FALSE)
  {
    Logfile.Write("Dir.OpenDir %s failed.\n",argv[2]); return -1;
  }

  char strname[301];
  char strLocalTime[21];
    
  while(Dir.ReadDir()==TRUE) 
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();
 
    if (MatchFileName(Dir.m_FileName,strmatchname) == FALSE) continue;	
    
    // ��ʼ�����ļ�
    Logfile.Write("process file %s...",Dir.m_FileName);

    memset(strname,0,sizeof(strname));
    strcpy(strname,strfilename);

    CmdStr.SplitToCmd(Dir.m_FileName,".");

    memset(strLocalTime,0,sizeof(strLocalTime));
    
    if(strstr(strname,"yyyymmddhh24miss") !=0)  
    { 
      LocalTime(strLocalTime,"yyyymmddhh24miss");
      UpdateStr(strname,"yyyymmddhh24miss",strLocalTime);
    }
    else if (strstr(strname,"yyyymmdd") !=0)  
    {
      LocalTime(strLocalTime,"yyyymmdd");
      UpdateStr(strname,"yyyymmdd",strLocalTime);
    }
    else if (strstr(strname,"hh24miss") !=0) 
    {
      LocalTime(strLocalTime,"hh24miss");
      UpdateStr(strname,"hh24miss",strLocalTime);
    }
    
    UpdateStr(strname,"*",CmdStr.m_vCmdStr[0].c_str());
  
    char strStdFileName[301];
    memset(strStdFileName,0,sizeof(strStdFileName));
    sprintf(strStdFileName,"%s/%s",strstdfilepath,strname);

    if (RENAME(Dir.m_FullFileName,strStdFileName) == FALSE) 
    {
       Logfile.Write("failed.RENAME %s to %s failed.\n",Dir.m_FullFileName,strStdFileName);
       return -1;
    }
    Logfile.Write("to %s...ok.\n",strStdFileName);
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

