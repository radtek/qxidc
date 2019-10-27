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

    printf("���������������ĵĹ�������ģ�飬���ڰѱ���һ����һЩ���ļ����Ϊ����СһЩ���ļ����ٽ�����բ������\n");
    printf("���������е���־�ļ���/tmp/htidc/log/splitfile.log��\n");
    printf("srcpath ���ǰ���ļ���ŵ�Ŀ¼��ע�⣬ֻ������ʱ����50��֮ǰ���ļ��Żᱻ��֡�\n");
    printf("dstpathtmp ���ʱ���������ļ��Ϳ����ļ���ŵ���ʱĿ¼��\n");
    printf("dstpath ��ֺ���������ļ��Ϳ����ļ���ŵ���ʽĿ¼����Ϊ��բ���ã�����ʶ���м�״̬���ļ������Ա������ڲ��ʱ���м�״̬�����ļ������dstpathtmpĿ¼�У����������ļ��Żᱻ�ƶ���dstpathĿ¼��ע�⣬dstpathtmp��dstpathһ��Ҫ��ͬһ�������У������޷�ת���ļ�������ʧ���˳���\n");
    printf("matchname ����ֵ��ļ�ƥ����ļ��������ô�дƥ�䣬��ƥ����ļ����ᱻ��֣����ֶξ��������þ�ȷ��\n");
    printf("filesize ������filesize��С��֣�filesize��ȡֵ���Ϊ1000�����������������1000000�ȽϺ��ʡ�\n");
    printf("deletesrcfile �����ɺ��Ƿ�ɾ�����ǰ��Դ�ļ���ȡֵΪTRUE��FALSE��\n\n\n");

    return 0;
  }
  
  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&logfile,"splitfile",300);

  if (logfile.Open("/tmp/htidc/log/splitfile.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/splitfile.log) failed.\n"); return -1;
  }

  //�򿪸澯
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
  LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-50); // ��ǰʱ���50��֮ǰ��ʱ��

  CDir Dir;
  int  fpsrc,fpdst;

  char strCMD[1024];

  // һ��Ҫ��fpsrc��fpdst��0����ΪҪ����fpdst�Ƿ�Ϊ0���ж��ļ��Ƿ�򿪡�
  fpsrc=fpdst=0;
 
  // ��Ŀ¼����ȡ�ļ�������������Ŀ¼
  if (Dir.OpenDir(srcpath,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",srcpath); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    // �ж��ļ����Ƿ��MatchStrƥ�䣬�����ƥ�䣬����ָ��ļ�
    if (MatchFileName(Dir.m_FileName,matchname) == FALSE) continue;

    // ����ļ���ʱ���ڵ�ǰʱ���ǰ50��֮�ڣ�����ʱ�����
    if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

    ProgramActive.WriteToFile();

    logfile.Write("split %s...",Dir.m_FullFileName);

    // ��Դ�ļ�
    if ( (fpsrc=open(Dir.m_FullFileName,O_RDWR)) < 0)
    {
      logfile.WriteEx("failed.\nopen %s failed.\n",Dir.m_FullFileName); return -1;
    }

    // ��ֺ���ļ������
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

      // ����ļ�Ϊ�գ��ʹ�һ���µ����ļ�
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

      // ����ļ���С�Ѵﵽ�˲�ֽ��ޣ��͹ص�����ļ���
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

    // �Ѳ���ļ������д������ļ�
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

    // �ж��Ƿ���Ҫɾ��Դ�ļ�
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

