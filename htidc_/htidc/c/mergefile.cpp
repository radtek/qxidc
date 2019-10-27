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

// �������ļ���ÿ�����ļ��Ƿ�����������������Ͳ�������������ļ�
BOOL CheckCTLFile(char *strCTLFileName);

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/mergefile srcpath dstpathtmp dstpath\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/mergefile /home/recv /tmp/htidc/tmp /tmp/htidc/ftpput\n\n\n");

    printf("���������������ĵĹ�������ģ�飬���ںϲ���splitfile�����ֵ��ļ���\n");
    printf("���������е���־�ļ���/tmp/htidc/log/mergefile.log��\n");
    printf("srcpath �ϲ�ǰ���ļ���ŵ�Ŀ¼���ɹ��ϲ���srcpathĿ¼�е�Դ�ļ�����ɾ����\n");
    printf("dstpathtmp �ϲ�ʱ�������м�״̬�ļ���ŵ���ʱĿ¼��\n");
    printf("dstpath �ϲ���������ļ���ŵ���ʽĿ¼��ע�⣬dstpathtmp��dstpath������ͬһĿ¼��\n\n\n");

    return 0;
  }
  
  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // ע�⣬����ʱ��300��
  ProgramActive.SetProgramInfo(&logfile,"mergefile",300);

  if (logfile.Open("/tmp/htidc/log/mergefile.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/mergefile.log) failed.\n"); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("mergefile");

  memset(srcpath,0,sizeof(srcpath));
  memset(dstpathtmp,0,sizeof(dstpathtmp));
  memset(dstpath,0,sizeof(dstpath));

  strncpy(srcpath,argv[1],300);
  strncpy(dstpathtmp,argv[2],300);
  strncpy(dstpath,argv[3],300);

  CDir Dir;
  int  fpsrc,fpdst;

  // һ��Ҫ��fpsrc��fpdst��0���������õ�ϰ��
  fpsrc=fpdst=0;
 
  // ��Ŀ¼����ȡ�ļ�������������Ŀ¼
  if (Dir.OpenDir(srcpath,TRUE) == FALSE)
  {
    printf("Dir.OpenDir(%s) failed.\n",srcpath); exit(-1);
  }

  while (Dir.ReadDir() == TRUE)
  {
    // �ж��ļ����Ƿ��ǿ����ļ����������ƥ�䣬����ָ��ļ�
    if (MatchFileName(Dir.m_FileName,"*.CTL") == FALSE) continue;

    // �������ļ���ÿ�����ļ��Ƿ�����������������Ͳ�������������ļ�
    if (CheckCTLFile(Dir.m_FullFileName) == FALSE) continue;

    ProgramActive.WriteToFile();

    char strfilenametmp[301],strdstfilenametmp[301],strdstfilename[301];

    memset(strfilenametmp,0,sizeof(strfilenametmp));
    memset(strdstfilenametmp,0,sizeof(strdstfilenametmp));
    memset(strdstfilename,0,sizeof(strdstfilename));

    // �ӿ����ļ����еõ�����ԭ�ļ�������ȥ��".ctl"�ĺ�׺
    strncpy(strfilenametmp,Dir.m_FileName,strlen(Dir.m_FileName)-4);
    snprintf(strdstfilenametmp,300,"%s/%s.tmp",dstpathtmp,strfilenametmp);
    snprintf(strdstfilename   ,300,"%s/%s"    ,dstpath   ,strfilenametmp);

    logfile.Write("merge %s...",strdstfilename);

    // ��������ԭ�ļ�����ʱ�ļ�
    if ( (fpdst=open(strdstfilenametmp,O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IXUSR)) < 0)
    {
      logfile.WriteEx("failed.\nopen %s failed.\n",strdstfilenametmp); return -1;
    }

    // ��ÿ�����ļ���������׷�ӵ�����ԭ�ļ���
    for (UINT ii=0;ii<vfileinfo.size();ii++)
    {
      fpsrc=0;

      // ��Դ�ļ�
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

// �������ļ���ÿ�����ļ��Ƿ�����������������Ͳ�������������ļ�
BOOL CheckCTLFile(char *strCTLFileName)
{
  // ������ڴ�Ÿ����ļ���Ϣ������
  vfileinfo.clear();

  // �ж��ļ��Ƿ���"</data>"����
  if ( CheckFileSTS(strCTLFileName,"</data>") == FALSE) 
  { 
    logfile.Write("%s is invalid.\n",strCTLFileName); return FALSE;
  }
 
  // �򿪿����ļ����ж�ÿ���ļ��ڷ���ڣ���С�Ƿ�һ��
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

    // ֱ�ӽ�ѹ�ļ�
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
