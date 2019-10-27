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
char strlocalpath[301];      // �����ļ���ŵ�Ŀ¼
char strremotepath[301];     // Զ�̷���������ļ���Ŀ¼
char strremotepathbak[301];  // Զ�̷���������ļ��ı���Ŀ¼
char strmatchname[301];      // ���ɼ��ļ�ƥ����ļ���
char strlistfilename[301];   // ����Ѳɼ��ļ��б��xml�ļ�
UINT udeleteremote;          // �ɼ��ɹ����Ƿ�ɾ��Զ�̷��������ļ���1-ɾ����2-��ɾ��
UINT utimeout;               // FTP�ɼ��ļ��ĳ�ʱʱ��
UINT umtime;                 // �ļ�mtime����Чʱ��
int  itimetvl;

// ������Զ�̷����������ӣ�����¼
BOOL ftplogin();

// ��Զ�̷�����get�ļ�
BOOL ftpgetfiles();

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

// ���ط������ļ��嵥������
vector<struct st_filelist> m_vlfilelist;
 
// ��ȡԶ�̷��������ļ��嵥
BOOL nlist();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpgetfiles logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpgetfiles /tmp/htidc/log/ftpgetfiles_211.139.255.23_oracle_gzradpic.log \"<remoteip>211.139.255.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>oracle1234HOTA</password><localpath>/tmp/htidc/ftpget/gzrad</localpath><remotepath>/tmp/radpic/gzrad</remotepath><remotepathbak></remotepathbak><matchname>*.GIF</matchname><listfilename>/tmp/htidc/list/ftpgetfiles_211.139.255.23_oracle_gzradpic.xml</listfilename><deleteremote>2</deleteremote><timeout>1800</timeout><timetvl>-8</timetvl><mtime>24</mtime>\"\n\n\n");

    printf("���������������ĵĹ�������ģ�飬���ڰ�Զ��FTP���������ļ��ɼ�������Ŀ¼��\n");
    printf("������רΪ��ȡ���ļ���Ƶģ�ȡһ����дһ����¼��XML�ļ�����Ϊ�����ļ��Ƚϴ�,"\
           "ʱ��ʱ�ͳ��ִ���ʧ�ܣ���û�а��Ѿ�����ɹ���д��XML�ļ��������ٴ��ش���\n");
    printf("logfilename�Ǳ��������е���־�ļ���һ�����/tmp/htidc/logĿ¼�£�"\
           "����ftpgetfiles_Զ�̵�ַ_ftp�û���_�ļ�����.log��������ʽ��\n");
    printf("xmlbufferΪ�ļ�����Ĳ��������£�\n");
    printf("<remoteip>211.139.255.23</remoteip> Զ�̷�������IP��\n");
    printf("<port>21</port> Զ�̷�����FTP�Ķ˿ڡ�\n");
    printf("<mode>pasv</mode> ����ģʽ��pasv��port����ѡ�ֶΣ�ȱʡ����pasvģʽ��\n");
    printf("<username>oracle</username> Զ�̷�����FTP���û�����\n");
    printf("<password>oracle1234HOTA</password> Զ�̷�����FTP�����롣\n");
    printf("<localpath>/tmp/htidc/ftpget</localpath> �����ļ���ŵ�Ŀ¼��\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> Զ�̷���������ļ���Ŀ¼��\n");
    printf("<remotepathbak>/tmp/radpic/gzradbak</remotepath> Զ�̷���������ļ��ı���Ŀ¼��"\
           "���Ϊ�գ����ʾ�����ݣ����Ƿ�ɾ������deleteremote����������\n");
    printf("<deleteremote>2</deleteremote> �ɼ��ɹ����Ƿ�ɾ��Զ�̷��������ļ���"\
           "1-ɾ����2-��ɾ�����˲���������һ��Ҫѯ��Զ�̷�������ϵͳ����Ա��"\
           "ֻ�е��Է������ҷ�����ɾ��Ȩ�޵�����²�������Ϊ1-ɾ�������deleteremoteΪ1��remotepathbak��ʧЧ��\n");
    printf("<matchname>*.GIF</matchname> ���ɼ��ļ�ƥ����ļ��������ô�дƥ�䣬"\
           "��ƥ����ļ����ᱻ�ɼ������ֶξ��������þ�ȷ����������*ƥ��ȫ�����ļ���\n");
    printf("<listfilename>/tmp/htidc/list/ftpgetfiles_211.139.255.23_oracle_gzradpic.xml</listfilename> ��"\
           "���Ѳɼ��ļ��б��xml�ļ���һ�����/tmp/htidc/listĿ¼�£�"\
           "����ftpgetfiles_Զ�̵�ַ_ftp�û���_�ļ�����.xml��������ʽ��\n");
    printf("<timeout>1800</timeout> FTP�ɼ��ļ��ĳ�ʱʱ�䣬��λ���룬ע�⣬����ȷ������ĳ��"\
           "���ļ���ʱ��С��timeout��ȡֵ���������ɴ���ʧ�ܵ������\n");
    printf("<mtime>24</mtime> �������ļ�ʱ��ķ�Χ����λ��Сʱ������ļ�ʱ���ڱ�����֮ǰ���Ͳ����䡣\n\n");
    printf("xmlbuffer���Դ���ʱ�������<timetvl>-8</timetvl> Ϊʱ�������ƫ��ʱ�䡣"\
           "Ŀǰ���Դ�������ʱ�������{YYYY}��4λ���꣩��{YYY}������λ���꣩��"\
           "{YY}������λ���꣩��{MM}�����£���{DD}�����գ���{HH}��ʱʱ����{MI}���ַ֣���{SS}�����룩��\n");
    printf("���ϵĲ���ֻ��mode��timetvl��remotepathbak��mtimeΪ��ѡ�����������Ķ����\n\n\n");

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
  logfile.SetAlarmOpt("ftpgetfiles");

  memset(strremoteip,0,sizeof(strremoteip));          // Զ�̷�������IP
  uport=21;                                           // Զ�̷�����FTP�Ķ˿�
  memset(strmode,0,sizeof(strmode));                  // ����ģʽ��port��pasv
  memset(strusername,0,sizeof(strusername));          // Զ�̷�����FTP���û���
  memset(strpassword,0,sizeof(strpassword));          // Զ�̷�����FTP������
  memset(strlocalpath,0,sizeof(strlocalpath));        // �����ļ���ŵ�Ŀ¼
  memset(strremotepath,0,sizeof(strremotepath));      // Զ�̷���������ļ���Ŀ¼
  memset(strremotepathbak,0,sizeof(strremotepathbak));// Զ�̷���������ļ��ı���Ŀ¼
  memset(strmatchname,0,sizeof(strmatchname));        // ���ɼ��ļ�ƥ����ļ���
  memset(strlistfilename,0,sizeof(strlistfilename));  // ����Ѳɼ��ļ��б��xml�ļ�
  udeleteremote=2;                                    // �ɼ��ɹ����Ƿ�ɾ��Զ�̷��������ļ���1-ɾ����2-��ɾ��
  utimeout=0;                                         // FTP�ɼ��ļ��ĳ�ʱʱ��
  umtime=0;
  itimetvl=0;

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);
  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);

  // ����xmlbuffer�е�ʱ�����
  MatchBuffer(strXmlBuffer,itimetvl);

  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"localpath",strlocalpath,300);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,300);
  GetXMLBuffer(strXmlBuffer,"remotepathbak",strremotepathbak,300);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"listfilename",strlistfilename,300);
  GetXMLBuffer(strXmlBuffer,"deleteremote",&udeleteremote);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);
  GetXMLBuffer(strXmlBuffer,"mtime",&umtime);

  // mode�ǿ���Ϊ�յģ����Ϊ�գ��Ͳ���pasvģʽ
  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);

  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (uport == 0) { logfile.Write("port is null.\n"); return -1; }
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strpassword) == 0) { logfile.Write("password is null.\n"); return -1; }
  if (strlen(strlocalpath) == 0) { logfile.Write("localpath is null.\n"); return -1; }
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1; }
  if (strlen(strmatchname) == 0) { logfile.Write("matchname is null.\n"); return -1; }
  if (strcmp(strmatchname,"*") == 0) { logfile.Write("matchname is vailed(* only).\n"); return -1; }
  if (strlen(strlistfilename) == 0) { logfile.Write("listfilename is null.\n"); return -1; }
  if (udeleteremote == 0) { logfile.Write("deleteremote is null.\n"); return -1; }
  if (utimeout == 0) { logfile.Write("timeout is null.\n"); return -1; }

  // ע�⣬����ʱ��utimeout��
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",utimeout);

  // ��¼Զ��FTP������
  if (ftplogin() == FALSE) return FALSE;

  // ��ȡԶ�̷��������ļ��嵥
  if (nlist() == FALSE) return FALSE;

  listfp=0;

  // �򿪱���XML�嵥�ļ�
  if ( (listfp=FOPEN(strlistfilename,"a+")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strlistfilename); return -1;
  }

  //fprintf(listfp,"<data>\n");

  for (UINT ii=0;ii<m_vrfilelist.size();ii++)
  {
    // ���ļ��ɼ������ط�����Ŀ¼
    if (m_vrfilelist[ii].ftpsts==1)
    {
      memcpy(&stfilelist,&m_vrfilelist[ii],sizeof(stfilelist));

      sleep(3);

      if (ftpgetfiles() == TRUE)
      {
        fprintf(listfp,"<filename>%s</filename><modtime>%s</modtime><filesize>%d</filesize><endl/>\n",stfilelist.remotefilename,stfilelist.modtime,stfilelist.filesize);
        fflush(listfp);
        sleep(1);
       }
    }
  }

  //fprintf(listfp,"</data>\n");

  fclose(listfp);

  ftp.logout();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  if (listfp != 0) fclose(listfp);

  logfile.Write("ftpgetfiles exit.\n");

  exit(0);
}

