#include "_public.h"

int main(int argc,char *argv[])
{
  char strPathName[201];
  UINT uSaveCount=0;

  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/deletearchive pathname savecount\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletearchive /oracle/archive 20\n\n");

    printf("�������ȡĿ��pathname¼�µ��ļ���Ϣ������ʱ�併��ֻ�������savecount���ļ��������Ķ�ɾ������\n");
    printf("��������Ҫ����ɾ��oracle���ݿ�Ĺ鵵��־�ļ���\n");
    printf("������д��־�ļ���Ҳ��������Ļ������κ���Ϣ��\n");
    printf("���������/bin/ls -lt pathname��ȡ�鵵��־�ļ���Ϣ��\n");
    printf("����������ֹ����У�Ҳ������procctl���ȡ�\n\n\n");

    printf("����oracle�鵵��־������������£�\n");
    printf("sqlplus /nolog\n");
    printf("connect / as sysdba;\n");
    printf("alter system set log_archive_dest_1='location=/home/oracle/oradata/EJETDB/archive';\n");
    printf("shutdown immediate;\n");
    printf("startup mount;\n");
    printf("alter database archivelog;\n");
    printf("alter database open;\n");
    printf("alter system switch logfile;\n\n\n");

    return -1;
  }

  memset(strPathName,0,sizeof(strPathName));

  strcpy(strPathName,argv[1]);
  uSaveCount=atoi(argv[2]);

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); 

  FILE *fp=NULL;

  char strCmd[301];
  memset(strCmd,0,sizeof(strCmd));
 
  sprintf(strCmd,"/bin/ls -lt %s",strPathName);

  if ( (fp=popen(strCmd,"r")) == NULL )
  {
    printf("popen %s failed.\n",strCmd); return -1;
  }

  CCmdStr CmdStr;

  UINT uFetchedCount=0;
  char strBuffer[1024],strFullFileName[201];

  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    if (FGETS(strBuffer,2000,fp) == FALSE) break;

    uFetchedCount++;

    if (uFetchedCount <= uSaveCount + 1 ) continue;

    Trim(strBuffer);
    UpdateStr(strBuffer,"  "," "); 

    CmdStr.SplitToCmd(strBuffer," ");

    memset(strBuffer,0,sizeof(strBuffer));
    CmdStr.GetValue(CmdStr.CmdCount()-1,strBuffer,200);

    memset(strFullFileName,0,sizeof(strFullFileName));
    snprintf(strFullFileName,200,"%s/%s",strPathName,strBuffer);

    REMOVE(strFullFileName);
  }

  pclose(fp);

  return 0;
}

