#include "_public.h"
#include "ftp.h"

void CallQuit(int sig);

Cftp           ftp;
CLogFile       logfile;
CProgramActive ProgramActive;

FILE *listfp;

char strremoteip[21];        // Զ�̷�������IP
UINT uport;                  // Զ�̷�����FTP�Ķ˿�
char strmode[11];            // ����ģʽ��port��pasv
char strusername[31];        // Զ�̷�����FTP���û���
char strpassword[31];        // Զ�̷�����FTP������
char strremotepath[301];     // Զ�̷���������ļ���Ŀ¼
char strmatchname[301];      // ���ɼ��ļ�ƥ����ļ���
UINT utimeout;               // FTP�ɼ��ļ��ĳ�ʱʱ��

// ������Զ�̷����������ӣ�����¼
BOOL ftplogin();

// ��Զ�̷�����get�ļ�
BOOL ftpdelfiles();

// �ļ��б����ݽṹ
struct st_filelist
{
  char remotefilename[301];
  char remotefilenamebak[301];
  char localfilename[301];
  char modtime[21];
  int  filesize;
  int  ftpsts;   // 1-�ƻ��ɼ���2-����ɼ���
};

// �ļ���Ϣ����ʱ���ݽṹ�����κκ����ж�����ֱ����
struct st_filelist stfilelist;

// Զ�̷������ļ��嵥������
vector<struct st_filelist> m_vrfilelist;

// ��ȡԶ�̷��������ļ��嵥
BOOL nlist();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpdelfiles logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpdelfiles /tmp/htidc/log/ftpdelfiles_211.139.255.23_oracle_gzradpic.log \"<remoteip>211.139.255.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>oracle</password><remotepath>/tmp/radpic/gzrad</remotepath><matchname>CSQX*.GIF</matchname><listfilename>/tmp/htidc/list/ftpdelfiles_211.139.255.23_oracle_gzradpic.xml</listfilename><timeout>1800</timeout>\"\n\n\n");

    printf("���������������ĵĹ�������ģ�飬����ɾ��Զ��FTP���������ļ���\n");
    printf("logfilename�Ǳ��������е���־�ļ���һ�����/tmp/htidc/logĿ¼�£�"\
           "����ftpdelfiles_Զ�̵�ַ_ftp�û���_�ļ�����.log��������ʽ��\n");
    printf("xmlbufferΪ�ļ�����Ĳ��������£�\n");
    printf("<remoteip>211.139.255.23</remoteip> Զ�̷�������IP��\n");
    printf("<port>21</port> Զ�̷�����FTP�Ķ˿ڡ�\n");
    printf("<mode>pasv</mode> ����ģʽ��pasv��port����ѡ�ֶΣ�ȱʡ����pasvģʽ��\n");
    printf("<username>oracle</username> Զ�̷�����FTP���û�����\n");
    printf("<password>oracle1234HOTA</password> Զ�̷�����FTP�����롣\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> Զ�̷���������ļ���Ŀ¼��\n");
    printf("<matchname>*.GIF</matchname> ��ɾ���ļ�ƥ����ļ��������ô�дƥ�䣬"\
           "��ƥ����ļ����ᱻ�ɼ������ֶξ��������þ�ȷ����������*ƥ��ȫ�����ļ���\n");
    printf("<timeout>1800</timeout> FTPɾ���ļ��ĳ�ʱʱ�䣬��λ���룬ע�⣬����ȷ������ĳ��"\
           "���ļ���ʱ��С��timeout��ȡֵ���������ɴ���ʧ�ܵ������\n");
  
    printf("port ģʽ�ǽ����ӷ������߶˿������ͻ���20�˿��������ӡ�\n");
    printf("pasv ģʽ�ǽ����ͻ��߶˿��������������ص����ݶ˿ڵ��������ӡ�\n\n");

    printf("port����������ʽ�����ӹ����ǣ��ͻ������������FTP�˿ڣ�Ĭ����21��������������"\
           "�������������ӣ�����һ��������·��\n");
    printf("����Ҫ��������ʱ����������20�˿���ͻ��˵Ŀ��ж˿ڷ����������󣬽���һ��������·���������ݡ�\n\n");

    printf("pasv����������ʽ�����ӹ����ǣ��ͻ������������FTP�˿ڣ�Ĭ����21��������������"\
           "�������������ӣ�����һ��������·��\n");
    printf("����Ҫ��������ʱ���ͻ�����������Ŀ��ж˿ڷ����������󣬽���һ��������·���������ݡ�\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001]; 

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("ftpdelfiles");

  memset(strremoteip,0,sizeof(strremoteip));          // Զ�̷�������IP
  uport=21;                                           // Զ�̷�����FTP�Ķ˿�
  memset(strmode,0,sizeof(strmode));                  // ����ģʽ��port��pasv
  memset(strusername,0,sizeof(strusername));          // Զ�̷�����FTP���û���
  memset(strpassword,0,sizeof(strpassword));          // Զ�̷�����FTP������
  memset(strremotepath,0,sizeof(strremotepath));      // Զ�̷���������ļ���Ŀ¼
  memset(strmatchname,0,sizeof(strmatchname));        // ���ɼ��ļ�ƥ����ļ���
  utimeout=0;                                         // FTP�ɼ��ļ��ĳ�ʱʱ��

  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);

  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);

  // mode�ǿ���Ϊ�յģ����Ϊ�գ��Ͳ���pasvģʽ
  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);
  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (uport == 0) { logfile.Write("port is null.\n"); return -1; }
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (strcmp(strmatchname,"*") == 0) { logfile.Write("matchname is vailed(* only).\n"); return -1; }
  if (utimeout == 0) { logfile.Write("timeout is null.\n"); return -1; }

  // ע�⣬����ʱ��utimeout��
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",utimeout);

  // ��¼Զ��FTP������
  if (ftplogin() == FALSE) return FALSE;

  // ��ȡԶ�̷��������ļ��嵥
  if (nlist() == FALSE) return FALSE;

  for (UINT ii=0;ii<m_vrfilelist.size();ii++)
  {
    if (ftp.ftpdelete(stfilelist.remotefilename) == FALSE)
    {
      logfile.Write("ftp.ftpdelete(%s) failed.\n%s",stfilelist.remotefilename,ftp.response()); continue;
    }
  }

  ftp.logout();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  logfile.Write("ftpdelfiles exit.\n");

  exit(0);
}

