#include "idcapp.h"

CLogFile logfile;
CProgramActive ProgramActive;
FILE *fp;
connection conn;

char connstr[101];
char tablename[51];
char outpathtmp[301];
char outpath[301];
char fieldval[100][2001];
char strallcolumn[2048];
char deleteflag[11];
char strtmpfilename[301],strstdfilename[301];
char nowtime[21];
char rowid[31];
int  sleeptime = 0;
UINT fieldcount=0;
UINT ufileseq=0;

void CallQuit(int sig);

int main(int argc, char *argv[])
{
  if ( (argc != 7) && (argc != 8) )
  {
    printf("\nUsage:./gettabdata username/password@tnsname tablename outpathtmp outpath sleeptime deleteflag [where]\n\n");

    printf("Example:/htidc/htidc/bin/procctl 80 /htidc/htidc/bin/gettabdata esa/esaserver CLIENT_INFO   /home/temp /home/jwtin/send  0 false\n");
    printf("        /htidc/htidc/bin/procctl 30 /htidc/htidc/bin/gettabdata esa/esaserver SMS_SEND_TEMP /home/temp /home/jwtin/send  2 false\n\n");

    printf("        /htidc/htidc/bin/procctl 20 /htidc/htidc/bin/gettabdata esa/esaserver SMS_RESP      /home/temp /home/jwtout/recv 2 true\n");
    printf("        /htidc/htidc/bin/procctl 20 /htidc/htidc/bin/gettabdata esa/esaserver SMS_REPORT    /home/temp /home/jwtout/recv 2 true\n");
    printf("        /htidc/htidc/bin/procctl 20 /htidc/htidc/bin/gettabdata esa/esaserver SMS_RECV      /home/temp /home/jwtout/recv 2 true\n\n");

    printf("本程序用于导出数据库里指定表的数据，生成xml文件,放到outpath目录中。\n");
    printf("程序运行的日志文件名为/tmp/htidc/log/gettabdata_表名.log。\n");
    printf("username/password@tnsname 为数据库的连接参数。\n");
    printf("tablename 为要处理的数据库表名。\n");
    printf("outpathtmp 为输出文件的临时目录。\n");
    printf("outpath 为输出文件的正式目录。\n");
    printf("sleeptime 表示每次执行数据导出的时间间隔，如果时间间隔为0，表示该程序执行一次导出后立即退出。\n");
    printf("deleteflag 表示导出数据成功后，是否要删除已被成功导出后的原始记录。\n");
    printf("[where] 是一个可选项，表示导出数据的条件，注意一定要用双引号把它括起来。\n\n\n");

    return 0;
  }

  sqlstatement stmtsel,stmtdel;

  memset(connstr,0,sizeof(connstr));
  memset(tablename,0,sizeof(tablename));
  memset(strallcolumn,0,sizeof(strallcolumn));
  memset(outpath,0,sizeof(outpath));
  memset(deleteflag,0,sizeof(deleteflag));
  memset(strtmpfilename,0,sizeof(strtmpfilename));
  memset(strstdfilename,0,sizeof(strstdfilename));
  memset(nowtime,0,sizeof(nowtime));
  memset(fieldval,0,sizeof(fieldval));

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止此进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal();
  signal(SIGTERM,CallQuit);   // 按ctl+c
  signal(SIGINT,CallQuit);    // kill 或 killall

  strncpy(connstr,argv[1],sizeof(connstr));
  strncpy(tablename, argv[2], 50); 
  strncpy(outpathtmp, argv[3], 300); 
  strncpy(outpath, argv[4], 300); 
  sleeptime = atoi(argv[5]);
  strncpy(deleteflag, argv[6],10); 
  ToUpper(deleteflag);

  char strLogFileName[301];
  memset(strLogFileName,0,sizeof(strLogFileName));
  snprintf(strLogFileName,300,"/tmp/htidc/log/gettabdata_%s.log",tablename);
  if (logfile.Open(strLogFileName,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strLogFileName); return -1;
  } 

  //打开告警
  logfile.SetAlarmOpt("gettabdata");

  // 注意，程序超时是80秒
  ProgramActive.SetProgramInfo(&logfile,"gettabdata",80);

  if (conn.connecttodb(connstr) != 0)
  {
    logfile.Write("connect to %s failed.\n",connstr); CallQuit(-1);
  }

  // 获取表的全部的列信息
  CTABFIELD TABFIELD;
  TABFIELD.GetALLField(&conn,tablename);
  if (TABFIELD.m_vALLFIELD.size() == 0)
  {
    logfile.Write("table %s is not exist.\n",tablename); CallQuit(-1);
  }

  fieldcount=0;

  strcpy(strallcolumn,"rowid");

  for (UINT ii=0;ii<TABFIELD.m_vALLFIELD.size();ii++)
  {
    if (strcmp(TABFIELD.m_vALLFIELD[ii].datatype, "char"    ) == 0 ||
        strcmp(TABFIELD.m_vALLFIELD[ii].datatype, "varchar" ) == 0 ||
        strcmp(TABFIELD.m_vALLFIELD[ii].datatype, "number"  ) == 0 ||
        strcmp(TABFIELD.m_vALLFIELD[ii].datatype, "rowid"   ) == 0 ||
        strcmp(TABFIELD.m_vALLFIELD[ii].datatype, "varchar2") == 0)
    {
      strcat(strallcolumn, ",");
      strcat(strallcolumn, TABFIELD.m_vALLFIELD[ii].fieldname);
      fieldcount++;
    }

    if (strcmp(TABFIELD.m_vALLFIELD[ii].datatype, "date") == 0)
    {
      char strtmp[101];
      memset(strtmp,0,sizeof(strtmp));
      snprintf(strtmp,100,",to_char(%s,'yyyy-mm-dd hh24:mi:ss')",TABFIELD.m_vALLFIELD[ii].fieldname);
      strcat(strallcolumn,strtmp);
      fieldcount++;
    }
  }

  stmtsel.connect(&conn);
  if (argc==7)
  {
    stmtsel.prepare("select %s from %s",strallcolumn,tablename);
  }
  else
  {
    stmtsel.prepare("select %s from %s %s",strallcolumn,tablename,argv[7]);
  }
  stmtsel.bindout(1,rowid,30);
  for (UINT ii = 0; ii < fieldcount; ii++)
  {
    stmtsel.bindout(ii+2, fieldval[ii], 2000);
  }

  stmtdel.connect(&conn);
  stmtdel.prepare("delete from %s where rowid = :1",tablename);
  stmtdel.bindin(1, rowid,30);

  while (TRUE)
  {
    ProgramActive.WriteToFile();

    if (stmtsel.execute() != 0)
    {
      logfile.Write("execute sql failed.\n%s\n%s\n",stmtsel.m_sql,stmtsel.cda.message); CallQuit(-1);
    }
   
    while (TRUE)
    {
      ProgramActive.WriteToFile();

      memset(rowid,0,sizeof(rowid));
      memset(fieldval,0,sizeof(fieldval));
  
      if (stmtsel.next() != 0) break;

      if (fp == 0)
      {
        LocalTime(nowtime,"yyyymmddhh24miss");      

        snprintf(strtmpfilename, 300, "%s/%s_%s_%lu.xml.tmp",outpathtmp,tablename,nowtime,ufileseq);
        snprintf(strstdfilename, 300, "%s/%s_%s_%lu.xml"    ,outpath   ,tablename,nowtime,ufileseq++);

        if ((fp=FOPEN(strtmpfilename,"w+")) == 0)
        {
          logfile.Write("FOPEN %s failed.\n", strtmpfilename); CallQuit(-1);
        }

        fprintf(fp, "<data>\n");
      }

      for(UINT ii = 0; ii < fieldcount; ii++)
      {
        fprintf(fp,"<%s>%s</%s>",TABFIELD.m_vALLFIELD[ii].fieldname,fieldval[ii],TABFIELD.m_vALLFIELD[ii].fieldname); 
      }

      fprintf(fp,"<endl/>\n");
  
      if (strcmp(deleteflag,"TRUE") == 0)
      {
        if (stmtdel.execute() != 0)
        {
          logfile.Write("execute sql failed.\n%s\n%s\n",stmtdel.m_sql,stmtdel.cda.message); CallQuit(-1);
        }
      }
    }
  
    if (fp != 0)
    {
      fprintf(fp, "</data>");
      fclose(fp);
      fp = 0;

      if (RENAME(strtmpfilename,strstdfilename) == FALSE)
      {
        logfile.Write("RENAME %s to %s failed.\n", strtmpfilename,strstdfilename); CallQuit(-1);
      }

      logfile.Write("create %s(%ld) ok.\n",strstdfilename,stmtsel.cda.rpc);
    }
  
    conn.commitwork();
  
    if (sleeptime == 0) break;

    sleep(sleeptime);
  }
  
  return 0;
}

void CallQuit(int sig)
{
  signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  if (fp != 0) { fclose(fp); fp=0; REMOVE(strtmpfilename); }

  logfile.Write("gettabdata exit.\n");

  exit(0);
}

