#include "idcapp.h"

void CallQuit(int sig);

#define MAXFIELDCOUNT  400  // 字段的最大数
#define MAXFIELDLEN   1000  // 字段值的最大长度

connection     connidc,conndst;
CLogFile       logfile;
CProgramActive ProgramActive;
UINT           uTaskType=0;
CEXPDTASK      EXPDTASK;

CFile File;

long exptable();


int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./pexptable logfilename connsrc tasktype\n\n");

    printf("Example:/htidc/htidc/bin/procctl 60 /htidc/htidc/bin/pexptable /log/szqx/pexptable_1.log htidc/pwdidc@SZQX_10.153.98.31 1\n\n");

    printf("该程序用于从数据中心或其它单位的数据库把数据导出到XML文件。\n");
    printf("导出的XML文件可交换给其它单位，也可以入库到数据中心。\n");
    printf("参数tasktype对应数据导出任务种类（T_EXPDTASK）表的tasktype。\n\n\n");

    return -1;
  }

  uTaskType=atoi(argv[3]);

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  // 注意，程序超时是60秒
  ProgramActive.SetProgramInfo(&logfile,"pexptable",60);

  // 连接数据中心的集群数据库
  if (connidc.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("connidc.connecttodb(%s) failed\n",argv[2]); CallQuit(-1);
  }

  EXPDTASK.BindConnLog(&connidc,&logfile);


  if (EXPDTASK.GetTaskByID(uTaskType) != 0)
  {
    logfile.Write("call EXPDTASK.GetTaskByID failed.\n"); CallQuit(-1);
  }

  while (EXPDTASK.GetTaskByIDNext() == 0)
  {
    if (exptable() != 0)
    {
      logfile.Write("call exptable failed.\n"); CallQuit(-1);
    }
  }

  if (EXPDTASK.selstmt.cda.rpc > 0) logfile.WriteEx("\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  File.Fclose();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("pexptable exit.\n");

  exit(0);
}

long exptable()
{
  struct st_EXPDTASK *pst= &EXPDTASK.m_stEXPDTASK;

  logfile.Write("taskname=%s ... ",pst->taskname);

  CCmdStr fieldstr,fieldlen;
  fieldstr.SplitToCmd(pst->fieldstr,",");
  fieldlen.SplitToCmd(pst->fieldlen,",");

  if ( (fieldstr.CmdCount()==0) || (fieldstr.CmdCount() != fieldlen.CmdCount()) )
  {
    logfile.WriteEx(" failed,fieldstr(%d) or fieldlen(%d) is invalid.\n",fieldstr.CmdCount(),fieldlen.CmdCount()); return -1;
  }

  char strfieldname[MAXFIELDCOUNT][51];
  char strfieldvalue[MAXFIELDCOUNT][MAXFIELDLEN+1];
  int  ifieldlen[MAXFIELDCOUNT];
  UINT uKeyID=0;
  int  ii;

  memset(strfieldname,0,sizeof(strfieldname));
  memset(strfieldvalue,0,sizeof(strfieldvalue));
  memset(&ifieldlen,0,sizeof(ifieldlen));

  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    fieldstr.GetValue(ii,strfieldname[ii],50);
    fieldlen.GetValue(ii,&ifieldlen[ii]);
  }
  
  // 连接数据远程数据库
  if (conndst.connecttodb(pst->tnsname,TRUE) != 0)
  {
    logfile.Write("conndst.connecttodb(%s) failed.\n",pst->tnsname); CallQuit(-1);
  }

  // 注意，连上数据库后，程序超时改为1200秒
  ProgramActive.SetProgramInfo(&logfile,"pexptable",1200);

  sqlstatement stmt;
  stmt.connect(&conndst);
  stmt.prepare(pst->selectsql);

  if (pst->exptype == 2)
  {
    stmt.bindin(1,&pst->position);
  }

  for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
  {
    stmt.bindout(ii+1,strfieldvalue[ii],ifieldlen[ii]);
  }

  // 如果是增量方式导出，还需要绑定keyid字段
  if (pst->exptype == 2)
  {
    stmt.bindout(ii+1,&uKeyID);
  }

  if (stmt.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",pst->selectsql,stmt.cda.message); 

    if (EXPDTASK.UptExpTime() != 0)
    {
      logfile.Write("call EXPDTASK.GetTaskByID failed.\n"); return -1;
    }

    conndst.commitwork();

    return stmt.cda.rc;
  }

  char strStdFileName[201],strLocalTime[20];
  memset(strLocalTime,0,sizeof(strLocalTime));
  memset(strStdFileName,0,sizeof(strStdFileName));

  LocalTime(strLocalTime,"yyyymmddhh24miss");
  sprintf(strStdFileName,"%s/%s%s%s_%d.xml",pst->outpath,pst->bfilename,strLocalTime,pst->efilename,getpid());

  while (stmt.next() == 0)
  {
    if (File.IsOpened() == FALSE)
    {
      if ( (File.OpenForRename(strStdFileName,"w+")) == 0 )
      {
        logfile.WriteEx("failed.OpenForRename %s failed.\n",strStdFileName); return -1;
      }

      if (strlen(pst->firstsql) != 0) File.Fprintf("execsql\n%s;\nendl\n",pst->firstsql);
    }

    for (ii=0;(UINT)ii<fieldstr.CmdCount();ii++)
    {
      File.Fprintf("<%s>%s</%s>",strfieldname[ii],strfieldvalue[ii],strfieldname[ii]);
    }

    File.Fprintf("endl\n");

    if (pst->exptype == 2) { if (pst->position < uKeyID) pst->position = uKeyID; }
  }

  if (File.IsOpened() == TRUE)
  {
    File.Fprintf("END"); 

    File.CloseAndRename();
  }

  logfile.WriteEx("ok.rows %ld.\n",stmt.cda.rpc);

  if (EXPDTASK.UptExpTime() != 0)
  {
    logfile.Write("call EXPDTASK.UptExpTime failed.\n"); return -1;
  }

  conndst.commitwork();
  stmt.disconnect();
  conndst.disconnect();

  return 0;
}

