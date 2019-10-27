#include "_public.h"

CLogFile logfile;

void EXIT(int sig);

// ��xmlbuffer�л�ȡ������Ҫ����ֵ�ߵĲ���
BOOL GetXMLBuffer(char *xmlbuffer);

char gxout[21];
char dispvalue[21];
char minvalue[21];
char maxvalue[21];
char poli[21];
char corp[51];

// ��γ�ȵĵ�ĸ�������ʼλ�ú͸����
char lonsum[11],lonstart[11],lonstep[11];
char latsum[11],latstart[11],latstep[11];

char txtfilename[301],datfilename[301],datmfilename[301];

char ctlfilename1[301],ctlfilename2[301];

char grbfilename[301];

char gsfilename[301];

char gradsroot[301];

char mapfilename[301];
char tmpgiffilename[301];
char giffilename[301];

// �������ڲ�ֵ��grib�ļ�
BOOL CrtGribFile();

// �����ڻ�ͼ��վ�����ݴ��ı��ļ�ת��Ϊ�������ļ�
BOOL DataTxtToBin();

// ���ɿ����ļ�
BOOL CrtCtlFile();

// ���ɻ�ͼ�Ľű��ļ�
BOOL CrtGSFile();

#ifndef PI
#define PI 3.14159265358979323846   // Բ����
#endif

