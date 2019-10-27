#include "_public.h"
#include "_oracle.h"

CFile  file;
CLogFile       logfile;
CProgramActive ProgramActive;
CCmdStr        CmdStr1,CmdStr2,CmdStr3;
connection     connsrc;

char buffer[4001];

struct st_TC
{
  char yyyy[11];
  char no[11];
  char zhtno[51];
  char tcename[21];
  char tccname[21];
  char number[11];
  char pro[101];
  char city[31];
  char city1[11];
  char citylevel[11];
  char mm[11];
  char dd[11];
  char hh[11];
  char landwind[11];
  char landp[11];
} tc1,tc2,tc3;

void splittocmd1();
void splittocmd2();
void splittocmd3();


int main( int argc,char *argv[])
{
  if(argc!=4 )
  {
    printf("Using: /htidc/htidc/bin/pcsvfile filename logfilename strconn\n");
    printf("本程序读取csv文件，并入库到数据库\n");
    printf("本程序用来处理郑群峰的台风基本信息的入库\n");
   
    return -1; 
  }
  

  // 打开日志文件 
  if (logfile.Open(argv[2],"a+") == FALSE) 
  {   
    printf("logfile.Open(%s) failed.\n",argv[1]); 
    return -1; 
   }
  
  logfile.SetAlarmOpt("pcsvfile");

  file.OpenForRead(argv[1],"r");

  memset(buffer,0,sizeof(buffer));
  memset(&tc1,0,sizeof(struct st_TC));
  memset(&tc2,0,sizeof(struct st_TC));
  memset(&tc3,0,sizeof(struct st_TC));
  

  // 连接数据源数据库
  if (connsrc.connecttodb(argv[3],TRUE) != 0)
  {
    logfile.Write("connsrc.connecttodb(%s) failed\n",argv[3]); return -1;
  }
  
  // 准备获取数据的SQL语句，绑定输出变量
  sqlstatement stmt;
  stmt.connect(&connsrc);
  
  logfile.Write("begin process csv.\n");
  int ii=0;
  
  while(TRUE) 
  {
    if(file.FFGETS(buffer,4000)==FALSE) return FALSE;
    Trim(buffer);
    splittocmd1();	
    if(atoi(tc1.number)==1)
    {
      if(strlen(tc1.mm)!=0 && strlen(tc1.dd)!=0 && strlen(tc1.hh)!=0)
      {
        stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=:3 , landp=:4  where tcno='%d%02d' ",atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),atoi(tc1.yyyy),atoi(tc1.no));
        stmt.bindin(1,tc1.pro,100);
        stmt.bindin(2,tc1.citylevel,10);
        stmt.bindin(3,tc1.landwind,10);
        stmt.bindin(4,tc1.landp,10);
        logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s, landp=%s  where tcno='%d%02d'.\n",tc1.pro,tc1.citylevel,atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),tc1.landwind,tc1.landp,atoi(tc1.yyyy),atoi(tc1.no));
      }

      else
      {
        stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landwind=:3 , landp=:4  where tcno='%d%02d' ",atoi(tc1.yyyy),atoi(tc1.no));
        stmt.bindin(1,tc1.pro,100);
        stmt.bindin(2,tc1.citylevel,10);
        stmt.bindin(3,tc1.landwind,10);
        stmt.bindin(4,tc1.landp,10);
        logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s, landp=%s  where tcno='%d%02d'.\n",tc1.pro,tc1.citylevel,atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),tc1.landwind,tc1.landp,atoi(tc1.yyyy),atoi(tc1.no));
      } 

      if(stmt.execute() != 0)
      {
        logfile.Write(" execute() failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
      }
	  
      if(stmt.cda.rpc == 0)
      {
        logfile.Write(" rpc==0.\n%s\n"); 
        return FALSE;
      }
	  
      connsrc.commitwork(); 
      memset(buffer,0,sizeof(buffer));
      ii++;
   }

  if(atoi(tc1.number)==2)
  {   
    memset(buffer,0,sizeof(buffer));

    if(file.FFGETS(buffer,4000)==FALSE) return FALSE;
    Trim(buffer);
    splittocmd2();
	
    if(atoi(tc1.city1)==1)
    {
     
     if(strlen(tc1.mm)!=0 && strlen(tc1.dd)!=0 && strlen(tc1.hh)!=0)
     {
       stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=:5 , landp=:6  where tcno='%d%02d' ",atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),atoi(tc1.yyyy),atoi(tc1.no));
       stmt.bindin(1,tc1.pro,100);
       stmt.bindin(2,tc1.citylevel,10);
       stmt.bindin(3,tc2.pro,100);
       stmt.bindin(4,tc2.citylevel,10);		  
       stmt.bindin(5,tc1.landwind,10);
       stmt.bindin(6,tc1.landp,10);
       logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s, landp=%s  where tcno='%d%02d' ",tc1.pro,tc1.citylevel,tc2.pro,tc2.citylevel, atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),tc1.landwind,tc1.landp, atoi(tc1.yyyy),atoi(tc1.no));
     }

    else
    {
      stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landwind=:5 , landp=:6  where tcno='%d%02d' ",atoi(tc1.yyyy),atoi(tc1.no));
      stmt.bindin(1,tc1.pro,100);
      stmt.bindin(2,tc1.citylevel,10);
      stmt.bindin(3,tc2.pro,100);
      stmt.bindin(4,tc2.citylevel,10);	
      stmt.bindin(5,tc1.landwind,10);
      stmt.bindin(6,tc1.landp,10);
      logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s, landwind=%s, landp=%s  where tcno='%d%02d' ",tc1.pro,tc1.citylevel,tc2.pro,tc2.citylevel,tc1.landwind,tc1.landp,atoi(tc1.yyyy),atoi(tc1.no));	
     } 

    if(stmt.execute() != 0)
    {
       logfile.Write(" execute() failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
     }
	    
    if(stmt.cda.rpc == 0)
    {
      logfile.Write(" rpc==0.\n%s\n"); 
      return FALSE;
    }

    connsrc.commitwork(); 
    memset(buffer,0,sizeof(buffer));
    ii++;
		
  }
	  
  else
  {
    if(strlen(tc2.mm)!=0 && strlen(tc2.dd)!=0 && strlen(tc2.hh)!=0)
    {
      stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=:5 , landp=:6  where tcno='%d%02d' ",atoi(tc2.dd),atoi(tc2.mm),atoi(tc2.yyyy),atoi(tc2.hh),atoi(tc2.yyyy),atoi(tc2.no));
      stmt.bindin(1,tc2.pro,100);
      stmt.bindin(2,tc2.citylevel,10);
      stmt.bindin(3,tc1.pro,100);
      stmt.bindin(4,tc1.citylevel,10);		  
      stmt.bindin(5,tc2.landwind,10);
      stmt.bindin(6,tc2.landp,10);
      logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s, landp=%s  where tcno='%d%02d' ",tc2.pro,tc2.citylevel,tc1.pro,tc1.citylevel, atoi(tc2.dd),atoi(tc2.mm),atoi(tc2.yyyy),atoi(tc2.hh),tc2.landwind,tc2.landp,atoi(tc2.yyyy),atoi(tc2.no));
    }

    else
    {
      stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landwind=:5 , landp=:6  where tcno='%d%02d' ",atoi(tc2.yyyy),atoi(tc2.no));
      stmt.bindin(1,tc2.pro,100);
      stmt.bindin(2,tc2.citylevel,10);
      stmt.bindin(3,tc1.pro,100);
      stmt.bindin(4,tc1.citylevel,10);	
      stmt.bindin(5,tc2.landwind,10);
      stmt.bindin(6,tc2.landp,10);
      logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s, landwind=%s, landp=%s  where tcno='%d%02d' ",tc2.pro,tc2.citylevel,tc1.pro,tc1.citylevel,tc2.landwind,tc2.landp,atoi(tc2.yyyy),atoi(tc2.no));
     } 

    if (stmt.execute() != 0)
    {
       logfile.Write(" execute() failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
     }
	    
    if (stmt.cda.rpc == 0)
    {
      logfile.Write(" rpc==0.\n%s\n"); 
      return FALSE;
    }
	    
    connsrc.commitwork(); 
    memset(buffer,0,sizeof(buffer));
    ii++;
  
    }
	 
  }
   
  if(atoi(tc1.number)==3)
  { 
      memset(buffer,0,sizeof(buffer));
      if(file.FFGETS(buffer,4000)==FALSE) return FALSE;
      Trim(buffer);
      splittocmd2();	

      memset(buffer,0,sizeof(buffer));
      if(file.FFGETS(buffer,4000)==FALSE) return FALSE;
      Trim(buffer);
      splittocmd3();
	    
    if(atoi(tc1.city1)==1)
    {

    if(strlen(tc1.mm)!=0 && strlen(tc1.dd)!=0 && strlen(tc1.hh)!=0)
    {
      stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landcity3=:5,city3levle=:6,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=:7 , landp=:8  where tcno='%d%02d' ",atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),atoi(tc1.yyyy),atoi(tc1.no));
      stmt.bindin(1,tc1.pro,100);
      stmt.bindin(2,tc1.citylevel,10);
      stmt.bindin(3,tc2.pro,100);
      stmt.bindin(4,tc2.citylevel,10);	
      stmt.bindin(5,tc3.pro,100);
      stmt.bindin(6,tc3.citylevel,10);		  
      stmt.bindin(7,tc1.landwind,10);
      stmt.bindin(8,tc1.landp,10);
      logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landcity3=%s,city3levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s , landp=%s  where tcno='%d%02d' ",tc1.pro,tc1.citylevel,tc2.pro,tc2.citylevel,tc3.pro,tc3.citylevel, atoi(tc1.dd),atoi(tc1.mm),atoi(tc1.yyyy),atoi(tc1.hh),tc1.landwind,tc1.landp,atoi(tc1.yyyy),atoi(tc1.no));
    }

    else
    {
      stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landcity3=:5,city3levle=:6,landwind=:7 , landp=:8  where tcno='%d%02d' ",atoi(tc1.yyyy),atoi(tc1.no));
      stmt.bindin(1,tc1.pro,100);
      stmt.bindin(2,tc1.citylevel,10);
      stmt.bindin(3,tc2.pro,100);
      stmt.bindin(4,tc2.citylevel,10);
      stmt.bindin(5,tc3.pro,100);
      stmt.bindin(6,tc3.citylevel,10);		  
      stmt.bindin(7,tc1.landwind,10);
      stmt.bindin(8,tc1.landp,10);
      logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landcity3=%s,city3levle=%s,landwind=%s , landp=%s  where tcno='%d%02d' ",tc1.pro,tc1.citylevel,tc2.pro,tc2.citylevel,tc3.pro,tc3.citylevel,tc1.landwind,tc1.landp,atoi(tc1.yyyy),atoi(tc1.no));
    } 

    if (stmt.execute() != 0)
    {
      logfile.Write(" execute() failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
    }
	    
    if (stmt.cda.rpc == 0)
    {
      logfile.Write(" rpc==0.\n%s\n"); 
      return FALSE;
    }
	    
    connsrc.commitwork(); 
    memset(buffer,0,sizeof(buffer));
    ii++; 
	
    } 
    
    else
    { 
      if(atoi(tc2.city1)==1)
      {
        if(strlen(tc2.mm)!=0 && strlen(tc2.dd)!=0 && strlen(tc2.hh)!=0)
        {
          stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landcity3=:5,city3levle=:6,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=:7 , landp=:8  where tcno='%d%02d' ",atoi(tc2.dd),atoi(tc2.mm),atoi(tc2.yyyy),atoi(tc2.hh),atoi(tc2.yyyy),atoi(tc2.no));
          stmt.bindin(1,tc2.pro,100);
          stmt.bindin(2,tc2.citylevel,10);
          stmt.bindin(3,tc1.pro,100);
          stmt.bindin(4,tc1.citylevel,10);		  
          stmt.bindin(5,tc3.pro,100);
          stmt.bindin(6,tc3.citylevel,10);
          stmt.bindin(7,tc2.landwind,10);
          stmt.bindin(8,tc2.landp,10);
          logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landcity3=%s,city3levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s , landp=%s  where tcno='%d%02d' ",tc2.pro,tc2.citylevel,tc1.pro,tc1.citylevel,tc3.pro,tc3.citylevel, atoi(tc2.dd),atoi(tc2.mm),atoi(tc2.yyyy),atoi(tc2.hh),tc2.landwind,tc2.landp,atoi(tc2.yyyy),atoi(tc2.no));
        }

        else
        {
          stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landcity3=:5,city3levle=:6,landwind=:7 , landp=:8  where tcno='%d%02d' ",atoi(tc2.yyyy),atoi(tc2.no));
          stmt.bindin(1,tc2.pro,100);
          stmt.bindin(2,tc2.citylevel,10);
          stmt.bindin(3,tc1.pro,100);
          stmt.bindin(4,tc1.citylevel,10);	
          stmt.bindin(5,tc3.pro,100);
          stmt.bindin(6,tc3.citylevel,10);
          stmt.bindin(7,tc2.landwind,10);
          stmt.bindin(8,tc2.landp,10);
          logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landcity3=%s,city3levle=%s, landwind=%s , landp=%s  where tcno='%d%02d' ",tc2.pro,tc2.citylevel,tc1.pro,tc1.citylevel,tc3.pro,tc3.citylevel,tc2.landwind,tc2.landp,atoi(tc2.yyyy),atoi(tc2.no));
         } 

         if (stmt.execute() != 0)
         {
            logfile.Write(" execute() failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
          }
	        
         if (stmt.cda.rpc == 0)
         {
           logfile.Write(" rpc==0.\n%s\n"); 
           return FALSE;
         }
	        
         connsrc.commitwork(); 
         memset(buffer,0,sizeof(buffer));
	 ii++;  
    }	
 
    else
    {
      if(atoi(tc3.city1)==1)
      {
        if(strlen(tc3.mm)!=0 && strlen(tc3.dd)!=0 && strlen(tc3.hh)!=0)
        {
          stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landcity3=:5,city3levle=:6,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=:7 , landp=:8  where tcno='%d%02d' ",atoi(tc3.dd),atoi(tc3.mm),atoi(tc3.yyyy),atoi(tc3.hh),atoi(tc3.yyyy),atoi(tc3.no));
          stmt.bindin(1,tc3.pro,100);
          stmt.bindin(2,tc3.citylevel,10);
          stmt.bindin(3,tc1.pro,100);
          stmt.bindin(4,tc1.citylevel,10);		  
          stmt.bindin(5,tc2.pro,100);
          stmt.bindin(6,tc2.citylevel,10);
          stmt.bindin(7,tc3.landwind,10);
          stmt.bindin(8,tc3.landp,10);
          logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landcity3=%s,city3levle=%s,landtime=to_date('%02d-%02d-%d %02d:00:00','dd-mm-yyyy hh24:mi:ss'), landwind=%s , landp=%s  where tcno='%d%02d' ",tc3.pro,tc3.citylevel,tc1.pro,tc1.citylevel,tc2.pro,tc2.citylevel, atoi(tc3.dd),atoi(tc3.mm),atoi(tc3.yyyy),atoi(tc3.hh),tc3.landwind,tc3.landp,atoi(tc3.yyyy),atoi(tc3.no));
        }

        else
        {
          stmt.prepare("update  t_tsinfo set landcity1=:1,city1levle=:2,landcity2=:3,city2levle=:4,landcity3=:5,city3levle=:6,landwind=:7 , landp=:8  where tcno='%d%02d' ",atoi(tc3.yyyy),atoi(tc3.no));
          stmt.bindin(1,tc3.pro,100);
          stmt.bindin(2,tc3.citylevel,10);
          stmt.bindin(3,tc1.pro,100);
          stmt.bindin(4,tc1.citylevel,10);	
          stmt.bindin(5,tc2.pro,100);
          stmt.bindin(6,tc2.citylevel,10);
          stmt.bindin(7,tc3.landwind,10);
          stmt.bindin(8,tc3.landp,10);
          logfile.Write("update  t_tsinfo set landcity1=%s,city1levle=%s,landcity2=%s,city2levle=%s,landcity3=%s,city3levle=%s, landwind=%s , landp=%s  where tcno='%d%02d' ",tc3.pro,tc3.citylevel,tc1.pro,tc1.citylevel,tc2.pro,tc2.citylevel,tc3.landwind,tc3.landp, atoi(tc3.yyyy),atoi(tc3.no));
        } 

        if(stmt.execute() != 0)
        {
          logfile.Write(" execute() failed.\n%s\n",stmt.cda.message); return stmt.cda.rc;
         }
	        
        if(stmt.cda.rpc == 0)
        {
          logfile.Write(" rpc==0.\n%s\n"); 
          return FALSE;
        }
	        
        connsrc.commitwork(); 
        memset(buffer,0,sizeof(buffer));
        ii++; 
       }
      }
     }
   }    
  }

  logfile.Write(" process ok.\n");
  logfile.Write(" %d ...\n",ii);	  	
  return 0;

}

void splittocmd1()
{
  CmdStr1.SplitToCmd(buffer,",");

  CmdStr1.GetValue(0,tc1.yyyy,10);
  CmdStr1.GetValue(1,tc1.no,10);
  CmdStr1.GetValue(2,tc1.zhtno,10);
  CmdStr1.GetValue(3,tc1.tcename,20);	
  CmdStr1.GetValue(4,tc1.tccname,20);
  CmdStr1.GetValue(5,tc1.number,10);
  CmdStr1.GetValue(6,tc1.pro,100);
  CmdStr1.GetValue(7,tc1.city,30);
  strcat(tc1.pro,tc1.city);
  CmdStr1.GetValue(8,tc1.city1,10);
  CmdStr1.GetValue(9,tc1.citylevel,10);
  CmdStr1.GetValue(10,tc1.mm,10);
  CmdStr1.GetValue(11,tc1.dd,10);
  CmdStr1.GetValue(12,tc1.hh,10);
  CmdStr1.GetValue(13,tc1.landwind,10);
  CmdStr1.GetValue(14,tc1.landp,10);

}

void splittocmd2()
{
  CmdStr2.SplitToCmd(buffer,",");
	
  CmdStr2.GetValue(0,tc2.yyyy,10);
  CmdStr2.GetValue(1,tc2.no,10);
  CmdStr2.GetValue(2,tc2.zhtno,10);
  CmdStr2.GetValue(3,tc2.tcename,20);	
  CmdStr2.GetValue(4,tc2.tccname,20);
  CmdStr2.GetValue(5,tc2.number,10);
  CmdStr2.GetValue(6,tc2.pro,100);
  CmdStr2.GetValue(7,tc2.city,30);
  strcat(tc2.pro,tc2.city);
  CmdStr2.GetValue(8,tc2.city1,10);
  CmdStr2.GetValue(9,tc2.citylevel,10);
  CmdStr2.GetValue(10,tc2.mm,10);
  CmdStr2.GetValue(11,tc2.dd,10);
  CmdStr2.GetValue(12,tc2.hh,10);
  CmdStr2.GetValue(13,tc2.landwind,10);
  CmdStr2.GetValue(14,tc2.landp,10);

}

void splittocmd3()
{
  CmdStr3.SplitToCmd(buffer,",");
	
  CmdStr3.GetValue(0,tc3.yyyy,10);
  CmdStr3.GetValue(1,tc3.no,10);
  CmdStr3.GetValue(2,tc3.zhtno,10);
  CmdStr3.GetValue(3,tc3.tcename,20);	
  CmdStr3.GetValue(4,tc3.tccname,20);
  CmdStr3.GetValue(5,tc3.number,10);
  CmdStr3.GetValue(6,tc3.pro,100);
  CmdStr3.GetValue(7,tc3.city,30);
  strcat(tc3.pro,tc3.city);
  CmdStr3.GetValue(8,tc3.city1,10);
  CmdStr3.GetValue(9,tc3.citylevel,10);
  CmdStr3.GetValue(10,tc3.mm,10);
  CmdStr3.GetValue(11,tc3.dd,10);
  CmdStr3.GetValue(12,tc3.hh,10);
  CmdStr3.GetValue(13,tc3.landwind,10);
  CmdStr3.GetValue(14,tc3.landp,10);

}
