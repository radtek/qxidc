#include "idcapp.h"

void CallQuit(int sig);
BOOL DealWithFile();

CLogFile logfile;
CDir Dir;
CProgramActive ProgramActive;
connection conn;
sqlstatement stmtins,stmtdel;
FILE *fp;
char fieldval[100][2001];
char deleteflag[11];
UINT ufilecount=0;

int main( int argc, char *argv[] )
{
  if ( argc != 5 )
  {
    printf("\nUsage:./instabdata username/password@tnsname filepath sleeptime deleteflag\n\n");

    printf("Example:/htidc/htidc/bin/procctl 20 /htidc/htidc/bin/instabdata esa/esaserver /home/jwtout/send 2 false\n");
    printf("        /htidc/htidc/bin/procctl 20 /htidc/htidc/bin/instabdata esa/esaserver /home/jwtin/recv  2 false\n\n");

    printf("�ó������ڽ�filepathĿ¼��XML�ļ����뵽���ݿ���У���������XML�ļ�����ȷ����\n" );
    printf("�������е���־�ļ���Ϊ/tmp/htidc/log/instabdata.log��\n");
    printf("username/password@tnsname Ϊ���ݿ�����Ӳ�����\n");
    printf("filepath Ϊ������xml�ļ���ŵ�Ŀ¼��\n");
    printf("sleeptime ��ʾÿ��ִ�����ݵ�����ʱ���������ʱ����Ϊ0����ʾ�ó���ִ��һ�ε����������˳���\n");
    printf("deleteflag ��ʾ��������ǰ�Ƿ���ɾ��Ŀ�ı����ݡ�\n\n\n" );

    return 0;
  }

  char connstr[101];
  char filepath[301];
  int  sleeptime = 0;

  memset(connstr,0,sizeof(connstr));
  memset(filepath,0,sizeof(filepath));
  memset(deleteflag,0,sizeof(deleteflag));

  strncpy(connstr,argv[1],50);
  strncpy(filepath,argv[2],300);
  sleeptime=atoi(argv[3]);
  strncpy(deleteflag,argv[4],10);
  ToUpper(deleteflag);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹ�˽���
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal();
  signal(SIGTERM,CallQuit);   // ��ctl+c
  signal(SIGINT,CallQuit);    // kill �� killall

  if (logfile.Open("/tmp/htidc/log/instabdata.log","a+") == FALSE)
  {
    printf("logfile.Open(/tmp/htidc/log/instabdata.log) failed.\n"); CallQuit(-1);
  }

  //�򿪸澯
  logfile.SetAlarmOpt("instabdata");

  // ע�⣬����ʱ��80��
  ProgramActive.SetProgramInfo(&logfile,"instabdata",80);

  if (conn.connecttodb(connstr)!=0)
  {
    logfile.Write("connect to db failed(%s)\n",connstr); CallQuit(-1);
  } 
  
  while ( TRUE )
  {
    if (Dir.OpenDir(filepath) == FALSE )
    {
      logfile.Write("Dir.Open(%s) failed.\n",filepath); CallQuit(-1);
    }
  
    // ע�⣬����ʱ��80��
    ProgramActive.SetProgramInfo(&logfile,"instabdata",80);
  
    while ( Dir.ReadDir() == TRUE )
    {
      // д����̻��Ϣ
      ProgramActive.WriteToFile();
  
      // �ļ�ȫ������
      if (strlen(Dir.m_FullFileName) >= 300) continue;
  
      // ����ļ���������.XML�������Ͳ�������ļ�
      if (MatchFileName(Dir.m_FileName,"*_20*.xml") == FALSE) continue;
  
      logfile.Write("process %s...",Dir.m_FileName );
  
      if (DealWithFile() == FALSE ) continue;

      if (REMOVE(Dir.m_FullFileName) == FALSE)
      {
        logfile.WriteEx("failed.\n,REMOVE %s failed.\n",Dir.m_FullFileName);
        conn.rollbackwork();
        continue;
      }

      logfile.WriteEx("ok(%lu).\n",ufilecount);

      conn.commitwork();
    }

    if (sleeptime == 0) break;

    sleep(sleeptime);
  }

  return 0;
}
      
