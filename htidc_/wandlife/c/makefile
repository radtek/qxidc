# oracle数据库的安装目录
#ORACLE_HOME=/home/oracle/oracle/product/10.2.0/db_1
# oracle头文件路径
ORAINCL = -I/htidc/htidc/c -I$(ORACLE_HOME)/rdbms/public -I.
# oracle库文件路径
ORALIB = -L$(ORACLE_HOME)/lib -L/htidc/htidc/c -L.
# oracle库
ORALIBS = -lclntsh

CFLAG = -O2 -Wall 
CFLAG = -O2
CFLAG = -g -Wall 

all: 	palertfile pindexfile wgettwstar


crtqxfwfile: crtqxfwfile.cpp  wandlife.h wandlife.cpp
	g++ $(CFLAG) -o crtqxfwfile crtqxfwfile.cpp /htidc/htidc/c/_public.cpp  /htidc/htidc/c/_oracle.cpp \
	wandlife.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) -L. -lm -lc
	cp -f crtwandlife ../bin/.

palertfile: palertfile.cpp  wandlife.h wandlife.cpp
	g++ $(CFLAG) -o palertfile palertfile.cpp /htidc/htidc/c/_public.cpp  /htidc/htidc/c/_oracle.cpp \
	wandlife.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) -L. -lm -lc
	cp -f palertfile ../bin/.

pindexfile: pindexfile.cpp  wandlife.h wandlife.cpp
	g++ $(CFLAG) -o pindexfile pindexfile.cpp /htidc/htidc/c/_public.cpp  /htidc/htidc/c/_oracle.cpp \
	wandlife.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) -L. -lm -lc
	cp -f pindexfile ../bin/.

crtcodefile: crtcodefile.cpp  qxfw.h qxfw.cpp
	g++ $(CFLAG) -o crtcodefile crtcodefile.cpp /htidc/htidc/c/_public.cpp  /htidc/htidc/c/_oracle.cpp \
	qxfw.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) -L. -lm -lc
	cp -f crtcodefile ../bin/.

wgettwstar: wgettwstar.cpp
	g++ $(CFLAG) -o wgettwstar wgettwstar.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc 
	cp -f wgettwstar ../bin/.

httpclient: httpclient.cpp
	g++ $(CFLAG) -o httpclient httpclient.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc 
	cp -f httpclient ../bin/.

analhttpfiles_weibo: analhttpfiles_weibo.cpp  qxfw.h qxfw.cpp
	g++ $(CFLAG) -o analhttpfiles_weibo analhttpfiles_weibo.cpp /htidc/htidc/c/_public.cpp  /htidc/htidc/c/_oracle.cpp \
	qxfw.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) -L. -lm -lc
	cp -f analhttpfiles_weibo ../bin/.

qxfwserver: qxfwserver.cpp
	g++ $(CFLAG) -o qxfwserver qxfwserver.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc 
	cp -f qxfwserver ../bin/.

qxfwclient: qxfwclient.cpp
	g++ $(CFLAG) -o qxfwclient qxfwclient.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc 
	cp -f qxfwclient ../bin/.

clean:
	rm -f *.o *.a 
	rm -f palertfile pindexfile wgettwstar
