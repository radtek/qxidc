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
// ����Υ����Ϊ�����ȡΥ����Ϊ����
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

    printf("�������������ĳ���ֻ�����عغͻ��ݵĳ���ҵ��\n");
    printf("connstr�������ݿ�Ĳ�����\n");
    printf("srcpath����Դ�ļ���Ŀ¼��\n");
    printf("logfilename�������������ɵ���־�ļ�����\n\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  // �жϷ�������ʱ�䣬ֻ��00-07��֮�䴦���ļ�������06��֮���ٴ����ļ�
  // ��Ϊ�����������ݵ�ʱ�򣬻�truncate����ٲ��룬������������ϣ�����
  // ֻ���ڰ�ҹ��������ڰ��촦�����ǵ��˺��û����п��ܲ鲻�����ݣ����𲻱�Ҫ��Ͷ�ߡ�
  char strLocalTime[21];
  memset(strLocalTime,0,sizeof(strLocalTime));
  LocalTime(strLocalTime,"hh24mi");
  //if (strncmp(strLocalTime,"07",2) > 0) return 0;


  // ��ѹȫ����*.gz�ļ�
  char strCMD[1024];
  memset(strCMD,0,sizeof(strCMD));
  snprintf(strCMD,1000,"/usr/bin/gunzip -f %s/*.gz 1>/dev/null 2>/dev/null",argv[2]);
  system(strCMD);

  if (logfile.Open(argv[3],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[3]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("uptdata");

  // ע�⣬����ʱ��500��
  ProgramActive.SetProgramInfo(&logfile,"uptdata",500);

  // ����Ӧ�����ݿ�
  if (conn.connecttodb(argv[1]) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",argv[1]); CallQuit(-1);
  }

  // ע�⣬����ʱ֮ǰ��500�룬���ڸ�Ϊ180�룬��Ϊ���������ݿ�ʧ��ʱ����Ҫ�ܳ�ʱ�䣬�����ʱ����Ϊ180�룬
  // ���ܵ��¸澯�޷�����
  ProgramActive.SetProgramInfo(&logfile,"uptdata",180);

  //logfile.Write("uptdata beging.\n");

  // ������Դ�ļ�Ŀ¼
  if (Dir.OpenDir(argv[2],FALSE) == FALSE)
  {
    logfile.Write("Dir.OpenDir %s failed.\n",argv[2]); CallQuit(-1);
  }

  // ���л�ȡÿ���ļ������
  while (Dir.ReadDir() == TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // ����ļ���ʱ���ڵ�ǰʱ���ǰ50��֮�ڣ�����ʱ����⣬��ô����Ŀ����Ϊ�˱�֤�����ļ��������ԡ�
    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-50);
    if (strcmp(Dir.m_ModifyTime,strLocalTime)>0) continue;

    // ����������Ϣ 
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

    // ��ʻԱ������Ϣ 
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

    // Υ��δ������Ϣ 
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

    // д����̻��Ϣ
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
    logfile.Write("�ļ�%s��״̬������.\n",Dir.m_FullFileName); return FALSE;
  }

  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0)
  {
    logfile.Write("��%sʧ��.\n",Dir.m_FullFileName); return FALSE;
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

    // ��ȡһ��
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
        ; //logfile.Write("insert VEHICLE failed.\n%s\n",stmt.cda.message);   // ֻ��¼���󣬲��˳�
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
    logfile.Write("�����UptDataLogʧ�ܣ�%ld\n",stmt.cda.rc);
  }


  return TRUE;
}

BOOL _uptdrivinglicense()
{
  fp=0;

  if ( CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    logfile.Write("�ļ�%s��״̬������.\n",Dir.m_FullFileName); return FALSE;
  }

  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0)
  {
    logfile.Write("��%sʧ��.\n",Dir.m_FullFileName); return FALSE;
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

    // ��ȡһ��
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
        ; //logfile.Write("insert DRIVINGLICENSE failed.\n%s\n",stmt.cda.message);   // ֻ��¼���󣬲��˳�
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
    logfile.Write("�����UptDataLogʧ�ܣ�%ld\n",stmt.cda.rc);
  }

  return TRUE;
}