// ����ֵ��0-�ɹ���1-Ӧ�����ݶ����2-���ļ�ʧ�ܻ��ļ�״̬����ȷ��3-�������ݿ�����
BOOL DealWithFile()
{
  ufilecount=0;
  CTABFIELD TABFIELD;
  fp=0;
  char tablename[51];  //���ڱ������
  char strBuffer[8001]; //���ڱ���һ���ļ�����
  char strtmp[51];  
  char strbind[501];
  char *pos;  //����ָ����������ַ�λ��

  memset(tablename,0,sizeof(tablename) );
  memset(strbind,0,sizeof(strbind) );
  memset(strtmp,0,sizeof(strtmp) );

  // �����ļ����ó���������һ����_20��ǰ��ľ��Ǳ���
  pos=strstr(Dir.m_FileName,"_20");

  strncpy(tablename,Dir.m_FileName,pos-Dir.m_FileName);
 
  // �õ�����ֶ���Ϣ
  TABFIELD.GetALLField(&conn, tablename);

  // �ֶ���Ϊ0˵��������
  if (TABFIELD.m_vALLFIELD.size() == 0)
  {
    logfile.WriteEx("failed.\ntable %s is not exist.\n",tablename); return FALSE;
  }

  // ��XML�ļ�
  if ( (fp=FOPEN(Dir.m_FullFileName,"r")) == 0 ) 
  { 
    logfile.WriteEx("failed.\nFOPEN %s failed.\n",Dir.m_FullFileName); return FALSE;
  }

  // �ж��ļ��Ƿ���"</data>"����
  if (CheckFileSTS(Dir.m_FullFileName,"</data>") == FALSE) 
  {
    fclose(fp); fp=0; return FALSE; 
  }
      
  // ���ݱ��ֶ���Ϣ��ƴ��values�ڲ������
  memset(strbind,0,sizeof(strbind));

  for (UINT ii=0; ii<TABFIELD.m_vALLFIELD.size(); ii++ )
  {
    if (strcmp(TABFIELD.m_vALLFIELD[ii].datatype,"date") == 0 ) 
    {
      if (ii == 0) sprintf(strtmp," to_date(:%lu,'yyyy-mm-dd hh24:mi:ss')",ii+1);
      if (ii >  0) sprintf(strtmp,",to_date(:%lu,'yyyy-mm-dd hh24:mi:ss')",ii+1);
    }
    else 
    {
      if (ii == 0) sprintf(strtmp," :%lu",ii+1);
      if (ii >  0) sprintf(strtmp,",:%lu",ii+1);
    }

    strcat(strbind,strtmp);
  }
       
  //�����Լ�ƴ�ӣ�TABFIELD��m_allfieldstr��Ա��������ȫ���ֶ�����Ϣ
  stmtins.connect( &conn );
  stmtins.prepare( "insert into %s(%s) values(%s)", tablename,TABFIELD.m_allfieldstr,strbind);

  for (UINT ii=0;ii<TABFIELD.m_vALLFIELD.size();ii++)
  {
    stmtins.bindin(ii+1,fieldval[ii],2000);
  }
  
  if (strcmp(deleteflag,"TRUE") == 0 )
  {
    stmtdel.connect(&conn );
    stmtdel.prepare("delete from %s",tablename);
    if (stmtdel.execute() != 0)
    {
      logfile.WriteEx("failed.\nexecute sql failed.\n%s\n%s\n",stmtdel.m_sql,stmtdel.cda.message); 
      fclose(fp); fp=0; return FALSE; 
    }
  }

  while (TRUE)
  {
    // д����̻��Ϣ
    ProgramActive.WriteToFile();

    memset(strBuffer,0,sizeof(strBuffer));
    memset(fieldval,0,sizeof(fieldval));

    // ��ȡһ��
    if (FGETS(strBuffer,8000,fp,"<endl/>") == FALSE) break;  

    // �е����ݲ�����С��5�����С��5��һ���ǿ��У���������
    if (strlen(strBuffer) < 5) continue;

    //����һ������ 
    for (UINT ii=0;ii<TABFIELD.m_vALLFIELD.size();ii++ )
    {
      GetXMLBuffer(strBuffer,TABFIELD.m_vALLFIELD[ii].fieldname,fieldval[ii],2000);
    }

    if (stmtins.execute() != 0)
    {
      // ���������ϢΪ1������ӡ
      // ��ʹ����������ҲҪ�����������������ݣ����ܷ��أ�Ҳ�����˳���
      if ( stmtins.cda.rc != 1 ) 
      {
        logfile.WriteEx("failed.\nexecute sql failed.\n%s\n%s\n",stmtins.m_sql,stmtins.cda.message); 
      }
    }

    ufilecount++;
  }

  fclose(fp); 

  fp=0; 

  return TRUE;
}

void CallQuit(int sig)
{
  signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  if (fp != 0) { fclose(fp); fp=0; }

  logfile.Write("instabdata exit.\n");

  exit(0);
}
