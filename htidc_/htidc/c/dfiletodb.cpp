#include "idcapp.h"

void CallQuit(int sig);

char strxmlbuffer[4001];
char strlogfilename[301];
char strconnstr[301];
char strstdpath[301];
char strstdbakpath[301];
char strstderrpath[301];
char striflog[31];
char strindexid[31]; 
char strLocalTime[21]; 
char strcharset[101];

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CDir           DFileDir;
CQXDATA        QXDATA;
FILE           *stdfp;
int            iFileVer;  // ������ļ��İ汾��1-���������°汾��2-�������ľɰ汾��3-ʡ��ͨ�ýӿڸ�ʽ
CRealMon       RealMon;

// ����ֵ��0-�ɹ���1-Ӧ�����ݶ������2-���ļ�ʧ�ܻ��ļ�״̬����ȷ��3-�������ݿ������
int _dfiletodb();

// ִ��XML�ļ��ײ���SQL
long ExecSQL();

int main(int argc,char *argv[])
{
  if (argc != 2) 
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/dfiletodb inifile\n");
    printf("Postgres Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/bin/dfiletodb \"<logfilename>/log/fs/gxpt/dfiletodb.log</logfilename><connstr>host=223.4.5.165 user=postgres password=123456 dbname=gxpt port=5433</connstr><stdpath>/qxdata/fs/gxpt/sdata/std</stdpath><stdbakpath>/qxdata/fs/gxpt/sdata/stdbak</stdbakpath><stderrpath>/qxdata/fs/gxpt/sdata/stderr</stderrpath><iflog>true</iflog><indexid>30001</indexid>\"\n\n");
    printf("Mysql Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/c/dfiletodb \"<logfilename>/log/szqx/dfiletodb_mysql.log</logfilename><connstr>10.153.121.120,root,123456,szqx,3306</connstr><stdpath>/qxdata/szqx/sdata/stdzdz</stdpath><stdbakpath>/qxdata/szqx/sdata/stdbak</stdbakpath><stderrpath>/qxdata/szqx/sdata/stderr</stderrpath><iflog>true</iflog><indexid>30001</indexid><charset>gbk</charset>\"\n\n");
    printf("Oracle Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/c/dfiletodb \"<logfilename>/log/szqx/dfiletodb_mysql.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.98.31</connstr><stdpath>/qxdata/szqx/sdata/stdzdz</stdpath><stdbakpath>/qxdata/szqx/sdata/stdbak</stdbakpath><stderrpath>/qxdata/szqx/sdata/stderr</stderrpath><iflog>true</iflog><indexid>30001</indexid><charset>Simplified Chinese_China.ZHS16GBK</charset>\"\n\n");
    
    printf("��ά������������������֧��oracle��mysql��pg���ݿ⡣\n");
    printf("logfilename ���������е���־�ļ���\n");
    printf("connstr ���ݿ����Ӳ�����oracle���ݿ���username/password@tnsname��pg���ݿ���host= user= password= dbname= port=��\n");
    printf("stdpath ������xml�ļ���ŵ�Ŀ¼��\n");
    printf("stdbakpath ������ļ����ݵ�Ŀ¼������Ϊ��\n");
    printf("stderrpath �����ⷢ������xml�ļ�����ת�Ƶ���Ŀ¼��\n");
    printf("iflog ���xml�ļ��еļ�¼���ʧ�ܣ��Ƿ��¼��־��true-��¼��false-����¼��\n\n");
    printf("indexid ���ݼ�ص����̱�ţ����Ϊ�գ����������ݲɼ�����ļ���\n");
    printf("ע�⣺\n");
    printf("  1��stdbakpathĿ¼�µ��ļ���deletefiles��ʱ������stderrpathĿ¼�µ��ļ�ϵͳ����ԱҪ���ڼ�飬�ҳ�ʧ��ԭ��\n"); 
    printf("  2��������õ���pg���ݿ⣬���������ݿ������������½ű�����������\n");
    printf("  create or replace function to_null(varchar) returns numeric as $$\n");
    printf("  begin\n");
    printf("  if (length($1)=0) then\n");
    printf("    return null;\n");
    printf("  else\n");
    printf("    return $1;\n");
    printf("  end if;\n");
    printf("  end\n");
    printf("  $$ LANGUAGE plpgsql;\n");
    printf("  3��������õ���oracle���ݿ⣬���������ݿ������������½ű�����������\n");
    printf("  create or replace function to_null(in_value in varchar2) return varchar2\n");
    printf("  is\n");
    printf("  begin\n");
    printf("    return in_value;\n");
    printf("  end;\n");
    printf("  /\n\n");
 
    return -1;
  }

  memset(strxmlbuffer,0,sizeof(strxmlbuffer));
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strstdpath,0,sizeof(strstdpath));
  memset(strstdbakpath,0,sizeof(strstdbakpath));
  memset(strstderrpath,0,sizeof(strstderrpath));
  memset(striflog,0,sizeof(striflog));
  memset(strindexid,0,sizeof(strindexid));
  memset(strcharset,0,sizeof(strcharset));

  strncpy(strxmlbuffer,argv[1],4000);

  GetXMLBuffer(strxmlbuffer,"logfilename",strlogfilename,300);
  GetXMLBuffer(strxmlbuffer,"connstr",strconnstr,300);
  GetXMLBuffer(strxmlbuffer,"stdpath",strstdpath,300);
  GetXMLBuffer(strxmlbuffer,"stderrpath",strstderrpath,300);
  GetXMLBuffer(strxmlbuffer,"iflog",striflog,30);
  GetXMLBuffer(strxmlbuffer,"charset",strcharset,100);
  GetXMLBuffer(strxmlbuffer,"indexid",strindexid,30);
  if(strstr(strxmlbuffer,"stdbakpath")!=0){ GetXMLBuffer(strxmlbuffer,"stdbakpath",strstdbakpath,300); }

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0) { printf("connstr is null.\n"); return -1; }
  if (strlen(strstdpath) == 0) { printf("stdpath is null.\n"); return -1; }
  // if (strlen(strstdbakpath) == 0) { printf("stdbakpath is null.\n"); return -1; }
  if (strlen(strstderrpath) == 0) { printf("stderrpath is null.\n"); return -1; }
  // if (strlen(striflog) == 0) { printf("iflog is null.\n"); return -1; }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("dfiletodb");

  // ע�⣬����ʱ��500��
  ProgramActive.SetProgramInfo(&logfile,"dfiletodb",500);

  // ����Ӧ�����ݿ⣬�����ݿ������������������ݴ���
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strconnstr); CallQuit(-1);
  }

  // �����ַ���
  if (strlen(strcharset) != 0) conn.character(strcharset);

  ProgramActive.SetProgramInfo(&logfile,"dfiletodb",180);

  logfile.Write("dfiletodb beging.\n");

  QXDATA.BindConnLog(&conn,&logfile);

  int timetvl=31;

  BOOL bContinue=FALSE;
  char strSTDBAKFileName[201];
  char strSTDERRFileName[201];

  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    // д���̻��־�ļ�
    RealMon.WriteToProActLog(strindexid,"dfiletodb",500);

    if (timetvl++>30) // ��������Ӧ��������������T_APPDTYPE��
    {
      timetvl=0;

      if (QXDATA.LoadFileCFG() != 0) CallQuit(-1);
    }

    char strCMD[1024];
    memset(strCMD,0,sizeof(strCMD));
    snprintf(strCMD,1000,"/usr/bin/gunzip -f %s/*.gz 1>/dev/null 2>/dev/null",strstdpath);
    system(strCMD);

    // �򿪱�׼��ʽ�ļ�Ŀ¼
    if (DFileDir.OpenDir(strstdpath) == FALSE)
    {
      logfile.Write("DFileDir.OpenDir %s failed.\n",strstdpath); CallQuit(-1);
    }

    bContinue = FALSE;

    // ���л�ȡÿ���ļ������
    while (DFileDir.ReadDir() == TRUE)
    {
      // д����̻��Ϣ
      ProgramActive.WriteToFile();

      // ����ļ���ʱ���ڵ�ǰʱ���ǰ5��֮�ڣ�����ʱ����⣬��ô����Ŀ����Ϊ�˱�֤�����ļ��������ԡ�
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-10);
      if (strcmp(DFileDir.m_ModifyTime,strLocalTime)>0) continue;

      memset(strSTDBAKFileName,0,sizeof(strSTDBAKFileName));
      memset(strSTDERRFileName,0,sizeof(strSTDERRFileName));
      
      snprintf(strSTDERRFileName,300,"%s/%s",strstderrpath,DFileDir.m_FileName);

      // �����*.XML.GZ���Ͳ�����������
      if ( MatchFileName(DFileDir.m_FileName,"*.XML.GZ") == TRUE) continue;

      // ����ļ���������.XML�������Ͳ��������ļ�
      if ( MatchFileName(DFileDir.m_FileName,"*.XML") == FALSE) 
      {
        logfile.Write("FileName %s is invalidation!\n",DFileDir.m_FullFileName); 
        REMOVE(DFileDir.m_FullFileName); 
        continue;
      }

      // ��ʼ����ÿ���ļ�
      logfile.Write("Process file %s...",DFileDir.m_FileName);

      // ����ֵ��0-�ɹ���1-Ӧ�����ݶ������2-���ļ�ʧ�ܻ��ļ�״̬����ȷ��3-�������ݿ������
      if (_dfiletodb() == 0)
      { 
        // ��������˱���Ŀ¼�������Ǿ��Ƶ�����Ŀ¼�������ɾ���ļ�
        if (strlen(strstdbakpath) != 0) 
        {
          snprintf(strSTDBAKFileName,300,"%s/%s",strstdbakpath,DFileDir.m_FileName); 

          // �ļ����ɹ��������ƶ�stdbakĿ¼
          if (rename(DFileDir.m_FullFileName,strSTDBAKFileName) != 0)
          {
            REMOVE(DFileDir.m_FullFileName); 
            logfile.WriteEx("failed.rename %s failed.\n",DFileDir.m_FullFileName); 
          }
          else
          {
            logfile.WriteEx("ok.\n"); bContinue = TRUE; 

	    if (conn.commitwork() != 0) 
	    {
	      logfile.Write("conn.commitwork() failed,message=%s\n",conn.lda.message);
	      break;
	    }
          }
        }
        else
        {
          REMOVE(DFileDir.m_FullFileName); logfile.WriteEx("ok.\n"); bContinue = TRUE; 

	  if (conn.commitwork() != 0) 
	  {
	    logfile.Write("conn.commitwork() failed,message=%s\n",conn.lda.message);
	    break;
	  }
        }
      }
      else
      {
        // �ļ����ʱ�����˴��󣬰����Ƶ�stderrĿ¼
        logfile.WriteEx("failed.\n"); 
        if (rename(DFileDir.m_FullFileName,strSTDERRFileName) != 0) REMOVE(DFileDir.m_FullFileName); 
      }

      // д����̻��Ϣ
      ProgramActive.WriteToFile();
    }

    // �������������־XML�ļ�
    if (RealMon.WriteToDbLog() == FALSE) logfile.Write("WriteToDbLog failed.\n"); 

    // ����֮��Ҫ�����������Ȼ�����ﻹ����ʷ���ݡ�
    RealMon.m_vTODBLOG.clear();

    if (bContinue == FALSE) sleep(5);
  }

  logfile.Write("dfiletodb exit.\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  if (conn.disconnect() !=0)logfile.Write("conn.disconnect() failed,%s\n",conn.lda.message);

  // �������������־XML�ļ�
  if (RealMon.WriteToDbLog() == FALSE) logfile.Write("WriteToDbLog failed.\n"); 

  logfile.Write("dfiletodb exit.\n");

  if (stdfp != 0) fclose(stdfp);

  exit(0);
}

