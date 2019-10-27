#ifndef _WANDLIFE_H
#define _WANDLIFE_H

#include "_oracle.h"
#include "_public.h"

struct st_ALERTINFO
{
  char puborg[21];
  char title[101];
  char link[301];
  char atype[21];
  char alevel[21];
  char astatus[21];
  char description[2001];
  char pubstd[2001];
  char prem[2001];
  char pubdate[21];
};

class CALERTINFO
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_ALERTINFO m_stALERTINFO;
  vector<struct st_ALERTINFO> m_vALERTINFO;

  CALERTINFO();
 ~CALERTINFO();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  BOOL SplitBuffer(char *strBuffer);

  BOOL UptTable();
};

#endif
