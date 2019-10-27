#include "_public.h"
#include "_ooci.h"

struct st_spcc
{
  int  cityid;
  char ddtime[20];
  int  ybsx;
  char t[8];
  char u[8];
  char wd[8];
  char wf[8];
  char p[8];
  char r[8];
  char tc[8];
  char lc[8];
  char w[11];
  char v[8];
  char maxt[8];
  char mint[8];
  char maxu[8];
  char minu[8];
  char r24[8];
  char r12[8];
  char tc12[8];
  char lc12[8];
  char w12[11];
  char wd12[8];
  char wf12[8];
}stspcc;


CDir  Dir;
CFile File;
CLogFile logfile;

// ���������ļ�
BOOL _pspccfiles();

int  citysum=0;
char strddtime[20];
int  cityid=0;
int  ybsum=0;
char strBuffer[301];
CCmdStr CmdStr;

// ��ȡ�ļ�������ʱ��
BOOL getddtime();
// ��ȡվ�����ͼ�¼��
BOOL getcityid();

// ����ÿһ������
BOOL procline();

connection conn;
sqlstatement stmt;

int totalcount=0;
// �����ݲ������
int inserttodb();

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./pspccfiles dfilepath connstr logfile\n\n");

    printf("Example:./pspccfiles /home/freec/wucz/data scott/tiger@orcl11g_193.112.167.234 /home/freec/wucz/log/pspccfiles.log\n\n");

    printf("������������̨��������Ԥ���еĸ�ʡ����Ԥ����Ʒ����SPCC�ļ���������ɺ�ֱ����Ᵽ�档\n");

    printf("dfilepath SPCC�ļ���ŵ�Ŀ¼��\n");
    printf("connstr ���ݿ�����Ӳ�����\n");
    printf("logfile ���������в�������־�ļ�����\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  void CloseIOAndSignal();
   
  /*
  1����Ŀ¼����ȡĿ¼��ȫ�����ļ�����
  2����ȡһ���ļ�����
  3�����ļ����ж��ļ��������ԣ���ȡ��ʱ�䡣
  4����ȡ�ļ���ÿһ�У���ֽ��룬�������ݿ����
  5��������һ�������ļ����ύ����ɾ��ԭʼ�����ļ�
  */

  // ����־
  if (logfile.Open(argv[3],"a+")==FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[3]); return -1;
  }

  // �������ݿ�
  if (conn.connecttodb(argv[2],"Simplified Chinese_China.ZHS16GBK")!=0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); return -1;
  }

  // ��Ŀ¼ 
  if (Dir.OpenDir(argv[1],"Z_SEVP_*-SPCC-*.TXT")==FALSE)
  {
    logfile.Write("Dir.OpenDir(%s) failed.\n",argv[1]); return -1;
  }

  while (TRUE)
  {
    if (Dir.ReadDir()==FALSE) break;

    logfile.Write("��ʼ����%s...",Dir.m_FileName);

    // ���������ļ�
    if (_pspccfiles()==FALSE) break;

    logfile.WriteEx("ok.(%d)\n",totalcount);
  }

  return 0;
}

// ���������ļ�
BOOL _pspccfiles()
{
  // �ж��ļ���������
  if (CheckFileEnd(Dir.m_FullFileName,"NNNN")==FALSE)
  {
    logfile.Write("CheckFileEnd(%s) failed.\n",Dir.m_FullFileName); return FALSE;
  }

  // ���ļ�
  if (File.OpenForRead(Dir.m_FullFileName,"rt")==FALSE)
  {
    logfile.Write("File.OpenForRead(%s) failed.\n",Dir.m_FullFileName); return FALSE;
  }

  totalcount=0;

  // ��ȡ��ʱ���վ��
  if (getddtime() == FALSE)
  {
    logfile.Write("�ļ��Ƿ����޷�������ʱ���վ����\n"); return FALSE;
  } 

  // logfile.Write("strddtime=%s,citysum=%d\n",strddtime,citysum);

  for (int ii=0;ii<citysum;ii++)
  {
    // ��ȡվ�����ͼ�¼��
    getcityid(); 

    // logfile.Write("cityid=%d,ybsum=%d\n",cityid,ybsum);

    for (int jj=0;jj<ybsum;jj++)
    {
      // ����ÿһ������
      if (procline()==FALSE) continue;

      // �����ݲ������
      if (inserttodb()!=0) return FALSE;

      totalcount++;
    }
  }

  conn.commit();

  // ɾ������Դ�ļ�
  File.CloseAndRemove();

  return TRUE;
}

