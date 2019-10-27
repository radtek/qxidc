// ��������������SQL��������ЩFTPû�вɼ����ļ���û�ɼ����ļ���FTPҪ���һ�¡�
//select * from t_fileftptask where ftptype=1 and rsts=1 and taskid not in (select taskid from t_fileftplist where ddatetime>sysdate-2);

#include "idcapp.h"
#include "_ftp.h"

// ��ftp��ʱ��ʱ�򣬵��øú���
void FtpLoginTimeOut(int sigid);
void CallQuit(int sig);

char strAPPConnStr[201]; 
char strLogPath[201]; 
char strTmpPath[201]; 

ftplib         ftp;
FILE          *listfp;
connection     conn;
CLogFile       logfile;
CFILEFTPTASK   FTASK;
CFILEFTPLIST   FLIST;
CProgramActive ProgramActive;
CDir           Dir;
char           strRemoteIP[21];

// ִ���ļ���������
BOOL _ExecFtpTask();

// ������Է������������ӣ�����¼
BOOL _FtpLogin();

// ��ȡ�Է����������ļ��嵥
BOOL GetRemoteFileList();

// ɾ���Է�Ŀ¼�в�ƥ���ļ�
BOOL DeleteRemoteFile();

// �ӶԷ�������get�ļ�
BOOL FtpGetFile(char *in_FileName);

// ���ļ����͵��Է�������Ŀ¼
BOOL FtpPutFile();

// �ж��ļ��Ĵ�С�Ƿ�Ϸ�
BOOL JudgeFileSizeStr(UINT uFileSize,char *strFileSizeStr);