UINT stcount=0;

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/drawwind logfilename xmlbuffer\n\n");

    printf("Examp:/htidc/htidc/bin/drawwind /tmp/drawwind.log \""\
                  "<gradsroot>/htidc/grads-2.0.a9</gradsroot>"\
                  "<lonstart>113.22</lonstart><lonstep>0.02</lonstep><lonsum>18</lonsum>"\
                  "<latstart> 22.73</latstart><latstep>0.02</latstep><latsum>18</latsum>"\
                  "<gxout>barb</gxout>"\
                  "<dispvalue>true</dispvalue>"\
                  "<minvalue> 1.0</minvalue>"\
                  "<maxvalue>35.0</maxvalue>"\
                  "<poli>on</poli>"\
                  "<corp>517x618+141+0</corp>"\
                  "<mapfilename>/tmp/map.gif</mapfilename>"\
                  "<txtfilename>/tmp/wind.txt</txtfilename>"\
                  "<giffilename>/tmp/wind.gif</giffilename>"\
                  "\"\n\n");

    printf("���������ڻ�վ��ķ�ʸͼ��logfilenameΪ��־�ļ�����xmlbufferΪ����ʸͼ�ĸ��ֲ�����\n");
    printf("���������/tmp/htidc/tmpĿ¼������һЩ���ڻ�ͼ����ʱ�ļ���\n\n");

    printf("logfilename    �������е���־�ļ�����\n");
    printf("xmlbuffer      �������еĸ��ֲ��������庬�����£�\n");
    printf("gradsroot      ����ͼ����İ�װĿ¼������ѯ�������ĵĺ�̨����Ա��\n");
    printf("lonstart       ��ͼ�������½ǵľ��ȣ���λ���ȡ�\n");
    printf("lonstep        ��ͼ����X������Ŀ�ȣ���λ���ȣ���վ����ܶȶ�����һ����0.02��0.10֮�䣬"\
           "վ��Խ�ܣ�ȡֵԽС��վ��Խ�裬ȡֵԽ��\n");
    printf("lonsum         ��ͼ����X�������������\n");
    printf("latstart       ��ͼ�������½ǵ�γ�ȣ���λ���ȡ�\n");
    printf("latstep        ��ͼ����Y������Ŀ�ȣ���λ���ȣ���վ����ܶȶ�����һ����0.02��0.10֮�䣬"\
           "վ��Խ�ܣ�ȡֵԽС��վ��Խ�裬ȡֵԽ��\n");
    printf("latsum         ��ͼ����Y�������������\n");
    printf("gxout          ����ͼ�ε����࣬vector-ʸ����ͷ��stream-������ʽ��barb-����ˣ�off-վ���ʸ��\n");
    printf("dispvalue      �Ƿ���ʾÿ��վ��ķ���ֵ��true-��ʾ��false-����ʾ��\n");
    printf("minvalue       վ��Ϸ����ݵ���Сֵ��С��minvalueֵ��վ�㽫�������������minvalueΪ�գ���ʾ����顣\n");
    printf("maxvalue       վ��Ϸ����ݵ����ֵ������maxvalueֵ��վ�㽫�������������maxvalueΪ�գ���ʾ����顣\n");
    printf("poli           �Ƿ���ʾGRADS��ͼ�ı߽磬on-��ʾ��off-����ʾ��һ����˵��"\
           "�ڵ��Խ׶Σ�poliѡ������У��GRADS���ɵ�ͼ���ͼ��ͼ��λ�ã���ʽӦ����һ�㲻���á�\n");
    printf("corp           ͼƬ���в�����618x618+80+0��ʾ��GRADS���ɵ�ԭͼ��80,0����λ�ÿ�ʼ��"\
           "���д�СΪ618x618��ͼƬ���ݡ�ע�⣬���к������ΪGRADS��ͼ���ڵ����ݣ���С���ͼ��ͬ��"\
           "ע�⣬���corpΪ�գ�ͼƬ�����С�\n");
    printf("mapfilename    ��ͼ�ļ�����һ��ҪGIF�ļ������ң����ڻ�ͼ���������͸�������ֶο���Ϊ�ա�"\
           "ע�⣬GRADS���ɵ�ͼƬ�Ĵ�С�ɾ�γ�ȵĴ�С��Χ���������ԣ���ͼ�Ĵ�С������GRADS���ɵ�ͼ"\
           "�����к���ͬ��С���������ֻ��ҡ������ͼ�ļ���Ϊ�գ����õ�GRADS���ɵ�ԭʼ��ͼ��\n");
    printf("txtfilename    վ�������ļ�����վ�������ļ�ΪCSV�ļ���ÿһ�б�ʾһ��վ������ݣ���ʽ\"γ��,����,����,����\"��\n");
    printf("giffilename    ���ͼƬ���ļ���������ΪGIF�ļ�����\n\n");
    
    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
  
  // ����־�ļ�
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("drawwind");

  // ����xmlbuffer����ȡ���ֲ���
  if (GetXMLBuffer(argv[2]) == FALSE) EXIT(-1);

  // �����ڻ�ͼ��վ�����ݴ��ı��ļ�ת��Ϊ�������ļ�
  if (DataTxtToBin() == FALSE) EXIT(-1);

  if (stcount<2)
  {
    logfile.Write("��Ч����վ��������2������ͼ��ֱ�������ͼ��\n");
    COPY(mapfilename,giffilename);
    EXIT(0);
  }

  // �������ڲ�ֵ��grib�ļ�
  if (CrtGribFile() == FALSE) EXIT(-1);
  
  // ���ɿ����ļ�
  if (CrtCtlFile() == FALSE) EXIT(-1);

  // ���ɻ�ͼ�Ľű��ļ�
  if (CrtGSFile() == FALSE) EXIT(-1);

  // ����Grads�İ�װĿ¼
  char gradsdir[301];
  memset(gradsdir,0,sizeof(gradsdir));
  snprintf(gradsdir,200,"%s/data",gradsroot);
  setenv("GADDIR",gradsdir,1);

  logfile.Write("create %s ...",giffilename);

  // ��ͼ
  char strCMD[2048];
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,2000,"%s/bin/grads -lcb \"run %s\"",gradsroot,gsfilename);
  system(strCMD);
  logfile.WriteEx("\n%s ... ok.\n",strCMD);

  // ����GRADS���ɵ�ͼ
  memset(strCMD,0,sizeof(strCMD));
  /*
  if (strlen(corp) == 0)
    snprintf(strCMD,2000,"/usr/local/bin/convert %s -trim +repage %s",tmpgiffilename,tmpgiffilename);
  else
    snprintf(strCMD,2000,"/usr/local/bin/convert %s -crop %s +repage %s",tmpgiffilename,corp,tmpgiffilename);
  */
  if (strlen(corp) > 0)
    snprintf(strCMD,2000,"/usr/local/bin/convert %s -crop %s +repage %s",tmpgiffilename,corp,tmpgiffilename);
  system(strCMD);
  logfile.WriteEx("%s ... ok.\n",strCMD);

  // �ϳɵ�ͼ
  if (strlen(mapfilename) > 0)
  {
    snprintf(strCMD,2000,"/usr/local/bin/composite -gravity center %s %s %s.tmp",mapfilename,tmpgiffilename,giffilename);
    system(strCMD);
    char strtmpfilename[301];
    memset(strtmpfilename,0,sizeof(strtmpfilename));
    sprintf(strtmpfilename,"%s.tmp",giffilename);
    RENAME(strtmpfilename,giffilename);

    logfile.WriteEx("%s ... ok.\n",strCMD);
  }
  else
  {
    COPY(tmpgiffilename,giffilename);
  }

  logfile.WriteEx("ok.\n");

  EXIT(0);
}