// ������Զ�̷����������ӣ�����¼
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // ���ô���ģʽ
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP���Ӻ͵�¼�ĳ�ʱʱ����Ϊ80�͹��ˡ�
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) FAILED.\n\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // ����ĳ�ʱʱ������Ϊutimeout�롣
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",utimeout);

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
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",1200);

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

  // ����ĳ�ʱʱ������Ϊutimeout�롣
  ProgramActive.SetProgramInfo(&logfile,"ftpgetfiles",utimeout);

  // �ж��嵥�ļ��Ƿ�Ϊ�գ�Ϊ�վ�ֱ�ӷ���
  if (FileSize(strListFileName) <= 0) return FALSE;

  // logfile.Write("nlist ok.\n");

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
    if (strlen(strremotepathbak) != 0)
    {
      snprintf(stfilelist.remotefilenamebak,300,"%s/%s",strremotepathbak,strLine);
    }
    snprintf(stfilelist.localfilename ,300,"%s/%s",strlocalpath ,strLine);

    stfilelist.ftpsts=1;

    m_vrfilelist.push_back(stfilelist);
  }

  fclose(listfp); 

  // logfile.Write("nlist load ok.\n");

  // ɾ����ʹ�õ�Զ�̷��������ļ��嵥�ļ���
  REMOVE(strListFileName);
  
  char strmtime[21];
  memset(strmtime,0,sizeof(strmtime));
  LocalTime(strmtime,"yyyymmddhh24miss",0-umtime*60*60);

  // ��ȡԶ�̷�����ÿ���ļ����޸�ʱ����ļ���С��Ϣ
  for (UINT ii=0;ii<m_vrfilelist.size();ii++)
  {
    // �����ȡԶ�̷�����ÿ���ļ����޸�ʱ����ļ���С��Ϣʧ�ܣ�����ʧ��
    if (ftp.mtime(m_vrfilelist[ii].remotefilename) == FALSE) 
    {
      logfile.Write("ftp.mtime(%s) FAILED.\n%s",m_vrfilelist[ii].remotefilename,ftp.response()); return FALSE; 
    }

    // ����ļ�ʱ��С��umtime���Ͷ�������ļ�
    if (umtime>0) 
    {
      if (strcmp(ftp.m_mtime,strmtime) < 0) 
      {
        m_vrfilelist.erase(m_vrfilelist.begin()+ii); ii--; continue;
      }
    }

    // �����ȡԶ�̷�����ÿ���ļ����޸�ʱ����ļ���С��Ϣʧ�ܣ�����ʧ��
    if (ftp.size(m_vrfilelist[ii].remotefilename) == FALSE) 
    {
      logfile.Write("ftp.size(%s) FAILED.\n%s",m_vrfilelist[ii].remotefilename,ftp.response()); return FALSE; 
    }

    strcpy(m_vrfilelist[ii].modtime,ftp.m_mtime);

    m_vrfilelist[ii].filesize=ftp.m_size;

    /*
    // ���ٲ��������ж�
    // ����ļ���С��0���ͷ�������ļ���������ᡣ
    if (m_vrfilelist[ii].filesize == 0)
    {
      m_vrfilelist.erase(m_vrfilelist.begin()+ii); ii--; continue;
    }
    */
  }

  // logfile.Write("nlist analy ok.\n");

  m_vlfilelist.clear();

  listfp=0;

  // �ѱ��ط��������ļ��嵥�ļ����ص�m_vlfilelist
  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // ���ļ��л�ȡһ��
      if (FGETS(strLine,500,listfp) == FALSE) break;
      if (strstr(strLine,"<endl/>") != 0)
      {
        memset(&stfilelist,0,sizeof(stfilelist));

	GetXMLBuffer(strLine,"filename", stfilelist.remotefilename,300);
	GetXMLBuffer(strLine,"modtime",  stfilelist.modtime,14);
	GetXMLBuffer(strLine,"filesize",&stfilelist.filesize);

	m_vlfilelist.push_back(stfilelist);
      } 
    }

    fclose(listfp);
  }

  for (UINT ii=0;ii<m_vrfilelist.size();ii++)
  {
    if (m_vlfilelist.size() == 0) continue;

    // ��Զ��Ŀ¼���ļ�������С�����ںͱ��ص��嵥�ļ��Ƚ�һ�£����ȫ����ͬ���Ͱ�ftpsts����Ϊ2�����ٲɼ�
    for (UINT jj=0;jj<m_vlfilelist.size();jj++)
    {
      if (strcmp(m_vrfilelist[ii].remotefilename,m_vlfilelist[jj].remotefilename) == 0) 
      {
        if ( (strcmp(m_vrfilelist[ii].modtime,m_vlfilelist[jj].modtime) == 0) &&
             (m_vrfilelist[ii].filesize==m_vlfilelist[jj].filesize) )
        {
          m_vrfilelist[ii].ftpsts=2; 
        }

        break;
      }
    }
  }

  return TRUE;
}

