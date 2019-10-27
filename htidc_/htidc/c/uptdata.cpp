#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CDir           Dir;

FILE *fp;

BOOL _uptvehicle();

BOOL _uptdrivinglicense();

BOOL _uptwzwcl();

sqlstatement selwzzd;
char DM[11],MC[201];
int  KF,FK;
// 根据违章行为代码获取违章行为名称
long GetWZZD();
long InsWZZDErr(char *wzsxh);

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/uptdata connstr srcpath logfilename\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/uptdata zqcz/zqczserver /home/oracle/zqcz/data /home/oracle/zqcz/src/uptdata.log\n");
    printf("        /htidc/htidc/bin/procctl 300 /htidc/htidc/bin/uptdata hzcz/hzczserver /home/oracle/hzcz/data /home/oracle/hzcz/src/uptdata.log\n");

    printf("处理车主数据入库的程序，只用于肇关和惠州的车主业务。\n");
    printf("connstr连接数据库的参数。\n");
    printf("srcpath数据源文件的目录。\n");
    printf("logfilename本程序运行生成的日志文件名。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // 判断服务器的时间，只在00-07点之间处理文件，超过06点之后不再处理文件
  // 因为本程序处理数据的时候，会truncate表后再插入，会造成数据闪断，所以
  // 只能在半夜处理，如果在白天处理，我们的人和用户都有可能查不到数据，引起不必要的投诉。
  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"hh24mi");
  //if (strncmp(strLocalTime,"07",2) > 0) return 0;


  // 解压全部的*.gz文件
  char strCMD[1024];
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,1000,"/usr/bin/gunzip -f %s/*.gz 1>/dev/null 2>/dev/null",argv[2]);
  system(strCMD);

  if (logfile.Open(argv[3],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[3]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("uptdata");

  // 注意，程序超时是500秒
  ProgramActive.SetProgramInfo(&logfile,"uptdata",500);

  // 连接应用数据库
  if (conn.connecttodb(argv[1]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[1]); CallQuit(-1);
  }

  // 注意，程序超时之前是500秒，现在改为180秒，因为上面连数据库失败时可能要很长时间，如果把时间设为180秒，
  // 可能导致告警无法捕获。
  ProgramActive.SetProgramInfo(&logfile,"uptdata",180);

  //logfile.Write("uptdata beging.\n");

  // 打开数据源文件目录
  if (Dir.OpenDir(argv[2],FALSE) == FALSE)
  {
    logfile.Write("Dir.OpenDir %s failed.\n",argv[2]); CallQuit(-1);
  }

  // 逐行获取每个文件并入库
  while (Dir.ReadDir() == TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 如果文件的时间在当前时间的前50秒之内，就暂时不入库，这么做的目的是为了保证数据文件的完整性。
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-50);
    if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

    // 车辆基本信息 
    if ( MatchFileName(Dir.m_FileName,"VEHICLE_*.XML") == TRUE) 
    {
      logfile.Write("process file %s...\n",Dir.m_FullFileName);

      if (_uptvehicle() == FALSE)
      {
        logfile.Write("failed.\n"); conn.rollbackwork();
      }

      logfile.Write("ok.\n"); 

      conn.commitwork();
    }

    // 驾驶员基本信息 
    if ( MatchFileName(Dir.m_FileName,"DRIVING_*.XML") == TRUE) 
    {
      logfile.Write("process file %s...\n",Dir.m_FullFileName);

      if (_uptdrivinglicense() == FALSE)
      {
        logfile.Write("failed.\n"); conn.rollbackwork();
      }

      logfile.Write("ok.\n"); 

      conn.commitwork();
    }

    // 违章未处理信息 
    if ( MatchFileName(Dir.m_FileName,"WZWCLJL_*.XML") == TRUE) 
    {
      logfile.Write("process file %s...\n",Dir.m_FullFileName);

      if (_uptwzwcl() == FALSE)
      {
        logfile.Write("failed.\n"); conn.rollbackwork();
      }

      logfile.Write("ok.\n"); 

      conn.commitwork();
    }

    // 写入进程活动信息
    ProgramActive.WriteToFile();
  }

  //logfile.Write("uptdata exit.\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  if (fp>0) { fclose(fp); fp=0; }

  conn.disconnect(); 

  logfile.Write("uptdata exit.\n");

  exit(0);
}