// ������Զ�̷����������ӣ�����¼
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // ���ô���ģʽ
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP���Ӻ͵�¼�ĳ�ʱʱ����Ϊ80�͹��ˡ�
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) FAILED.\n\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // ����ĳ�ʱʱ������Ϊutimeout�롣
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",utimeout);

  return TRUE;
}


// ��ȡԶ�̷��������ļ��嵥
BOOL nlist()
{
  m_vrfilelist.clear();

  char strListFileName[301];
  memset(strListFileName,0,sizeof(strListFileName));
  snprintf(strListFileName,300,"/tmp/htidc/tmp/ftp_%s_%s_%d.list",strremoteip,strusername,getpid());
  MKDIR("/tmp/htidc/tmp",FALSE);

  // ����Ŀ��Ŀ¼
  if (ftp.chdir(strremotepath) == FALSE)
  {
    // �������Ŀ��Ŀ¼ʧ�ܣ��п�����Ŀ��Ŀ¼��δ����
    logfile.Write("ftp.chdir(%s) failed.\n%s",strremotepath,ftp.response()); return FALSE;
  }

  // logfile.Write("chdir ok.\n");

  char strMatch[2];
  memset(strMatch,0,sizeof(strMatch));

  // ��ȡ�Է����ļ��嵥ʱ��������Ҫ������ʱ�䣬������������Ϊ1200��
  ProgramActive.SetProgramInfo(&logfile,"ftpdelfiles",1200);

  // ��ȡԶ�̷��������ļ��嵥���ֱ���"."��"*"��" "���ԡ�
  for (int ii=0;ii<3;ii++)
  {
    if (ii==0) strMatch[0]='*';
    if (ii==1) strMatch[0]='.';
    if (ii==2) strMatch[0]=0;

    REMOVE(strListFileName); 

    ftp.nlist(strMatch,strListFileName);

    ProgramActive.WriteToFile();

    if (FileSize(strListFileName) > 2) break;

    REMOVE(strListFileName); 
  }

  // �ж��嵥�ļ��Ƿ�Ϊ�գ�Ϊ�վ�ֱ�ӷ���
  if (FileSize(strListFileName) <= 0) return FALSE;

  char strLine[301];

  listfp=0;

  // ��Զ�̷��������ļ��嵥�ļ����ص�m_vrfilelist
  if ( (listfp=FOPEN(strListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strListFileName); REMOVE(strListFileName); return FALSE; 
  }

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    // ���ļ��л�ȡһ��
    if (FGETS(strLine,300,listfp) == FALSE) break;

    // ���Զ�̵��ļ���ƥ��TMP,TEMP,*TOPWALK*,*SWP������������ļ�
    if (MatchFileName(strLine,"*TMP,*TEMP,*TOPWALK*,*SWP") == TRUE) continue;

    // ���Զ�̵��ļ�����ƥ��strmatchname������������ļ�
    if (MatchFileName(strLine,strmatchname) == FALSE) continue;

    memset(&stfilelist,0,sizeof(stfilelist));

    snprintf(stfilelist.remotefilename,300,"%s/%s",strremotepath,strLine);

    stfilelist.ftpsts=1;

    m_vrfilelist.push_back(stfilelist);
  }

  fclose(listfp); 

  // ɾ����ʹ�õ�Զ�̷��������ļ��嵥�ļ���
  REMOVE(strListFileName);

  return TRUE;
}

