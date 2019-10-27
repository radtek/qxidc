#include "_public.h"
#include "_oracle.h"

// ³ÌÐòÒì³£ÖÐ¶Ï´¦Àíº¯Êý
void EXIT(int sig);

CLogFile       logfile;                // ÈÕÖ¾ÎÄ¼þÀà
CProgramActive ProgramActive;          // ºóÌ¨½ø³Ì¼à¿ØÀà
CFile          File;
CDir           Dir;
connection     conn;

char pathname[301];
char andchild[11];
char mobilenostr[1001];
char strconnstr[101];
char strdwid[3];
char strywid[3];
char strclientname[21];
char strsendport[21];
char alarmcontent[301];
char strid[51];
long alarmcount;

struct st_DISKSTAT
{
  char filesystem[101];
  char total[31];
  char used[31];
  char available[31];
  char usep[31];
  char mount[31];
}m_stDISKSTAT;

// °Ñ¸æ¾¯²åÈëµ½¶ÌÐÅÆ½Ì¨
BOOL PalarmToSMS();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/checkdirandalarm checkdirandalarm.xml ipaddr\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/checkdirandalarm /htidc/szqx/c/checkdirandalarm.xml 10.153.130.58\n\n");

    printf("³ÌÐò¶ÁÈ¡checkdirandalarm.xml²ÎÊýÎÄ¼þ£¬·ÖÎö´ý¼à¿ØÄ¿Â¼ÏÂµÄÎÄ¼þÊý¡¢ÎÄ¼þ´óÐ¡¡¢´ÅÅÌ¿Õ¼äÊÇ·ñ³¬³ö·§Öµ£¬Èç¹û³¬³ö£¬¾Í·¢³ö¶ÌÐÅ¸æ¾¯¡£\n");
    printf("ÎÄ¼þ´óÐ¡Éè¶¨Îª³¬¹ý200MB£¬´ÅÅÌ¿Õ¼äÉè¶¨Îª³¬¹ý50%%¾Í¸æ¾¯¡£\n");
    printf("±¾³ÌÐòµÄ¬ÈÕÖ¾ÎÄ¼þÃûÎª/tmp/htidc/log/checkdirandalarm.log¡£\n\n");

    return -1;
  }

  // ¹Ø±ÕÈ«²¿µÄÐÅºÅºÍÊäÈëÊä³ö
  // ÉèÖÃÐÅºÅ,ÔÚshell×´Ì¬ÏÂ¿ÉÓÃ "kill + ½ø³ÌºÅ" Õý³£ÖÕÖ¹Ð©½ø³Ì
  // µ«Çë²»ÒªÓÃ "kill -9 +½ø³ÌºÅ" Ç¿ÐÐÖÕÖ¹
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // ´ò¿ª³ÌÐòÔËÐÐÈÕÖ¾
  if (logfile.Open("/tmp/htidc/log/checkdirandalarm.log","a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n","/tmp/htidc/log/checkdirandalarm.log"); return -1;
  }

  logfile.SetAlarmOpt("checkdirandalarm");

  // ×¢Òâ£¬³ÌÐò³¬Ê±ÊÇ300Ãë
  ProgramActive.SetProgramInfo(&logfile,"checkdirandalarm",300);

  logfile.Write("¿ªÊ¼·ÖÎö.\n");

  if (File.OpenForRead(argv[1],"r") == FALSE)
  {
    logfile.Write("File.OpenForRead(%s) failed.\n",argv[2]); EXIT(-1);
  }

  char strLocalTime[21];
  char strxmlbuffer[1024];
  UINT ii =0;

  while (TRUE)
  {
    memset(strxmlbuffer,0,sizeof(strxmlbuffer));

    if (File.FFGETS(strxmlbuffer,1000) == FALSE) break;

    if (strstr(strxmlbuffer,"#") != 0) continue;

    if (strstr(strxmlbuffer,"sendport") != 0)
    {
      memset(mobilenostr,0,sizeof(mobilenostr));
      memset(strconnstr,0,sizeof(strconnstr));
      memset(strdwid,0,sizeof(strdwid));
      memset(strywid,0,sizeof(strywid));
      memset(strclientname,0,sizeof(strclientname));
      memset(strsendport,0,sizeof(strsendport));

      GetXMLBuffer(strxmlbuffer,"connstr",strconnstr,100);
      GetXMLBuffer(strxmlbuffer,"dwid",strdwid,2);
      GetXMLBuffer(strxmlbuffer,"ywid",strywid,2);
      GetXMLBuffer(strxmlbuffer,"clientname",strclientname,20);
      GetXMLBuffer(strxmlbuffer,"sendport",strsendport,20);
      GetXMLBuffer(strxmlbuffer,"mobilenostr",mobilenostr,1000);
     
      logfile.Write("connstr=%s\ndwid=%s\nywid=%s\nclientname=%s\nsendport=%s\nmobilenostr=%s\n",strconnstr,strdwid,strywid,strclientname,strsendport,mobilenostr);
    }
    else
    {
      memset(pathname,0,sizeof(pathname));
      memset(andchild,0,sizeof(andchild));
      alarmcount=0;

      GetXMLBuffer(strxmlbuffer,"pathname",pathname,300);
      GetXMLBuffer(strxmlbuffer,"andchild",andchild,5);
      GetXMLBuffer(strxmlbuffer,"alarmcount",&alarmcount);
    }

    if ( (strlen(pathname)==0) || (strlen(andchild)==0) ) continue;

    BOOL bAndChild=FALSE;

    if ( (strcmp(andchild,"true")==0) || (strcmp(andchild,"TRUE")==0) ) bAndChild=TRUE;

    if (Dir.OpenDir(pathname,bAndChild) == FALSE)
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",pathname); continue;
    }

    logfile.Write("pathname=%s,andchild=%s,alarmcount=%ld,fileexist=%lu.\n",pathname,andchild,alarmcount,Dir.m_vFileName.size());

    if (Dir.m_vFileName.size() >= (UINT)alarmcount)
    {
      logfile.Write("Ä¿Â¼%s£¨ÊÇ·ñ°üÀ¨×ÓÄ¿Â¼£º%s£©ÖÐµÄÎÄ¼þÊýÎª%lu£¬³¬¹ýÁË¼à¿ØµÄ·§Öµ%ld¡£\n",pathname,andchild,Dir.m_vFileName.size(),alarmcount);
      
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyymmddhh24miss");
      snprintf(strid,30,"%s%ld",strLocalTime,++ii);

      memset(alarmcontent,0,sizeof(alarmcontent));
      snprintf(alarmcontent,300,"(%s)Ä¿Â¼%sÖÐµÄÎÄ¼þÊýÎª%lu£¬³¬¹ýÁË¼à¿ØµÄ·§Öµ%ld¡£\n",argv[2],pathname,Dir.m_vFileName.size(),alarmcount);
     
      if (PalarmToSMS() == FALSE) logfile.Write("PalarmToSMS() failed.\n");

    }

    // 20190111 ÐÂÔö¼à¿ØÄ¿Â¼ÏÂµÄÎÄ¼þ´óÐ¡£¬³¬¹ý200MB¾Í¸æ¾¯¡£
    while (TRUE)
    {
      if (Dir.ReadDir() == FALSE) break;
   
      if (Dir.m_FileSize >= 200*1024*1024)
      {
        logfile.Write("ÎÄ¼þ£º%s µÄ´óÐ¡Îª%ldMB£¬³¬¹ýÁË¼à¿ØµÄ·§Öµ100MB¡£\n",Dir.m_FullFileName,Dir.m_FileSize/1024/1024);
      
        memset(strLocalTime,0,sizeof(strLocalTime));
        LocalTime(strLocalTime,"yyyymmddhh24miss");
        snprintf(strid,30,"%s%ld",strLocalTime,++ii);

        memset(alarmcontent,0,sizeof(alarmcontent));
        snprintf(alarmcontent,300,"(%s)ÎÄ¼þ£º%s µÄ´óÐ¡Îª%ldMB£¬³¬¹ýÁË¼à¿ØµÄ·§Öµ100MB¡£\n",argv[2],Dir.m_FullFileName,Dir.m_FileSize/1024/1024);
     
        if (PalarmToSMS() == FALSE) logfile.Write("PalarmToSMS() failed.\n");
      }
    }

  }

  logfile.Write("·ÖÎöÄ¿Â¼ÖÐµÄÎÄ¼þÊý¼°ÎÄ¼þ´óÐ¡Íê³É.\n");

  // 20190111 ÐÂÔö¼à¿Ø·þÎñÆ÷´ÅÅÌ¿Õ¼ä£¬³¬¹ý50% ¾Í¸æ¾¯¡£
  FILE *fp=0;

  if ( (fp=popen("df -k --block-size=1M","r")) == NULL )
  {
    logfile.Write("exec /bin/df -k failed.\n"); return FALSE;
  }

  CCmdStr CmdStr;

  char strLine[512],strLine1[512];

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    memset(&m_stDISKSTAT,0,sizeof(m_stDISKSTAT));

    if (FGETS(strLine,500,fp) == FALSE) break;

    // Èç¹ûÃ»ÓÐÕÒµ½¡°%¡±£¬¾ÍÔÙ¶ÁÈ¡Ò»ÐÐ
    if (strstr(strLine,"%") == 0)
    {
      memset(strLine1,0,sizeof(strLine1));
      if (FGETS(strLine1,500,fp) == FALSE)  break;
      strcat(strLine," "); strcat(strLine,strLine1);
    }

    // Èç¹ûÕÒ²»µ½¡°/¡±£¬¾ÍÔ½¹ý¸ÃÐÐ£¬ËüºÜ¿ÉÄÜÊÇµÚÒ»ÐÐµÄ±êÌâ
    if (strstr(strLine,"/dev") == 0) continue;

    // ÔÚÓÐÐ©·þÎñÆ÷ÉÏ,ÄÚ´æÒ²ÊÇ×÷ÎªÎÄ¼þÏµÍ³mountÉÏÈ¥µÄ,ÒÔÏÂÁ½ÐÐ´úÂëÉ¾³ýÄÚ´æÎÄ¼þÏµÍ³
    if (strstr(strLine,"none")     != 0) continue;
    if (strstr(strLine,"/boot")    != 0) continue;
    if (strstr(strLine,"/dev/shm") != 0) continue;

    // ¹ÒÔØÄ¿Â¼Ò²ÅÅ³ý(´øIPµØÖ·µÄ¾ÍÊÇ¹ÒÔØÄ¿Â¼)
    if (MatchFileName(strLine,"*.*.*.*")==TRUE) continue;

    // É¾³ý×Ö·û´®Ç°ºóµÄ¿Õ¸ñ
    Trim(strLine);

    // °Ñ×Ö·û´®ÖÐ¼äµÄ¶à¸ö¿Õ¸ñÈ«²¿×ª»»ÎªÒ»¸ö¿Õ¸ñ
    UpdateStr(strLine,"  "," ");

    CmdStr.SplitToCmd(strLine," ");

    CmdStr.GetValue(0,m_stDISKSTAT.filesystem,100);
    CmdStr.GetValue(1,m_stDISKSTAT.total,30);
    CmdStr.GetValue(2,m_stDISKSTAT.used,30);
    CmdStr.GetValue(3,m_stDISKSTAT.available,30);
    CmdStr.GetValue(4,m_stDISKSTAT.usep,30);
    CmdStr.GetValue(5,m_stDISKSTAT.mount,30);

    snprintf(m_stDISKSTAT.usep,30,"%d",atoi(m_stDISKSTAT.usep));

    if (atoi(m_stDISKSTAT.usep) >= 50)
    {
      logfile.Write("Ä¿Â¼£º%s µÄÊ¹ÓÃÂÊÎª%s%%£¬³¬¹ýÁË¼à¿ØµÄ·§Öµ50%¡£\n",m_stDISKSTAT.mount,m_stDISKSTAT.usep);
      
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyymmddhh24miss");
      snprintf(strid,30,"%s%ld",strLocalTime,++ii);

      memset(alarmcontent,0,sizeof(alarmcontent));
      snprintf(alarmcontent,300,"(%s)Ä¿Â¼£º%s µÄÊ¹ÓÃÂÊÎª%s%%£¬³¬¹ýÁË¼à¿ØµÄ·§Öµ50%%¡£\n",argv[2],m_stDISKSTAT.mount,m_stDISKSTAT.usep);
     
      if (PalarmToSMS() == FALSE) logfile.Write("PalarmToSMS() failed.\n");
    }
  }

  logfile.Write("·ÖÎö·þÎñÆ÷´ÅÅÌ¿Õ¼äÍê³É.\n");

  return 0;
}

