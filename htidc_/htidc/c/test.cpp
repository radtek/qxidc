#include "_public.h"

CFile  FileStd;

int main()
{
  if( FileStd.OpenForWrite("/tmp/getdata.sh","w+") == FALSE) printf("open failed\n");

  int kk = 10080;
  for (int ii=1; ii<2500; ii++)
  {
     kk = kk-5; 

     FileStd.Fprintf("/htidc/qxidc/bin/getRACAwst4Prov \"<logfilename>/log/hdqx/getRACAwst4Prov_his.log</logfilename><stdpath>/qxdata/hdqx/sdata/zdzxmlfile</stdpath><username>gmcrgz</username><password>guangz123</password><lcstdname>LCOBTDATA</lcstdname><lcobtmath>59284,G1024,G1025,G1026,G1027,G1043,G1058,G1059,G1073,G1081,G3104,G3159,G3246,G3251,G3252,G3253,G3254,G3255,G3256,G3257,G3258,G3259,G3260,G3325,G3411,G3412,G3413,G3414,G3415,G3416,G3417,G3418,G3419,G3432,G3440,G3441,G9483,G9484,G9485,G9486,G3109,G3119,G3126,G3409,G3420,G3452,G9482</lcobtmath><gdstdname>GDOBTDATA</gdstdname><gdvisstdname>GDOBTVIS</gdvisstdname><timetvl>-%d</timetvl>\"\n",kk);
     FileStd.Fprintf("/htidc/qxidc/bin/getRACAwst4Iiiii \"<logfilename>/log/hdqx/getRACAwst4Iiiii_his.log</logfilename><stdpath>/qxdata/hdqx/sdata/zdzxmlfile</stdpath><username>gmcrgz</username><password>guangz123</password><lcstdname>LCOBTDATA</lcstdname><obtids>G9482,G9483,G9484,G9485,G9486</obtids><gdstdname>GDOBTDATA</gdstdname></gdvisstdname>GDOBTVIS</gdvisstdname><timetvl>-%d</timetvl>\"\n",kk);

   if (kk==0) break;
  }

  FileStd.Fclose();

  return 0;
}
