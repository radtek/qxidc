#include "_public.h"
#include "ftp.h"

void CallQuit(int sig);

FILE           *timefp,*listfp;
Cftp           ftp;
CLogFile       logfile;
CProgramActive ProgramActive;

char strremoteip[21];        // Ô¶³Ì·şÎñÆ÷µÄIP
UINT uport;                  // Ô¶³Ì·şÎñÆ÷FTPµÄ¶Ë¿Ú
char strmode[11];            // ´«ÊäÄ£Ê½£¬portºÍpasv
char strusername[31];        // Ô¶³Ì·şÎñÆ÷FTPµÄÓÃ»§Ãû
char strpassword[31];        // Ô¶³Ì·şÎñÆ÷FTPµÄÃÜÂë
char strlocalpath[201];      // ±¾µØÎÄ¼ş´æ·ÅµÄÄ¿Â¼
char strlocalpathbak[201];   // ±¾µØÎÄ¼ş·¢ËÍ³É¹¦ºó£¬´æ·ÅµÄ±¸·İÄ¿Â¼£¬Èç¹û¸ÃÄ¿Â¼Îª¿Õ£¬ÎÄ¼ş·¢ËÍ³É¹¦ºóÖ±½ÓÉ¾³ı
char strlistfilename[301];   // ´æ·ÅÒÑ´«Êä¯ÎÄ¼şÁĞ±íµÄxmlÎÄ¼ş
char strtimefilename[301];   // Ê±¼ä±êÇ©ÎÄ¼ş
char strremotepath[201];     // Ô¶³Ì·şÎñÆ÷´æ·ÅÎÄ¼şµÄ×îÖÕÄ¿Â¼
char strmatchname[4096];     // ´ı·¢ËÍÎÄ¼şÆ¥ÅäµÄÎÄ¼şÃû
UINT utimeout;               // FTP·¢ËÍÎÄ¼şµÄ³¬Ê±Ê±¼ä
int  itimetvl;
char strtrlog[11];           // ÊÇ·ñÇĞ»»ÈÕÖ¾
char strdeleteremote[21];
char strFullFileName[501];   // ÎÄ¼şÈ«Ãû£¬°üÀ¨Â·¾¶

 
// ½¨Á¢Óë¶Ô·½·şÎñÆ÷µÄÁ¬½Ó£¬²¢µÇÂ¼
BOOL ftplogin();

// °ÑÎÄ¼ş·¢ËÍµ½¶Ô·½·şÎñÆ÷Ä¿Â¼
BOOL ftpputforponds();

// °ÑÒÑ¾­ÍÆËÍ¹ıµÄÎÄ¼şĞÅÏ¢Ğ´ÈëListÎÄ¼ş
BOOL  WriteToXML();

// ÎÄ¼şÁĞ±íÊı¾İ½á¹¹
struct st_filelist
{
  char localfilename[301];
  char modtime[21];
  int  filesize;
  int  ftpsts;   // 1-Î´ÍÆËÍ,2-ÒÑÍÆËÍ¡£
};

// ÎÄ¼şĞÅÏ¢µÄÁÙÊ±Êı¾İ½á¹¹£¬ÔÚÈÎºÎº¯ÊıÖĞ¶¼¿ÉÒÔÖ±½ÓÓÃ
struct st_filelist stfilelist;

// ±¾´Î·şÎñÆ÷ÎÄ¼şÇåµ¥µÄÈİÆ÷
vector<struct st_filelist> v_newfilelist;