// ��xmlbuffer�л�ȡ������Ҫ����ֵ�ߵĲ���
BOOL GetXMLBuffer(char *xmlbuffer)
{
  // ��γ�ȵĵ�ĸ�������ʼλ�ú͸����
  GetXMLBuffer(xmlbuffer,"gradsroot",gradsroot,100);
  GetXMLBuffer(xmlbuffer,"lonsum",lonsum,10);
  GetXMLBuffer(xmlbuffer,"lonstart",lonstart,10);
  GetXMLBuffer(xmlbuffer,"lonstep",lonstep,10);
  GetXMLBuffer(xmlbuffer,"latsum",latsum,10);
  GetXMLBuffer(xmlbuffer,"latstart",latstart,10);
  GetXMLBuffer(xmlbuffer,"latstep",latstep,10);
  GetXMLBuffer(xmlbuffer,"gxout",gxout,10);
  GetXMLBuffer(xmlbuffer,"dispvalue",dispvalue,10);
  GetXMLBuffer(xmlbuffer,"minvalue",minvalue,10);
  GetXMLBuffer(xmlbuffer,"maxvalue",maxvalue,10);
  GetXMLBuffer(xmlbuffer,"poli",poli,10);
  GetXMLBuffer(xmlbuffer,"corp",corp,50);
  GetXMLBuffer(xmlbuffer,"txtfilename",txtfilename,300);
  GetXMLBuffer(xmlbuffer,"mapfilename",mapfilename,300);
  GetXMLBuffer(xmlbuffer,"giffilename",giffilename,300);

  if (strlen(gradsroot) == 0) { logfile.Write("gradsroot is null.\n"); return FALSE; }
  if (strlen(lonsum) == 0) { logfile.Write("lonsum is null.\n"); return FALSE; }
  if (strlen(lonstart) == 0) { logfile.Write("lonstart is null.\n"); return FALSE; }
  if (strlen(lonstep) == 0) { logfile.Write("lonstep is null.\n"); return FALSE; }
  if (strlen(latsum) == 0) { logfile.Write("latsum is null.\n"); return FALSE; }
  if (strlen(latstart) == 0) { logfile.Write("latstart is null.\n"); return FALSE; }
  if (strlen(latstep) == 0) { logfile.Write("latstep is null.\n"); return FALSE; }
  if (strlen(gxout) == 0) { logfile.Write("gxout is null.\n"); return FALSE; }
  if (strlen(dispvalue) == 0) { logfile.Write("dispvalue is null.\n"); return FALSE; }
  if (strlen(poli) == 0) { logfile.Write("poli is null.\n"); return FALSE; }
  // if (strlen(corp) == 0) { logfile.Write("corp is null.\n"); return FALSE; }
  if (strlen(txtfilename) == 0) { logfile.Write("txtfilename is null.\n"); return FALSE; }
  // if (strlen(mapfilename) == 0) { logfile.Write("mapfilename is null.\n"); return FALSE; }
  if (strlen(giffilename) == 0) { logfile.Write("giffilename is null.\n"); return FALSE; }

  memset(tmpgiffilename,0,sizeof(tmpgiffilename));
  snprintf(tmpgiffilename,300,"/tmp/htidc/tmp/grads_%06d.gif",getpid());

  memset(datfilename,0,sizeof(datfilename));
  snprintf(datfilename,300,"/tmp/htidc/tmp/grads_%06d.dat",getpid());

  memset(datmfilename,0,sizeof(datmfilename));
  snprintf(datmfilename,300,"/tmp/htidc/tmp/grads_%06d.map",getpid());

  memset(grbfilename,0,sizeof(grbfilename));
  snprintf(grbfilename,300,"/tmp/htidc/tmp/grads_%06d.grb",getpid());

  memset(ctlfilename1,0,sizeof(ctlfilename1));
  snprintf(ctlfilename1,300,"/tmp/htidc/tmp/grads_%06d_1.ctl",getpid());

  memset(ctlfilename2,0,sizeof(ctlfilename2));
  snprintf(ctlfilename2,300,"/tmp/htidc/tmp/grads_%06d_2.ctl",getpid());

  memset(gsfilename,0,sizeof(gsfilename));
  snprintf(gsfilename,300,"/tmp/htidc/tmp/grads_%06d.gs",getpid());

  return TRUE;
}

