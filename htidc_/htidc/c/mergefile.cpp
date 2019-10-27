#include "_public.h"

CLogFile logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

char srcpath[301];
char dstpathtmp[301];
char dstpath[301];

struct st_fileinfo
{
  char filename[301];
  UINT filesize;
};

struct st_fileinfo stfileinfo;

vector<struct st_fileinfo> vfileinfo;

// 检查控制文件和每个子文件是否完整，如果不完整就不处理这个控制文件
BOOL CheckCTLFile(char *strCTLFileName);

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/mergefile srcpath dstpathtmp dstpath\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/mergefile /home/recv /tmp/htidc/tmp /tmp/htidc/ftpput\n\n\n");

    printf("本程序是数据中心的公共功能模块，用于合并由splitfile程序拆分的文件。\n");
    printf("本程序运行的日志文件是/tmp/htidc/log/mergefile.log。\n");
    printf("srcpath 合并前的文件存放的目录，成功合并后，srcpath目录中的源文件将被删除。\n");
    printf("dstpathtmp 合并时产生的中间状态文件存放的临时目录。\n");
    printf("dstpath 合并后产生的文件存放的正式目录，注意，dstpathtmp和dstpath可以是同一目录。\n\n\n");

    return 0;
  }
  
  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&logfile,"mergefile",300);

  if (logfile.Open("/tmp/htidc/log/mergefile.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/mergefile.log) failed.\n"); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("mergefile");

  memset(srcpath,0,sizeof(srcpath));
  memset(dstpathtmp,0,sizeof(dstpathtmp));
  memset(dstpath,0,sizeof(dstpath));

  strncpy(srcpath,argv[1],300);
  strncpy(dstpathtmp,argv[2],300);
  strncpy(dstpath,argv[3],300);

  CDir Dir;
  int  fpsrc,fpdst;

  // 一定要把fpsrc和fpdst清0，这是良好的习惯
  fpsrc=fpdst=0;
 
  // 打开目录，读取文件，包括它的子目录
  if (Dir.OpenDir(srcpath,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",srcpath); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    // 判断文件名是否是控制文件名，如果不匹配，不拆分该文件
    if (MatchFileName(Dir.m_FileName,"*.CTL") == FALSE) continue;

    // 检查控制文件和每个子文件是否完整，如果不完整就不处理这个控制文件
    if (CheckCTLFile(Dir.m_FullFileName) == FALSE) continue;

    ProgramActive.WriteToFile();

    char strfilenametmp[301],strdstfilenametmp[301],strdstfilename[301];

    memset(strfilenametmp,0,sizeof(strfilenametmp));
    memset(strdstfilenametmp,0,sizeof(strdstfilenametmp));
    memset(strdstfilename,0,sizeof(strdstfilename));

    // 从控制文件名中得到数据原文件名，即去掉".ctl"的后缀
    strncpy(strfilenametmp,Dir.m_FileName,strlen(Dir.m_FileName)-4);
    snprintf(strdstfilenametmp,300,"%s/%s.tmp",dstpathtmp,strfilenametmp);
    snprintf(strdstfilename   ,300,"%s/%s"    ,dstpath   ,strfilenametmp);

    logfile.Write("merge %s...",strdstfilename);

    // 创建数据原文件的临时文件
    if ( (fpdst=open(strdstfilenametmp,O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IXUSR)) < 0)
    {
      logfile.WriteEx("failed.\nopen %s failed.\n",strdstfilenametmp); return -1;
    }

    // 打开每个子文件，把内容追加到数据原文件中
    for (UINT ii=0;ii<vfileinfo.size();ii++)
    {
      fpsrc=0;

      // 打开源文件
      if ( (fpsrc=open(vfileinfo[ii].filename,O_RDWR)) < 0)
      {
        logfile.WriteEx("failed.\nopen %s failed.\n",vfileinfo[ii].filename); close(fpdst); return -1;
      }

      int nread,nwrite,buffer[1000];
    
      while (TRUE)
      {
        memset(buffer,0,sizeof(buffer));

        nread=nwrite=0;

        if ( (nread=read(fpsrc,buffer,1000)) <= 0 ) break;

        nwrite=write(fpdst,buffer,nread);

        if (nwrite != nread)
        {
          logfile.WriteEx("failed.\nwrite %s failed(nread=%d,nwrite=%d).\n",strdstfilenametmp,nread,nwrite); close(fpsrc); close(fpdst); return -1;
        }
      }

      close(fpsrc);

      if (REMOVE(vfileinfo[ii].filename) == FALSE)
      {
        logfile.WriteEx("failed.\nREMOVE %s failed.\n",vfileinfo[ii].filename); close(fpdst); return -1;
      }
    }

    close(fpdst);

    if (REMOVE(Dir.m_FullFileName) == FALSE)
    {
      logfile.WriteEx("failed.\nREMOVE %s failed.\n",Dir.m_FullFileName); return -1;
    }

    if (RENAME(strdstfilenametmp,strdstfilename) == FALSE)
    {
      logfile.WriteEx("failed.\nRENAME %s to %s failed.\n",strdstfilenametmp,strdstfilename); return -1;
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

// 检查控制文件和每个子文件是否完整，如果不完整就不处理这个控制文件
BOOL CheckCTLFile(char *strCTLFileName)
{
  // 清空用于存放各子文件信息的容器
  vfileinfo.clear();

  // 判断文件是否以"</data>"结束
  if ( CheckFileSTS(strCTLFileName,"</data>") == FALSE) 
  { 
    logfile.Write("%s is invalid.\n",strCTLFileName); return FALSE;
  }
 
  // 打开控制文件，判断每个文件在否存在，大小是否一致
  FILE *fp=0;
  if ( (fp=FOPEN(strCTLFileName,"r")) == 0)
  {
    logfile.Write("FOPEN %s failed.\n",strCTLFileName); return FALSE;
  }

  char strLine[1024],strFileName[301],strCMD[1024];

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
    memset(strFileName,0,sizeof(strFileName));

    memset(&stfileinfo,0,sizeof(stfileinfo));

    if (FGETS(strLine,1000,fp,"<endl/>") == FALSE) break;

    GetXMLBuffer(strLine,"filename",strFileName,300);
    snprintf(stfileinfo.filename,300,"%s/%s",srcpath,strFileName);
    GetXMLBuffer(strLine,"filesize",&stfileinfo.filesize);

    // 直接解压文件
    memset(strCMD,0,sizeof(strCMD));
    snprintf(strCMD,1000,"cd %s;/bin/tar zxvf %s.tgz;rm %s.tgz 1>/dev/null 2>/dev/null",srcpath,strFileName,strFileName);
    system(strCMD);

    if (access(stfileinfo.filename,R_OK) != 0)
    {
      logfile.Write("%s not exist.\n",stfileinfo.filename); fclose(fp); return FALSE;
    }

    if (FileSize(stfileinfo.filename) != (int)stfileinfo.filesize)
    {
      logfile.Write("%s filesize is invalid.\n",stfileinfo.filename); fclose(fp); return FALSE;
    }

    vfileinfo.push_back(stfileinfo);

    //logfile.Write("%s,%d\n",stfileinfo.filename,stfileinfo.filesize);
  }

  fclose(fp);

  return TRUE;
}