BOOL _uptwzwcl()
{
  fp=0;

  if ( CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    logfile.Write("�ļ�%s��״̬������.\n",Dir.m_FullFileName); return FALSE;
  }

  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0)
  {
    logfile.Write("��%sʧ��.\n",Dir.m_FullFileName); return FALSE;
  }

  char strLine[4096];
  char wzsxh[51],cardid[21],hphm[31],hpzl[11],wzdd1[101],wzsj[21]; 
  char wzxwmc[1001],cldd[81],zxdh[51],jc[3],wzxwdm[65],xzqhdm[16];
  int  bcfz,fkje;

  UINT totalcount=0;
  char strbegintime[20];

  memset(strbegintime,0,sizeof(strbegintime));
  LocalTime(strbegintime);

  // ����Υ���ֵ��������
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

    // ��ȡһ��
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
    UpdateStr(wzdd1,"�㶫ʡ","");
    UpdateStr(cldd,"\n",""); 
    UpdateStr(cldd,"�㶫ʡ","");
    UpdateStr(zxdh,"\n","");
    UpdateStr(wzxwdm,"\n",""); 
    UpdateStr(wzxwdm,"��",","); 
    UpdateStr(wzxwdm,".",",");
    UpdateStr(wzxwdm," ","");

    CmdStr.SplitToCmd(wzxwdm,",");

    // ����Υ����Ϊ���룬��Υ���ֵ��л�ȡΥ����Ϊ���ơ��۷ֺͷ����ֶ�
    jj=1;
    for (ii=0;ii<CmdStr.CmdCount();ii++)
    {
      strncpy(DM,CmdStr.m_vCmdStr[ii].c_str(),10);

      if (strlen(DM) < 2) continue;
      
      // ����Υ�´����ѯΥ���ֵ�
      if (GetWZZD() != 0)
      {
        logfile.Write("����GetWZZDʧ�ܣ�%ld\n");
      }

      memset(strTemp,0,sizeof(strTemp));

      if (selwzzd.next() != 0)
      {
        // ���Υ�´��벻���ڣ��Ѵ�����־��¼��wzzd_err����
        InsWZZDErr(wzsxh);
        
        sprintf(strTemp,"%lu�����ꣻ",jj++);
      }
      else
      {
        sprintf(strTemp,"%lu��%s��",jj++,MC);
      }

      strcat(wzxwmc,strTemp);

      // �ۼƿ۷ֺͷ����ֶ�
      bcfz=bcfz+KF;
      fkje=fkje+FK;
    }

    // ���Υ����Ϊ����ֻ��һ������ȥ��ǰ��ı��
    if (jj == 2) strcpy(wzxwmc,wzxwmc+3);

    // ��Υ����Ϊ���ķֺŻ��ɾ��
    if (strncmp(wzxwmc+strlen(wzxwmc)-2,"��",2) == 0)
      strncpy(wzxwmc+strlen(wzxwmc)-2,"��",2);

    // �����������Υ�²������в������ݣ���Υ����Ϊ����Ϊ����
    // ���Υ��˳���WZSXH����B��ͷ����ʾ��������Υ�£�A�ǹ���
    if ( (wzsxh[0]=='B') && (strstr(wzxwmc,"����")!=0) )
    {
      strcpy(wzxwmc,"���꣬���������н���֧����վwww.stc.gov.cn��ѯ���Υ����Ϣ��");
      bcfz=0; fkje=0;
    }

    if (stmt.execute() != 0)
    {
      if (stmt.cda.rc != 1)
      {
        ; //logfile.Write("insert WZWCLJL failed.\n%s\n",stmt.cda.message);   // ֻ��¼���󣬲��˳�
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
    logfile.Write("�����UptDataLogʧ�ܣ�%ld\n",stmt.cda.rc);
  }

  return TRUE;
}

// ����Υ����Ϊ�����ȡΥ����Ϊ����
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
    logfile.Write("��ѯWZZD����%ld\n",selwzzd.cda.rc);
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