int main(int argc,char *argv[])
{
  if (argc != 5)
  {
    printf("\n");
    printf("Using:./ftpserver username/password@tnsname tmppath logpath remoteip\n");

    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/ftpserver sqxt/pwdidc@SZQX_10.148.124.85 /htidc/sqxt/tmp /htidc/sqxt/log 172.22.1.15\n");
    printf("Example:/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/ftpserver sqxt/pwdidc@SZQX_10.148.124.85 /htidc/sqxt/tmp /htidc/sqxt/log 172.22.1.17\n\n");
 
    printf("���������������ĵĹ�������ģ�飬�ļ��ɼ����ַ���Ŀ¼ά���ķ������Ŀǰֻ֧��FTPЭ�顣\n");
    printf("�������ȡusername/password@tnsname���ݿ���T_FILEFTPTASK������������ִ������\n");
    printf("���������ʱ�ļ������tmppathĿ¼�У���־�ļ������logpath�У�remoteip��Զ�̷�������IP��\n");
    printf("ÿһ�����͵�������һ��������ftpserver����ִ�У�remoteip����ָ����FTPԶ�̷�������IP��\n");
    printf("Ϊ�����Ч�ʣ���T_FILEFTPTASK����ȫ��������Զ��IP��Ϊ����������ͣ��������š�\n");
    printf("�����Ҫִ�ж��remoteip��FTP���񣬾ͱ����������ftpserver����\n");
    printf("remoteip������Զ�̷�������IP��Ҳ������Ӣ��������\n");
 
    return -1;
  }

  listfp=0;

  memset(strAPPConnStr,0,sizeof(strAPPConnStr));
  memset(strTmpPath,0,sizeof(strTmpPath));
  memset(strLogPath,0,sizeof(strLogPath));

  strcpy(strAPPConnStr,argv[1]);
  strcpy(strTmpPath,argv[2]);
  strcpy(strLogPath,argv[3]);
  strcpy(strRemoteIP,argv[4]);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,200,"%s/ftpserver%s.log",strLogPath,strRemoteIP);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }
  
  //�򿪸澯
  logfile.SetAlarmOpt("ftpserver");

  char strprogname[101];
  memset(strprogname,0,sizeof(strprogname));
  sprintf(strprogname,"ftpserver_%s",strRemoteIP);
  ProgramActive.SetProgramInfo(&logfile,strprogname,500);

  // �������ݿ⣬�����ݿ������������������ݴ����ڴ˳����У������Զ��ύģʽ
  if (conn.connecttodb(strAPPConnStr,TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strAPPConnStr); CallQuit(-1);
  }

  // ע�⣬����ʱ��300�룬����Ϊ300���㹻�ˣ�����������г����ɡ�
  // ע�⣬����ʱ֮ǰ��500�룬���ڸ�Ϊ300�룬��Ϊ���������ݿ�ʧ��ʱ����Ҫ�ܳ�ʱ�䣬�����ʱ����Ϊ300�룬
  // ���ܵ��¸澯�޷�����
  // ���ɼ�DVBSϵͳ���ݵ�ʱ�򣬶����ļ�̫�࣬list��Ҫ�ϳ���ʱ�䣬������ʱ��300�롣
  ProgramActive.SetProgramInfo(&logfile,strprogname,300);

  logfile.Write("ftpserver beging.\n");

  FTASK.BindConnLog(&conn,&logfile);
  FLIST.BindConnLog(&conn,&logfile);

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    // ��ȡȫ����Ҫ����ִ�е��ļ�ͬ������
    if (FTASK.LoadFtpTask(strRemoteIP) != 0)
    {
      logfile.Write("FTASK.SelFtpTask failed.\n"); CallQuit(-1);
    }

    // ���д���ÿ���ļ��ɼ�����
    while (FTASK.LoadFtpTaskNext() == 0)
    {

      // ����Է�Ŀ¼�ʹ���ȡ�ļ����е�ʱ������
      if (FTASK.dstpathmatchbz==1) 
      {
        ProcMatchDTime(FTASK.matchstr); ProcMatchDTime(FTASK.dstpath); 
      }

      /*
      logfile.Write(\
        "task=%s,url=%s:%ld,login=%s,%s,dst=%s,local=%s,filename=%s,OK.\n",\
         FTASK.taskname,FTASK.remoteip,FTASK.port,\
         FTASK.username,FTASK.password,FTASK.dstpath,FTASK.localpath,FTASK.matchstr);
      */

      if (FTASK.ftpcmode==1) ftp.mp_ftphandle->cmode=ftp.ftplib::pasv;

      if (FTASK.ftpcmode==2) ftp.mp_ftphandle->cmode=ftp.ftplib::port;

      FTASK.ftpsts=1;

      // ִ���ļ���������
      _ExecFtpTask();

      // ÿ��ִ����FTP����֮�󣬹ص����
      ftp.CloseHandle();
      
      // ftpsts��1-����;2-����ʧ�ܣ�3-��¼ʧ�ܡ�
      FTASK.UptFtpSTS();
    }

    // ��ȡ�����Ҫִ�е������¼��ʱ��͵�ǰʱ����������
    FTASK.SelLeastTimeTvl(strRemoteIP); 

    ProgramActive.WriteToFile();

    if (FTASK.m_sleeptvl <  10) FTASK.m_sleeptvl=10;

    if (FTASK.m_sleeptvl > 120) FTASK.m_sleeptvl=120;

    sleep(FTASK.m_sleeptvl);

    ProgramActive.WriteToFile();
  }

  logfile.Write("ftpserver exit.\n");

  return 0;
}

// ��ftp��ʱ��ʱ�򣬵��øú������ر�ftp��socket�����ftp��Ӧ�ú����ͻ������
void FtpLoginTimeOut(int sigid)
{
  FTASK.ftpsts = 2;

  FTASK.UptFtpSTS();

  logfile.Write("���ӷ�����%s��%s/%s����ʱ.\n",FTASK.remoteip,FTASK.username,FTASK.password);

  ftp.m_timeout=1;

  ftp.CloseHandle();

  CallQuit(-1);
}

