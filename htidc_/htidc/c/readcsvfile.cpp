#include "_public.h"

CFile  file;
connection     connsrc;
CLogFile       logfile;
CProgramActive ProgramActive;
CCmdStr        CmdStr;

char strconn[101];
char buffer[4001];

// ��ű���˳�������
char atitles[50][31];
// �ֶ����ݴ�ŵ�����
char afields[50][31];

// �ѱ���������в�ִ���atitles��afields�����У�itype==1���⣬itype==2����
BOOL SplitTitleOrField(char *strBuffer,int itype);

// �����ֶ�����ȡ���ֶε�����BOOL 
GetValue(const char *in_fieldname,char *out_filevalue,int ilen);

struct st_TC
{
  char yyyy[5];
  char no[3];
  char zhtno[51];
  char tcename[21];
  char tccname[21];
  char number[2];
  char pro[11];
  char city[31];
  char city1[2];
  char level[3];
  char mm[3];
  char dd[3];
  char hh[3];
  char landwind[11];
  char landp[11];
};

BOOL buffertotitle();

struct st_TC stTC;
vector<struct st_TC> vTC;

int main( int argc,char *argv[])
{
  if(argc!=4 )
  {
    printf("Using: /htidc/htidc/bin/readcsvfile filename logfilename strconn\n");
    printf("�������ȡcsv�ļ�������⵽���ݿ�");
   
    return -1; 
  }
  
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // ����־�ļ� 
  if (logfile.Open(argv[2],"a+") == FALSE) 
  {   
    printf("logfile.Open(%s) failed.\n",argv[1]); 
    return -1; 
   }
  
  logfile.SetAlarmOpt("readcsvfile");


  file.OpenForRead("argv[1],"r");

  memset(buffer,0,sizeof(buffer));

  while(TRUE) 
  {
    if(file.FFGETS(buffer,4000)==FALSE) return FALSE;

    if(strstr(buffer,"YYYY")!=0)) continue;
    
    CmdStr.SplitToCmd(buffer,",");
    Cmdstr.GetValue(0,m_stALLAWSMIND.obtid,5);
   




    memset(buffer,0,sizeof(buffer));
   }
  
  return 0;

}