// ÉÏ´Î·şÎñÆ÷ÎÄ¼şÇåµ¥µÄÈİÆ÷
vector<struct st_filelist> v_oldfilelist;

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpputforponds logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/ftpputforponds /tmp/htidc/log/ftpputforponds_192.168.201.14_oracle_gzradpic.log \"<remoteip>192.168.201.14</remoteip><port>2138</port><mode>pasv</mode><username>data</username><password>Data#1620</password><localpath>/mnt/data9733/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</localpath><localpathbak>/mnt/data9733/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</localpathbak><remotepath>/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</remotepath><timefilename>/mnt/data9733/OpticalFlowDataUrs/NewTREC/trecPonds.over</timefilename><matchname>TREC_Wind_{YYYY}Äê{MM}ÔÂ{DD}ÈÕ{HH}Ê±{MI}·Ö00Ãë_.png,Meta_TREC_GD_{YYYY}Äê{MM}ÔÂ{DD}ÈÕ{HH}Ê±{MI}·Ö00Ãë_138·ÖÖÓ.png,Meta_TREC_GD_{YYYY}Äê{MM}ÔÂ{DD}ÈÕ{HH}Ê±{MI}·Ö00Ãë_144·ÖÖÓ.png</matchname><listfilename>/tmp/htidc/list/ftpputforponds_192.168.201.14_gzradpic.xml</listfilename><timeout>180</timeout><trlog>TRUE</trlog><deleteremote>TRUE</deleteremote>\"\n\n");

    printf("±¾³ÌĞòÊÇ×¨ÃÅÓÃÓÚpondsÊı¾İÉÏ´«£¬¿ÉÔöÁ¿°Ñponds·Ç½á¹¹»¯ÎÄ¼ş·¢ËÍµ½Ô¶³ÌµÄFTP·şÎñÆ÷¡£\n");
    printf("logfilenameÊÇ±¾³ÌĞòÔËĞĞµÄÈÕÖ¾ÎÄ¼ş£¬Ò»°ã·ÅÔÚ/tmp/htidc/logÄ¿Â¼ÏÂ£¬"\
           "²ÉÓÃftpputforponds_Ô¶³ÌµØÖ·_ftpÓÃ»§Ãû.logÃüÃû¡£\n");
    printf("xmlbufferÎªÎÄ¼ş´«ÊäµÄ²ÎÊı£¬ÈçÏÂ£º\n");
    printf("<remoteip>192.168.201.14</remoteip> Ô¶³Ì·şÎñÆ÷µÄIP¡£\n");
    printf("<port>21</port> Ô¶³Ì·şÎñÆ÷FTPµÄ¶Ë¿Ú¡£\n");
    printf("<mode>pasv</mode> ´«ÊäÄ£Ê½£¬pasvºÍport£¬¿ÉÑ¡×Ö¶Î£¬È±Ê¡²ÉÓÃpasvÄ£Ê½£¬portÄ£Ê½Ò»°ã²»ÓÃ¡£\n");
    printf("<username>oracle</username> Ô¶³Ì·şÎñÆ÷FTPµÄÓÃ»§Ãû¡£\n");
    printf("<password>oracle1234HOTA</password> Ô¶³Ì·şÎñÆ÷FTPµÄÃÜÂë¡£\n");
    printf("<localpath>/tmp/htidc/ftpget/gzrad</localpath> ±¾µØÎÄ¼ş´æ·ÅµÄÄ¿Â¼¡£\n");
    printf("<localpathbak>/tmp/htidc/ftpget/gzradbak</localpathbak> ±¾µØÎÄ¼ş·¢ËÍ³É¹¦ºó£¬´æ·ÅµÄ±¸·İÄ¿Â¼£¬"\
           "Èç¹û¸ÃÄ¿Â¼Îª¿Õ£¬ÎÄ¼ş·¢ËÍ³É¹¦ºóÖ±½ÓÉ¾³ı£¬Èç¹ûlocalpathµÈÓÚlocalpathbak£¬"\
           "ÎÄ¼ş·¢ËÍ³É¹¦ºó½«±£ÁôÔ´ÎÄ¼ş²»±ä¡£Õı³£Çé¿öÏÂ£¬±¾µØÎÄ¼ş´«ÊäºóÓ¦¸ÃÉ¾³ı»ò±¸·İ£¬"\
           "²»É¾³ı²»±¸·İÖ»ÊÊÓÃÓÚ´«ÊäÈÕÖ¾ÎÄ¼şµÄÇé¿ö¡£\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> Ô¶³Ì·şÎñÆ÷´æ·ÅÎÄ¼şµÄÄ¿Â¼¡£\n");
    printf("timefilename>/mnt/data9733/DualPolarimetricRadar/Radar/time/dbz_time.txt</timefilename> Ê±¼ä±êÇ©ÎÄ¼ş£¬"\
           "Ã¿ÖÖÊı¾İ¶¼ÓĞÒ»¸öÊ±¼ä±êÇ©ÎÄ¼ş×ö¼ÇÂ¼£¬ÏÈ¶ÁÈ¡Ê±¼ä±êÇ©ÄÚÈİ£¬¾Í¿ÉÒÔÖªµÀ×îĞÂµÄÎÄ¼şÊÇÔÚÄÄ¸öÄ¿Â¼¡¢Ê²Ã´Ê±ºòµÄ¡£\n");
    printf("<matchname>WX_AWS_dbz_ShenZhen_{yyyy}{mm}{dd}{hh24}{mi}00.png,WX_Mate_dbz_GD_{yyyy}{mm}{dd}{hh24}{mi}00.png</matchname> ´ı·¢ËÍÎÄ¼şÆ¥ÅäµÄÎÄ¼şÃû£¬Ö±½Ó¾«×¼Æ¥Åä£¬Çø·Ö´óĞ¡Ğ´£¬"\
           "Ä¿Ç°¿ÉÒÔ´¦ÀíÒÔÏÂÊ±¼ä±äÁ¿£º{YYYY}£¨4Î»µÄÄê£©¡¢{YYY}£¨ºóÈıÎ»µÄÄê£©¡¢"\
           "{YY}£¨ºóÁ½Î»µÄÄê£©¡¢{MM}£¨ÔÂÔÂ£©¡¢{DD}£¨ÈÕÈÕ£©¡¢{HH}£¨Ê±Ê±£©¡¢{MI}£¨·Ö·Ö£©¡¢{SS}£¨ÃëÃë£©¡£\n");
    printf("<listfilename>/tmp/htidc/list/ftpputforponds_192.168.201.14_gzradpic.xml</listfilename> ´æ"\
           "·ÅÒÑÍÆËÍÎÄ¼şÁĞ±íµÄxmlÎÄ¼ş£¬Ò»°ã·ÅÔÚ/tmp/htidc/listÄ¿Â¼ÏÂ£¬"\
           "²ÉÓÃftpgetfiles_Ô¶³ÌµØÖ·_ftpÓÃ»§Ãû_ÎÄ¼şÀàĞÍ.xmlµÄÃüÃû·½Ê½¡£\n");
    printf("<timeout>1800</timeout> FTP·¢ËÍÎÄ¼şµÄ³¬Ê±Ê±¼ä£¬µ¥Î»£ºÃë£¬×¢Òâ£¬±ØĞëÈ·±£´«ÊäÄ³"\
           "¸öÎÄ¼şµÄÊ±¼äĞ¡ÓÚtimeoutµÄÈ¡Öµ£¬·ñÔò»áÔì³É´«ÊäÊ§°ÜµÄÇé¿ö¡£\n");
    printf("<trlog>TRUE</trlog> µ±ÈÕÖ¾ÎÄ¼ş´óÓÚ100MÊ±£¬ÊÇ·ñÇĞ»»ÈÕÖ¾ÎÄ¼ş£¬TRUE-ÇĞ»»£»FALSE-²»ÇĞ»»¡£\n");
    printf("<deleteremote>TRUE</deleteremote> Èç¹ûÔ¶³ÌÄ¿Â¼´æÔÚ¸ÃÎÄ¼ş£¬ÊÇ·ñÉ¾³ıÔ¶³ÌÄ¿Â¼µÄÁÙÊ±ÎÄ¼şºÍÕıÊ½ÎÄ¼ş\n");
    printf("<timetvl>-8</timetvl> ÎªÊ±¼ä±äÁ¿µÄÆ«ÒÆÊ±¼ä£¬ÔİÊ±²»ÆôÓÃ¡£\n");
    printf("ÒÔÉÏµÄ²ÎÊıÖ»ÓĞmode¡¢timetvl¡¢localpathbak¡¢trlogÎª¿ÉÑ¡²ÎÊı£¬ÆäËüµÄ¶¼±ØÌî¡£\n\n");

    printf("port Ä£Ê½ÊÇ½¨Á¢´Ó·şÎñÆ÷¸ß¶Ë¿ÚÁ¬µ½¿Í»§¶Ë20¶Ë¿ÚÊı¾İÁ¬½Ó¡£\n");
    printf("pasv Ä£Ê½ÊÇ½¨Á¢¿Í»§¸ß¶Ë¿ÚÁ¬µ½·şÎñÆ÷·µ»ØµÄÊı¾İ¶Ë¿ÚµÄÊı¾İÁ¬½Ó¡£\n\n");

    printf("port£¨Ö÷¶¯£©·½Ê½µÄÁ¬½Ó¹ı³ÌÊÇ£º¿Í»§¶ËÏò·şÎñÆ÷µÄFTP¶Ë¿Ú£¨Ä¬ÈÏÊÇ21£©·¢ËÍÁ¬½ÓÇëÇó£¬"\
           "·şÎñÆ÷½ÓÊÜÁ¬½Ó£¬½¨Á¢Ò»ÌõÃüÁîÁ´Â·¡£\n");
    printf("µ±ĞèÒª´«ËÍÊı¾İÊ±£¬·şÎñÆ÷´Ó20¶Ë¿ÚÏò¿Í»§¶ËµÄ¿ÕÏĞ¶Ë¿Ú·¢ËÍÁ¬½ÓÇëÇó£¬½¨Á¢Ò»ÌõÊı¾İÁ´Â·À´´«ËÍÊı¾İ¡£\n\n");

    printf("pasv£¨±»¶¯£©·½Ê½µÄÁ¬½Ó¹ı³ÌÊÇ£º¿Í»§¶ËÏò·şÎñÆ÷µÄFTP¶Ë¿Ú£¨Ä¬ÈÏÊÇ21£©·¢ËÍÁ¬½ÓÇëÇó£¬"\
           "·şÎñÆ÷½ÓÊÜÁ¬½Ó£¬½¨Á¢Ò»ÌõÃüÁîÁ´Â·¡£\n");
    printf("µ±ĞèÒª´«ËÍÊı¾İÊ±£¬¿Í»§¶ËÏò·şÎñÆ÷µÄ¿ÕÏĞ¶Ë¿Ú·¢ËÍÁ¬½ÓÇëÇó£¬½¨Á¢Ò»ÌõÊı¾İÁ´Â·À´´«ËÍÊı¾İ¡£\n\n");

    return -1;
  }

  // ¹Ø±ÕÈ«²¿µÄĞÅºÅºÍÊäÈëÊä³ö
  // ÉèÖÃĞÅºÅ,ÔÚshell×´Ì¬ÏÂ¿ÉÓÃ "kill + ½ø³ÌºÅ" Õı³£ÖÕÖ¹Ğ©½ø³Ì
  // µ«Çë²»ÒªÓÃ "kill -9 +½ø³ÌºÅ" Ç¿ĞĞÖÕÖ¹
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001]; 

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // ÅĞ¶ÏÊÇ·ñÇĞ»»ÈÕÖ¾
  BOOL btrlog=TRUE;

  if (strcmp(strtrlog,"FALSE")==0) btrlog=FALSE;

  // ´ò¿ªÈÕÖ¾
  if (logfile.Open(argv[1],"a+",btrlog) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //´ò¿ª¸æ¾¯
  logfile.SetAlarmOpt("ftpputforponds");

  memset(strremoteip,0,sizeof(strremoteip));          // Ô¶³Ì·şÎñÆ÷µÄIP
  memset(strmode,0,sizeof(strmode));                  // ´«ÊäÄ£Ê½£¬portºÍpasv
  memset(strusername,0,sizeof(strusername));          // Ô¶³Ì·şÎñÆ÷FTPµÄÓÃ»§Ãû
  memset(strpassword,0,sizeof(strpassword));          // Ô¶³Ì·şÎñÆ÷FTPµÄÃÜÂë
  memset(strlocalpath,0,sizeof(strlocalpath));        // ±¾µØÎÄ¼ş´æ·ÅµÄÄ¿Â¼
  memset(strlocalpathbak,0,sizeof(strlocalpathbak));  // ¬Èç¹û¸ÃÄ¿Â¼Îª¿Õ£¬ÎÄ¼ş·¢ËÍ³É¹¦ºóÖ±½ÓÉ¾³ı
  memset(strtimefilename,0,sizeof(strtimefilename));  // Ê±¼ä±êÇ©ÎÄ¼ş
  memset(strremotepath,0,sizeof(strremotepath));      // Ô¶³Ì·şÎñÆ÷´æ·ÅÎÄ¼şµÄ×îÖÕÄ¿Â¼
  memset(strmatchname,0,sizeof(strmatchname));        // ´ı·¢ËÍÎÄ¼şÆ¥ÅäµÄÎÄ¼şÃû
  memset(strtrlog,0,sizeof(strtrlog));        
  memset(strdeleteremote,0,sizeof(strdeleteremote));
  memset(strlistfilename,0,sizeof(strlistfilename));
  uport=21;                                           // Ô¶³Ì·şÎñÆ÷FTPµÄ¶Ë¿Ú
  utimeout=0;                                         // FTP·¢ËÍÎÄ¼şµÄ³¬Ê±Ê±¼ä
  itimetvl=0;

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);
  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);
  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);
  GetXMLBuffer(strXmlBuffer,"trlog",strtrlog,8);
  GetXMLBuffer(strXmlBuffer,"deleteremote",strdeleteremote,8);
  GetXMLBuffer(strXmlBuffer,"listfilename",strlistfilename,300);
  GetXMLBuffer(strXmlBuffer,"timefilename",strtimefilename,300);

  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);

  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strpassword) == 0) { logfile.Write("password is null.\n"); return -1; }
  if (uport == 0)               { logfile.Write("port is null.\n"); return -1;     }
  if (utimeout == 0)            { logfile.Write("timeout is null.\n"); return -1;  }

  // ×¢Òâ£¬³ÌĞò³¬Ê±ÊÇutimeoutÃë
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",utimeout);

  ProgramActive.WriteToFile();

  char strLine[501];

  // ¶ÁÈ¡Ê±¼ä±êÇ©ÎÄ¼şÄÚÈİ,²¢´¦ÀíxmlbufferÖĞµÄÊ±¼ä±äÁ¿¡£
  if ( (timefp=FOPEN(strtimefilename,"r")) != NULL)
  {
    memset(strLine,0,sizeof(strLine));

    // ´ÓÎÄ¼şÖĞ»ñÈ¡Ò»ĞĞ
    FGETS(strLine,100,timefp);
    fclose(timefp);

    // ´Õ¹»14Î»µÄÄêÔÂÈÕÊ±·ÖÃë
    strncat(strLine,"000000",14-strlen(strLine));

    char YYYY[5],YYY[4],YY[3],MM[3],DD[3],HH[3],MI[3],SS[3];

    memset(YYYY,0,sizeof(YYYY));
    memset(YYY,0,sizeof(YYY));
    memset(YY,0,sizeof(YY));
    memset(MM,0,sizeof(MM));
    memset(DD,0,sizeof(DD));
    memset(HH,0,sizeof(HH));
    memset(MI,0,sizeof(MI));
    memset(SS,0,sizeof(SS));

    strncpy(YYYY,strLine,4);
    strncpy(YYY,strLine+1,3);
    strncpy(YY,strLine+2,2);
    strncpy(MM,strLine+4,2);
    strncpy(DD,strLine+6,2);
    strncpy(HH,strLine+8,2);
    strncpy(MI,strLine+10,2);
    strncpy(SS,strLine+12,2);

    UpdateStr(strXmlBuffer,"{YYYY}",YYYY);
    UpdateStr(strXmlBuffer,"{YYY}",YYY);
    UpdateStr(strXmlBuffer,"{YY}",YY);
    UpdateStr(strXmlBuffer,"{MM}",MM);
    UpdateStr(strXmlBuffer,"{DD}",DD);
    UpdateStr(strXmlBuffer,"{HH}",HH);
    UpdateStr(strXmlBuffer,"{MI}",MI);
    UpdateStr(strXmlBuffer,"{SS}",SS);

    UpdateStr(strXmlBuffer,"{yyyy}",YYYY);
    UpdateStr(strXmlBuffer,"{yyy}",YYY);
    UpdateStr(strXmlBuffer,"{yy}",YY);
    UpdateStr(strXmlBuffer,"{mm}",MM);
    UpdateStr(strXmlBuffer,"{dd}",DD);
    UpdateStr(strXmlBuffer,"{hh}",HH);
    UpdateStr(strXmlBuffer,"{mi}",MI);
    UpdateStr(strXmlBuffer,"{ss}",SS);
  }

  // ±ØĞëÔÚ´¦ÀíxmlbufferÖĞµÄÊ±¼ä±äÁ¿Ö®ºó²ÅÈ¡Öµ
  GetXMLBuffer(strXmlBuffer,"localpath",strlocalpath,200);
  GetXMLBuffer(strXmlBuffer,"localpathbak",strlocalpathbak,200);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,200);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,4000);

  if (strlen(strlocalpath) == 0)  { logfile.Write("localpath is null.\n"); return -1; }
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1;}
  if (strlen(strmatchname) == 0)  { logfile.Write("matchname is null.\n"); return -1; }

  v_oldfilelist.clear();

  // ÉÏ´Î·şÎñÆ÷ÎÄ¼şÇåµ¥¼ÓÔØµ½v_oldfilelist
  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // ´ÓÎÄ¼şÖĞ»ñÈ¡Ò»ĞĞ
      if (FGETS(strLine,500,listfp,"<endl/>") == FALSE) break;

      memset(&stfilelist,0,sizeof(stfilelist));

      GetXMLBuffer(strLine,"filename", stfilelist.localfilename,300);
      GetXMLBuffer(strLine,"modtime",  stfilelist.modtime,14);
      GetXMLBuffer(strLine,"filesize",&stfilelist.filesize);

      v_oldfilelist.push_back(stfilelist);
    }

    fclose(listfp);
  }

  v_newfilelist.clear();

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strmatchname,",");

  for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
  { 
    memset(strFullFileName,0,sizeof(strFullFileName));
  
    snprintf(strFullFileName,500,"%s/%s",strlocalpath,CmdStr.m_vCmdStr[ii].c_str());

    // ¼ì²âÊÇ·ñ´æÔÚÕâ¸öÎÄ¼ş£¬Èç¹ûÃ»ÓĞ£¬Ôò·µ»Ø¡£
    if (access(strFullFileName,F_OK) != 0) continue;

    memset(&stfilelist,0,sizeof(stfilelist));

    struct stat st_filestat;      // ÎÄ¼şĞÅÏ¢½á¹¹Ìå
    stat(strFullFileName,&st_filestat); // »ñÈ¡ÎÄ¼şĞÅÏ¢

    stfilelist.filesize = st_filestat.st_size; // ÎÄ¼ş´óĞ¡

    struct tm nowtimer;

    // ÎÄ¼ş×îºóÒ»´Î±»ĞŞ¸ÄµÄÊ±¼ä
    nowtimer = *localtime(&st_filestat.st_mtime); nowtimer.tm_mon++;
    snprintf(stfilelist.modtime,20,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    strncpy(stfilelist.localfilename,CmdStr.m_vCmdStr[ii].c_str(),300); // ÎÄ¼şÃû
    stfilelist.ftpsts = 1; // Ä¬ÈÏÉÏ´«£¬ÉÏ´«-1£¬²»ÉÏ´«-2£¡

    v_newfilelist.push_back(stfilelist);

  }

  // °Ñ±¾´ÎÎÄ¼şºÍÉÏ´ÎÎÄ¼ş×ö¶Ô±È£¬µÃ³öĞèÒªÉÏ´«µÄÎÄ¼ş¡£
  for (UINT ii=0;ii<v_newfilelist.size();ii++)
  {
    if (v_newfilelist.size() == 0) continue;

    // °ÑÔ¶³ÌÄ¿Â¼µÄÎÄ¼şÃû¡¢´óĞ¡ºÍÈÕÆÚºÍ±¾µØµÄÇåµ¥ÎÄ¼ş±È½ÏÒ»ÏÂ£¬Èç¹ûÈ«²¿ÏàÍ¬£¬¾Í°ÑftpstsÉèÖÃÎª2£¬²»ÔÙ²É¼¯
    for (UINT jj=0;jj<v_oldfilelist.size();jj++)
    {
      if (strcmp(v_newfilelist[ii].localfilename,v_oldfilelist[jj].localfilename) == 0)
      {
        if ( (strcmp(v_newfilelist[ii].modtime,v_oldfilelist[jj].modtime) == 0) &&
             (v_newfilelist[ii].filesize==v_oldfilelist[jj].filesize) )
        {
          v_newfilelist[ii].ftpsts=2;
        }

        break;
      }
    }
  }

  // ÉÏ´«ÎÄ¼ş
  ftpputforponds();

  // °ÑÒÑ¾­ÍÆËÍ¹ıµÄÎÄ¼şĞÅÏ¢Ğ´ÈëListÎÄ¼ş
  WriteToXML();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  // °ÑÒÑ¾­ÍÆËÍ¹ıµÄÎÄ¼şĞÅÏ¢Ğ´ÈëListÎÄ¼ş
  WriteToXML();

  logfile.Write("ftpputforponds exit.\n");

  exit(0);
}


