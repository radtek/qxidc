#include "_public.h"

void CallQuit(int sig);

CLogFile       logfile;
CProgramActive ProgramActive;

int main(int argc,char *argv[])
{
  if (argc != 7)
  {
    printf("\n");
    printf( "Usage: /htidc/qxfw/bin/wgettwstar logfile url tmppath outpath filecount mapfilename\n" );
    printf( "Example: /htidc/htidc/bin/procctl 300 /htidc/qxfw/bin/wgettwstar /log/qxfw/wgettwstar_cloud.log \"http://www.cwb.gov.tw/V7/observe/satellite/Data/HS1P/HS1P-yyyy-mm-dd-hh-mi.jpg\" /qxfw/star/wget_cloud /qxfw/star/cloud 2 /htidc/qxfw/map/forstar_yellow.png\n\n" );

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  // 注意，不能启用CloseIOAndSignal函数，否则获取到的文件格式有问题，打不开
  // CloseIOAndSignal(); 
  signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("wgettwstar");

  //logfile.Write("wgettwstar beginning.\n");

  // 超时设置为180秒
  ProgramActive.SetProgramInfo(&logfile,"wgettwstar",180);

  char strLocalTime[21],strFileTime[21];

  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss"); 
  strncpy(strLocalTime+14,"30:00",5);

  char strUrl[301];

  char strCmd[1024];

  MKDIR(argv[3],FALSE);

  char strSrcFileName[301],strTmpFileName[301],strDstFileName[301];

  char *pos;
  char strDDateTime[21];

  for (int ii=0;ii<atoi(argv[5]);ii++)
  {
    ProgramActive.WriteToFile();

    memset(strFileTime,0,sizeof(strFileTime));

    AddTime(strLocalTime,strFileTime,0-ii*30*60,"yyyy-mm-dd hh24:mi:ss");

    strFileTime[16]=0;
    UpdateStr(strFileTime,":","-");
    UpdateStr(strFileTime," ","-");

    memset(strUrl,0,sizeof(strUrl));
    strcpy(strUrl,argv[2]);
    UpdateStr(strUrl,"yyyy-mm-dd-hh-mi",strFileTime);

    chdir(argv[3]);

    // 如果文件已存在，就不获取了。
    pos=0;
    if (strstr(strUrl,"HSAO") != 0) pos=strstr(strUrl,"HSAO-201");
    if (strstr(strUrl,"HS1P") != 0) pos=strstr(strUrl,"HS1P-201");
    if (strstr(strUrl,"HS1Q") != 0) pos=strstr(strUrl,"HS1Q-201");
    if (strstr(strUrl,"HS1O") != 0) pos=strstr(strUrl,"HS1O-201");

    if (pos==0) continue;
    memset(strSrcFileName,0,sizeof(strSrcFileName));
    snprintf(strSrcFileName,300,"%s/%s",argv[3],pos);
    if (FileSize(strSrcFileName) > 300000) continue;

    // logfile.Write("%s\n",strSrcFileName);

    // 获取文件
    memset(strCmd,0,sizeof(strCmd));
    snprintf(strCmd,1000,"wget -U firefox %s 1>/dev/null 2>/dev/null",strUrl);
    logfile.Write("cmd=%s\n",strCmd);
    system(strCmd);

    // 一般来说，台湾云图文件是500K左右，这里判断是否有300K，如果没有，就表示没有获取到文件，或文件有问题
    if (FileSize(strSrcFileName) < 300000) continue;

    memset(strDDateTime,0,sizeof(strDDateTime));
    pos=strstr(strSrcFileName,"-201");
    strncpy(strDDateTime,pos+1,16);
    UpdateStr(strDDateTime,"-","");

    memset(strTmpFileName,0,sizeof(strTmpFileName));
    snprintf(strTmpFileName,300,"%s/%s.jpg",argv[3],strDDateTime);

    memset(strDstFileName,0,sizeof(strDstFileName));
    snprintf(strDstFileName,300,"%s/%s.jpg",argv[4],strDDateTime);

    // 对图片进行剪切
    memset(strCmd,0,sizeof(strCmd));
    // 如果不合成底图，就采用这一行代码就可以了
    // snprintf(strCmd,1000,"convert %s -crop 640x640+0+45 +repage -quality 50%% %s",strSrcFileName,strTmpFileName);
    snprintf(strCmd,1000,"convert %s -crop 640x640+0+45 +repage %s",strSrcFileName,strTmpFileName);
    logfile.Write("cmd=%s\n",strCmd);
    system(strCmd);

    // 合成底图，转换图片质量
    memset(strCmd,0,sizeof(strCmd));
    snprintf(strCmd,1000,"composite -gravity center %s %s -quality 50%% %s",argv[6],strTmpFileName,strTmpFileName);
    logfile.Write("cmd=%s\n",strCmd);
    system(strCmd);
    
    RENAME(strTmpFileName,strDstFileName);
  }


  // 删除该目录下的乱七八的其它文件
  CDir Dir;
  if (Dir.OpenDir(argv[3]) == FALSE)
  {
    logfile.Write("Dir.OpenDir(%s) failed.\n",argv[3]); return -1;
  }

  while (TRUE)
  {
    if (Dir.ReadDir() == FALSE) break;

    if (MatchFileName(Dir.m_FileName,"HS*.JPG") == FALSE) { REMOVE(Dir.m_FullFileName); continue; }
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("wgettwstar exit.\n");

  exit(0);
}