// ����ֵ��0-�ɹ���1-Ӧ�����ݶ������2-���ļ�ʧ�ܻ��ļ�״̬����ȷ��3-�������ݿ������
int _dfiletodb()
{
  // �����ļ������ļ����ͻ�ȡ�����ļ����Ͳ���
  strcpy(QXDATA.m_filename,DFileDir.m_FileName);

  // 1-Ӧ�����ݶ����
  if (QXDATA.GETFILECFG() == FALSE) { logfile.WriteEx("QXDATA.GETFILECFG "); return 1; }

  stdfp=0;

  // ��XML�ļ�
  if ( (stdfp=FOPEN(DFileDir.m_FullFileName,"r")) == 0 ) { logfile.WriteEx("FOPEN "); return 2; }

  // �ж��ļ��Ƿ���"</data>"����,����END����
  if ( (CheckFileSTS(DFileDir.m_FullFileName,"</data>") == FALSE) &&
       (CheckFileSTS(DFileDir.m_FullFileName,"END") == FALSE) &&
       (CheckFileSTS(DFileDir.m_FullFileName,"</DS>") == FALSE) )
  { 
    logfile.WriteEx(" state  "); fclose(stdfp); stdfp=0; return 2; 
  }

  iFileVer=2;
  if (CheckFileSTS(DFileDir.m_FullFileName,"</data>") == TRUE) iFileVer=1;
  if (CheckFileSTS(DFileDir.m_FullFileName,"END") == TRUE)     iFileVer=2;
  if (CheckFileSTS(DFileDir.m_FullFileName,"</DS>") == TRUE)   iFileVer=3;

  // ��ȡ��Ӧ�����ֶ���Ϣ
  if (QXDATA.GETTABCOLUMNS() != 0) { logfile.WriteEx("QXDATA.GETTABCOLUMNS "); fclose(stdfp); stdfp=0; return 3; }

  // ���û�л�ȡ���ֶ���Ϣ�����Ǳ������ڣ�����
  if (QXDATA.m_vTABCOLUMNS.size() == 0) { logfile.WriteEx("table %s no exist ",QXDATA.m_tname); fclose(stdfp); stdfp=0; return 1; }

  // ���ɲ������ݱ���SQL���
  QXDATA.CrtSQL();

  // ׼���������ݱ���SQL��䣬������ѯ�����ºͲ���
  if (QXDATA.PreSQL() != 0) { logfile.WriteEx("QXDATA.PreSQL "); fclose(stdfp); stdfp=0; return 3; }

  // ִ��XML�ļ��ײ���SQL
  if (ExecSQL() != 0) { logfile.WriteEx("ExecSQL "); fclose(stdfp); stdfp=0; return 3; }
 
  UINT uInsUptCount=0;       // ����͸��µ��ܼ�¼��
  UINT uActiveCount=0;
  char strBuffer[81920];

  uInsUptCount=0;

  // �����ļ���ȫ����
  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    // ��ȡһ��
    if (iFileVer==1)
    {
      if (FGETS(strBuffer,80000,stdfp,"<endl/>") == FALSE) break;
    }
    if (iFileVer==2)
    {
      if (FGETS(strBuffer,80000,stdfp,"endl") == FALSE) break;
    }
    if (iFileVer==3)
    {
      if (FGETS(strBuffer,80000,stdfp,"</R>") == FALSE) break;
    }

    // һ�е����ݲ�����С��5�����С��5��һ���ǿ��У���������
    if (strlen(strBuffer) < 5) continue;

    // ��ÿ�е����ݽ�����ֶ���
    QXDATA.UNPackBuffer(strBuffer,iFileVer);

    QXDATA.BindParams();

    // �������µ�����
    if (QXDATA.InsertTable(strBuffer,striflog) != 0) { fclose(stdfp); stdfp=0; return 3; }

    /*
    // �����������Ч�ļ�¼�������
    if ( uInsUptCount == (QXDATA.m_InsCount+QXDATA.m_UptCount) )
    {
      logfile.Write("cda.rc=%ld,line=%s\n",QXDATA.stmtinserttable.cda.rc,strBuffer);
    }

    uInsUptCount=QXDATA.m_InsCount+QXDATA.m_UptCount;
    */

    // ÿ����1000����¼ʱд��һ�ν��̻��Ϣ
    if (uActiveCount >= 1000) { uActiveCount=0; ProgramActive.WriteToFile(); }

    uActiveCount++;
  }

  fclose(stdfp); stdfp=0;

  QXDATA.m_DiscardCount=QXDATA.m_TotalCount-QXDATA.m_UptCount-QXDATA.m_InsCount;

  logfile.WriteEx("total=%lu,insert=%lu,update=%lu,discard=%lu.",\
                   QXDATA.m_TotalCount,QXDATA.m_InsCount,QXDATA.m_UptCount,QXDATA.m_DiscardCount);

  // �ռ������Ϣ���������������־��
  if (strlen(strindexid) != 0)
  {
    memset(&RealMon.m_stTODBLOG,0,sizeof(RealMon.m_stTODBLOG));
    strcpy(RealMon.m_stTODBLOG.indexid,strindexid);
    getLocalIP(RealMon.m_stTODBLOG.serverip);
    strcpy(RealMon.m_stTODBLOG.programname,"dfiletodb");

    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyymmddhh24miss");
    strcpy(RealMon.m_stTODBLOG.ddatetime,strLocalTime);

    strcpy(RealMon.m_stTODBLOG.filetime,DFileDir.m_ModifyTime);
    strcpy(RealMon.m_stTODBLOG.filename,DFileDir.m_FullFileName);
    snprintf(RealMon.m_stTODBLOG.filesize,50,"%lu",DFileDir.m_FileSize);
    strcpy(RealMon.m_stTODBLOG.tname,QXDATA.m_tname);
    snprintf(RealMon.m_stTODBLOG.total,50,"%lu",QXDATA.m_TotalCount);
    snprintf(RealMon.m_stTODBLOG.insrows,50,"%lu",QXDATA.m_InsCount);
    snprintf(RealMon.m_stTODBLOG.uptrows,50,"%lu",QXDATA.m_UptCount);
    snprintf(RealMon.m_stTODBLOG.disrows,50,"%lu",QXDATA.m_DiscardCount);

    RealMon.m_vTODBLOG.push_back(RealMon.m_stTODBLOG);
  }

  return 0;
}