// ��ȡ�ļ�������ʱ��
BOOL getddtime()
{
  memset(strddtime,0,sizeof(strddtime));

  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    if (File.FFGETS(strBuffer,300)==FALSE) break;
   
    DeleteLRChar(strBuffer,' ');

    UpdateStr(strBuffer,"  "," ");

    CmdStr.SplitToCmd(strBuffer," ");

    if ( (CmdStr.m_vCmdStr[0]=="SPCC")&& (CmdStr.CmdCount()==2) )
    {
      // ��ȡ��ȡʱ��
      CmdStr.GetValue(1,strddtime,10);
      strcat(strddtime,"0000"); 

      // �ٶ�һ�У���ȡվ�����ų��������е����
      while (TRUE)
      {
        memset(strBuffer,0,sizeof(strBuffer));

        if (File.FFGETS(strBuffer,300)==FALSE) return FALSE;

        citysum=atoi(strBuffer);

        if (citysum == 0) continue;

        return TRUE;
      }
    }
  }

  return FALSE;
}


// ��ȡվ�����ͼ�¼��
BOOL getcityid()
{
  // 59134   118.07   24.48    139.40     14     21
  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    if (File.FFGETS(strBuffer,300)==FALSE) return FALSE;

    DeleteLRChar(strBuffer,' ');

    UpdateStr(strBuffer,"  "," ");

    CmdStr.SplitToCmd(strBuffer," ");

    if (CmdStr.CmdCount() != 6) continue;

    CmdStr.GetValue(0,&cityid);
    CmdStr.GetValue(4,&ybsum);

    return TRUE;
  }
}

// ����ÿһ������
BOOL procline()
{
  memset(strBuffer,0,sizeof(strBuffer));

  if (File.FFGETS(strBuffer,300)==FALSE) return FALSE;

  DeleteLRChar(strBuffer,' ');

  UpdateStr(strBuffer,"  "," ");

  // logfile.WriteEx("%s\n",strBuffer);

  memset(&stspcc,0,sizeof(struct st_spcc));
      
  CmdStr.SplitToCmd(strBuffer," ");

  if (CmdStr.CmdCount() != 22) return FALSE;
     
  stspcc.cityid=cityid;
  strcpy(stspcc.ddtime,strddtime);
  CmdStr.GetValue( 0,&stspcc.ybsx);
  CmdStr.GetValue( 1, stspcc.t,7);
  CmdStr.GetValue( 2, stspcc.u,7);
  CmdStr.GetValue( 3, stspcc.wd,7);
  CmdStr.GetValue( 4, stspcc.wf,7);
  CmdStr.GetValue( 5, stspcc.p,7);
  CmdStr.GetValue( 6, stspcc.r,7);
  CmdStr.GetValue( 7, stspcc.tc,7);
  CmdStr.GetValue( 8, stspcc.lc,7);
  CmdStr.GetValue( 9, stspcc.w,10);
  CmdStr.GetValue(10, stspcc.v,7);
  CmdStr.GetValue(11, stspcc.maxt,7);
  CmdStr.GetValue(12, stspcc.mint,7);
  CmdStr.GetValue(13, stspcc.maxu,7);
  CmdStr.GetValue(14, stspcc.minu,7);
  CmdStr.GetValue(15, stspcc.r24,7);
  CmdStr.GetValue(16, stspcc.r12,7);
  CmdStr.GetValue(17, stspcc.tc12,7);
  CmdStr.GetValue(18, stspcc.lc12,7);
  CmdStr.GetValue(19, stspcc.w12,10);
  CmdStr.GetValue(20, stspcc.wd12,7);
  CmdStr.GetValue(21, stspcc.wf12,7);

  if (strcmp(stspcc.t,"999.9")==0) memset(stspcc.t,0,sizeof(stspcc.t));
  if (strcmp(stspcc.u,"999.9")==0) memset(stspcc.u,0,sizeof(stspcc.u));
  if (strcmp(stspcc.wd,"999.9")==0) memset(stspcc.wd,0,sizeof(stspcc.wd));
  if (strcmp(stspcc.wf,"999.9")==0) memset(stspcc.wf,0,sizeof(stspcc.wf));
  if (strcmp(stspcc.p,"999.9")==0) memset(stspcc.p,0,sizeof(stspcc.p));
  if (strcmp(stspcc.r,"999.9")==0) memset(stspcc.r,0,sizeof(stspcc.r));
  if (strcmp(stspcc.tc,"999.9")==0) memset(stspcc.tc,0,sizeof(stspcc.tc));
  if (strcmp(stspcc.lc,"999.9")==0) memset(stspcc.lc,0,sizeof(stspcc.lc));
  if (strcmp(stspcc.w,"999.9")==0) memset(stspcc.w,0,sizeof(stspcc.w));
  if (strcmp(stspcc.v,"999.9")==0) memset(stspcc.v,0,sizeof(stspcc.v));
  if (strcmp(stspcc.maxt,"999.9")==0) memset(stspcc.maxt,0,sizeof(stspcc.maxt));
  if (strcmp(stspcc.mint,"999.9")==0) memset(stspcc.mint,0,sizeof(stspcc.mint));
  if (strcmp(stspcc.maxu,"999.9")==0) memset(stspcc.maxu,0,sizeof(stspcc.maxu));
  if (strcmp(stspcc.minu,"999.9")==0) memset(stspcc.minu,0,sizeof(stspcc.minu));
  if (strcmp(stspcc.r24,"999.9")==0) memset(stspcc.r24,0,sizeof(stspcc.r24));
  if (strcmp(stspcc.r12,"999.9")==0) memset(stspcc.r12,0,sizeof(stspcc.r12));
  if (strcmp(stspcc.tc12,"999.9")==0) memset(stspcc.tc12,0,sizeof(stspcc.tc12));
  if (strcmp(stspcc.lc12,"999.9")==0) memset(stspcc.lc12,0,sizeof(stspcc.lc12));
  if (strcmp(stspcc.w12,"999.9")==0) memset(stspcc.w12,0,sizeof(stspcc.w12));
  if (strcmp(stspcc.wd12,"999.9")==0) memset(stspcc.wd12,0,sizeof(stspcc.wd12));
  if (strcmp(stspcc.wf12,"999.9")==0) memset(stspcc.wf12,0,sizeof(stspcc.wf12));

  return TRUE;
}

