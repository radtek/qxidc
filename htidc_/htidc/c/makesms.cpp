#include "_public.h"

CLogFile logfile;
CCmdStr cmdstr;

FILE *datefile,*out; 
char mobileno[11];
int ii;

int main( int argc,char *argv[])
{
  if(argc!=4)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/makesms logfilename clientname dwid ywid mobilenostr\n\n");
    printf("Sample:/htidc/htidc/bin/procctl 120 /htidc/htidc/bin/makesms /tmp/htidc/log/makesms.log /tmp/mobile.xml /tmp/smssend\n");
     return -1;
   }
sprintf(outputpath,300,"%s/fromtcp/%s",SMSCFG.m_smsdata,dwid
  char strCurTime[20];
  char filename[300];
  
  memset(strCurTime,0,sizeof(strCurTime));
  memset(filename,0,sizeof(filename));
  int aa=0; 
  int ii=0;
  
  datefile=fopen(argv[2],"r");
  if(datefile==NULL)
   {
    logfile.Write("logfile.Open(%s) failed.\n",argv[2]);
    return -1;
    }
  
  if(logfile.Open(argv[1],"a+")==FALSE)
   {
    logfile.Write("logfile.Open(%s) failed.\n",argv[1]);
    return -1;
   }
   
    
  while(FGETS(mobileno,13,datefile)) 
  {

   if(ii%10000==0)
    {
     if(ii!=0)
     	{
        fprintf(out,"</data>\n");
        logfile.Write("dminingtxt end  111\n");
        fclose(out);
      }     
     LocalTime( strCurTime, "yyyymmddhh24miss" );
     snprintf( filename, 300, "%s/1_fromtcp_%s_S0008_00%d.xml",argv[3],strCurTime,aa++ );

     out=fopen(filename,"w+");
     fprintf(out,"<data>\n");
    }

    logfile.Write("dminingtxt beginning.\n");
    fprintf(out,"<serialno>S00080099%s%d</serialno><clientname>S0008</clientname><sendport>9559898</sendport><mobileno>%s</mobileno><content>�����ٿ��㶫������˾2011����ۺ�����������֪ͨ��ʱ�䣺8��22��9:00ʱ������ǰ15���ӵ�����</content><mesgtype>1</mesgtype><srr>1</srr><crttime>%s</crttime><endl/>\n",strCurTime,ii,mobileno,strCurTime); 
    ii++;
  }
   fprintf(out,"</data>\n");
   logfile.Write("dminingtxt end\n");
   logfile.Close();
   fclose(datefile); 
   fclose(out);
   return 0;
}
 
 
    







