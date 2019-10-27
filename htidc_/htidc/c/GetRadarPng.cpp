#include "_public.h"

void EXIT(int sig);

char strlogfilename[301];  // ��������־�ļ���
char strstdpath[301];      // �ļ���Ŀ��·��
char strtemppath[301];
char strfilename[301];
char strxmlbuffer[4001];
char strbuffer[4001];

char strLocalTime[21],yyyy[5],mm[3],dd[3],hh[3];

CFile          File;
CDir           Dir;
CLogFile       logfile;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  
  if ( argc != 2 )
  {
    printf("\n");
    printf("Using:/htidc/qxidc/bin/GetRadarPng xmlbuffer\n");

    printf("Example:/htidc/htidc/bin/procctl_ssqx 60 /htidc/qxidc/bin/GetRadarPng \"<logfilename>/log/ssqx/GetRadarPng.log</logfilename><temppath>/tmp/22</temppath><stdpath>/qxdata/ssqx/sdata/std1</stdpath>\"\n\n");
   
    printf("�˳�����÷�ɽ���ݹ���ƽ̨��ͨ�ýӿڣ���ȡ�״�ͼ���ݡ�\n");
    printf("logfilename ���������е���־�ļ�����\n");
    printf("stdpath ���ص��ļ���ŵ�Ŀ¼��\n");
	printf("���ȳ����ܴ���60�롣\n");

    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strstdpath,0,sizeof(strstdpath));
  memset(strxmlbuffer,0,sizeof(strxmlbuffer));
  memset(strtemppath,0,sizeof(strtemppath));

  strncpy(strxmlbuffer,argv[1],4000);

  GetXMLBuffer(strxmlbuffer,"logfilename",strlogfilename,300);
  GetXMLBuffer(strxmlbuffer,"temppath"  ,strtemppath,300);
  GetXMLBuffer(strxmlbuffer,"stdpath"  ,strstdpath,300);

  if (strlen(strlogfilename)==0)  { printf("logfilename ����Ϊ��.\n"); return -1; }
  if (strlen(strtemppath)==0)      { printf("temppath ����Ϊ��.\n"); return -1; }
  if (strlen(strstdpath)==0)      { printf("stdpath ����Ϊ��.\n"); return -1; }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("GetRadarPng");

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&logfile,"GetRadarPng",300); 

  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(yyyy,0,sizeof(yyyy));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(hh,0,sizeof(hh));
  
  char strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyymmddhh24miss",0-8*60*60);
   
  strncpy(yyyy,strLocalTime,4);
  strncpy(mm,strLocalTime+4,2);
  strncpy(dd,strLocalTime+6,2);
  strncpy(hh,strLocalTime+8,2);

  //����00��01������
  int sn[2]={ 0,1 };
  char strcmd[1024];
  int jj,mi;

  //��Сʱ
  for(jj=0;jj<10;jj++) 
  {
    mi=6*jj;
  
    for( int ii=0;ii<2;ii++)
    {
      memset(strfilename,0,sizeof(strfilename));
      memset(strcmd,0,sizeof(strcmd));
      snprintf(strfilename,300,"%d%02d%02d.%02d%02d%02d.02.19.200.png",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),mi,sn[ii]);	    
      snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s  http://10.151.64.202:8111/filelist/radar/%s 1>>/dev/null 2>>/dev/null",strtemppath,strfilename);
      system(strcmd);
      logfile.Write("strcmd=%s\n",strcmd);
    }

  }

  //��һСʱ
  for(jj=0;jj<10;jj++)  
  {
    mi=6*jj;

    for( int ii=0;ii<2;ii++)
    {
      memset(strfilename,0,sizeof(strfilename));
      memset(strcmd,0,sizeof(strcmd));
      snprintf(strfilename,300,"%d%02d%02d.%02d%02d%02d.02.19.200.png",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,mi,sn[ii]);      
      snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s  http://10.151.64.202:8111/filelist/radar/%s 1>>/dev/null 2>>/dev/null",strtemppath,strfilename);
      system(strcmd);
      logfile.Write("strcmd=%s\n",strcmd);
    }

  }

  if (Dir.OpenDir(strtemppath) == FALSE)
  {
     printf("Dir.OpenDir(%s) failed.\n",strtemppath); exit(-1);
  }
  
  while (Dir.ReadDir() == TRUE)
  {
   
    // д����̻��Ϣ
    ProgramActive.WriteToFile();
    
    if (MatchFileName(Dir.m_FileName,"*02.19.200.png")==FALSE) continue;
  
    // ��ʼ�����ļ�
    logfile.Write("Process file %s...\n",Dir.m_FileName);
 
    // �򿪴����������Դ�ļ�
    if ((File.OpenForRead(Dir.m_FullFileName,"r")) == FALSE)
    {
       logfile.Write("File.OpenForRead(%s) failed.\n",Dir.m_FullFileName); continue;
     }
    
    while(TRUE)
    { 
      memset(strbuffer,0,sizeof(strbuffer));
      if(File.FFGETS(strbuffer,4000) == FALSE) break;
      if(strstr(strbuffer,"404")!=0)
      {
        File.CloseAndRemove();
        logfile.Write("REMOVE %s...\n",Dir.m_FileName);
      }
 
    }

    if(access(Dir.m_FullFileName,F_OK)==0)
    {
      char strstdpathname[301];
      memset(strstdpathname,0,sizeof(strstdpathname));
      snprintf(strstdpathname,300,"%s/%s",strstdpath,Dir.m_FileName);

      RENAME(Dir.m_FullFileName,strstdpathname);
      logfile.Write("RENAME %s to %s ...\n",Dir.m_FullFileName,strstdpathname);
    }
  }

  return 0;
}

void EXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("GetRadarPng exit.\n");

  exit(0);
}