// °ÑÒÑ¾­ÍÆËÍ¹ıµÄÎÄ¼şĞÅÏ¢Ğ´ÈëListÎÄ¼ş
BOOL WriteToXML()
{
  if ( (listfp=FOPEN(strlistfilename,"w")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strlistfilename); return FALSE;
  }

  fprintf(listfp,"<data>\n");

  for (UINT nn=0;nn<v_newfilelist.size();nn++)
  {
    if (v_newfilelist[nn].ftpsts==2)
    {
      fprintf(listfp,"<filename>%s</filename><modtime>%s</modtime><filesize>%d</filesize><endl/>\n",v_newfilelist[nn].localfilename,v_newfilelist[nn].modtime,v_newfilelist[nn].filesize);
    }
  }

  fprintf(listfp,"</data>\n");
  
  return TRUE;
}

// ½¨Á¢Óë¶Ô·½·şÎñÆ÷µÄÁ¬½Ó£¬²¢µÇÂ¼
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // ÉèÖÃ´«ÊäÄ£Ê½
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTPÁ¬½ÓºÍµÇÂ¼µÄ³¬Ê±Ê±¼äÉèÎª80¾Í¹»ÁË¡£
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) failed.\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // ³ÌĞòµÄ³¬Ê±Ê±¼äÔÙÉèÎªutimeoutÃë¡£
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",utimeout);

  // ÔÚ¶Ô·½·şÎñÆ÷ÉÏ´´½¨Ä¿Â¼
  ftp.mkdir(strremotepath);

  return TRUE;
}