// ִ��XML�ļ��ײ���SQL
long ExecSQL()
{
  char strLine[2048],strSQL[2048];
  memset(strLine,0,sizeof(strLine));
  memset(strSQL,0,sizeof(strSQL));

  if (iFileVer==1)
  {
    if (FGETS(strLine,2000,stdfp,"<endl/>") == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
    if (GetXMLBuffer(strLine,"sql",strSQL,2000) == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
  }
  if (iFileVer==2)
  {
    if (FGETS(strLine,1000,stdfp) == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
    if (strcmp(strLine,"execsql") != 0) { fseek(stdfp,0L,SEEK_SET); return 0; }
    if (FGETS(strSQL,2000,stdfp,"endl") == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
  }

  if (iFileVer==3) return 0;

  sqlstatement stmt;
  stmt.connect(&conn);
  if (strcmp(conn.m_dbtype,"oracle")==0)
  {
    stmt.prepare("\
      BEGIN\
        %s\
      END;",strSQL);
  }
  else
  {
    stmt.prepare(strSQL);
  }
  if (stmt.execute() != 0)
  {
    logfile.Write("%s\n%s\n",strSQL,stmt.cda.message);
  }

  // ע�⣬���´��벻�����ã������ﲻ���ٶ�λ���ļ����ײ������ܰ�sql���е������ݴ�����
  // fseek(stdfp,0L,SEEK_SET);

  return stmt.cda.rc;
}

/*
create or replace function to_null(varchar) returns numeric as $$
begin
if (length($1)=0) then
  return null;
else
  return $1;
end if;
end
$$ LANGUAGE plpgsql;
*/