// ִ���ļ���������
BOOL _ExecFtpTask()
{
  // ���ӷ�����
  if (_FtpLogin() == FALSE)
  {
    logfile.Write("�������ftpserver����¼������%s��%s/%s��ʧ��!",FTASK.remoteip,FTASK.username,FTASK.password);
    return FALSE;
  }

  ProgramActive.WriteToFile();

  int iFileCount=0;
  //logfile.Write("Login OK.\n");

  UINT ii;

  // �ɼ�
  if (FTASK.ftptype == 1)
  {
    FLIST.m_taskid=FTASK.taskid;

    // �ӶԷ���������ȡ�ļ�
    // ��ȡ�Է����������ļ��嵥
    if (GetRemoteFileList() == FALSE)
    {
      logfile.Write("GetRemoteFileList Failed,%s.\n",ftp.mp_ftphandle->response); return FALSE;
    }

    // ���л�ȡÿһ����¼���ӶԷ���������ȡ�ļ�
    for (ii=0;ii<FLIST.m_vRemoteFile.size();ii++)
    {
      memcpy(&FLIST.m_stRemoteFile,&FLIST.m_vRemoteFile[ii],sizeof(FLIST.m_stRemoteFile));

      // �ӶԷ�������Ftp�ļ��������ȡ�ɹ����Ͱ��ļ���Ϣ����T_FILEFTPLIST����
      if (FtpGetFile(FLIST.m_stRemoteFile.filename) == TRUE)
      {
        if (FLIST.InsertList() != 0)
        {
          logfile.Write("FLIST.InsertList failed.\n"); return FALSE;
        }
      }

      if (iFileCount++>10) { iFileCount=0; ProgramActive.WriteToFile(); }
    }
  }

  // �ַ�

  if (FTASK.ftptype == 2)
  {
    // ��Է������������ļ�
    if (Dir.OpenDir(FTASK.localpath) == FALSE) 
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",FTASK.localpath); return FALSE;
    }

    // ���뷢��Ŀ¼
    chdir(FTASK.localpath);

    // ����Ŀ��Ŀ¼
    if (ftp.Chdir(FTASK.dstpath) == FALSE)
    {
      // �������Ŀ��Ŀ¼ʧ�ܣ��п�����Ŀ��Ŀ¼��δ���������������һ���Ǵ������Բ��ط���ʧ�ܡ�
      // logfile.Write("ftp.Chdir(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
      return TRUE;
    }

    while (Dir.ReadDir() == TRUE)
    {
      // �ж��ļ����Ƿ��MatchStrƥ�䣬�����ƥ�䣬���ַ����ļ�
      if (MatchFileName(Dir.m_FileName,FTASK.matchstr) == FALSE) continue;

      // ������ַ����ļ���ƥ��tmp��ƥ����*topwalk*����������������ļ�
      if (MatchFileName(Dir.m_FileName,"*TMP,*TOPWALK*") == TRUE) continue;

      // ����ļ���ʱ���ڵ�ǰʱ���ǰ10��֮�ڣ�����ʱ������
      char strLocalTime[21]; 
      LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-10); // ��ǰʱ���10��֮ǰ��ʱ��
      if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

      // ��Է�������Ftp�ļ�
      FtpPutFile();
    }
  }

  // ά��
  if (FTASK.ftptype == 3)
  {
    // ɾ���Է�Ŀ¼��ƥ���ļ�
    if (DeleteRemoteFile() == FALSE)
    {
      logfile.Write("DeleteRemoteFile Failed,%s.\n",ftp.mp_ftphandle->response); return FALSE;
    }
  }

  return TRUE;
}

// ������Է������������ӣ�����¼
BOOL _FtpLogin()
{
  char strURL[51];
  memset(strURL,0,sizeof(strURL));

  snprintf(strURL,50,"%s:%ld",FTASK.remoteip,FTASK.port);

  signal(SIGALRM,FtpLoginTimeOut); alarm(CONNECTTIMEOUT);

  BOOL bConnect=ftp.Connect(strURL);

  signal(SIGALRM,SIG_IGN); 

  if (bConnect == FALSE)
  {
    FTASK.ftpsts=2; 
    logfile.Write("ftp.Connect(%s) Failed,%s.\n",strURL,ftp.mp_ftphandle->response); 
    return FALSE;
  }

  if (ftp.Login(FTASK.username,FTASK.password) == FALSE)
  {
    logfile.Write("ftp.Login(%s,%s/%s) Failed,%s.\n",strURL,FTASK.username,FTASK.password,ftp.mp_ftphandle->response); 
    FTASK.ftpsts=3; 
    return FALSE;
  } 

  return TRUE;
}

