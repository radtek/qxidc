
ORAINCL = -I$(ORACLE_HOME)/rdbms/public

ORALIB =  -L$(ORACLE_HOME)/lib -L.

ORALIBS = -lclntsh

#########################################

CFLAGS = -O2 
#CFLAGS = -Wall
#CFLAGS = -g -Wall  -Wno-write-strings


all: crtsufdata libftp.a demo18

libftp.a: ftplib.c
	gcc -c -o libftp.a ftplib.c

demo18: demo18.cpp _ftp.h _ftp.cpp libftp.a
	g++ $(CFLAGS) -o demo18 demo18.cpp _public.cpp _ftp.cpp libftp.a -lm -lc


crtsufdata: crtsufdata.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o crtsufdata crtsufdata.cpp _public.cpp
	cp crtsufdata ../bin/.
clean:
	rm -rf crtsurfdata
