// xxxxxxxxxxxxxxxxxxxxxx�鿴exp�ļ�ʧ�ܵ�ʱ��list�ļ�¼�Ƿ�һ��
#include "_public.h"
#include "_oracle.h"

char strconnstr[301]; // ���ݿ����Ӳ���
char strlogfilename[301];    // ��־�ļ�Ŀ¼
char strlistfilename[301];
char strexportsql[1024];
char strtopathtmp[301];    // �����ļ���ʱ��ŵ�Ŀ¼
char strtopath[301];
char strconvertcfg[301];

connection conn;
CLogFile       logfile;
CIniFile       IniFile;
CProgramActive ProgramActive;

struct st_FILELIST
{
   char filename[201];
   char ddatetime[20];
   char crttime[20];
   char upttime[20];
   int  filesize;
};

struct st_FILELIST stFILELIST;
vector<struct st_FILELIST> vOldFILELIST;
vector<struct st_FILELIST> vNewFILELIST;

// ���Ѵ����ļ������ݶ���vOldFILELIST
BOOL ReadFromListFile();

// �����ݿ�����µ��ļ�������
long ExpDBFiles();

// ���ӱ��л�ȡ���ļ���Ϣ��֮ǰ�ѵ������ļ���Ϣ�Ƿ���ͬ�������ͬ����continue
BOOL CheckInOldList();

// ��vNewFILELIST�е�����д��list�ļ�
BOOL WriteToListFile();

BOOL convertsmall();

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if (argc != 2) 
  {
    printf("\n");
    printf("Using:./expdbfiles xmlbuffer\n");

    printf("Example:/htidc/htidc/bin/procctl 20 /htidc/htidc/bin/expdbfiles \"<connstr>szidc/pwdidc@SZQX_10.153.98.13</connstr><logfilename>/log/szqx/expdbfiles_radorg_tohk.log</logfilename><listfilename>/qxdata/szqx/explist/radorg_tohk.list</listfilename><exportsql>select filename,to_char(upttime,'yyyymmddhh24miss'),filesize,filecontent from T_RADORGFILES where ddatetime>=sysdate-0.1 and upper(filename) like 'Z_RADR_I_Z9755_%%%%' order by ddatetime</exportsql><topathtmp>/qxdata/szqx/exptmp</topathtmp><topath>/qxdata/szqx/tohk</topath><convertcfg></convertcfg>\"\n\n");

    printf("�˳��������İѶ������ļ������ݵı�����ȡ����������ָ����Ŀ¼�¡�\n");
    printf("connstr ���ݿ����Ӳ�����\n");
    printf("logfilename ���������в�������־�ļ�����\n");
    printf("listfilename �ѵ����ļ����嵥�ļ�����\n");
    printf("exportsql ��ѯ�����������ļ���ŵı�Ĳ�����\n");
    printf("topathtmp �����Ķ����������ļ���ŵ���ʱĿ¼��\n");
    printf("topath �����Ķ����������ļ���ŵ���ʽĿ¼��\n");
    printf("convertcfg �����ǿ�ѡ�ģ������������ͼƬ�ļ����ò���ָ����������ͼ�Ĳ�����\n\n\n");

    return -1;
  }

  memset(strconnstr,0,sizeof(strconnstr));
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strlistfilename,0,sizeof(strlistfilename));
  memset(strexportsql,0,sizeof(strexportsql));
  memset(strtopathtmp,0,sizeof(strtopathtmp));
  memset(strtopath,0,sizeof(strtopath));
  memset(strconvertcfg,0,sizeof(strconvertcfg));

  
  GetXMLBuffer(argv[1],"connstr",strconnstr,300);
  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"listfilename",strlistfilename,300);
  GetXMLBuffer(argv[1],"exportsql",strexportsql,1000);
  GetXMLBuffer(argv[1],"topathtmp",strtopathtmp,300);
  GetXMLBuffer(argv[1],"topath",strtopath,300);
  GetXMLBuffer(argv[1],"convertcfg",strconvertcfg,300);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  // ע�⣬����ʱ��1800��
  ProgramActive.SetProgramInfo(&logfile,"expdbfiles",1800);

  // �������ݿ�
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // ���Ѵ����ļ������ݶ���vOldFILELIST
  ReadFromListFile();

  // �����ݿ�����µ��ļ�������
  if (ExpDBFiles() != 0)
  {
    logfile.Write("ExpDBFiles failed.\n"); CallQuit(-1);
  }

  // ��vNewFILELIST�е�����д��list�ļ�
  if (WriteToListFile() == FALSE)
  {
    logfile.Write("WriteToListFile failed.\n"); CallQuit(-1);
  }
  
  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("expdbfiles exit.\n");

  exit(0);
}

