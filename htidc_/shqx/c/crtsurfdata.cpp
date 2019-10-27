/*
 *  ��������������ȫ������վ��۲�ķ������ݡ�
*/
#include "_public.h"
#include "_shqx.h"

vector<struct st_stcode> vstcode;   // ���ȫ��վ�����������
vector<struct st_surfdata> vsurfdata;   // ���ȫ������վ����ӹ۲����ݵ�����

// ��վ������ļ��м��ص�vstcode������
bool LoadSTCode(const char *inifile);

// ����ȫ������վ����ӹ۲����ݣ������vsurfdata������
void CrtSurfData();

// ������vsurfdata�е�ȫ������վ����ӹ۲�����д���ļ�
bool CrtSurfFile(const char *outpath);

CLogFile logfile;

void EXIT(int sig);

int main(int argc,char *argv[],char *envp[])
{
  if (argc!=4)
  {
    printf("\n��������������ȫ������վ��۲�ķ������ݡ�\n");
    printf("/htidc/shqx/bin/crtsurfdata վ����� �����ļ���ŵ�Ŀ¼ ��־�ļ���\n");
    printf("���磺/htidc/shqx/bin/crtsurfdata /htidc/shqx/ini/stcode.ini /data/shqx/ftp/surfdata /log/shqx/crtsurfdata.log\n");
    return -1;
  }

  // �ر�ȫ�����źź��������
  CloseIOAndSignal();

  // ��������˳����ź�
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  if (logfile.Open(argv[3],"a+")==false)
  {
    printf("����־�ļ�ʧ�ܣ�%s����\n",argv[3]); return -1;
  }

  while (true)
  {
    // ��վ������ļ��м��ص�vstcode������
    if (LoadSTCode(argv[1])==false) { sleep(60); continue; }

    logfile.Write("���ز����ļ���%s���ɹ���\n",argv[1]);

    CrtSurfData();  // ����ȫ������վ����ӹ۲����ݣ������vsurfdata������

    // ������vsurfdata�е�ȫ������վ����ӹ۲�����д���ļ�
    if (CrtSurfFile(argv[2])==false) { sleep(60); continue; }

    sleep(60);
  }

  return 0;
}

// ��վ������ļ��м��ص�vstcode������
bool LoadSTCode(const char *inifile)
{
  vstcode.clear();

  CCmdStr CmdStr;
  struct st_stcode stcode;

  CFile File;

  if (File.Open(inifile,"r") == false)
  {
    logfile.Write("File.Open(%s) ʧ�ܡ�\n",inifile); return false;
  }

  char strbuffer[101];

  while (true)
  {
    memset(&stcode,0,sizeof(struct st_stcode));

    if (File.Fgets(strbuffer,100)==false) break;

    CmdStr.SplitToCmd(strbuffer,",",true);

    CmdStr.GetValue(0, stcode.provname);
    CmdStr.GetValue(1, stcode.obtid);
    CmdStr.GetValue(2, stcode.cityname);
    CmdStr.GetValue(3,&stcode.lat);
    CmdStr.GetValue(4,&stcode.lon);
    CmdStr.GetValue(5,&stcode.height);

    vstcode.push_back(stcode);
  }

  return true;
}

// ����ȫ������վ����ӹ۲����ݣ������vsurfdata������
void CrtSurfData()
{
  vsurfdata.clear();  // �������

  srand(time(0));     // �����������

  char strLocalTime[21];
  LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi");
  strcat(strLocalTime,":00");

  struct st_surfdata stsurfdata;
  for (int ii=0;ii<vstcode.size();ii++)
  {
    memset(&stsurfdata,0,sizeof(struct st_surfdata));
    STRCPY(stsurfdata.obtid,10,vstcode[ii].obtid);  // վ�����

    STRCPY(stsurfdata.ddatetime,20,strLocalTime);   // ����ʱ����õ�ǰʱ��

    stsurfdata.t=rand()%351;       // ���£���λ��0.1���϶�
    stsurfdata.p=rand()%265+10000; // ��ѹ��0.1����
    stsurfdata.u=rand()%100+1;     // ���ʪ�ȣ�0-100֮���ֵ��
    stsurfdata.wd=rand()%360;      // ����0-360֮���ֵ��
    stsurfdata.wf=rand()%150;      // ���٣���λ0.1m/s
    stsurfdata.r=rand()%16;        // ��������0.1mm
    stsurfdata.vis=rand()%5001+100000;  // �ܼ��ȣ�0.1��

    vsurfdata.push_back(stsurfdata);
  }
}

// ������vsurfdata�е�ȫ������վ����ӹ۲�����д���ļ�
bool CrtSurfFile(const char *outpath)
{
  CFile File;

  char strLocalTime[21];
  LocalTime(strLocalTime,"yyyymmddhh24miss");

  char strFileName[301];
  SNPRINTF(strFileName,300,"%s/SURF_ZH_%s_%d.txt",outpath,strLocalTime,getpid());
   
  if (File.OpenForRename(strFileName,"w")==false)
  {
    logfile.Write("File.Open(%s) ʧ��!\n",strFileName); return false;
  }

  for (int ii=0;ii<vsurfdata.size();ii++)
  {
    // վ�����,����ʱ��,����,��ѹ,���ʪ��,����,����,������,�ܼ���
    File.Fprintf("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",\
         vsurfdata[ii].obtid,vsurfdata[ii].ddatetime,vsurfdata[ii].t/10.0,vsurfdata[ii].p/10.0,\
         vsurfdata[ii].u,vsurfdata[ii].wd,vsurfdata[ii].wf/10.0,vsurfdata[ii].r/10.0,vsurfdata[ii].vis/10.0);
  }

  File.CloseAndRename();   // �ر��ļ�

  logfile.Write("���������ļ���%s���ɹ�������ʱ��=%s����¼��=%d��\n\n",strFileName,vsurfdata[0].ddatetime,vsurfdata.size());

  vstcode.clear(); vsurfdata.clear();

  return true;
}

void EXIT(int sig)
{
  logfile.Write("�����˳���sig=%d\n\n",sig);

  exit(0);
}
