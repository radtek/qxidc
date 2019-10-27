#########################################
# 请注意，编译时，确定以下几点：
# 1、已安装oracle的客户端或oracle服务端程序，并且必须安装好OCI软件包
# 2、设置好ORACLE_HOME环境变量
#
#########################################

# oracle头文件路径
ORAINCL = -I$(ORACLE_HOME)/rdbms/public

# oracle库文件路径
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle的oci库
ORALIBS = -lclntsh

CC=g++

# CFLAGS = -O2
# CFLAGS = -O2 -Wall

CFLAGS = -g -Wno-write-strings -Wno-unused-variable


all: libftp.a demo11 demo12 demo13 demo14 demo18 ftpgetfiles deletefiles deletetables tcpputfiles\
     tcpfileserver tcpgetfiles exptables ftpputfiles httpclient

libftp.a: ftplib.c
	gcc -c -o libftp.a ftplib.c

demo11: demo11.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o demo11 demo11.cpp _public.cpp -lm -lc

demo12: demo12.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o demo12 demo12.cpp _public.cpp -lm -lc

demo13: demo13.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o demo13 demo13.cpp _public.cpp -lm -lc

demo14: demo14.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o demo14 demo14.cpp _public.cpp -lm -lc

demo18: demo18.cpp _ftp.h _ftp.cpp  libftp.a
	$(CC) $(CFLAGS) -o demo18 demo18.cpp _public.cpp _ftp.cpp libftp.a -lm -lc

tcpputfiles: tcpputfiles.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o tcpputfiles tcpputfiles.cpp _public.cpp -lm -lc
	cp tcpputfiles ../bin/.

tcpgetfiles: tcpgetfiles.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o tcpgetfiles tcpgetfiles.cpp _public.cpp -lm -lc
	cp tcpgetfiles ../bin/.

tcpfileserver: tcpfileserver.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o tcpfileserver tcpfileserver.cpp _public.cpp -lm -lc
	cp tcpfileserver ../bin/.

ftpputfiles: ftpputfiles.cpp _ftp.h _ftp.cpp  libftp.a
	$(CC) $(CFLAGS) -o ftpputfiles ftpputfiles.cpp _public.cpp _ftp.cpp libftp.a -lm -lc
	cp ftpputfiles ../bin/.

ftpgetfiles: ftpgetfiles.cpp _ftp.h _ftp.cpp  libftp.a
	$(CC) $(CFLAGS) -o ftpgetfiles ftpgetfiles.cpp _public.cpp _ftp.cpp libftp.a -lm -lc
	cp ftpgetfiles ../bin/.

deletefiles: deletefiles.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o deletefiles deletefiles.cpp _public.cpp -lm -lc
	cp deletefiles ../bin/.

httpclient: httpclient.cpp _public.h _public.cpp
	$(CC) $(CFLAGS) -o httpclient httpclient.cpp _public.cpp -lm -lc
	cp httpclient ../bin/.

deletetables: deletetables.cpp _public.h  _public.cpp _ooci.h _ooci.cpp
	$(CC) $(CFLAGS) -o deletetables deletetables.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _public.cpp _ooci.cpp -lm -lc
	cp deletetables ../bin/.

exptables: exptables.cpp _public.h  _public.cpp _ooci.h _ooci.cpp
	$(CC) $(CFLAGS) -o exptables exptables.cpp  $(ORAINCL) $(ORALIB) $(ORALIBS) _public.cpp _ooci.cpp -lm -lc
	cp exptables ../bin/.


clean:
	rm -rf libftp.a demo11 demo12 demo13 demo14 demo18 ftpgetfiles deletefiles deletetables
	rm -rf tcpputfiles tcpgetfiles tcpfileserver exptables ftpputfiles httpclient