// ��ȡ�Է����������ļ��嵥
BOOL GetRemoteFileList()
{
  char strListFileName[201];

  memset(strListFileName,0,sizeof(strListFileName));
  snprintf(strListFileName,200,"%s/ftp_%s.list",strTmpPath,strRemoteIP);

  // ��նԷ���������ȡ�ļ��б�
  FLIST.m_vRemoteFile.clear();

  // ����Ŀ��Ŀ¼
  if (ftp.Chdir(FTASK.dstpath) == FALSE)
  {
    // �������Ŀ��Ŀ¼ʧ�ܣ��п�����Ŀ��Ŀ¼��δ���������������һ���Ǵ������Բ��ط���ʧ�ܡ�
    // logfile.Write("ftp.Chdir(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
    return TRUE;
  }

  // ��ȡ�Է����������ļ��嵥
  if (ftp.Nlst(strListFileName,"*") == FALSE)
  {
    logfile.Write("ftp.Nlst(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
  }

  ProgramActive.WriteToFile();

  //logfile.Write("Nlst OK.\n");

  // �ж��嵥�ļ��Ƿ�Ϊ��
  if (FileSize(strListFileName) == 0) return TRUE;

  // �򿪶Է����������ļ��嵥�ļ�
  listfp=0;
  if ( (listfp=FOPEN(strListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN(%s) failed.\n",strListFileName); return FALSE;
  }

  // �ӶԷ����������ļ��嵥�ļ���ȡ��ÿ�У��ж��Ƿ�Ҫ�ػ�ȡ���ļ������Ҫ���ͷŵ���ȡ�ļ��б���
  char strLine[201];
  vector<string> vFileName;

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));
   
    // ���ļ��л�ȡһ��
    if (FGETS(strLine,100,listfp) == FALSE) break;

    // ����Է����ļ���ƥ��tmp��ƥ����*topwalk*����������������ļ�
    if (MatchFileName(strLine,"*TMP,*TOPWALK*") == TRUE) continue;

    // ��ЩFTP�����ļ�����ǰҳ�ӡ�./��������Ҫȥ������
    UpdateStr(strLine,"./","");

    vFileName.push_back(strLine);
  }

  fclose(listfp); listfp=0;

  // ���ļ�������
  sort(vFileName.begin(),vFileName.end());

  for (UINT uPOS=0;uPOS<vFileName.size();uPOS++)
  {
    // �ж��ļ����Ƿ��MatchStrƥ�䣬�����ƥ�䣬����ȡ���ļ�
    if (MatchFileName(vFileName[uPOS].c_str(),FTASK.matchstr) == FALSE) continue;

    ProgramActive.WriteToFile();

    memset(&FLIST.m_stRemoteFile,0,sizeof(FLIST.m_stRemoteFile));

    strcpy(FLIST.m_stRemoteFile.filename,vFileName[uPOS].c_str());

    // ���ѻ�ȡ���ļ��嵥�л�ȡ���ļ��Ĵ�С���޸�����
    if (FLIST.FindFList() != 0)
    {
      logfile.Write("FLIST.FindFList failed.\n"); return FALSE;
    }

    // ����ļ������ѻ�ȡ���ļ��嵥�л�ȡ�ļ��У���ʾ��һ�����ļ���
    if (FLIST.FindFListNext() != 0)
    {
      FLIST.m_vRemoteFile.push_back(FLIST.m_stRemoteFile); continue;
    }

    // ����������ļ����ж��Ƿ���Ҫ����ļ���ʱ�䣬����ļ���ʱ�����ǰ�Ĳ�ͬ���ͰѸ��ļ������б���
    if (FTASK.cmodtime == 1)
    {
      char strModTime[21]; memset(strModTime,0,sizeof(strModTime));

      if (ftp.ModDate(vFileName[uPOS].c_str(),strModTime,14) == FALSE) continue;

      if (strncmp(strModTime,FLIST.m_stRemoteFile.modtime,14) != 0)
      {
        // һ��Ҫ��filesize��գ���Ϊ���filesize�Ǵӱ���ȡ�����ģ������Ǹ��ļ���ǰ��
        // �Է��������ϵ���ʵfilesize
        FLIST.m_stRemoteFile.filesize=0;
        strncpy(FLIST.m_stRemoteFile.modtime,strModTime,14);
        FLIST.m_vRemoteFile.push_back(FLIST.m_stRemoteFile); 
        continue;
      }
    }

    // ����������ļ����ж��Ƿ���Ҫ����ļ��Ĵ�С������ļ��Ĵ�С����ǰ�Ĳ�ͬ���ͰѸ��ļ������б���
    if (FTASK.cfilesize == 1)
    {
      int iFileSize=0;

      if (ftp.Size(vFileName[uPOS].c_str(),&iFileSize) == FALSE) continue;

      if (iFileSize != FLIST.m_stRemoteFile.filesize)
      {
        // һ��Ҫ��modtime��գ���Ϊ���modtime�Ǵӱ���ȡ�����ģ������Ǹ��ļ���ǰ��
        // �Է��������ϵ���ʵmodtime
        memset(FLIST.m_stRemoteFile.modtime,0,sizeof(FLIST.m_stRemoteFile.modtime));
        FLIST.m_stRemoteFile.filesize = iFileSize;
        FLIST.m_vRemoteFile.push_back(FLIST.m_stRemoteFile); 
        continue;
      }
    }
  }

  ProgramActive.WriteToFile();

  //logfile.Write("Check OK.\n");

  return TRUE;
}

