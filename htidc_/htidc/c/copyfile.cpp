#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;
CIniFile       IniFile;
CProgramActive ProgramActive;
CDir           SrcDir;

char strlogfilename[501];
char strsrcfilepath[501];
char strdstfilepath[501];
char strsecond[501];
char strmatchfile[501];
char strifdeletesrcfile[501];
int  itimetvl;

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\nExample:/htidc/htidc/bin/procctl 20 /htidc/htidc/bin/copyfile \"<logfilename>/log/szqx/copyfile.log</logfilename><srcfilepath>/tmp/11</srcfilepath><dstfilepath>/tmp/22</dstfilepath><second>5</second><matchfile>*.A</matchfile><ifdeletesrcfile>TRUE</ifdeletesrcfile><timetvl>-8</timetvl>\"\n\n");

    printf("�˳������ڰѱ���/����Ŀ¼�е��ļ����Ƶ�Ŀ��Ŀ¼��������֮�����ѡ���Ƿ�ɾ��Դ�ļ���\n");
    printf("logfilename: ��־�ļ�����\n");
    printf("srcfilepath: Դ�ļ�Ŀ¼��\n");
    printf("dstfilepath: Ŀ�Ĵ��Ŀ¼��\n");
    printf("seconde: ��λ:�룬����ļ���ʱ���ڵ�ǰʱ���ǰN��֮�ڣ�����ʱ������,���ڱ�֤�ļ��������ԡ�\n");
    printf("matchfile: �ļ���ƥ�䷽ʽ�����ô�д��\n");
    printf("ifdeletesrcfile: ������֮���Ƿ�ɾ��Դ�ļ���\n");
    printf("xmlbuffer���Դ���ʱ�������{�{YYYY}��4λ���꣩��{YYY}������λ���꣩��"\
           "{YY}������λ���꣩��{MM}�����£���{DD}�����գ���{HH}��ʱʱ����{MI}���ַ֣���{SS}�����룩��\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strsrcfilepath,0,sizeof(strsrcfilepath));
  memset(strdstfilepath,0,sizeof(strdstfilepath));
  memset(strsecond,0,sizeof(strsecond));
  memset(strmatchfile,0,sizeof(strmatchfile));
  memset(strifdeletesrcfile,0,sizeof(strifdeletesrcfile));
  itimetvl=0;

  char strXmlBuffer[4001];
  memset(strXmlBuffer,0,sizeof(strXmlBuffer));
  strncpy(strXmlBuffer,argv[1],4000);

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);

  // ����xmlbuffer�е�ʱ�����
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"logfilename",strlogfilename,500);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,500);
  GetXMLBuffer(strXmlBuffer,"dstfilepath",strdstfilepath,500);
  GetXMLBuffer(strXmlBuffer,"second",strsecond,500);
  GetXMLBuffer(strXmlBuffer,"matchfile",strmatchfile,500);
  GetXMLBuffer(strXmlBuffer,"ifdeletesrcfile",strifdeletesrcfile,500);
  
  if (strlen(strlogfilename) == 0)     { printf("logfilename is null.\n"); return -1;     }
  if (strlen(strsrcfilepath) == 0)     { printf("srcfilepath is null.\n"); return -1;     }
  if (strlen(strdstfilepath) == 0)     { printf("dstfilepath is null.\n"); return -1;     }
  if (strlen(strsecond) == 0)          { printf("second is null.\n"); return -1;          }
  if (strlen(strmatchfile) == 0)       { printf("matchfile is null.\n"); return -1;       }
  if (strlen(strifdeletesrcfile) == 0) { printf("ifdeletesrcfile is null.\n"); return -1; }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&logfile,"copyfile",300);

  char strLocalTime[21];
  char strTempFileName[501];
  char strStdFileName[501];

  if (SrcDir.OpenDir(strsrcfilepath) == FALSE)
  {
    logfile.Write("SrcDir.OpenDir %s failed.\n",strsrcfilepath); return -1;
  }

  while (SrcDir.ReadDir() == TRUE)
  {
    ProgramActive.WriteToFile();

    // ����ļ���ʱ���ڵ�ǰʱ���ǰN��֮�ڣ�����ʱ������
    LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-atoi(strsecond));

    if ( (strcmp(SrcDir.m_ModifyTime,strLocalTime)>0) ) continue;

    if ( (MatchFileName(SrcDir.m_FileName,strmatchfile)==FALSE) ) continue;

    logfile.Write("copy file %s",SrcDir.m_FullFileName);

    memset(strTempFileName,0,sizeof(strTempFileName));
    memset(strStdFileName,0,sizeof(strStdFileName));
    
    snprintf(strTempFileName,500,"%s/%s.tmp",strdstfilepath,SrcDir.m_FileName);
    snprintf(strStdFileName,500,"%s/%s",strdstfilepath,SrcDir.m_FileName);
    
    COPY(SrcDir.m_FullFileName,strTempFileName);
    RENAME(strTempFileName,strStdFileName);
   
    if (strcmp(strifdeletesrcfile,"TRUE") == 0) REMOVE(SrcDir.m_FullFileName);

    logfile.WriteEx(" ok.\n");
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("copyfile exit.\n");

  exit(0);
}