// �������ڲ�ֵ��grib�ļ�
BOOL CrtGribFile()
{
  int x=atoi(lonsum);
  int y=atoi(latsum);

  float p=1;

  int m_fd=-1;

  m_fd=open(grbfilename,O_WRONLY|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IXUSR);

  if (m_fd < 0)
  {
    logfile.Write("open %s failed.\n",grbfilename); return TRUE;
  }

  for (int ii=0;ii<x*y;ii++)
  {
    write(m_fd,&p,sizeof(p));
  }

  close(m_fd);

  return TRUE;
}

// �����ڻ�ͼ��վ�����ݴ��ı��ļ�ת��Ϊ�������ļ�
BOOL DataTxtToBin()
{
  struct st_stdata
  {
    char id[8];           /* Character station id           */
    float lat;            /* Latitude of report             */
    float lon;            /* Longitude of report            */
    float t;              /* Time in relative grid units    */
    int nlev;             /* Number of levels following     */
    int flag;             /* Level independent var set flag */
  } stdata;

  float dd,df,uu,vv;

  char strLine[101];

  CFile TxtFile,DatFile;

  if (TxtFile.OpenForRead(txtfilename,"r") == FALSE)  
  { 
    logfile.Write("Fopen %s failed.\n",txtfilename); return FALSE; 
  }

  if (DatFile.OpenForWrite(datfilename,"wb") == FALSE) 
  { 
    logfile.Write("Fopen %s failed.\n",datfilename); return FALSE; 
  }

  CCmdStr CmdStr;

  while (TRUE)
  {
    if (TxtFile.FFGETS(strLine,100) == FALSE) break;

    CmdStr.SplitToCmd(strLine,",");

    if (CmdStr.CmdCount() != 4) continue;

    if ( (CmdStr.m_vCmdStr[0].length()==0) || (CmdStr.m_vCmdStr[1].length()==0) || (CmdStr.m_vCmdStr[2].length()==0) || (CmdStr.m_vCmdStr[3].length()==0) ) continue;

    memset(&stdata,0,sizeof(stdata));

    stdata.lat=atof(CmdStr.m_vCmdStr[0].c_str());
    stdata.lon=atof(CmdStr.m_vCmdStr[1].c_str());

    stdata.nlev = 1;
    stdata.flag = 1;
    stdata.t = 0;

    dd=atof(CmdStr.m_vCmdStr[2].c_str());
    df=atof(CmdStr.m_vCmdStr[3].c_str());

    if ( (dd<-0.01) || (dd>360.01) ) continue;
    if ( (df< 0.01) || (df> 50.01) ) continue;

    // ���վ�����ݵĺϷ���
    if (strlen(minvalue) > 0) { if (df<atof(minvalue)-0.001) continue; }
    if (strlen(maxvalue) > 0) { if (df>atof(maxvalue)+0.001) continue; }

    uu=df*-1.0*sin(dd*PI/180);
    vv=df*-1.0*cos(dd*PI/180);

    fwrite(&stdata,sizeof(struct st_stdata),1,DatFile.m_fp);

    fwrite(&uu,sizeof(float),1,DatFile.m_fp);
    fwrite(&vv,sizeof(float),1,DatFile.m_fp);
    fwrite(&df,sizeof(float),1,DatFile.m_fp);

    stcount++;
  }

  memset(&stdata,0,sizeof(stdata));

  fwrite(&stdata,sizeof(struct st_stdata),1,DatFile.m_fp);

  TxtFile.Fclose();

  DatFile.Fclose();

  return TRUE;
}

