#include "idcapp.h"
#include "_public.h"

void CallQuit(int sig);

char strIDCConnStr[201]; // ���ݿ����Ӳ���
char strLogPath[201];    // ��־�ļ�Ŀ¼
char strTmpPath[201];    // ���̻��Ϣ�ļ�Ŀ¼
char strIniFile[201]; 
//char strDType[11];
int  iDays;
char strObtID[11];

connection     conn;
CLogFile       logfile;
CIniFile       IniFile;
CProgramActive ProgramActive;
CAWSDAYD       AWSDAYD;

long _pawsdayd();

int main(int argc,char *argv[])
{
  if ( argc != 3 && argc != 4)
  {
    printf("\n");
    printf("Using:./pawsdayd inifile days [obtid]\n");

    printf("Example:/htidc/htidc/bin/procctl 1800 /htidc/htidc/bin/pawsdayd /htidc/sqxt/ini/sqxt.xml 1 G3501\n");
    printf("        /htidc/htidc/bin/procctl 7200 /htidc/htidc/bin/pawsdayd /htidc/sqxt/ini/sqxt.xml 1 \n\n");
 
    printf("�˳�������ͳ���Զ�վ�������ݣ�ÿ��ͳ��ȫ��վ�����days��ļ�¼��\n");
    printf("�ó���ͳ�Ƶ���������������ͳ�ƣ�����ͬ��pcalobtdayd����\n");
    printf("Ŀǰֻͳ�������Զ�վ�����ݣ�û��ͳ��ȫʡ�Զ�վ�����ݡ�\n");
  
    return -1;
  }

  memset(strIniFile,0,sizeof(strIniFile));
//  memset(strDType,0,sizeof(strDType));
  memset(strObtID,0,sizeof(strObtID));
  iDays=0;

  strcpy(strIniFile,argv[1]);
  iDays=atoi(argv[2]);
//  strncpy(strDType,argv[3],10);
  if (argc==4) strncpy(strObtID,argv[3],5);

  // ��������ļ�
  if (IniFile.LoadFile(strIniFile) == FALSE)
  {
    printf("IniFile.LoadFile(%s) failed.\n",strIniFile); return -1;
  }

  // �Ӳ����ļ��л�ȡ���ݿ����Ӳ���
  memset(strIDCConnStr,0,sizeof(strIDCConnStr));
  if (IniFile.GetValue("appconnstr",strIDCConnStr) == FALSE)
  {
    printf("IniFile.GetValue field(appconnstr) failed.\n"); return -1;
  }

  // �Ӳ����ļ��л�ȡ��־�ļ����Ŀ¼����
  memset(strLogPath,0,sizeof(strLogPath));
  if (IniFile.GetValue("logpath",strLogPath) == FALSE)
  {
    printf("IniFile.GetValue field(logpath) failed.\n"); return -1;
  }
  
  // �Ӳ����ļ��л�ȡ��ʱ�ļ����Ŀ¼����
  memset(strTmpPath,0,sizeof(strTmpPath));
  if (IniFile.GetValue("tmppath",strTmpPath) == FALSE)
  {
    printf("IniFile.GetValue field(tmppath) failed.\n"); return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strLogFileName[201]; memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,201,"%s/pawsdayd.log",strLogPath);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt( "%s", argv[0] );

  // ע�⣬����ʱ��1200��
  ProgramActive.SetProgramInfo(&logfile,"pawsdayd",1200);

  //logfile.Write("pawsdayd begin.\n");

  AWSDAYD.BindConnLog(&conn,&logfile);

  // �������ݿ�
  /*
   drop   sequence SEQ_LOCALAWSDAYD;
   create sequence SEQ_LOCALAWSDAYD increment by 1 minvalue 1 nocycle;
  */

  if (conn.connecttodb(strIDCConnStr,TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strIDCConnStr); CallQuit(-1);
  }

  // ָ�����ݱ������
//  strncpy(AWSDAYD.m_dtype,strDType,5);

  _pawsdayd();

  logfile.WriteEx("\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("pawsdayd exit.\n");

  exit(0);
}

long _pawsdayd()
{
  char strMinTName[51],strHourTName[51],strRainTName[51];

  memset(strMinTName,0,sizeof(strMinTName));
  memset(strHourTName,0,sizeof(strHourTName));
  memset(strRainTName,0,sizeof(strRainTName));

  if (iDays <= 5)
  {
    sprintf(strMinTName ,"T_OBTMIND@SZQX_10_148_124_85");
  }
  else
  {
    sprintf(strMinTName ,"V_OBTMIND@SZQX_10_148_124_85");
  }

  sprintf(strHourTName,"T_OBTHOURD@SZQX_10_148_124_85");
  sprintf(strRainTName,"T_OBTRAIND@SZQX_10_148_124_85");

  struct st_OBTDAYD *pst=&AWSDAYD.m_stOBTDAYD;

  int okcount=0;

  while (TRUE)
  {
    // ��ȡһ����Ҫͳ�Ƶ���Ŀ�ʼʱ�䣬����ʱ��Ȳ���
    if (AWSDAYD.GetADayInfo(iDays--) == FALSE) break;

    logfile.Write("datetime=%s,begintime=%s,endtime=%s\n",AWSDAYD.m_ddatetime,AWSDAYD.m_begintime,AWSDAYD.m_endtime);

    // ���Զ�վ������������ͼ�л�ȡ��һ���ȫ�����Զ�վ����
    if (AWSDAYD.GetAllObtID(strHourTName,strObtID) != 0) { logfile.Write("call AWSDAYD.GetAllObtID failed.\n"); return -1; }

    while (AWSDAYD.GetAllObtIDNext() == 0)
    {
      //logfile.Write("obtid=%s\n",AWSDAYD.m_obtid);

      if (okcount++ > 10) { okcount=0; ProgramActive.WriteToFile(); }

      memset(pst,0,sizeof(AWSDAYD.m_stOBTDAYD));
   
      strcpy(pst->ddatetime,AWSDAYD.m_ddatetime);
      strcpy(pst->obtid,AWSDAYD.m_obtid);

      // ����20-08ʱ������08-20ʱ������������
      AWSDAYD.CALDayR(strHourTName);

      // �������ʮ���ӻ��������ͳ���ʱ��
      AWSDAYD.GetMathValue(strRainTName,"r10m","max",pst->maxr10m,"and substr(hdrbz,3,1)='1'");

      if (atoi(pst->maxr10m) == 0)
      {
        // û�н��������ձ�����ʮ���ӻ��������ͳ���ʱ���ֶ�
        memset(pst->maxr10m,0,sizeof(pst->maxr10m)); 
        memset(pst->maxr10mtime,0,sizeof(pst->maxr10mtime));
      }
      else
      {
        AWSDAYD.GetRecordDate(strRainTName,"r10m",pst->maxr10m,pst->maxr10mtime,"and substr(hdrbz,3,1)='1'");
      }

      // �������Сʱ���������ͳ���ʱ��
      AWSDAYD.GetMathValue(strRainTName,"r01h","max",pst->maxr01h,"and substr(hdrbz,5,1)='1'");
      if (atoi(pst->maxr01h) == 0)
      {
        // û�н��������ձ�����Сʱ���������ͳ���ʱ���ֶ�
        memset(pst->maxr01h,0,sizeof(pst->maxr01h)); 
        memset(pst->maxr01htime,0,sizeof(pst->maxr01htime));
      }
      else
      {
        AWSDAYD.GetRecordDate(strRainTName,"r01h",pst->maxr01h,pst->maxr01htime,"and substr(hdrbz,5,1)='1'");
      }

      // ������ߡ���Ϳ����¶Ⱥ����ĳ���ʱ��
      AWSDAYD.GetMathValue(strMinTName ,"t","max",pst->maxt);
      AWSDAYD.GetRecordDate(strMinTName,"t",pst->maxt,pst->maxttime);
      AWSDAYD.GetMathValue(strMinTName ,"t","min",pst->mint);
      AWSDAYD.GetRecordDate(strMinTName,"t",pst->mint,pst->minttime);

      // ������ߡ���ͱ�վ��ѹ�����ĳ���ʱ��
      AWSDAYD.GetMathValue(strMinTName ,"p","max",pst->maxp);
      AWSDAYD.GetRecordDate(strMinTName,"p",pst->maxp,pst->maxptime);
      AWSDAYD.GetMathValue(strMinTName ,"p","min",pst->minp);
      AWSDAYD.GetRecordDate(strMinTName,"p",pst->minp,pst->minptime);

      // ������ߡ���ͺ�ƽ����ѹ�����ĳ���ʱ��
      AWSDAYD.GetMathValue(strMinTName ,"p0","max",pst->maxp0);
      AWSDAYD.GetRecordDate(strMinTName,"p0",pst->maxp0,pst->maxp0time);
      AWSDAYD.GetMathValue(strMinTName ,"p0","min",pst->minp0);
      AWSDAYD.GetRecordDate(strMinTName,"p0",pst->minp0,pst->minp0time);

      // ������ߡ�������ʪ�Ⱥ����ĳ���ʱ��
      AWSDAYD.GetMathValue(strMinTName ,"rh","max",pst->maxrh);
      AWSDAYD.GetRecordDate(strMinTName,"rh",pst->maxrh,pst->maxrhtime);
      AWSDAYD.GetMathValue(strMinTName ,"rh","min",pst->minrh);
      AWSDAYD.GetRecordDate(strMinTName,"rh",pst->minrh,pst->minrhtime);

      // �������˲ʱ���ٺͶ�Ӧ�ķ��򼰳���ʱ��
      AWSDAYD.GetMathValue(strMinTName,"wdidf","max",pst->wd3smaxdf);
      AWSDAYD.GetDDAndTime(strMinTName,"wdidf","wdidd",pst->wd3smaxdf,pst->wd3smaxdd,pst->wd3smaxtime);

      // �������2���ӷ��ٺͶ�Ӧ�ķ��򼰳���ʱ��
      AWSDAYD.GetMathValue(strMinTName,"wd2df","max",pst->wd2maxdf);
      AWSDAYD.GetDDAndTime(strMinTName,"wd2df","wd2dd",pst->wd2maxdf,pst->wd2maxdd,pst->wd2maxtime);

      // �������10���ӷ��ٺͶ�Ӧ�ķ��򼰳���ʱ��
      //AWSDAYD.GetMathValue(strHourTName,"wd10maxdf","max",pst->wd10maxdf,"and (rddatetime>=54 or rddatetime=0)");
      AWSDAYD.GetMathValue(strMinTName,"wd10df","max",pst->wd10maxdf);
      AWSDAYD.GetDDAndTime(strMinTName,"wd10df","wd10dd",pst->wd10maxdf,pst->wd10maxdd,pst->wd10maxtime);

      // ͳ��ƽ��ֵ��Ŀ
      // ���һ����û��ȱ�⣬����ƽ��Ϊ24�ε�ƽ����
      // �����ȱ�⵫02��08��14��20��û��ȱ�⣬����ƽ�����Ĵι۲��ͳ�ƣ�
      // ����������������������㣬��ȱ�����С�ڵ���6��ʱ��ʵ�д���ͳ�ƣ�����6��ʱ��ȱ�⴦��
      AWSDAYD.GetAvgValue(strHourTName,"t",pst->avgt);
      AWSDAYD.GetAvgValue(strHourTName,"p0",pst->avgp0);
      AWSDAYD.GetAvgValue(strHourTName,"p",pst->avgp);
      AWSDAYD.GetAvgValue(strHourTName,"rh",pst->avgrh);
      AWSDAYD.GetAvgValue(strHourTName,"wdidf",pst->avgwdidf);
      AWSDAYD.GetAvgValue(strHourTName,"wd2df",pst->avgwd2df);
      AWSDAYD.GetAvgValue(strHourTName,"wd10df",pst->avgwd10df);

      // ����������ݵ�ʱ�䡢���Ӽ�¼�����������¼������
      AWSDAYD.GetLTimeCount(strMinTName,strHourTName);
/*
      // ������������Զ�վ���ݣ�����Ҫ������С�ܼ��ȼ������ʱ������ʪ��
      if (strcmp(strDType,"LOCAL") == 0)
      {
        AWSDAYD.GetMathValue("T_LOCALOBTV@SZQX_10_153_98_5" ,"v","max",pst->maxv);
        AWSDAYD.GetMathValue("T_LOCALOBTV@SZQX_10_153_98_5" ,"v","min",pst->minv);

        if ( (atoi(pst->maxv) == 0) || (atoi(pst->minv) == 0) )
        {
          memset(pst->maxv,0,sizeof(pst->maxv));
          memset(pst->maxvtime,0,sizeof(pst->maxvtime));
          memset(pst->minv,0,sizeof(pst->minv));
          memset(pst->minvtime,0,sizeof(pst->minvtime));
          memset(pst->minvtimeu,0,sizeof(pst->minvtimeu));
          memset(pst->avgv,0,sizeof(pst->avgv));
        }
        else
        {
          AWSDAYD.GetRecordDate("T_LOCALOBTV@SZQX_10_153_98_5","v",pst->maxv,pst->maxvtime);
          AWSDAYD.GetRecordDate("T_LOCALOBTV@SZQX_10_153_98_5","v",pst->minv,pst->minvtime);
          AWSDAYD.GetMinVTimeU("V_LOCALOBTMIND@SZQX_10_153_98_5",pst->minvtime,pst->minvtimeu);
          AWSDAYD.GetAvgValue("T_LOCALOBTV@SZQX_10_153_98_5","v",pst->avgv);
        }
      }

*/
      // �ѽ�����µ������ݱ���
      if (AWSDAYD.UPTDAYD() != 0) { logfile.Write("call AWSDAYD.UPTDAYD failed.\n"); return -1; }

      conn.commitwork();
    }

  }

  return 0;
}

