#include "_public.h"
#include "_oracle.h"

char strconnstr[301];      // ���ݿ����Ӳ���
char strlogfilename[301];  // ��־�ļ�Ŀ¼
char strexportsql[1024];   // SQL���
char strtopathtmp[301];    // �����ļ���ʱ��ŵ�Ŀ¼
char strtopath[301];       // �����ļ����մ�ŵ�Ŀ¼
char strifdel[11];         // �������ݺ��Ƿ�ɾ������
char strtablename[51];     // ����

connection     conn;
CLogFile       logfile;
CIniFile       IniFile;
CCmdStr        CmdStr;
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

// �����ݿ�����µ��ļ�������
long ExpDBFiles();

void CallQuit(int sig);

int main(int argc,char *argv[])
{
  if (argc != 2) 
  {
    printf("\n");
    printf("Using:./expdbfiles1 xmlbuffer\n");

    printf("Example:/htidc/htidc/bin/procctl 20 /htidc/htidc/bin/expdbfiles1 \"<connstr>szidc/pwdidc@SZQX_10.153.98.13</connstr><logfilename>/log/szqx/expdbfiles1_radorg_tohk.log</logfilename><exportsql>select filename,to_char(ddatetime,'yyyymmddhh24miss'),filesize,filecontent from T_RADORGFILES where ddatetime>=sysdate-0.1 and upper(filename) like 'Z_RADR_I_Z9755_%%%%' order by ddatetime</exportsql><topathtmp>/qxdata/szqx/exptmp</topathtmp><topath>/qxdata/szqx/tohk/{YYYY}/{MM}/{DD}</topath><ifdel>TRUE</ifdel>\"\n\n");

    printf("�˳���Ѷ������ļ������ݵı�����ȡ����������ָ����Ŀ¼�£�������ָ���Ƿ�ɾ�����е����ݡ�\n");
    printf("connstr ���ݿ����Ӳ�����\n");
    printf("logfilename ���������в�������־�ļ�����\n");
    printf("exportsql ��ѯ�����������ļ���ŵı�Ĳ�����filename,ddatetime,filesize,filecontent �������ֶι̶���\n");
    printf("topathtmp �����Ķ����������ļ���ŵ���ʱĿ¼��\n");
    printf("topath �����Ķ����������ļ���ŵ���ʽĿ¼��\n");
    printf("ifdel �����ɹ����Ƿ�ɾ�����ݣ�TRUE-ɾ����FALSE-��ɾ����\n");
    printf("Ŀǰ���Դ�������ʱ�������{YYYY}��4λ���꣩��{MM}�����£���{DD}�����գ���{HH}��ʱʱ����{MI}���ַ֣���{SS}�����룩��\n");

    return -1;
  }

  memset(strconnstr,0,sizeof(strconnstr));
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strexportsql,0,sizeof(strexportsql));
  memset(strtopathtmp,0,sizeof(strtopathtmp));
  memset(strtopath,0,sizeof(strtopath));
  memset(strifdel,0,sizeof(strifdel));
  
  GetXMLBuffer(argv[1],"connstr",strconnstr,300);
  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"exportsql",strexportsql,1000);
  GetXMLBuffer(argv[1],"topathtmp",strtopathtmp,300);
  GetXMLBuffer(argv[1],"topath",strtopath,300);
  GetXMLBuffer(argv[1],"ifdel",strifdel,10);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  // ע�⣬����ʱ��3600��
  ProgramActive.SetProgramInfo(&logfile,"expdbfiles1",3600);

  // �������ݿ�
  if (conn.connecttodb(strconnstr,TRUE) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // ȡ������
  memset(strtablename,0,sizeof(strtablename));
  CmdStr.SplitToCmd(strexportsql," ");
  for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
  {
    if(strcmp(CmdStr.m_vCmdStr[ii].c_str(),"from") == 0 || strcmp(CmdStr.m_vCmdStr[ii].c_str(),"FROM") == 0)
    {
      strcpy(strtablename,CmdStr.m_vCmdStr[ii+1].c_str());
      break;
    }
  }

  // �����ݿ�����µ��ļ�������
  if (ExpDBFiles() != 0)
  {
    logfile.Write("ExpDBFiles failed.\n"); CallQuit(-1);
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("expdbfiles1 exit.\n");

  exit(0);
}

// �����ݿ�����µ��ļ�������
long ExpDBFiles()
{
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare(strexportsql);
  stmt.bindout(1, stFILELIST.filename,200);
  stmt.bindout(2, stFILELIST.ddatetime,20);
  stmt.bindout(3,&stFILELIST.filesize);
  stmt.bindblob(4);

  if (stmt.execute() != 0)
  {
    logfile.Write("%s\n%s\n",strexportsql,stmt.cda.message); return stmt.cda.rc;
  }

  int  iret;
  char strTempFileName[201],strFullFileName[201];
  char yyyy[5],mm[3],dd[3],hh[3],mi[3],ss[3];
  char strstdpathtmp[201],strstdpath[201];

  sqlstatement stdel;
  stdel.connect(&conn);
  stdel.prepare("delete from %s where filename=:1",strtablename);
  stdel.bindin(1,stFILELIST.filename,200);

  while (TRUE)
  {
    memset(&stFILELIST,0,sizeof(st_FILELIST));

    if (stmt.next() != 0) break;

    memset(yyyy,0,sizeof(yyyy));
    memset(mm,0,sizeof(mm));
    memset(dd,0,sizeof(dd));
    memset(hh,0,sizeof(hh));
    memset(mi,0,sizeof(mi));
    memset(ss,0,sizeof(ss));
 
    strncpy(yyyy,stFILELIST.ddatetime,4);
    strncpy(mm,stFILELIST.ddatetime+4,2);
    strncpy(dd,stFILELIST.ddatetime+6,2);
    strncpy(hh,stFILELIST.ddatetime+8,2);
    strncpy(mi,stFILELIST.ddatetime+10,2);
    strncpy(ss,stFILELIST.ddatetime+12,2);

    memset(strstdpathtmp,0,sizeof(strstdpathtmp));
    memset(strstdpath,0,sizeof(strstdpath));

    strcpy(strstdpathtmp,strtopathtmp);
    strcpy(strstdpath,strtopath);

    UpdateStr(strstdpathtmp,"{YYYY}",yyyy);
    UpdateStr(strstdpathtmp,"{MM}",mm);
    UpdateStr(strstdpathtmp,"{DD}",dd);
    UpdateStr(strstdpathtmp,"{HH}",hh);
    UpdateStr(strstdpathtmp,"{MI}",mi);
    UpdateStr(strstdpathtmp,"{SS}",ss);

    UpdateStr(strstdpath,"{YYYY}",yyyy);
    UpdateStr(strstdpath,"{MM}",mm);
    UpdateStr(strstdpath,"{DD}",dd);
    UpdateStr(strstdpath,"{HH}",hh);
    UpdateStr(strstdpath,"{MI}",mi);
    UpdateStr(strstdpath,"{SS}",ss);

    memset(strTempFileName,0,sizeof(strTempFileName));
    memset(strFullFileName,0,sizeof(strFullFileName));
    sprintf(strTempFileName,"%s/%s.%d",strstdpathtmp,stFILELIST.filename,getpid());
    sprintf(strFullFileName,"%s/%s",strstdpath,stFILELIST.filename);

    // ��ʼ�����ļ�
    logfile.Write("export %s to %s ...",stFILELIST.filename,strstdpath);

    iret = stmt.lobtofile(strFullFileName);

    if ( (iret == 0) && (stFILELIST.filesize==FileSize(strFullFileName)) )
    {
      rename(strTempFileName,strFullFileName); logfile.WriteEx("ok.\n");
      
      if ( strcmp(strifdel,"TRUE") == 0 || strcmp(strifdel,"true") == 0)
      {
        if (stdel.execute() != 0)
        {
          logfile.Write("delete from %s where filename=%s failed.\n%s\n",strtablename,stFILELIST.filename,stdel.cda.message); 
          return FALSE;
        }
        else
        {
          logfile.Write("delete from %s where filename=%s ok.\n",strtablename,stFILELIST.filename); 
        }
      }
    }
    else
    {
      logfile.WriteEx("FAILED(filesize=%d).\n%d,%s\n",FileSize(strFullFileName),stmt.cda.rc,stmt.cda.message);
    }

    ProgramActive.WriteToFile();
  }

  return 0;
}