// ���ɿ����ļ�
BOOL CrtCtlFile()
{
  CFile File;

  if (File.OpenForWrite(ctlfilename1,"w+") == FALSE) 
  {
    logfile.Write("Fopen %s failed.\n",ctlfilename1); return FALSE; 
  }

  File.Fprintf("DSET %s\n",datfilename);
  File.Fprintf("DTYPE STATION\n");
  File.Fprintf("STNMAP %s\n",datmfilename);
  File.Fprintf("UNDEF -999.9\n");
  File.Fprintf("TITLE no title\n");
  File.Fprintf("TDEF 1 linear 23Z7JUL2008 3hr\n");
  File.Fprintf("VARS 3\n");
  File.Fprintf("uu 0 99 wf for lat\n");
  File.Fprintf("vv 0 99 wf for lon\n");
  File.Fprintf("df 0 99 wf value\n");
  File.Fprintf("ENDVARS\n");

  File.Fclose();

  if (File.OpenForWrite(ctlfilename2,"w+") == FALSE) 
  {
    logfile.Write("Fopen %s failed.\n",ctlfilename2); return FALSE; 
  }

  File.Fprintf("DSET %s\n",grbfilename);
  File.Fprintf("TITLE no title\n");
  File.Fprintf("UNDEF -999.9\n");
  File.Fprintf("XDEF %s LINEAR %s %s\n",lonsum,lonstart,lonstep);
  File.Fprintf("YDEF %s LINEAR %s %s\n",latsum,latstart,latstep);
  File.Fprintf("TDEF 1 linear 23Z7JUL2008 3hr\n");
  File.Fprintf("ZDEF 1 levels 1000\n");
  File.Fprintf("VARS 1\n");
  File.Fprintf("stgrib 0 99 afd\n");
  File.Fprintf("ENDVARS\n");

  File.Fclose();

  return TRUE;
}