// �ӶԷ�������get�ļ�
BOOL FtpGetFile(char *in_FileName)
{
  ProgramActive.WriteToFile();

  // ��ȡ�ļ���ʱ��
  if (strlen(FLIST.m_stRemoteFile.modtime) == 0)
  {
    if (ftp.ModDate(in_FileName,FLIST.m_stRemoteFile.modtime,14)==FALSE) return FALSE;
  }

  // ��ȡ�ļ��Ĵ�С
  if (FLIST.m_stRemoteFile.filesize == 0)
  {
    if (ftp.Size(in_FileName,&FLIST.m_stRemoteFile.filesize)==FALSE) return FALSE;
  }

  // ����޸�ʱ��С����Чʱ�䣬ֱ�ӷ��سɹ�������ȡ�ļ�
  if (strncmp(FLIST.m_stRemoteFile.modtime,FTASK.m_validdays,14) < 0) return TRUE;

  // ����ļ��Ĵ�С��0�����ڴ��Ĵ�С��Χ��
  if ( (FLIST.m_stRemoteFile.filesize==0) || (JudgeFileSizeStr(FLIST.m_stRemoteFile.filesize,FTASK.filesizestr) == FALSE) ) return FALSE;

  logfile.Write("transfer(get) %s ...",in_FileName);

  char strDstFileName[201],strDstFileName1[201],strTmpFileName[201],strLocalFileName[201];

  memset(strDstFileName,0,sizeof(strDstFileName));
  memset(strDstFileName1,0,sizeof(strDstFileName1));
  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strLocalFileName,0,sizeof(strLocalFileName));

  // ����Զ���ļ�����������ʱ�ļ����ͱ����ļ���
  snprintf(strDstFileName,200,"%s",in_FileName);
  snprintf(strDstFileName1,200,"%s/%s",FTASK.renamepath,in_FileName);
  snprintf(strTmpFileName,200,"%s/file_%s.tmp",strTmpPath,strRemoteIP);
  snprintf(strLocalFileName,200,"%s/%s",FTASK.localpath,FLIST.m_stRemoteFile.filename);

  // ��ȡ�ļ�
  if (ftp.Get(strTmpFileName,strDstFileName,ftplib::image) == FALSE)
  {
    logfile.WriteEx("Failed,ftp.Get,%s\n",ftp.mp_ftphandle->response); return FALSE;
  }

  // ��ȡ�ļ�֮�����ж��ļ��Ƿ�ı�
  char strModTime[21];  // �ɼ�֮���ļ���ʱ��
  int  iFileSize=0;     // �ɼ�֮���ļ��Ĵ�С

  memset(strModTime,0,sizeof(strModTime));

  // �ٴλ�ȡ�ļ���ʱ��
  if (ftp.ModDate(in_FileName,strModTime,14)==FALSE)
  {
    logfile.WriteEx("Failed,ftp.ModDate,%s.\n",ftp.mp_ftphandle->response); return FALSE;
  }

  // �ٴλ�ȡ�ļ��Ĵ�С
  if (ftp.Size(in_FileName,&iFileSize)==FALSE)
  {
    logfile.WriteEx("Failed,ftp.Size,%s\n",ftp.mp_ftphandle->response); return FALSE;
  }

  // �Ƚϻ�ȡ�ļ�ǰ����ļ���С���޸�ʱ����û�иı�
  if ( (strncmp(FLIST.m_stRemoteFile.modtime,strModTime,14) != 0) || 
       (FLIST.m_stRemoteFile.filesize != iFileSize) )
  {
    logfile.WriteEx("Failed,file changed during transfer(%s,%d,%s,%d).\n",FLIST.m_stRemoteFile.modtime,FLIST.m_stRemoteFile.filesize,strModTime,iFileSize); return FALSE;
  }

  // �����Ҫɾ���Է�Ŀ¼�ϵ��ļ���
  if (FTASK.deletebz == 1) ftp.Delete(strDstFileName);

  // �����Ҫ�ѶԷ�Ŀ¼�ϵ��ļ��Ƶ�������Ŀ¼����
  if (FTASK.renamebz == 1) ftp.Rename(strDstFileName,strDstFileName1);

  if (RENAME(strTmpFileName,strLocalFileName) == FALSE) 
  { 
    logfile.WriteEx("RENAME failed.\n"); return FALSE; 
  }

  logfile.WriteEx("ok,size=%d.\n",iFileSize); 

  return TRUE; 
}

