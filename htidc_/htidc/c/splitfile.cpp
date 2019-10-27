#include "_public.h"

CLogFile logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

char srcpath[301];
char dstpathtmp[301];
char dstpath[301];
char matchname[301];
UINT filesize;
BOOL deletesrcfile;

struct st_fileinfo
{
  char filename[301];
  UINT filesize;
};

struct st_fileinfo stfileinfo;

vector<struct st_fileinfo> vfileinfo;

int main(int argc,char *argv[])
{
  if (argc != 7)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/splitfile srcpath dstpathtmp dstpath matchname filesize deletesrcfile[TRUE|FALSE]\n\n");

    printf("Sample:/htidc/htidc/bin/procctl   300 /htidc/htidc/bin/splitfile /tmp/htidc/ftpput /home/sendtmp /home/send \"*.GZ,*.TGZ\"  1000000 TRUE\n\n\n");

    printf("本程序是数据中心的公共功能模块，用于把本地一个大一些的文件拆分为若干小一些的文件，再交给网闸交换。\n");
    printf("本程序运行的日志文件是/tmp/htidc/log/splitfile.log。\n");
    printf("srcpath 拆分前的文件存放的目录，注意，只有生成时间在50秒之前的文件才会被拆分。\n");
    printf("dstpathtmp 拆分时产生的子文件和控制文件存放的临时目录。\n");
    printf("dstpath 拆分后产生的子文件和控制文件存放的正式目录，因为网闸很烂，不能识别中间状态的文件，所以本程序在拆分时把中间状态的子文件存放在dstpathtmp目录中，完整的子文件才会被移动到dstpath目录，注意，dstpathtmp和dstpath一定要在同一个分区中，否则无法转移文件，程序将失败退出。\n");
    printf("matchname 待拆分的文件匹配的文件名，采用大写匹配，不匹配的文件不会被拆分，本字段尽可能设置精确。\n");
    printf("filesize 本程序按filesize大小拆分，filesize的取值最好为1000的整数倍，建议采用1000000比较合适。\n");
    printf("deletesrcfile 拆分完成后，是否删除拆分前的源文件，取值为TRUE或FALSE。\n\n\n");

    return 0;
  }
  
  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&logfile,"splitfile",300);

  if (logfile.Open("/tmp/htidc/log/splitfile.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/splitfile.log) failed.\n"); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("splitfile");

  memset(srcpath,0,sizeof(srcpath));
  memset(dstpathtmp,0,sizeof(dstpathtmp));
  memset(dstpath,0,sizeof(dstpath));
  memset(matchname,0,sizeof(matchname));
  filesize=0;
  deletesrcfile=FALSE;

  strncpy(srcpath,argv[1],300);
  strncpy(dstpathtmp,argv[2],300);
  strncpy(dstpath,argv[3],300);
  strncpy(matchname,argv[4],300);
  filesize=atoi(argv[5]);
  if ( (strcmp(argv[6],"TRUE")==0) || (strcmp(argv[6],"true")==0) ) deletesrcfile=TRUE; 

  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-50); // 当前时间点50秒之前的时间

  CDir Dir;
  int  fpsrc,fpdst;

  char strCMD[1024];

  // 一定要把fpsrc和fpdst清0，因为要根据fpdst是否为0来判断文件是否打开。
  fpsrc=fpdst=0;
 
  // 打开目录，读取文件，包括它的子目录
  if (Dir.OpenDir(srcpath,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",srcpath); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    // 判断文件名是否和MatchStr匹配，如果不匹配，不拆分该文件
    if (MatchFileName(Dir.m_FileName,matchname) == FALSE) continue;

    // 如果文件的时间在当前时间的前50秒之内，就暂时不拆分
    if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

    ProgramActive.WriteToFile();

    logfile.Write("split %s...",Dir.m_FullFileName);

    // 打开源文件
    if ( (fpsrc=open(Dir.m_FullFileName,O_RDWR)) < 0)
    {
      logfile.WriteEx("failed.\nopen %s failed.\n",Dir.m_FullFileName); return -1;
    }

    // 拆分后的文件的序号
    UINT upart=0;

    char buffer[1024];

    UINT totalread,nread,nwrite,totalwrite,onefilesize;

    totalread=nread=nwrite=totalwrite=onefilesize=0;

    char strdstfilenametmp[301];
    char strdstfilename[301];
    char strpartfilename[301];

    vfileinfo.clear();

    while (TRUE)
    {
      memset(buffer,0,sizeof(buffer));

      nread=nwrite=0;

      if ( (nread=read(fpsrc,buffer,1000)) <= 0 ) break;
      //if ( (nread=read(fpsrc,buffer,1)) <= 0 ) break;

      totalread=totalread+nread;

      // 如果文件为空，就打开一个新的子文件
      if (fpdst==0)
      {
        memset(strdstfilenametmp,0,sizeof(strdstfilenametmp));
        memset(strdstfilename,0,sizeof(strdstfilename));
        memset(strpartfilename,0,sizeof(strpartfilename));

        upart++; onefilesize=0;

        snprintf(strpartfilename  ,300,"%s.%lu",Dir.m_FileName,upart);
        snprintf(strdstfilenametmp,300,"%s/%s",dstpathtmp,strpartfilename);
        snprintf(strdstfilename   ,300,"%s/%s",dstpath   ,strpartfilename);

        if ( (fpdst=open(strdstfilenametmp,O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IXUSR)) < 0)
        {
          logfile.WriteEx("failed.\nopen %s failed.\n",strdstfilenametmp); close(fpsrc); return -1;
        }
      }

      nwrite=write(fpdst,buffer,nread);

      if (nwrite != nread) 
      {
        logfile.WriteEx("failed.\nwrite %s failed(nread=%d,nwrite=%d).\n",strdstfilenametmp,nread,nwrite); close(fpsrc); close(fpdst); return -1;
      }

      onefilesize=onefilesize+nwrite;

      totalwrite=totalwrite+nwrite;

      // 如果文件大小已达到了拆分界限，就关掉这个文件。
      if (onefilesize>=filesize)
      {
        close(fpdst); 

        fpdst=0;

        memset(strCMD,1000,sizeof(strCMD));
        snprintf(strCMD,1000,"cd %s;/bin/tar zcvf %s.tgz %s 1>/dev/null 2>/dev/null",dstpathtmp,strpartfilename,strpartfilename);
        system(strCMD);
        REMOVE(strdstfilenametmp);

        strcat(strdstfilenametmp,".tgz");
        strcat(strdstfilename,".tgz");
        
        if (RENAME(strdstfilenametmp,strdstfilename) == FALSE)
        {
          logfile.WriteEx("failed.\nRENAME %s to %s failed.\n",strdstfilenametmp,strdstfilename); close(fpsrc); return -1;
        }

        memset(&stfileinfo,0,sizeof(stfileinfo));
        strcpy(stfileinfo.filename,strpartfilename);
        stfileinfo.filesize=onefilesize;
        vfileinfo.push_back(stfileinfo); 
      }
    }

    if (fpdst > 0)
    {
      close(fpdst);

      fpdst=0;

      memset(strCMD,1000,sizeof(strCMD));
      snprintf(strCMD,1000,"cd %s;/bin/tar zcvf %s.tgz %s 1>/dev/null 2>/dev/null",dstpathtmp,strpartfilename,strpartfilename);
      system(strCMD);
      REMOVE(strdstfilenametmp);

      strcat(strdstfilenametmp,".tgz");
      strcat(strdstfilename,".tgz");

      if (RENAME(strdstfilenametmp,strdstfilename) == FALSE)
      {
        logfile.WriteEx("failed.\nRENAME %s to %s failed.\n",strdstfilenametmp,strdstfilename); close(fpsrc); return -1;
      }

      memset(&stfileinfo,0,sizeof(stfileinfo));
      strcpy(stfileinfo.filename,strpartfilename);
      stfileinfo.filesize=onefilesize;
      vfileinfo.push_back(stfileinfo); 
    }

    close(fpsrc);
 
    fpsrc=0;

    // 把拆分文件的情况写入控制文件
    char strctlfilenametmp[301],strctlfilename[301];
    memset(strctlfilenametmp,0,sizeof(strctlfilenametmp));
    memset(strctlfilename,0,sizeof(strctlfilename));
    snprintf(strctlfilenametmp,300,"%s/%s.ctl.tmp",dstpathtmp,Dir.m_FileName);
    snprintf(strctlfilename   ,300,"%s/%s.ctl"    ,dstpath   ,Dir.m_FileName);
    FILE *fp=0;
    if ( (fp=FOPEN(strctlfilenametmp,"w+")) == 0)
    {
      logfile.WriteEx("failed.\nFOPEN %s failed.\n",strctlfilenametmp); return -1;
    }

    fprintf(fp,"<data>\n");

    for (UINT ii=0;ii<vfileinfo.size();ii++)
    {
      fprintf(fp,"<filename>%s</filename><filesize>%lu</filesize><endl/>\n",vfileinfo[ii].filename,vfileinfo[ii].filesize);
    }

    fprintf(fp,"</data>\n");

    fclose(fp);

    if (RENAME(strctlfilenametmp,strctlfilename) == FALSE)
    {
      logfile.WriteEx("failed.\nRENAME %s to %s failed.\n",strctlfilenametmp,strctlfilename); return -1;
    }

    // 判断是否需要删除源文件
    if (deletesrcfile==TRUE)
    {
      if (REMOVE(Dir.m_FullFileName) == FALSE)
      {
        logfile.WriteEx("failed.\nREMOVE %s failed.\n",Dir.m_FullFileName); return -1;
      }
    }

    logfile.WriteEx("ok.\n");
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  exit(0);
}

