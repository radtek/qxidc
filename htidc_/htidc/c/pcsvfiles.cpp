#include "idcapp.h"

void CallQuit(int sig);
BOOL SplitBuffer(char *strline);

CCmdStr cmdfieldstr;           // �����ֶκ���Ҫ�ֶβ�ֺʹ��
vector<char *> vstrline;       // ���һ������

CDir           Dir;
CLogFile       logfile;
CProgramActive ProgramActive;

CFile filecsv,fileout;         //  csv�����ļ���out����ļ� 
char strXmlBuffer[4001];
char strsrcfilepath[301];
char strfieldstr[2001];
char strstdname[31];
char strstdpath[201];
char strTmpFileName[301];
char strdiscard[201];
char strandchild[11];
BOOL bandchild=FALSE;
char strmatchname[301];
UINT Seq;

BOOL _pcsvfiles();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/pcsvfiles logfilename xmlbuffer\n");
    printf("Sample:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/pcsvfiles /tmp/htidc/log/pcsvfiles.log \"<srcfilepath>/qxdata/szqx/sdata/tmp</srcfilepath><matchname>*.CSV</matchname><andchild>FALSE</andchild><fieldstr>obtid,lon,lat,height,,,city,,town</fieldstr><stdname>OBTHOURD</stdname><stdpath>/tmp/htidc/tmp</stdpath><discard>XK_WSH,XK_XMMC</discard>\"\n\n");

    printf("���������������ĵĹ�������ģ�飬���ڴ�csv�ļ��вɼ����ݣ��������xml�ļ������stdpath�С�\n");
    printf("logfilename�Ǳ��������е���־�ļ���\n");
    printf("xmlbufferΪ�����ھ�Ĳ��������£�\n");
    printf("��ȡ���������ݶ�Ӧ��xml�ֶ��� <fieldstr>obtid,lon,lat,height,,,city,,town</fieldstr>\n");
    printf("Դ�ļ�Ҫ���б����У�Ҫȥ�������У�Դ�ļ��ж��ٸ��ֶΣ�ȫ���ֶζ�Ҫд����Ҫ���ֶξ����գ��ö��ŷ���\n");
    printf("xml�ļ���ǰ׺ <stdname>OBTHOURD</stdname>\n");
    printf("xml�ļ��������Ŀ¼ <outpath>/tmp/htidc/ftpput</outpath>��\n");
    printf("discard ������ݰ���������ģ��Ͷ����������ݣ�һ�������������ݰ����˱����У��Ǿ�Ҫȥ��������\n");
    printf("���������в���������Ϊ�գ��������Ϊ�գ�����ᱨ���˳���ע��:csv�ļ����ֶ�ֵ���ð���\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("pcsvfiles");

  memset(strsrcfilepath,0,sizeof(strsrcfilepath));
  memset(strfieldstr,0,sizeof(strfieldstr));
  memset(strstdname,0,sizeof(strstdname));
  memset(strstdpath,0,sizeof(strstdpath));
  memset(strTmpFileName,0,sizeof(strTmpFileName));
  memset(strdiscard,0,sizeof(strdiscard));
  memset(strmatchname,0,sizeof(strmatchname));
  memset(strandchild,0,sizeof(strandchild));

  GetXMLBuffer(strXmlBuffer,"fieldstr",strfieldstr,2000);
  GetXMLBuffer(strXmlBuffer,"srcfilepath",strsrcfilepath,300);
  GetXMLBuffer(strXmlBuffer,"stdname",strstdname,30);
  GetXMLBuffer(strXmlBuffer,"stdpath",strstdpath,200);
  GetXMLBuffer(strXmlBuffer,"discard",strdiscard,200);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,300);
  GetXMLBuffer(strXmlBuffer,"andchild",strandchild,10);

  // �����������Ϊ�գ�����ᱨ���˳���
  if (strlen(strsrcfilepath) == 0) { logfile.Write("srcfilepath is null.\n"); return -1; }
  if (strlen(strfieldstr) == 0)    { logfile.Write("fieldstr is null.\n"); return -1;    }
  if (strlen(strstdname) == 0)     { logfile.Write("stdname is null.\n"); return -1;     }
  if (strlen(strstdpath) == 0)     { logfile.Write("stdpath is null.\n"); return -1;     }
  if (strlen(strmatchname) == 0)   { logfile.Write("strmatchname is null.\n"); return -1;}
  if (strlen(strandchild) == 0)    { logfile.Write("andchild is null.\n"); return -1;    }

  if ( (strcmp(strandchild,"TRUE") == 0) || (strcmp(strandchild,"true") == 0) ) bandchild=TRUE;

  logfile.Write("pcsvfiles beginning.\n");

  // ע�⣬����ʱ��600��
  ProgramActive.SetProgramInfo(&logfile,"pcsvfiles",600);

  // �ֶ���ҪСд����Ȼ���������
  // ��xml���ֶ�����CCmdStr���ֿ��������m_vCmdStr
  ToLower(strfieldstr);
  cmdfieldstr.SplitToCmd(strfieldstr,",");

  if (cmdfieldstr.CmdCount() == 0) { logfile.Write("cmdallfieldstr or cmdfieldstr is invalid.\n"); return -1; }

  // д����̻��Ϣ
  ProgramActive.WriteToFile();

  // �򿪴������ļ���Ŀ¼
  if (Dir.OpenDirNoSort(strsrcfilepath,bandchild) == FALSE)
  {
    logfile.Write("Dir.OpenDirNoSort(%s) failed.\n",strsrcfilepath); 
    return -1;
  } 

  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ��ȡһ���ļ�
    if (Dir.ReadDir() == FALSE) break;

    // ����ļ�����ƥ����׺ΪTMP���Ͳ����䣬������
    if ( (MatchFileName(Dir.m_FileName,strmatchname)==FALSE) ||
         (MatchFileName(Dir.m_FileName,"*.TMP,*.SWP")== TRUE) ) continue;

    logfile.Write("Process file %s...",Dir.m_FileName);

    if (_pcsvfiles() == FALSE) continue;

    logfile.Write("ok.\n");

    REMOVE(Dir.m_FullFileName);
 
  }

  return 0;

}

