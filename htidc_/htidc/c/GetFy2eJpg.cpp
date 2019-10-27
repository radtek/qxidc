#include "_public.h"

void EXIT(int sig);

char strlogfilename[301];
char strstdpath[301];
char strfilename[301];
char strxmlbuffer[4001];

char strLocalTime[21],yyyy[5],mm[3],dd[3],hh[3];

CLogFile      logfile;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if( argc !=2 )
  {
    printf("\n");
    
    return -1;
  }
 
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strstdpath,0,sizeof(strstdpath));
  memset(strxmlbuffer,0,sizeof(strxmlbuffer));

  strncpy(strxmlbuffer,argv[1],4000);

  GetXMLBuffer(strxmlbuffer,"logfilename",strlogfilename,300);
  GetXMLBuffer(strxmlbuffer,"stdpath",strstdpath,300); 
  
  if(strlen(strlogfilename)==0){ printf("strlogfilename 不能为空.\n"); return -1; }
  if(strlen(strstdpath)==0) { printf("stdpath 不能为空.\n"); return -1; }

  CloseIOAndSignal();signal(SIGINT,EXIT);signal(SIGTERM,EXIT);

  if(logfile.Open(strlogfilename,"a+")==FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename);
    return -1;
  }

  logfile.SetAlarmOpt("GetFy2eJpg");

  ProgramActive.SetProgramInfo(&logfile,"GetFy2eJpg",300);
  

  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(yyyy,0,sizeof(yyyy));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(hh,0,sizeof(hh));

  LocalTime(strLocalTime,"yyyymmddhh24miss",0-8*60*60);

  strncpy(yyyy,strLocalTime,4);
  strncpy(mm,strLocalTime+4,2);
  strncpy(dd,strLocalTime+6,2);
  strncpy(hh,strLocalTime+8,2);

  int sn[4]={ 1,2,31,32 };
  char strcmd[1024];
 
  //本小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%d_%02d_%02d_%02d_%02d_M_PJ2_IR4.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%d_%02d_%02d_%02d_%02d_M_PJ2_IR4.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }
 
  //本小时 
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_Pic.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_Pic.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //本小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_VIS.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_VIS.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //本小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_W.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_W.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //本小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_3D.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_3D.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //本小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_IR1.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }
  
  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_IR1.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //本小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_IR2.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh),sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  //上一小时
  for(int ii=0;ii<4;ii++)
  {
    memset(strfilename,0,sizeof(strfilename));
    memset(strcmd,0,sizeof(strcmd));
    snprintf(strfilename,300,"FY2E_%02d_%02d_%02d_%02d_%02d_M_PJ2_IR2.JPG",atoi(yyyy),atoi(mm),atoi(dd),atoi(hh)-1,sn[ii]);
    snprintf(strcmd,1000,"/usr/bin/wget -c --directory-prefix=%s http://10.151.64.202:8111/filelist/cloud/fy2e/%s 1>/dev/null 2>/dev/null",strstdpath,strfilename);
    system(strcmd);
    logfile.Write("strcmd=%s\n",strcmd);
  }

  logfile.Write("wget ok.\n\n");

  return 0;

}

void EXIT(int sig)
{
  if(sig>0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("GetFy2eJpg exit.\n");
 
  exit(0);

}