void EXIT(int sig)
{
  if (sig > 0) 
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  logfile.Write("checkdirandalarm EXIT.\n");

  exit(0);
}

// °Ñ¸æ¾¯²åÈëµ½¶ÌÐÅÆ½Ì¨
BOOL PalarmToSMS()
{
  // Á¬½Ó¶ÌÐÅÆ½Ì¨
  if (conn.connecttodb(strconnstr,TRUE) != 0)
  {
     logfile.Write("conn.connecttodb(%s) failed.\n",strconnstr); return FALSE;
  }
  
  sqlstatement stmtins;
  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_SMSSENDLIST(id,dwid,ywid,clientname,sendport,mobileno,content,crttime)\
          values(:1,:2,:3,:4,:5,:6,:7,sysdate)");
  stmtins.bindin(1,strid,30);
  stmtins.bindin(2,strdwid,2);
  stmtins.bindin(3,strywid,2);
  stmtins.bindin(4,strclientname,20);
  stmtins.bindin(5,strsendport,20);
  stmtins.bindin(6,mobilenostr,11);
  stmtins.bindin(7,alarmcontent,300);

  if (stmtins.execute() != 0)
  {
    logfile.Write("insert T_SMSSENDLIST failed.%s\n",stmtins.cda.message); return FALSE;
  }

  return TRUE;
}