// °ÑÎÄ¼ş·¢ËÍµ½¶Ô·½·şÎñÆ÷Ä¿Â¼
BOOL ftpputforponds()
{
  char strFullLocalFileName[301];
  char strFullLocalFileNameBak[301];
  char strFullRemoteFileName[301];
  char strFullRemoteFileNameTmp[301];
  BOOL bConnected=FALSE;

  for (UINT kk=0;kk<v_newfilelist.size();kk++)
  {
    // °ÑÎÄ¼ş²É¼¯µ½±¾µØ·şÎñÆ÷Ä¿Â¼
    if (v_newfilelist[kk].ftpsts==1)
    {
      memcpy(&stfilelist,&v_newfilelist[kk],sizeof(stfilelist));

      // Èç¹ûÃ»ÓĞÁ¬ÉÏ¶Ô·½µÄFTP·şÎñÆ÷£¬¾ÍÁ¬½Ó
      if (bConnected==FALSE)
      {
        // Á¬½Ó·şÎñÆ÷
        if (ftplogin() == FALSE) return FALSE;

        ProgramActive.WriteToFile();

        bConnected=TRUE;
      }

      // Èç¹ûÎÄ¼ş´óĞ¡Îª0£¬ÔòÌø¹ı¡£
      if (stfilelist.filesize == 0) continue;

      memset(strFullLocalFileName,0,sizeof(strFullLocalFileName));
      memset(strFullLocalFileNameBak,0,sizeof(strFullLocalFileNameBak));
      memset(strFullRemoteFileName,0,sizeof(strFullRemoteFileName));
      memset(strFullRemoteFileNameTmp,0,sizeof(strFullRemoteFileNameTmp));

      snprintf(strFullLocalFileName,300,"%s/%s",strlocalpath,stfilelist.localfilename);
      snprintf(strFullLocalFileNameBak,300,"%s/%s",strlocalpathbak,stfilelist.localfilename);
      snprintf(strFullRemoteFileName,300,"%s/%s",strremotepath,stfilelist.localfilename);
      snprintf(strFullRemoteFileNameTmp,300,"%s/%s.tmp",strremotepath,stfilelist.localfilename);

      // É¾³ıÔ¶³ÌÄ¿Â¼µÄÁÙÊ±ÎÄ¼şºÍÕıÊ½ÎÄ¼ş£¬ÒòÎªÈç¹û¶Ô·½Ä¿Â¼ÒÑ´æÔÚ¸ÃÎÄ¼ş£¬¿ÉÄÜ»áµ¼ÖÂÎÄ¼ş´«Êä²»³É¹¦ÄÜÇé¿ö
      // µ«ÊÇ£¬ĞÂµÄÀà¿â²»ÖªµÀ»¹»á²»»áÕâÑù
      if (strcmp(strdeleteremote,"TRUE")==0) { ftp.ftpdelete(strFullRemoteFileNameTmp); ftp.ftpdelete(strFullRemoteFileName); }

      CTimer Timer;

      Timer.Beginning();

      logfile.Write("put %s(size=%ld,mtime=%s)...",strFullLocalFileName,stfilelist.filesize,stfilelist.modtime);

      BOOL bIsLogFile=MatchFileName(stfilelist.localfilename,"*.LOG");

      // ·¢ËÍÎÄ¼ş
      if (ftp.put(strFullLocalFileName,strFullRemoteFileName,bIsLogFile) == FALSE)
      {
        logfile.WriteEx("failed.\n%s",ftp.response()); continue;
      }
      else
      {
        // ±ê¼ÇÎªÒÑÉÏ´«
        v_newfilelist[kk].ftpsts=2;

        logfile.WriteEx("ok(time=%d).\n",Timer.Elapsed());
      }

      if (strlen(strlocalpathbak) == 0)
      {
        // É¾³ı±¾µØÄ¿Â¼µÄ¸ÃÎÄ¼ş
        if (REMOVE(strFullLocalFileName) == FALSE) 
        { 
          logfile.WriteEx("\nREMOVE(%s) failed.\n",strFullLocalFileName); continue; 
        }
      }
      else
      {
        // °Ñ±¾µØÄ¿Â¼µÄ¸ÃÎÄ¼şÒÆµ½±¸·İµÄÄ¿Â¼È¥£¬Èç¹ûlocalpathµÈÓÚlocalpathbak£¬±£ÁôÔ´ÎÄ¼ş²»±ä
        if (strcmp(strlocalpath,strlocalpathbak) != 0)
        {
          if (RENAME(strFullLocalFileName,strFullLocalFileNameBak) == FALSE) 
          { 
            logfile.WriteEx("\nRENAME(%s,%s) failed.\n",strFullLocalFileName,strFullLocalFileNameBak); 

            continue; 
          }
        }
      }
    }
  }

  ftp.logout();

  return TRUE;
}
