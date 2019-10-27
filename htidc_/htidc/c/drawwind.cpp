#include "_public.h"

CLogFile logfile;

void EXIT(int sig);

// 从xmlbuffer中获取各种需要画等值线的参数
BOOL GetXMLBuffer(char *xmlbuffer);

char gxout[21];
char dispvalue[21];
char minvalue[21];
char maxvalue[21];
char poli[21];
char corp[51];

// 经纬度的点的个数、起始位置和格点宽度
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

// 生成用于插值的grib文件
BOOL CrtGribFile();

// 把用于画图的站点数据从文本文件转换为二进制文件
BOOL DataTxtToBin();

// 生成控制文件
BOOL CrtCtlFile();

// 生成画图的脚本文件
BOOL CrtGSFile();

#ifndef PI
#define PI 3.14159265358979323846   // 圆周率
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

    printf("本程序用于画站点的风矢图，logfilename为日志文件名，xmlbuffer为画风矢图的各种参数。\n");
    printf("本程序会在/tmp/htidc/tmp目录下生成一些用于画图的临时文件。\n\n");

    printf("logfilename    程序运行的日志文件名。\n");
    printf("xmlbuffer      程序运行的各种参数，具体含义如下：\n");
    printf("gradsroot      气象画图软件的安装目录，请咨询数据中心的后台程序员。\n");
    printf("lonstart       画图区域左下角的经度，单位：度。\n");
    printf("lonstep        画图区域X方向格点的宽度，单位：度，视站点的密度而定，一般在0.02至0.10之间，"\
           "站点越密，取值越小，站点越疏，取值越大。\n");
    printf("lonsum         画图区域X方向格点的数量。\n");
    printf("latstart       画图区域左下角的纬度，单位：度。\n");
    printf("latstep        画图区域Y方向格点的宽度，单位：度，视站点的密度而定，一般在0.02至0.10之间，"\
           "站点越密，取值越小，站点越疏，取值越大。\n");
    printf("latsum         画图区域Y方向格点的数量。\n");
    printf("gxout          生成图形的种类，vector-矢量箭头，stream-流线形式，barb-风向杆，off-站点风矢。\n");
    printf("dispvalue      是否显示每个站点的风速值，true-显示，false-不显示。\n");
    printf("minvalue       站点合法数据的最小值，小于minvalue值的站点将被丢弃掉，如果minvalue为空，表示不检查。\n");
    printf("maxvalue       站点合法数据的最大值，大于maxvalue值的站点将被丢弃掉，如果maxvalue为空，表示不检查。\n");
    printf("poli           是否显示GRADS地图的边界，on-显示，off-不显示，一般来说，"\
           "在调试阶段，poli选项用于校验GRADS生成的图与地图底图的位置，正式应用中一般不启用。\n");
    printf("corp           图片剪切参数，618x618+80+0表示从GRADS生成的原图的80,0像素位置开始，"\
           "剪切大小为618x618的图片内容。注意，剪切后的内容为GRADS画图框内的内容，大小与底图相同。"\
           "注意，如果corp为空，图片不剪切。\n");
    printf("mapfilename    底图文件名，一定要GIF文件，并且，用于画图的区域必须透明，该字段可以为空。"\
           "注意，GRADS生成的图片的大小由经纬度的大小范围决定，所以，底图的大小必须与GRADS生成的图"\
           "（剪切后）相同大小，否则会出现混乱。如果底图文件名为空，将得到GRADS生成的原始的图。\n");
    printf("txtfilename    站点数据文件名，站点数据文件为CSV文件，每一行表示一个站点的数据，格式\"纬度,经度,风向,风速\"。\n");
    printf("giffilename    输出图片的文件名，必须为GIF文件名。\n\n");
    
    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
  
  // 打开日志文件
  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("drawwind");

  // 解析xmlbuffer，读取各种参数
  if (GetXMLBuffer(argv[2]) == FALSE) EXIT(-1);

  // 把用于画图的站点数据从文本文件转换为二进制文件
  if (DataTxtToBin() == FALSE) EXIT(-1);

  if (stcount<2)
  {
    logfile.Write("有效数据站点数少于2，不画图，直接输出地图。\n");
    COPY(mapfilename,giffilename);
    EXIT(0);
  }

  // 生成用于插值的grib文件
  if (CrtGribFile() == FALSE) EXIT(-1);
  
  // 生成控制文件
  if (CrtCtlFile() == FALSE) EXIT(-1);

  // 生成画图的脚本文件
  if (CrtGSFile() == FALSE) EXIT(-1);

  // 设置Grads的安装目录
  char gradsdir[301];
  memset(gradsdir,0,sizeof(gradsdir));
  snprintf(gradsdir,200,"%s/data",gradsroot);
  setenv("GADDIR",gradsdir,1);

  logfile.Write("create %s ...",giffilename);

  // 画图
  char strCMD[2048];
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,2000,"%s/bin/grads -lcb \"run %s\"",gradsroot,gsfilename);
  system(strCMD);
  logfile.WriteEx("\n%s ... ok.\n",strCMD);

  // 剪切GRADS生成的图
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

  // 合成地图
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