BOOL _uptvehicle()
{
  fp=0;

  if ( CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    logfile.Write("文件%s的状态不完整.\n",Dir.m_FullFileName); return FALSE;
  }

  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0)
  {
    logfile.Write("打开%s失败.\n",Dir.m_FullFileName); return FALSE;
  }

  char strLine[4096];
  char hphm[31],hpzl[10],yxqz[21];

  UINT totalcount=0;
  char strbegintime[20];

  memset(strbegintime,0,sizeof(strbegintime));
  LocalTime(strbegintime);

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("truncate table VEHICLE");
  if (stmt.execute() != 0)
  {
    logfile.Write("delete VEHICLE failed.\n%s\n",stmt.cda.message); fclose(fp); fp=0; return FALSE;
  }

  //logfile.Write("delete VEHICLE ok.\n");

  stmt.prepare("insert into VEHICLE(hphm,hpzl,yxqz) values(:1,:2,to_date(:3,'yyyymmdd'))");
  stmt.bindin(1,hphm,30);
  stmt.bindin(2,hpzl,10);
  stmt.bindin(3,yxqz,8);

  UINT ucount=5001;

  while (TRUE)
  {
    ucount++;

    if (ucount>=5000) { ucount=0; ProgramActive.WriteToFile(); }

    memset(strLine,0,sizeof(strLine));

    // 读取一行
    if (FGETS(strLine,4000,fp,"<endl/>") == FALSE) break;

    memset(hphm,0,sizeof(hphm));
    memset(hpzl,0,sizeof(hpzl));
    memset(yxqz,0,sizeof(yxqz));

    GetXMLBuffer(strLine,"hphm",hphm,30);
    GetXMLBuffer(strLine,"hpzl",hpzl,10);
    GetXMLBuffer(strLine,"yxqz",yxqz,8);

    if (stmt.execute() != 0)
    {
      if (stmt.cda.rc != 1)
      {
        ; //logfile.Write("insert VEHICLE failed.\n%s\n",stmt.cda.message);   // 只记录错误，不退出
      }
    }
    
    if (stmt.cda.rc == 0) totalcount++;

    //logfile.WriteEx("hphm=%s,hpzl=%s,yxqz=%s\n",hphm,hpzl,yxqz);
  }

  fclose(fp);

  fp=0;

  REMOVE(Dir.m_FullFileName);

  stmt.prepare("insert into UPTDATALOG(logid,datatype,filename,begintime,spantime,totalcount)\
                       values(SEQ_UPTDATALOG.nextval,1,:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),\
                       (sysdate-to_date(:3,'yyyy-mm-dd hh24:mi:ss'))*24*60,:4)");
  stmt.bindin(1, Dir.m_FileName,100);
  stmt.bindin(2, strbegintime,19);
  stmt.bindin(3, strbegintime,19);
  stmt.bindin(4,&totalcount);

  if (stmt.execute() != 0)
  {
    logfile.Write("插入表UptDataLog失败：%ld\n",stmt.cda.rc);
  }


  return TRUE;
}