// ���ļ����͵��Է�������Ŀ¼
BOOL FtpPutFile()
{
  ProgramActive.WriteToFile();

  char strFileName[201];
  char strFileNameTmp[201];
  char strFileNameRename[201];

  memset(strFileName,0,sizeof(strFileName));
  memset(strFileNameTmp,0,sizeof(strFileNameTmp));
  memset(strFileNameRename,0,sizeof(strFileNameRename));

  snprintf(strFileName,200,"%s",Dir.m_FileName);
  snprintf(strFileNameTmp,200,"%s.tmp",Dir.m_FileName);
  snprintf(strFileNameRename,200,"%s/%s",FTASK.renamepath,Dir.m_FileName);

  logfile.Write("transfer(put) %s ...",strFileName);

  // ��鱾��Ŀ¼���ļ��Ĵ�С
  int iLocalFileSize=FileSize(strFileName);

  // �������Ŀ¼���ļ��Ĵ�С��0���ͷ��أ������͸��ļ�
  if (iLocalFileSize <= 0) { logfile.WriteEx("failed,filesize=0\n"); REMOVE(strFileName); return FALSE; }

  // �����ļ�
  if (ftp.Put(strFileName,strFileNameTmp,ftplib::image) == FALSE)
  {
    logfile.WriteEx("Failed,%s\n",ftp.mp_ftphandle->response); return FALSE;
  }

  if (FTASK.renamebz == 1) ftp.Rename(strFileNameTmp,strFileNameRename);
  if (FTASK.renamebz == 2) ftp.Rename(strFileNameTmp,strFileName);

  int iRemoteFileSize=0;

  if (FTASK.renamebz == 1) ftp.Size(strFileNameRename,&iRemoteFileSize);
  if (FTASK.renamebz == 2) ftp.Size(strFileName ,&iRemoteFileSize);

  // �ȽϷ����ļ�ǰ����ļ���С���޸�ʱ����û�иı�
  if (iLocalFileSize != iRemoteFileSize)
  {
    logfile.WriteEx("Failed,file changed during transfer(%d,%d).\n",iLocalFileSize,iRemoteFileSize); return FALSE;
  }

  strcpy(FTASK.m_stLocalFile.filename,Dir.m_FileName);
  FTASK.m_stLocalFile.taskid=FTASK.taskid;
  strcpy(FTASK.m_stLocalFile.modtime ,Dir.m_ModifyTime);
  FTASK.m_stLocalFile.filesize=Dir.m_FileSize;

  // ���ѷ��͵��ļ�����Ϣ�����ļ��ַ���־��
  if (FTASK.InsertPutList() != 0)
  {
    logfile.WriteEx("failed,call FTASK.InsertPutList failed.\n"); return FALSE;
  }

  // ɾ������Ŀ¼�ĸ��ļ�
  if (REMOVE(strFileName) == FALSE) { logfile.WriteEx("failed,REMOVE failed.\n"); return FALSE; }

  logfile.WriteEx("ok.\n"); 

  return TRUE; 
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.CloseHandle();

  if (listfp != 0) { fclose(listfp); listfp=0; }

  logfile.Write("ftpserver exit.\n");

  exit(0);
}