// �����ݲ������
int inserttodb()
{
  if (stmt.m_state == 0)
  {
    stmt.connect(&conn);
    stmt.prepare("insert into t_spcc(cityid,ddtime,ybsx,t,u,wd,wf,p,r,tc,lc,w,v,maxt,mint,maxu,minu,r24,r12,tc12,lc12,w12,wd12,wf12,keyid) values(:1,to_date(:2,'yyyy-mm-dd hh24:mi:ss'),:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:15,:16,:17,:18,:19,:20,:21,:22,:23,:24,SEQ_spcc.nextval)");
    stmt.bindin( 1,&stspcc.cityid);
    stmt.bindin( 2, stspcc.ddtime,14);
    stmt.bindin( 3,&stspcc.ybsx);
    stmt.bindin( 4, stspcc.t,7);
    stmt.bindin( 5, stspcc.u,7);
    stmt.bindin( 6, stspcc.wd,7);
    stmt.bindin( 7, stspcc.wf,7);
    stmt.bindin( 8, stspcc.p,7);
    stmt.bindin( 9, stspcc.r,7);
    stmt.bindin(10, stspcc.tc,7);
    stmt.bindin(11, stspcc.lc,7);
    stmt.bindin(12, stspcc.w,10);
    stmt.bindin(13, stspcc.v,7);
    stmt.bindin(14, stspcc.maxt,7);
    stmt.bindin(15, stspcc.mint,7);
    stmt.bindin(16, stspcc.maxu,7);
    stmt.bindin(17, stspcc.minu,7);
    stmt.bindin(18, stspcc.r24,7);
    stmt.bindin(19, stspcc.r12,7);
    stmt.bindin(20, stspcc.tc12,7);
    stmt.bindin(21, stspcc.lc12,7);
    stmt.bindin(22, stspcc.w12,10);
    stmt.bindin(23, stspcc.wd12,7);
    stmt.bindin(24, stspcc.wf12,7);
  }

  if (stmt.execute() != 0)
  {
    logfile.Write("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return stmt.m_cda.rc;
  }
  
  return 0;
}