BOOL _pcsvfiles()
{
  char strBeginTime[21]; 
  memset(strBeginTime,0,sizeof(strBeginTime));
  LocalTime(strBeginTime,"yyyymmddhh24miss");

  snprintf(strTmpFileName,300,"%s/%s_%s_%d_%lu.xml",strstdpath,strstdname,strBeginTime,getpid(),Seq);

  if (filecsv.OpenForRead(Dir.m_FullFileName,"r") == FALSE) { logfile.Write("Open %s failed.\n",Dir.m_FullFileName); return FALSE; }

  if (fileout.OpenForRename(strTmpFileName,"a+") == FALSE)  { logfile.Write("fileout.OpenForRename is failed.\n"); return FALSE; }

  char strtmpline[8092];
  memset(strtmpline,0,sizeof(strtmpline));

  int totalcount=0;

  fileout.Fprintf("<data>\n");

  while (TRUE)
  {
    memset(strtmpline,0,sizeof(strtmpline));

    if ( filecsv.FFGETS(strtmpline,8000) == FALSE ) break;   

    if (strstr(strtmpline,strdiscard) != 0) continue;

    if ( SplitBuffer(strtmpline) == FALSE ) continue; 

    for(UINT i=0;i<cmdfieldstr.CmdCount();i++)
    { 
      if (strlen(cmdfieldstr.m_vCmdStr[i].c_str()) == 0 || strlen(vstrline[i]) == 0) continue;
      fileout.Fprintf("<%s>%s</%s>",cmdfieldstr.m_vCmdStr[i].c_str(),vstrline[i],cmdfieldstr.m_vCmdStr[i].c_str());
    }

    fileout.Fprintf("<endl/>\n");

    totalcount++;
  }

  logfile.Write("create %s(%lu) ok.\n",fileout.m_filename,totalcount);

  fileout.Fprintf("</data>\n");

  fileout.CloseAndRename();

  filecsv.Fclose();
 
  return TRUE;

}

BOOL SplitBuffer(char *strline)
{
  if ( strlen(strline) == 0 ) return FALSE;

  vstrline.clear();
   
  CCmdStr cmdstr;
  cmdstr.SplitToCmd(strline,",");
  for(UINT i=0; i<cmdfieldstr.CmdCount();i++)
  {
    vstrline.push_back((char *)cmdstr.m_vCmdStr[i].c_str());
  }
   
  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // ��������쳣�˳���ɾ���м�״̬���ļ�
  REMOVE(strTmpFileName);

  filecsv.Fclose();  

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("pcsvfiles exit.\n");

  exit(0);
}