// ��Զ�̷�����get�ļ�
BOOL ftpgetfiles()
{
  ProgramActive.WriteToFile();

  // ���ٲ��������ж�
  // ����ļ���С��0���ͷ�������ļ���������ᡣ
  // if (stfilelist.filesize == 0) return TRUE;
  
  CTimer Timer;

  Timer.Beginning();

  logfile.Write("get %s ...%s.",stfilelist.remotefilename,stfilelist.modtime);

  BOOL bIsLogFile=MatchFileName(stfilelist.localfilename,"*.LOG");

  MKDIR(stfilelist.localfilename,TRUE);

  // ��ȡ�ļ�
  if (ftp.get(stfilelist.remotefilename,stfilelist.localfilename,bIsLogFile) == FALSE)
  {
    logfile.WriteEx("FAILED.\n%s",ftp.response()); return FALSE;
  }

  logfile.WriteEx("ok(size=%d,time=%d).\n",stfilelist.filesize,Timer.Elapsed()); 

  // �ж��Ƿ���Ҫɾ��Զ�̷��������ļ������ɾ�����ɹ���Ҳ���ط���ʧ�ܡ�
  if (udeleteremote == 1)  
  {
    if (ftp.ftpdelete(stfilelist.remotefilename) == FALSE)
    {
      logfile.Write("ftp.ftpdelete(%s) failed.\n%s",stfilelist.remotefilename,ftp.response()); return TRUE;
    }
  }
  else
  {
    // �������Ҫɾ�����ж��Ƿ���Ҫ����������������ɹ���Ҳ���ط���ʧ�ܡ�
    if (strlen(strremotepathbak) != 0)
    {
      if (ftp.ftprename(stfilelist.remotefilename,stfilelist.remotefilenamebak) == FALSE)
      {
        logfile.Write("ftp.ftprename(%s,%s) failed.\n%s",stfilelist.remotefilename,stfilelist.remotefilenamebak,ftp.response()); return TRUE;
      }
    }
  }

  return TRUE;
}