BOOL _uptdrivinglicense()
{
  fp=0;

  if ( CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    logfile.Write("文件%s的状态不完整.\n",Dir.m_FullFileName); return FALSE;
  }

  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0)
  {
    logfile.Write("打开%s失败.\n",Dir.m_FullFileName); return FALSE;
  }

  char strLine[4096];
  char sfzmhm[21],zjcx[21],syrq[21],yxqz[21],ljjf[11];

  UINT totalcount=0;
  char strbegintime[20];

  memset(strbegintime,0,sizeof(strbegintime));
  LocalTime(strbegintime);

  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("truncate table DRIVINGLICENSE");
  if (stmt.execute() != 0)
  {
    logfile.Write("delete from DRIVINGLICENSE failed.\n%s\n",stmt.cda.message); fclose(fp); fp=0; return FALSE;
  }
  
  //logfile.Write("delete DRIVINGLICENSE ok.\n");

  stmt.prepare("insert into DRIVINGLICENSE(sfzmhm,zjcx,syrq,yxqz,ljjf) values(:1,:2,to_date(:3,'yyyymmdd'),to_date(:4,'yyyymmdd'),:5)");
  stmt.bindin(1,sfzmhm,20);
  stmt.bindin(2,zjcx,20);
  stmt.bindin(3,syrq,8);
  stmt.bindin(4,yxqz,8);
  stmt.bindin(5,ljjf,3);

  UINT ucount=5001;

  while (TRUE)
  {
    ucount++;

    if (ucount>=5000) { ucount=0; ProgramActive.WriteToFile(); }

    memset(strLine,0,sizeof(strLine));

    // 读取一行
    if (FGETS(strLine,4000,fp,"<endl/>") == FALSE) break;

    memset(sfzmhm,0,sizeof(sfzmhm));
    memset(zjcx,0,sizeof(zjcx));
    memset(syrq,0,sizeof(syrq));
    memset(yxqz,0,sizeof(yxqz));
    memset(ljjf,0,sizeof(ljjf));

    GetXMLBuffer(strLine,"sfzmhm",sfzmhm,20);
    GetXMLBuffer(strLine,"zjcx",zjcx,20);
    GetXMLBuffer(strLine,"syrq",syrq,8);
    GetXMLBuffer(strLine,"yxqz",yxqz,8);
    GetXMLBuffer(strLine,"ljjf",ljjf,3);

    if (stmt.execute() != 0)
    {
      if (stmt.cda.rc != 1)
      {
        ; //logfile.Write("insert DRIVINGLICENSE failed.\n%s\n",stmt.cda.message);   // 只记录错误，不退出
      }
    }
    
    if (stmt.cda.rc == 0) totalcount++;

    //logfile.WriteEx("sfzmhm=%s,zjcx=%s,syrq=%s,yxqz=%s,ljjf=%s\n",sfzmhm,zjcx,syrq,yxqz,ljjf);
  }

  fclose(fp);

  fp=0;

  REMOVE(Dir.m_FullFileName);

  stmt.prepare("insert into UPTDATALOG(logid,datatype,filename,begintime,spantime,totalcount)\
                       values(SEQ_UPTDATALOG.nextval,3,:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),\
                       (sysdate-to_date(:3,'yyyy-mm-dd hh24:mi:ss'))*24*60,:4)");
  stmt.bindin(1, Dir.m_FileName,100);
  stmt.bindin(2, strbegintime,19);
  stmt.bindin(3, strbegintime,19);
  stmt.bindin(4,&totalcount);

  if (stmt.execute() != 0)
  {
    logfile.Write("插入表UptDataLog失败：%ld\n",stmt.cda.rc);
  }

  return TRUE;
}