// �ж��ļ��Ĵ�С�Ƿ�Ϸ�
BOOL JudgeFileSizeStr(UINT uFileSize,char *strFileSizeStr)
{
  // ���ƥ����ʽ��*����ֱ�ӷ���TRUE
  if (strcmp(strFileSizeStr,"*") == 0) return TRUE;

  CCmdStr CmdStr;

  if (strchr(strFileSizeStr,'-') > 0)
  {
    CmdStr.SplitToCmd(strFileSizeStr,"-");
    char strMinSize[21],strMaxSize[21];
    memset(strMinSize,0,sizeof(strMinSize));
    memset(strMaxSize,0,sizeof(strMaxSize));
    UINT uMinSize,uMaxSize;
    uMinSize=uMaxSize=0;
    CmdStr.GetValue(0,&uMinSize);
    CmdStr.GetValue(1,&uMaxSize);
    CmdStr.GetValue(0,strMinSize,15);
    CmdStr.GetValue(1,strMaxSize,15);
    if ( (uFileSize>=uMinSize) && (uFileSize<=uMaxSize) ) return TRUE;
    if ( (uFileSize>=uMinSize) && (strstr(strMaxSize,"*")!=0) ) return TRUE;
    if ( (strstr(strMinSize,"*")!=0) && (uFileSize<=uMaxSize) ) return TRUE;
  }
  else
  {
    CmdStr.SplitToCmd(strFileSizeStr,",");
    UINT uSize;
    for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
    {
      uSize=0;
      CmdStr.GetValue(ii,&uSize);
      if (uFileSize == uSize) return TRUE;
    }
  }
  
  return FALSE;
}

// ɾ���Է�Ŀ¼�в�ƥ���ļ�
BOOL DeleteRemoteFile()
{
  char strListFileName[201];

  memset(strListFileName,0,sizeof(strListFileName));
  snprintf(strListFileName,200,"%s/ftp_%s.list",strTmpPath,strRemoteIP);

  // ����Ŀ��Ŀ¼
  if (ftp.Chdir(FTASK.dstpath) == FALSE)
  {
    // �������Ŀ��Ŀ¼ʧ�ܣ��п�����Ŀ��Ŀ¼��δ���������������һ���Ǵ������Բ��ط���ʧ�ܡ�
    // logfile.Write("ftp.Chdir(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
    return TRUE;
  }

  // ��ȡ�Է����������ļ��嵥
  if (ftp.Nlst(strListFileName,"*") == FALSE)
  {
    logfile.Write("ftp.Nlst(%s) Failed,%s.\n",FTASK.dstpath,ftp.mp_ftphandle->response); return FALSE;
  }

  ProgramActive.WriteToFile();

  // �򿪶Է����������ļ��嵥�ļ�
  listfp=0;
  if ( (listfp=FOPEN(strListFileName,"r")) == NULL)
  {
    logfile.Write("FOPEN(%s) failed.\n",strListFileName); return FALSE;
  }

  // �ӶԷ����������ļ��嵥�ļ���ȡ��ÿ�У��ж��Ƿ�Ҫ�ػ�ȡ���ļ������Ҫ���ͷŵ���ȡ�ļ��б���
  char strFileName[201];
  char strModTime[21]; 
  char strFullFileName[301];

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    memset(strFileName,0,sizeof(strFileName));
    memset(strModTime,0,sizeof(strModTime));
    memset(strFullFileName,0,sizeof(strFullFileName));

    // ���ļ��л�ȡһ��
    if (FGETS(strFileName,100,listfp) == FALSE) break;

    snprintf(strFullFileName,200,"%s/%s",FTASK.dstpath,strFileName);

    if (ftp.ModDate(strFullFileName,strModTime,14) == FALSE) continue;
    
    // �Ƚ϶Է��ļ��Ƿ�����Чʱ��֮�ڣ�������ļ���ʱ��С����Чʱ�䣬��ɾ������
    if (strncmp(strModTime,FTASK.m_validdays,14) < 0)
    {
      // �ж��ļ����Ƿ��MatchStrƥ�䣬�����ƥ�䣬����ȡ���ļ�
      if (MatchFileName(strFileName,FTASK.matchstr) == FALSE) continue;

      if (ftp.Delete(strFullFileName) == TRUE)
      {
        logfile.Write("delete %s(%s,%s) ok.\n",strFileName,FTASK.m_validdays,strModTime);
      }
      else
      {
        logfile.Write("delete %s(%s,%s) Failed,%s.\n",strFileName,FTASK.m_validdays,strModTime,ftp.mp_ftphandle->response);
      }
    }
  }

  fclose(listfp); listfp=0;

  ProgramActive.WriteToFile();

  return TRUE;
}
