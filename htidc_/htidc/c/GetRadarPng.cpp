#include "_public.h"

void EXIT(int sig);

char strlogfilename[301];  // 本程序日志文件名
char strstdpath[301];      // 文件的目标路径
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
   
    printf("此程序调用佛山数据共享平台的通用接口，获取雷达图数据。\n");
    printf("logfilename 本程序运行的日志文件名。\n");
    printf("stdpath 下载的文件存放的目录。\n");
	printf("调度程序不能大于60秒。\n");

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

  if (strlen(strlogfilename)==0)  { printf("logfilename 不能为空.\n"); return -1; }
  if (strlen(strtemppath)==0)      { printf("temppath 不能为空.\n"); return -1; }
  if (strlen(strstdpath)==0)      { printf("stdpath 不能为空.\n"); return -1; }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("GetRadarPng");

  // 注意，程序超时是300秒
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

  //秒有00和01这两种
  int sn[2]={ 0,1 };
  char strcmd[1024];
  int jj,mi;

  //本小时
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

  //上一小时
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
   
    // 写入进程活动信息
    ProgramActive.WriteToFile();
    
    if (MatchFileName(Dir.m_FileName,"*02.19.200.png")==FALSE) continue;
  
    // 开始处理文件
    logfile.Write("Process file %s...\n",Dir.m_FileName);
 
    // 打开待处理的数据源文件
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