// ���Ѵ����ļ������ݶ���vOldFILELIST
BOOL ReadFromListFile()
{
  vOldFILELIST.clear();

  FILE *fp=0;

  // ����ǵ�һ�����У�list�ļ������Ͳ�����
  if ( (fp=fopen(strlistfilename,"r")) == NULL) return TRUE;

  char strLine[1024];

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    if (FGETS(strLine,1000,fp,"endl") == FALSE) break;

    memset(&stFILELIST,0,sizeof(stFILELIST));

    GetXMLBuffer(strLine,"filename", stFILELIST.filename);
    GetXMLBuffer(strLine,"upttime" , stFILELIST.upttime);
    GetXMLBuffer(strLine,"filesize",&stFILELIST.filesize);

    vOldFILELIST.push_back(stFILELIST);
  }

  return TRUE;
}

// ��vNewFILELIST�е�����д��list�ļ�
BOOL WriteToListFile()
{
  FILE *fp=0;

  if ( (fp=fopen(strlistfilename,"w")) == NULL)
  {
    logfile.Write("fopen %s failed.\n",strlistfilename); return FALSE;
  }
  
  for (UINT ii=0; ii< vNewFILELIST.size(); ii++)
  {
    fprintf(fp,"<filename>%s</filename><upttime>%s</upttime><filesize>%d</filesize>endl\n",\
               vNewFILELIST[ii].filename,vNewFILELIST[ii].upttime,vNewFILELIST[ii].filesize);
  }

  fprintf(fp,"END");

  fclose(fp);

  return TRUE;
}

// �����ݿ�����µ��ļ�������
long ExpDBFiles()
{
  vNewFILELIST.clear();

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare(strexportsql);
  stmt.bindout(1, stFILELIST.filename,200);
  stmt.bindout(2, stFILELIST.upttime ,14);
  stmt.bindout(3,&stFILELIST.filesize);
  stmt.bindblob(4);

  if (stmt.execute() != 0)
  {
    logfile.Write("%s\n%s\n",strexportsql,stmt.cda.message); return stmt.cda.rc;
  }

  int  iret;
  char strTempFileName[201],strFullFileName[201];

  while (TRUE)
  {
    memset(&stFILELIST,0,sizeof(stFILELIST));

    if (stmt.next() != 0) break;

    // ���ӱ��л�ȡ���ļ���Ϣ��֮ǰ�ѵ������ļ���Ϣ�Ƿ���ͬ�������ͬ����continue
    if (CheckInOldList() == TRUE) { vNewFILELIST.push_back(stFILELIST); continue; }

    // ��ʼ�����ļ�
    logfile.Write("export %s ...",stFILELIST.filename);

    memset(strTempFileName,0,sizeof(strTempFileName));
    memset(strFullFileName,0,sizeof(strFullFileName));
    sprintf(strTempFileName,"%s/%s.%d",strtopathtmp,stFILELIST.filename,getpid());
    sprintf(strFullFileName,"%s/%s",strtopath,stFILELIST.filename);

    iret = stmt.lobtofile(strFullFileName);

    if ( (iret == 0) && (stFILELIST.filesize==FileSize(strFullFileName)) )
    {
      rename(strTempFileName,strFullFileName); logfile.WriteEx("ok.\n");

      // ת��Ϊ����ͼ
      if (strlen(strconvertcfg) != 0) convertsmall(); 

      vNewFILELIST.push_back(stFILELIST);
    }
    else
    {
      logfile.WriteEx("FAILED(filesize=%d).\n%d,%s\n",FileSize(strFullFileName),stmt.cda.rc,stmt.cda.message);
    }

    ProgramActive.WriteToFile();
  }

  return 0;
}

// ���ӱ��л�ȡ���ļ���Ϣ��֮ǰ�ѵ������ļ���Ϣ�Ƿ���ͬ�������ͬ����continue
BOOL CheckInOldList()
{
  for (UINT ii=0; ii< vOldFILELIST.size(); ii++)
  {
    if ( (strcmp(stFILELIST.filename,vOldFILELIST[ii].filename) == 0) && 
         (strcmp(stFILELIST.upttime ,vOldFILELIST[ii].upttime)  == 0) && 
         (stFILELIST.filesize == vOldFILELIST[ii].filesize) ) 
    {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL convertsmall()
{
  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strconvertcfg,",");
  
  char strCmd[501];

  for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
  {
    memset(strCmd,0,sizeof(strCmd));
    sprintf(strCmd,"/usr/local/bin/convert %s/%s %s %s/small%lu_%s",strtopath,stFILELIST.filename,CmdStr.m_vCmdStr[ii].c_str(),strtopath,ii+1,stFILELIST.filename);
    system(strCmd);
  }

  return TRUE;
}
