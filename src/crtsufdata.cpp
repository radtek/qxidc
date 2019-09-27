
#include "_public.h"




//全国气象站点数据结构
//北京,54406,延庆,40.27,115.58,487.9

struct st_stcode
{
  char provname[31];  //省份名
  char obtid[11];     //站点参数
  char cityname[30];  //城市名字
  double lat;         //纬度
  double lon;         //经度
  double height;      //海拔高度
};

//存放站点参数的容器
vector<struct st_stcode> vstcode;

//全国气象站点分钟观测数据结构

struct st_surfdata
{
  char obtid[11];       //站点代码
  char ddatetime[21];   //数据时间：格式yyyy-mm-dd hh24:mi:ss
  int t;                //气温：单位，0.1摄氏度 小数点后一位
  int p;                //气压：0.1百帕
  int u;                //相对湿度 0-100之间的值
  int wd;               //风向 0-360之间的值
  int wf;               //风速：单位0.1m/s   
  int r;                //降雨量：0.1mm
  int vis;              //能见度：0.1m
};

//double用整数来表达


vector<struct st_surfdata> vsurfdata;   //存放观测数据的容器


//加载参数文件，内容存放在 vstcode容器中
bool LoadSTCode(const char *inifile);

void CrtSurfData();

bool CrtSurfFile(const char *outpath);

CLogFile logfile;

int main(int argc,char *argv[])
{

  if (argc != 4)
  {
    printf("本程序用于生成全国气象站点观测的分钟数据. \n");

    printf("用法：/root/qxidc/src/crtsufdata 参数文件名： \n");

    printf("例如：/root/qxidc/src/crtsufdata /root/qxidc/ini/stcode.ini  /root/qxidc/data/ftp/sufdata  /root/qxidc/log/crtsurfdata.log \n");

    printf("参数文件是全国气象站点参数，如有疑问，请联系qq29625169 \n");

    return -1;
  }



  
  //关闭全部的信号和输入输出
  // LoadSTCode(argv[1]);
  void CloseIOAndSignal();
  //
  
  //处理程序退出的信号

 // signal(SIGINT,EXIT);signal(SIGTERM,EXIT);

  

  if (logfile.Open(argv[3],"a+")==false)
  {
    printf("打开日志文件失败（%s）",argv[3]);
    return -1;
  }


  logfile.Write("打开日志文件成功（%s）\n",argv[3]);
  

  if (LoadSTCode(argv[1]) == false) return -1;

  /*
  for (int ii=0;ii<1000000;ii++)
  {
    logfile.Write("例如：/root/qxidc/src/crtsufdata /root/qxidc/ini/stcode.ini  /root/qxidc/data/ftp/sufdata  /root/qxidc/log/crtsurfdata.log\n");
  }  
  */
  logfile.Write("加载参数文件%s成功!.\n",argv[1]);

  while(1)
  {
    CrtSurfData();    //创建全国气象站点分钟观测数据,存放在vsurfdata容器中

    //把容器vsurfdata中的全国气象站点分钟观测数据写入文件

    if (CrtSurfFile(argv[2])==false)  return -1;
    
    sleep(60);
  }
  return 0;
}


bool LoadSTCode(const char *inifile)
{
  CCmdStr CmdStr;
  struct st_stcode stcode;
  CFile File;

  if (File.OpenForRead(inifile,"r")==false)
  {
    printf("File.Open(%s) 失败。\n",inifile);
    return false;
  }

  char strBuffer[101];

  while (true)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    memset(&stcode,0,sizeof(struct st_stcode));

    if (File.FFGETS(strBuffer,100)==false)    break;
  
    CmdStr.SplitToCmd(strBuffer,",",true);
 
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


void CrtSurfData()
{
  srand(time(0));     //播随机数种子
 

  char strLocalTime[21];
  LocalTime(strLocalTime,"yyyy-mm-dd hh:mi");
  strcat(strLocalTime,":00"); 

  struct st_surfdata stsurfdata;

  for ( int ii = 0; ii<vstcode.size();ii++)
  {

    memset(&stsurfdata,0,sizeof(struct st_surfdata));  

    STRCPY(stsurfdata.obtid,10,vstcode[ii].obtid);   //站点代码 

    STRCPY(stsurfdata.ddatetime,20,strLocalTime);  //数据时间采用当前时间
    //LocalTime(stsurfdata.ddatetime);              //数据时间采用当前时间
   
    stsurfdata.t = rand()%351;                    //温：单位，0.1摄氏度 小数点后一位
    stsurfdata.p = rand()%265+10000;              //气压：0.1百帕  上下偏132
    stsurfdata.u = rand()%100 + 1;                //相对湿度 0-100之间的值  
    stsurfdata.wd = rand()%360;                   //风向 0-360之间的值
    stsurfdata.wf = rand()%150;                   //风速：单位0.1m/s
    stsurfdata.r = rand()%16;                     //降雨量：0.1mm
    stsurfdata.vis = rand()%5001+10000;           //能见度：0.1m

    vsurfdata.push_back(stsurfdata);
  } 
}
 

bool CrtSurfFile(const char *outpath)
{
  CFile File;
  char strFileName[301];
  char strLocalTime[21];

  memset(strFileName,0,sizeof(strFileName));
 
  LocalTime(strLocalTime,"yyyymmddhhmiss");
  SNPRINTF(strFileName,300,"%s/SURF_ZH_%s_%d.txt",outpath,strLocalTime,getpid()); 
  
  if (File.OpenForWrite(strFileName,"w")==false)
  {
    printf("File.Open(%s)failed！\n",strFileName);  return false;
  }

  for (int ii=0;ii<vsurfdata.size();ii++)
  {
    //站点代码，数据时间，气温 气压 相对湿度 风向 风速 降雨量 能见度
    File.Fprintf("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",\
                  vsurfdata[ii].obtid,vsurfdata[ii].ddatetime,\
                  vsurfdata[ii].t/10.0,vsurfdata[ii].p/10.0,vsurfdata[ii].u,\
                  vsurfdata[ii].wd,vsurfdata[ii].wf/10.0,vsurfdata[ii].r/10.0,\
                  vsurfdata[ii].vis/10.0);
  }

  logfile.Write("生成数据文件（%s）成功，数据时间=%s.,记录数=%d!\n",strFileName,vsurfdata[0].ddatetime,vsurfdata.size());

  //File.Close();  //关闭文件 
  return true;
}

void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d\n",sig);

  exit(0);
}