// ���ɻ�ͼ�Ľű��ļ�
BOOL CrtGSFile()
{
  CFile File;

  if (File.OpenForWrite(gsfilename,"w+") == FALSE) 
  {
    logfile.Write("Fopen %s failed.\n",gsfilename); return FALSE; 
  }

  //��ʼ��grads������������
  File.Fprintf("'reinit'\n");
  File.Fprintf("'reset'\n");
  File.Fprintf("'clear'\n");
  File.Fprintf("'!%s/bin/stnmap -i %s'\n",gradsroot,ctlfilename1);

  // �򿪿����ļ�
  File.Fprintf("'open %s'\n",ctlfilename2);
  File.Fprintf("'open %s'\n",ctlfilename1);

  // ��������
  File.Fprintf("'set grid off'\n");

  // ���õ�ֵ������style��1 ʵ�ߣ�2 �����ߣ�3 �����ߣ�4 �������ߣ�5 ���ߡ�clear ��display���������á�
  File.Fprintf("'set cstyle 1'\n");

  // ���õ�ֵ���߿�thckns��ȡֵ1-10 ֮�����������Ļ��һ��ȡС��6��ȱʡֵΪ4����Ҫ���ڿ���Ӳ�������
  File.Fprintf("'set cthick 3'\n");

  // ����С�����λ��Ϊ1λ
  File.Fprintf("'set dignum 1'\n");

  // �����ַ���С��hsiz ���ַ���ˮƽ���ֵ����λΪ��ҳӢ��
  File.Fprintf("'set strsiz 0.1'\n");

  // �������α߿�
  File.Fprintf("'set frame on'\n");

  // �������壬�����number����ȡ0,1,...,5
  File.Fprintf("'set font 1'\n");

  //����ѡ���Ƿ��ӡ��GrADS ���
  File.Fprintf("'set grads off'\n");

  // ���þ�γ�ȵķ�Χ
  File.Fprintf("'set lon %.2f %.2f'\n",atof(lonstart),atof(lonstart)+atof(lonsum)*atof(lonstep));
  File.Fprintf("'set lat %.2f %.2f'\n",atof(latstart),atof(latstart)+atof(latsum)*atof(latstep));

  // ���õ�ǰ��ͼͶӰ��ʽ
  // latlon-ȱʡ���ã��ù̶���ͶӰ�ǽ���Lat/LonͶӰ
  // scaled-�ò��̶���ͶӰ�ǽ���Lat/LonͶӰ��ͶӰ������������ͼ��
  // off-ͬscaled ����������ͼ��������Ҳ������Lat/Lon�����ǣ�ʵ��֤������γ��������Ȼ����
  File.Fprintf("'set mproj latlon'\n");

  // ����������ֵ�⻬���أ��ٶ������������á���ɫ�ĵ�ֵ��ͼû�������⻬
  // ������csmooth ѡ����׼ȷ�ؽ���ֵ������ɫͼ��Ե�غ�
  File.Fprintf("'set cterp on'\n");

  // ��ȡon �ڻ��ֵ��ͼǰ�����β�ֵ��������ֵ�嵽����ϸ������
  // �ò�ֵ����ɸ�ֵ�⻬��ʧ�棬�縺��ˮ
  File.Fprintf("'set csmooth on'\n");

  // ���õ�ͼ���ݼ�
  // lowres Ϊȱʡ�Ĵֱַ��ʵ�ȫ���ͼ
  // mres ��hires �ֱ�Ϊ�зֱ�͸߷ֱ���ȫ���ͼ��ͬʱ���й�����ݽ�
  // nam Ϊ�����޵�ͼ
  File.Fprintf("'set mpdset guangdong_q'\n");

  // �ö��Ƶ���ɫ�����ͺ��߿�汳����ͼ
  File.Fprintf("'set map 1 1 3 4'\n");

  // ��mres��hires��ͼ�п���ѡ���Ƿ�ѡ�������߽磬ȱʡΪon
  File.Fprintf("'set poli %s'\n",poli);

  // �������������ʾ
  File.Fprintf("'set xlab off'\n");
  File.Fprintf("'set ylab off'\n");

  // ��ʾ��ֵ�ߵı�ǣ�����ֵ���ϵ���ֵ
  File.Fprintf("'set clab on'\n");

  // ��ʾ�涨���������ֵ�߱�ʾ��ֵ���ò�������������
  File.Fprintf("'set clskip 1'\n");

  // ���õ��������ڵ�ֵ�߻�ͼ����ͼ��ͼ�����߻�ͼ��������������ҳӢ��Ϊ��λ��ȱʡʱ���Զ���ͼ���������û�ͼ����
  File.Fprintf("'set parea 0 11 0 8.5'\n");

  /*
  // �����ٵĵ�ֵ�ߣ���ʱ��֧�֣�ע�⣬���Ҫ����ֵ�ߣ�һ��Ҫ���ڻ���ʸͼǰ��
  File.Fprintf("'set gxout shaded'\n");
  File.Fprintf("'define cc=oacres(stgrib,df.2,8,6,4,3,2,1)'\n");
  File.Fprintf("'d cc'\n");
  */

  //File.Fprintf("'set gxout barb'\n");

  // ������ֵ��ÿ�����ķ�ʸ
  File.Fprintf("'define aa=oacres(stgrib,uu.2,8,6,4,3,2,1)'\n");
  File.Fprintf("'define bb=oacres(stgrib,vv.2,8,6,4,3,2,1)'\n");

  if (strcmp(gxout,"barb") == 0)
  {
    // ���û�ͼ�ķ�ʽ
    File.Fprintf("'set gxout barb'\n");
    File.Fprintf("'d aa*2.5;bb*2.5'\n");
  }

  if (strcmp(gxout,"vector") == 0)
  {
    // ���û�ͼ�ķ�ʽ
    File.Fprintf("'set gxout vector'\n");
    File.Fprintf("'d aa*2.5;bb*2.5'\n");
  }

  if (strcmp(gxout,"stream") == 0)
  {
    // ���û�ͼ�ķ�ʽ
    File.Fprintf("'set gxout stream'\n");
    File.Fprintf("'d aa*2.5;bb*2.5'\n");
  }

  if (strcmp(gxout,"off") == 0)
  {
    // ��ʾվ���ʸ
    File.Fprintf("'set gxout barb'\n");
    File.Fprintf("'d uu.2*2.5;vv.2*2.5'\n");
  }

  // ��ʾվ������
  if ( (strcmp(dispvalue,"true") == 0) || (strcmp(dispvalue,"TRUE") == 0) )
  {
    File.Fprintf("'d df.2'\n");
  }

  File.Fprintf("'printim %s white'\n",tmpgiffilename);

  File.Fprintf("'quit'\n");

  File.Fclose();

  return TRUE;
}

void EXIT(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  if (sig != 0) 
  {
    logfile.Write("catching the signal(%d).\n",sig);

    logfile.Write("ftpgetfiles exit.\n");
  }

  REMOVE(txtfilename);
  REMOVE(datfilename);
  REMOVE(datmfilename);
  REMOVE(ctlfilename1);
  REMOVE(ctlfilename2);
  REMOVE(grbfilename);
  REMOVE(gsfilename);
  REMOVE(tmpgiffilename);

  exit(0);
}

