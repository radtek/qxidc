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

    printf("���������ڵ������ݿ���ָ��������ݣ�����xml�ļ�,�ŵ�outpathĿ¼�С�\n");
    printf("�������е���־�ļ���Ϊ/tmp/htidc/log/gettabdata_����.log��\n");
    printf("username/password@tnsname Ϊ���ݿ�����Ӳ�����\n");
    printf("tablename ΪҪ��������ݿ������\n");
    printf("outpathtmp Ϊ����ļ�����ʱĿ¼��\n");
    printf("outpath Ϊ����ļ�����ʽĿ¼��\n");
    printf("sleeptime ��ʾÿ��ִ�����ݵ�����ʱ���������ʱ����Ϊ0����ʾ�ó���ִ��һ�ε����������˳���\n");
    printf("deleteflag ��ʾ�������ݳɹ����Ƿ�Ҫɾ���ѱ��ɹ��������ԭʼ��¼��\n");
    printf("[where] ��һ����ѡ���ʾ�������ݵ�������ע��һ��Ҫ��˫���Ű�����������\n\n\n");

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

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹ�˽���
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal();
  signal(SIGTERM,CallQuit);   // ��ctl+c
  signal(SIGINT,CallQuit);    // kill �� killall

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

  //�򿪸澯
  logfile.SetAlarmOpt("gettabdata");

  // ע�⣬����ʱ��80��
  ProgramActive.SetProgramInfo(&logfile,"gettabdata",80);

  if (conn.connecttodb(connstr) != 0)
  {
    logfile.Write("connect to %s failed.\n",connstr); CallQuit(-1);
  }

  // ��ȡ���ȫ��������Ϣ
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

