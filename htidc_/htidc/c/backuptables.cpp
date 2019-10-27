#include "idcapp.h"

void CallQuit(int sig);

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CALLTABLE      ALLTABLE;

char strEXPFilePath[201];

int main(int argc,char *argv[])
{
  if (argc != 4)
  {
    printf("\n");
    printf("Using:./htidc/htidc/bin/backuptables logfilepath connstr expfilepath\n"); 

    printf("Example:/htidc/htidc/bin/procctl 3600 /htidc/htidc/bin/backuptables /log/ssqx/backuptables.log qxidc/pwdidc@EJETDB_221.179.6.136 /qxdata/ssqx/dmp\n\n");

    printf("此程序用于自动备份数据中心主数据库（其它数据库不需要备份）的表，流程如下：\n"); 
    printf("1、刷新数据字典表T_ALLTABLE和序列生成器字典表T_SEQANDTABLE的记录。\n"); 
    printf("2、获取T_ALLTABLE表中的记录，条件是ifbackup=1 and (sysdate-backuptime>backuptvl or backuptime is null)。\n");
    printf("3、判断表名是否以T_开头，如果是就备份它，不是就不备份。\n");
    printf("4、备份时生成DMPDAT_DDATETIME_TNAME.dmp文件和DMPLOG_DDATETIME_TNAME.log两个文件并压缩。\n");
    printf("5、备份生成的数据文件和日志文件存放在expfilepath目录中。\n\n\n");

    return -1;
  }

  memset(strEXPFilePath,0,sizeof(strEXPFilePath));

  strcpy(strEXPFilePath,argv[3]);

  MKDIR(strEXPFilePath);

  
  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(argv[1],"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  logfile.SetAlarmOpt("backuptables");

  ProgramActive.SetProgramInfo(&logfile,"backuptables",60);

  ALLTABLE.BindConnLog(&conn,0,&logfile,&ProgramActive);

  if (conn.connecttodb(argv[2],TRUE) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed.\n",argv[2]); CallQuit(-1);
  }

  // 注意，程序超时是7200秒
  ProgramActive.SetProgramInfo(&logfile,"backuptables",7200);

  // 把TAB数据字典中存在，  但T_ALLTABLE表中不存在的记录插入T_ALLTABLE中
  // 把TAB数据字典中不存在，但T_ALLTABLE表中存在的记录删除掉
  // 把USER_SEQUENCES数据字典中存在，  但T_SEQANDTABLE表中不存在的记录插入T_SEQANDTABLE
  // 把USER_SEQUENCES数据字典中不存在，但T_SEQANDTABLE表中存在的记录删除掉
  if (ALLTABLE.UpdateALLTABLEAndSEQ() != 0)
  {
    logfile.Write("ALLTABLE.UpdateALLTABLEAndSEQ failed.\n"); CallQuit(-1);
  }

  // 获取全部需要备份的记录，并放入m_vALLTABLE中
  if (ALLTABLE.LoadEXPRecord() != 0)
  {
    logfile.Write("ALLTABLE.LoadEXPRecord failed.\n"); CallQuit(-1);
  }

  for (UINT ii=0; ii<ALLTABLE.m_vALLTABLE.size(); ii++)
  {
    ProgramActive.WriteToFile();

    // 备份的表的表名必须是以"T_"做为前缀
    if (strncmp(ALLTABLE.m_vALLTABLE[ii].tname,"T_",2) != 0) continue;

    logfile.Write("backup %s ...",ALLTABLE.m_vALLTABLE[ii].tname);

    if (ALLTABLE.ExpTable(strEXPFilePath,strEXPFilePath,argv[2],ALLTABLE.m_vALLTABLE[ii].tname) == FALSE)
    {
      logfile.WriteEx("failed\n"); 
    }
    else
    {
      logfile.WriteEx("ok\n");
    }

    // 更新T_ALLTABLE表的备份时间字段
    if (ALLTABLE.UptBackupTime(ALLTABLE.m_vALLTABLE[ii].tname) != 0)
    {
      logfile.Write("ALLTABLE.UptBackupTime failed.\n"); CallQuit(-1);
    }
  }

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("backuptables exit.\n");

  exit(0);
}

