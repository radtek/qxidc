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


/*
struct st_OBTCODE
{
  char obtid[21];
  char obtname[51];
  char obtname_old[51];
  char obtlevel[21];
  char provname[21];
  double lon;
  double lat;
  double height;
  char w3a[31];
  int  rsts;
  char memo[301];
};

class COBTCODE
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_OBTCODE m_stOBTCODE;
  vector<struct st_OBTCODE> m_vOBTCODE;

  COBTCODE();
 ~COBTCODE();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  BOOL LoadObtCode();
};

struct st_ZHCITYCODE
{
  char cityid[11];
  char cityname[21];
  char cityname_old[21];
  char obtid[21];
  char obtidsk[21];
  char obtidyb[21];
  char provname[21];
  char memo[301];
  int  orderby;
  int  rsts;
};

class CZHCITYCODE
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_ZHCITYCODE m_stZHCITYCODE;
  vector<struct st_ZHCITYCODE> m_vZHCITYCODE;

  CZHCITYCODE();
 ~CZHCITYCODE();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  BOOL LoadCityCode();
};
*/

struct st_INDEXINFO
{
  char cityid[21];
  char jrrc[6];
  char jrrl[6];
  char mrrc[6];
  char mrrl[6];
  char gm[1001];
  char wrks[1001];
  char zs[1001];
  char zwx[1001];
  char cy[1001];
  char ssd[1001];
  char hz[1001];
  char mf[1001];
  char xc[1001];
  char lk[1001];
  char jt[1001];
  char ly[1001];
  char yd[1001];
  char cl[1001];
  char dy[1001];
  char hc[1001];
  char yh[1001];
  char gj[1001];
  char ls[1001];
  char ys[1001];
  char ganmao[1001];
  char pubdate[21];
  char urlstationname[21];
  char urlcity[21];
  char urlprovince[21];
};

class CINDEXINFO
{
public:
  connection *m_conn;
  CLogFile   *m_logfile;

  struct st_INDEXINFO m_stINDEXINFO;
  vector<struct st_INDEXINFO> m_vINDEXINFO;

  CINDEXINFO();
 ~CINDEXINFO();

  void BindConnLog(connection *in_conn,CLogFile *in_logfile);

  // ±¸·ÝT_INDEXINFO±í
  BOOL BakIndexInfo();

  BOOL UptIndexInfo();
};

#endif
