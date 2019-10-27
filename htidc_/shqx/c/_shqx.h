#ifndef _SHQX_H
#define _SHQX_H

#include "_public.h"
#include "_ooci.h"

// 全国气象站点参数数据结构
struct st_stcode
{
  char provname[31];   // 省名称
  char obtid[11];      // 站点代码
  char cityname[31];   // 城市名
  double lat;          // 纬度
  double lon;          // 经度
  double height;       // 海拔高度
};

// 全国气象站点分钟观测数据结构
struct st_surfdata
{
  char obtid[11];      // 站点代码
  char ddatetime[21];  // 数据时间：格式yyyy-mm-dd hh:mi:ss。
  int  t;              // 气温：单位，0.1摄氏度
  int  p;              // 气压：0.1百帕
  int  u;              // 相对湿度，0-100之间的值。
  int  wd;             // 风向，0-360之间的值。
  int  wf;             // 风速：单位0.1m/s
  int  r;              // 降雨量：0.1mm
  int  vis;            // 能见度：0.1米
};

class CSURFDATA
{
public:
  int totalcount,insertcount,updatecount,invalidcount;  // 记录总数据、插入数、更新数、无效记录数。
  struct st_surfdata m_stsurfdata;

  CSURFDATA(connection *conn,CLogFile *logfile);
 ~CSURFDATA();

  void initdata();  // 数据初始化

  connection *m_conn;
  CLogFile   *m_logfile;

  int iccount;
  sqlstatement stmtsel,stmtins,stmtupt;

  // 把用逗号分隔的记录拆分到m_stsurfdata结构中。
  bool SplitBuffer(const char *strBuffer);

  // 把xml格式的记录拆分到m_stsurfdata结构中。
  bool SplitBuffer1(const char *strBuffer);

  // 把m_stsurfdata结构中的值更新到T_SURFDATA表中。
  long InsertTable();
};

struct st_signallog
{
  char obtid[11];
  char ddatetime[20];
  char signalname[2];
  char signalcolor[2];
};

class CSIGNALLOG
{
public:
  int totalcount,insertcount,updatecount,invalidcount;  // 记录总数据、插入数、更新数、无效记录数。
  struct st_signallog m_stsignallog;
  vector<struct st_signallog> vsignallog;   // 容器存放一个文件的全部记录

  CSIGNALLOG(connection *conn,CLogFile *logfile);
 ~CSIGNALLOG();

  void initdata();  // 数据初始化

  connection *m_conn;
  CLogFile   *m_logfile;

  int iccount;
  sqlstatement stmtsel,stmtins,stmtupt;

  // 把记录拆分到m_stsignallog结构中。
  bool SplitBuffer(const char *strBuffer);

  // 把m_stsignallog结构中的值更新到T_SIGNALDATA表中。
  long InsertTable();
};


#endif