BOOL _uptwzwcl()
{
  fp=0;

  if ( CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    logfile.Write("文件%s的状态不完整.\n",Dir.m_FullFileName); return FALSE;
  }

  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0)
  {
    logfile.Write("打开%s失败.\n",Dir.m_FullFileName); return FALSE;
  }

  char strLine[4096];
  char wzsxh[51],cardid[21],hphm[31],hpzl[11],wzdd1[101],wzsj[21]; 
  char wzxwmc[1001],cldd[81],zxdh[51],jc[3],wzxwdm[65],xzqhdm[16];
  int  bcfz,fkje;

  UINT totalcount=0;
  char strbegintime[20];

  memset(strbegintime,0,sizeof(strbegintime));
  LocalTime(strbegintime);

  // 清理违章字典错误代码表
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare("truncate table WZZD_ERR");
  if (stmt.execute() != 0)
  {
    logfile.Write("delete from WZZD_ERR failed.\n%s\n",stmt.cda.message); fclose(fp); fp=0; return FALSE;
  }

  stmt.prepare("truncate table WZWCLJL");
  if (stmt.execute() != 0)
  {
    logfile.Write("delete from WZWCLJL failed.\n%s\n",stmt.cda.message); fclose(fp); fp=0; return FALSE;
  }

  //logfile.Write("delete WZWCLJL ok.\n");

  stmt.prepare("insert into WZWCLJL(wzsxh,cardid,hphm,hpzl,wzdd1,wzsj,wzxwmc,bcfz,fkje,cldd,zxdh,jc,wzxwdm,xzqhdm) values(:1,:2,:3,:4,:5,to_date(:6,'yyyymmddhh24miss'),:7,:8,:9,:10,:11,:12,:13,:14)");
  stmt.bindin( 1, wzsxh,20);
  stmt.bindin( 2, cardid,20);
  stmt.bindin( 3, hphm,30);
  stmt.bindin( 4, hpzl,10);
  stmt.bindin( 5, wzdd1,100);
  stmt.bindin( 6, wzsj,14);
  stmt.bindin( 7, wzxwmc,1000);
  stmt.bindin( 8,&bcfz);
  stmt.bindin( 9,&fkje);
  stmt.bindin(10, cldd,80);
  stmt.bindin(11, zxdh,50);
  stmt.bindin(12, jc,2);
  stmt.bindin(13, wzxwdm,64);
  stmt.bindin(14, xzqhdm,15);


  UINT ucount=5001;

  UINT ii,jj;
  char strTemp[1024];
  CCmdStr CmdStr;

  while (TRUE)
  {
    ucount++;

    if (ucount>=5000) { ucount=0; ProgramActive.WriteToFile(); }

    memset(strLine,0,sizeof(strLine));

    // 读取一行
    if (FGETS(strLine,4000,fp,"<endl/>") == FALSE) break;

    memset(wzsxh,0,sizeof(wzsxh));
    memset(cardid,0,sizeof(cardid));
    memset(hphm,0,sizeof(hphm));
    memset(hpzl,0,sizeof(hpzl));
    memset(wzdd1,0,sizeof(wzdd1));
    memset(wzsj,0,sizeof(wzsj));
    memset(wzxwmc,0,sizeof(wzxwmc));
    bcfz=0;
    fkje=0;
    memset(cldd,0,sizeof(cldd));
    memset(zxdh,0,sizeof(zxdh));
    memset(jc,0,sizeof(jc));
    memset(wzxwdm,0,sizeof(wzxwdm));
    memset(xzqhdm,0,sizeof(xzqhdm));

    GetXMLBuffer(strLine,"wzsxh",wzsxh,20);
    GetXMLBuffer(strLine,"cardid",cardid,20);
    GetXMLBuffer(strLine,"hphm",hphm,30);
    GetXMLBuffer(strLine,"hpzl",hpzl,10);
    GetXMLBuffer(strLine,"wzdd1",wzdd1,100);
    GetXMLBuffer(strLine,"wzsj",wzsj,14); if (strlen(wzsj)==12) strncat(wzsj,"00",2); 
    //GetXMLBuffer(strLine,"wzxwmc",wzxwmc,1000);
    //GetXMLBuffer(strLine,"bcfz",bcfz,3);
    //GetXMLBuffer(strLine,"fkje",fkje,5);
    GetXMLBuffer(strLine,"cldd",cldd,80); 
    GetXMLBuffer(strLine,"zxdh",zxdh,50); 
    GetXMLBuffer(strLine,"jc",jc,2);
    GetXMLBuffer(strLine,"wzxwdm",wzxwdm,64);  
    GetXMLBuffer(strLine,"xzqhdm",xzqhdm,15);

    if (strlen(wzsxh) < 10) 
    {
      memset(wzsxh,0,sizeof(wzsxh)); snprintf(wzsxh,21,"%s%s%s",hphm,hpzl,wzsj+2);
    }

    UpdateStr(wzdd1,"\n",""); 
    UpdateStr(wzdd1,"广东省","");
    UpdateStr(cldd,"\n",""); 
    UpdateStr(cldd,"广东省","");
    UpdateStr(zxdh,"\n","");
    UpdateStr(wzxwdm,"\n",""); 
    UpdateStr(wzxwdm,"，",","); 
    UpdateStr(wzxwdm,".",",");
    UpdateStr(wzxwdm," ","");

    CmdStr.SplitToCmd(wzxwdm,",");

    // 根据违章行为代码，从违章字典中获取违章行为名称、扣分和罚款字段
    jj=1;
    for (ii=0;ii<CmdStr.CmdCount();ii++)
    {
      strncpy(DM,CmdStr.m_vCmdStr[ii].c_str(),10);

      if (strlen(DM) < 2) continue;
      
      // 根据违章代码查询违章字典
      if (GetWZZD() != 0)
      {
        logfile.Write("调用GetWZZD失败：%ld\n");
      }

      memset(strTemp,0,sizeof(strTemp));

      if (selwzzd.next() != 0)
      {
        // 如果违章代码不存在，把错误日志记录在wzzd_err表中
        InsWZZDErr(wzsxh);
        
        sprintf(strTemp,"%lu．不详；",jj++);
      }
      else
      {
        sprintf(strTemp,"%lu．%s；",jj++,MC);
      }

      strcat(wzxwmc,strTemp);

      // 累计扣分和罚款字段
      bcfz=bcfz+KF;
      fkje=fkje+FK;
    }

    // 如果违章行为内容只有一条，就去掉前面的编号
    if (jj == 2) strcpy(wzxwmc,wzxwmc+3);

    // 把违章行为最后的分号换成句号
    if (strncmp(wzxwmc+strlen(wzxwmc)-2,"；",2) == 0)
      strncpy(wzxwmc+strlen(wzxwmc)-2,"．",2);

    // 如果是在深圳违章并且已有不详内容，把违章行为定义为不详
    // 如果违章顺序号WZSXH是以B开头，表示是在深圳违章，A是广州
    if ( (wzsxh[0]=='B') && (strstr(wzxwmc,"不详")!=0) )
    {
      strcpy(wzxwmc,"不详，可在深圳市交警支队网站www.stc.gov.cn查询相关违法信息．");
      bcfz=0; fkje=0;
    }

    if (stmt.execute() != 0)
    {
      if (stmt.cda.rc != 1)
      {
        ; //logfile.Write("insert WZWCLJL failed.\n%s\n",stmt.cda.message);   // 只记录错误，不退出
      }
    }

    if (stmt.cda.rc == 0) totalcount++;

    //logfile.WriteEx("wzsxh=%s,wzsj=%s,bcfz=%s,fkje=%s\n",wzsxh,wzsj,bcfz,fkje);
  }

  fclose(fp);

  fp=0;

  REMOVE(Dir.m_FullFileName);

  stmt.prepare("insert into UPTDATALOG(logid,datatype,filename,begintime,spantime,totalcount)\
                       values(SEQ_UPTDATALOG.nextval,5,:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),\
                       (sysdate-to_date(:3,'yyyy-mm-dd hh24:mi:ss'))*24*60,:4)");
  stmt.bindin(1, Dir.m_FileName,100);
  stmt.bindin(2, strbegintime,19);
  stmt.bindin(3, strbegintime,19);
  stmt.bindin(4,&totalcount);

  if (stmt.execute() != 0)
  {
    logfile.Write("插入表UptDataLog失败：%ld\n",stmt.cda.rc);
  }

  return TRUE;
}

// 根据违章行为代码获取违章行为名称
long GetWZZD()
{
  if (selwzzd.state == selwzzd.not_opened)
  {
    selwzzd.connect(&conn);
    selwzzd.prepare("select MC,KF,FK from WZZD where DM=trim(:1)");
    selwzzd.bindin(1,DM,10);
    selwzzd.bindout(1,MC,200);
    selwzzd.bindout(2,&KF);
    selwzzd.bindout(3,&FK);
  }

  memset(MC,0,sizeof(MC));

  KF=FK=0;

  if (selwzzd.execute() != 0)
  {
    logfile.Write("查询WZZD错误：%ld\n",selwzzd.cda.rc);
  }

  return selwzzd.cda.rc;
}

long InsWZZDErr(char *wzsxh)
{
  sqlstatement stmt;
  
  stmt.connect(&conn);
  stmt.prepare("insert into wzzd_err values(:1,:2)");
  stmt.bindin(1,wzsxh,20);
  stmt.bindin(2,DM,10);

  return stmt.execute();
}