// 从xmlbuffer中获取各种需要画等值线的参数
BOOL GetXMLBuffer(char *xmlbuffer)
{
  // 经纬度的点的个数、起始位置和格点宽度
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

// 生成用于插值的grib文件
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

// 把用于画图的站点数据从文本文件转换为二进制文件
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

    // 检查站点数据的合法性
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

// 生成控制文件
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

// 生成画图的脚本文件
BOOL CrtGSFile()
{
  CFile File;

  if (File.OpenForWrite(gsfilename,"w+") == FALSE) 
  {
    logfile.Write("Fopen %s failed.\n",gsfilename); return FALSE; 
  }

  //初始化grads环境，清屏。
  File.Fprintf("'reinit'\n");
  File.Fprintf("'reset'\n");
  File.Fprintf("'clear'\n");
  File.Fprintf("'!%s/bin/stnmap -i %s'\n",gradsroot,ctlfilename1);

  // 打开控制文件
  File.Fprintf("'open %s'\n",ctlfilename2);
  File.Fprintf("'open %s'\n",ctlfilename1);

  // 不画网格
  File.Fprintf("'set grid off'\n");

  // 设置等值线线形style：1 实线，2 长虚线，3 短虚线，4 长短虚线，5 点线。clear 或display即重新设置。
  File.Fprintf("'set cstyle 1'\n");

  // 设置等值线线宽thckns，取值1-10 之间的整数，屏幕上一般取小于6，缺省值为4，主要用于控制硬拷贝输出
  File.Fprintf("'set cthick 3'\n");

  // 设置小数点后位数为1位
  File.Fprintf("'set dignum 1'\n");

  // 设置字符大小，hsiz 是字符的水平宽度值，单位为虚页英寸
  File.Fprintf("'set strsiz 0.1'\n");

  // 不画矩形边框
  File.Fprintf("'set frame on'\n");

  // 设置字体，字体号number，可取0,1,...,5
  File.Fprintf("'set font 1'\n");

  //开关选择是否打印出GrADS 标记
  File.Fprintf("'set grads off'\n");

  // 设置经纬度的范围
  File.Fprintf("'set lon %.2f %.2f'\n",atof(lonstart),atof(lonstart)+atof(lonsum)*atof(lonstep));
  File.Fprintf("'set lat %.2f %.2f'\n",atof(latstart),atof(latstart)+atof(latsum)*atof(latstep));

  // 设置当前地图投影方式
  // latlon-缺省设置，用固定的投影角进行Lat/Lon投影
  // scaled-用不固定的投影角进行Lat/Lon投影，投影区充满整个绘图区
  // off-同scaled 但不画出地图，坐标轴也不代表Lat/Lon，但是，实际证明，经纬度坐标依然有用
  File.Fprintf("'set mproj latlon'\n");

  // 设置样条插值光滑开关，再定义后才重新设置。填色的等值线图没有样条光滑
  // 但可用csmooth 选项来准确地将等值线与填色图边缘重合
  File.Fprintf("'set cterp on'\n");

  // 如取on 在绘等值线图前用三次插值将现网格值插到更精细网格上
  // 该插值可造成负值光滑或失真，如负降水
  File.Fprintf("'set csmooth on'\n");

  // 设置地图数据集
  // lowres 为缺省的粗分辨率的全球地图
  // mres 和hires 分别为中分辨和高分辨率全球地图，同时含有国界和州界
  // nam 为北美洲地图
  File.Fprintf("'set mpdset guangdong_q'\n");

  // 用定制的颜色，线型和线宽绘背景地图
  File.Fprintf("'set map 1 1 3 4'\n");

  // 在mres或hires地图中开关选择是否选用行政边界，缺省为on
  File.Fprintf("'set poli %s'\n",poli);

  // 设置坐标轴的显示
  File.Fprintf("'set xlab off'\n");
  File.Fprintf("'set ylab off'\n");

  // 显示等值线的标记，即等值线上的数值
  File.Fprintf("'set clab on'\n");

  // 表示规定间隔几条等值线标示数值，该参数好象不起作用
  File.Fprintf("'set clskip 1'\n");

  // 设置的区域用于等值线绘图、地图绘图、单线绘图，该区域内以虚页英寸为单位。缺省时，自动按图形类型设置绘图区域。
  File.Fprintf("'set parea 0 11 0 8.5'\n");

  /*
  // 画风速的等值线，暂时不支持，注意，如果要画等值线，一定要放在画风矢图前面
  File.Fprintf("'set gxout shaded'\n");
  File.Fprintf("'define cc=oacres(stgrib,df.2,8,6,4,3,2,1)'\n");
  File.Fprintf("'d cc'\n");
  */

  //File.Fprintf("'set gxout barb'\n");

  // 画出插值后，每个格点的风矢
  File.Fprintf("'define aa=oacres(stgrib,uu.2,8,6,4,3,2,1)'\n");
  File.Fprintf("'define bb=oacres(stgrib,vv.2,8,6,4,3,2,1)'\n");

  if (strcmp(gxout,"barb") == 0)
  {
    // 设置画图的方式
    File.Fprintf("'set gxout barb'\n");
    File.Fprintf("'d aa*2.5;bb*2.5'\n");
  }

  if (strcmp(gxout,"vector") == 0)
  {
    // 设置画图的方式
    File.Fprintf("'set gxout vector'\n");
    File.Fprintf("'d aa*2.5;bb*2.5'\n");
  }

  if (strcmp(gxout,"stream") == 0)
  {
    // 设置画图的方式
    File.Fprintf("'set gxout stream'\n");
    File.Fprintf("'d aa*2.5;bb*2.5'\n");
  }

  if (strcmp(gxout,"off") == 0)
  {
    // 显示站点风矢
    File.Fprintf("'set gxout barb'\n");
    File.Fprintf("'d uu.2*2.5;vv.2*2.5'\n");
  }

  // 显示站点数据
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

