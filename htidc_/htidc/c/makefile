#########################################
# mysql开发相关文件存放目录
MYSQL_HOME= /htidc/mysql/64

# mysql头文件存放路径
MYSQLINCL = -I$(MYSQL_HOME)/include

# mysql库文件保存路径
MYSQLLIB = -L$(MYSQL_HOME)/lib

#oracle头文件路径
ORAINCL = -I$(ORACLE_HOME)/rdbms/public 

#oracle库文件路径
ORALIB =  -L$(ORACLE_HOME)/lib -L/opt/PostgresPlus/9.0SS/lib -L/htidc/htidc/c -L.

ORALIBS = -lclntsh 
#ORALIBS = -lpq

#CFLAGS = -O2
CFLAGS = -O2 -Wall

CFLAGS = -g -Wall


# 注意，dminingsqlserver和dminingmysql没有放在这里，因为在很多服务器上没有编译环境
# 如有需要，请手工make dminingsqlserver dminingmysql
all:	libftp.a procctl dbcp checkprogram deletefiles deletearchive gzipfiles synctable synctable_update synctable_update1\
	tablecp1 tablecp2 tableupt cpdb managedict dfiletodb diskstat rinetd wfiletodb synctable_increment synctable_increment1\
        backupdbuser backupapp ftpputfiles ftpgetfiles dminingoracle dminingtxt crtsynctrigger clearsynclog pexptable\
	uptdata backuptables counttable managesynonym crtview managetable bissuedata manageview synctable_update2\
	gettabdata instabdata crttable cpustat dbfilestat sessionstat alarmtotab migratetable deletetable\
        fileclient fileserver drawcontour drawwind crtsequence countformon montable httpclient\
        synctable_append gzipfiles1 killidlesession expdbfiles pcsvfiles extdataalarm bzipfiles\
	expdbfiles1 synctable_update3 syncoracletomysql synctable_increment2 synctable_increment3 ftpdelfiles\
	copyfile

install:
	rm -rf /htidc/htidc/bin
	mkdir  /htidc/htidc/bin
	cd /htidc/freetds-0.82
	./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
	make
	make install

oracle:
	cp -f _oracle.cpp.fororacle  _oracle.cpp
	cp -f _oracle.h.fororacle    _oracle.h

pg:
	cp -f _pgdb.cpp _oracle.cpp
	cp -f _pgdb.h _oracle.h

mysql:
	cp -f _mysql.cpp.formysql _oracle.cpp
	cp -f _mysql.h.formysql _oracle.h

libftp.a: ftplib.c
	cc -c -o libftp.a ftplib.c

fileclient: fileclient.cpp
	g++ $(CFLAGS) -o fileclient fileclient.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc
	cp -f fileclient ../bin/.

fileserver: fileserver.cpp
	g++ $(CFLAGS) -o fileserver fileserver.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc
	cp -f fileserver ../bin/.

fileserver_test: fileserver_test.cpp
	g++ $(CFLAGS) -o fileserver_test fileserver_test.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc
	cp -f fileserver_test ../bin/.

test: test.cpp
	g++ $(CFLAGS) -o test test.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc

httpclient: httpclient.cpp
	g++ $(CFLAGS) -o httpclient httpclient.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc
	cp -f httpclient ../bin/.

wgetclient: wgetclient.cpp
	g++ $(CFLAGS) -o wgetclient wgetclient.cpp /htidc/htidc/c/_public.cpp \
	-I/htidc/htidc/c -L. -lm -lc
	cp -f wgetclient ../bin/.

procctl: procctl.cpp
	g++ $(CFLAGS) -o procctl procctl.cpp -lm -lc
	cp -f procctl ../bin/.

killidlesession: killidlesession.cpp _public.h _public.cpp 
	g++ $(CFLAGS) -o killidlesession killidlesession.cpp _public.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _oracle.cpp -lm -lc
	cp -f killidlesession ../bin/.

dbcp: dbcp.cpp _public.h _public.cpp 
	g++ $(CFLAGS) -o dbcp dbcp.cpp _public.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _oracle.cpp -lm -lc
	cp -f dbcp ../bin/.

cpdb: cpdb.cpp _public.h _public.cpp 
	g++ $(CFLAGS) -o cpdb cpdb.cpp _public.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _oracle.cpp -lm -lc
	cp -f cpdb ../bin/.

checkprogram: checkprogram.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o checkprogram checkprogram.cpp _public.cpp -lm -lc
	cp -f checkprogram ../bin/.

deletefiles: deletefiles.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o deletefiles deletefiles.cpp _public.cpp -lm -lc
	cp -f deletefiles ../bin/.

splitfile: splitfile.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o splitfile splitfile.cpp _public.cpp -lm -lc
	cp -f splitfile ../bin/.

mergefile: mergefile.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o mergefile mergefile.cpp _public.cpp -lm -lc
	cp -f mergefile ../bin/.

deletearchive: deletearchive.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o deletearchive deletearchive.cpp _public.cpp -lm -lc
	cp -f deletearchive ../bin/.

gzipfiles: gzipfiles.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o gzipfiles gzipfiles.cpp _public.cpp -lm -lc
	cp -f gzipfiles ../bin/.

bzipfiles: bzipfiles.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o bzipfiles bzipfiles.cpp _public.cpp -lm -lc
	cp -f bzipfiles ../bin/.

gzipfiles1: gzipfiles1.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o gzipfiles1 gzipfiles1.cpp _public.cpp -lm -lc
	cp -f gzipfiles1 ../bin/.

synctable: synctable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable synctable.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable ../bin/.

synctable_update2: synctable_update2.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_update2 synctable_update2.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_update2 ../bin/.

synctable_update: synctable_update.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_update synctable_update.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_update ../bin/.

synctable_update1: synctable_update1.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_update1 synctable_update1.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_update1 ../bin/.

synctable_update3: synctable_update3.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_update3 synctable_update3.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_update3 ../bin/.

synctable_increment: synctable_increment.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_increment synctable_increment.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_increment ../bin/.

synctable_increment1: synctable_increment1.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_increment1 synctable_increment1.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_increment1 ../bin/.

synctable_increment2: synctable_increment2.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_increment2 synctable_increment2.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_increment2 ../bin/.

synctable_increment3: synctable_increment3.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_increment3 synctable_increment3.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_increment3 ../bin/.

synctable_append: synctable_append.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctable_append synctable_append.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctable_append ../bin/.

crtsynctrigger: crtsynctrigger.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o crtsynctrigger crtsynctrigger.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f crtsynctrigger ../bin/.

clearsynclog: clearsynclog.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o clearsynclog clearsynclog.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f clearsynclog ../bin/.

tablecp1: tablecp1.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o tablecp1 tablecp1.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f tablecp1 ../bin/.

tablecp2: tablecp2.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o tablecp2 tablecp2.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f tablecp2 ../bin/.

tableupt: tableupt.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o tableupt tableupt.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f tableupt ../bin/.

managedict: managedict.cpp _public.h _public.cpp idcapp.h _oracle.cpp idcapp.cpp
	g++ $(CFLAGS) -o managedict managedict.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f managedict ../bin/.

ftpputfiles: ftpputfiles.cpp _public.h _public.cpp ftp.h ftp.cpp
	g++ $(CFLAGS) -o ftpputfiles ftpputfiles.cpp _public.cpp ftp.cpp libftp.a -lm -lc
	cp -f ftpputfiles ../bin/.

ftpgetfiles: ftpgetfiles.cpp _public.h _public.cpp idcapp.h idcapp.cpp ftp.h ftp.cpp 
	g++ $(CFLAGS) -o ftpgetfiles ftpgetfiles.cpp _oracle.cpp _public.cpp ftp.cpp idcapp.cpp libftp.a \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f ftpgetfiles ../bin/.

ftpgetfiles1: ftpgetfiles1.cpp _public.h _public.cpp ftp.h ftp.cpp
	g++ $(CFLAGS) -o ftpgetfiles1 ftpgetfiles1.cpp _public.cpp ftp.cpp libftp.a -lm -lc
	cp -f ftpgetfiles1 ../bin/.

dfiletodb: dfiletodb.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o dfiletodb dfiletodb.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) $(MYSQLINCL) $(MYSQLLIB) $(PUBLIC) -lmysqlclient -lm -lc -g
	cp -f dfiletodb ../bin/.

pexptable: pexptable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o pexptable pexptable.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f pexptable ../bin/.

wfiletodb: wfiletodb.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o wfiletodb wfiletodb.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f wfiletodb ../bin/.

countformon: countformon.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o countformon countformon.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f countformon ../bin/.

montable: montable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o montable montable.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f montable ../bin/.

dminingoracle: dminingoracle.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o dminingoracle dminingoracle.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f dminingoracle ../bin/.

expdbfiles: expdbfiles.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o expdbfiles expdbfiles.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f expdbfiles ../bin/.

expdbfiles1: expdbfiles1.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o expdbfiles1 expdbfiles1.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f expdbfiles1 ../bin/.

deletetable: deletetable.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f deletetable ../bin/.

dminingsqlserver: dminingsqlserver.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o dminingsqlserver dminingsqlserver.cpp _public.cpp \
	/usr/local/freetds/lib/libsybdb.so.5.0.0 -lm -lc
	cp -f dminingsqlserver ../bin/.

dminingtxt: dminingtxt.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o dminingtxt dminingtxt.cpp _public.cpp -lm -lc
	cp -f dminingtxt ../bin/.

cpustat: cpustat.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o cpustat cpustat.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f cpustat ../bin/.

dbfilestat: dbfilestat.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o dbfilestat dbfilestat.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f dbfilestat ../bin/.

sessionstat: sessionstat.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o sessionstat sessionstat.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f sessionstat ../bin/.

alarmtotab: alarmtotab.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o alarmtotab alarmtotab.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f alarmtotab ../bin/.

diskstat: diskstat.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o diskstat diskstat.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f diskstat ../bin/.

backupdbuser: backupdbuser.cpp   _public.h _public.cpp
	g++ $(CFLAGS) -o backupdbuser backupdbuser.cpp  _public.cpp -L. -lm -lc
	cp -f backupdbuser ../bin/.

backupapp: backupapp.cpp   _public.h _public.cpp
	g++ $(CFLAGS) -o backupapp backupapp.cpp  _public.cpp -L. -lm -lc
	cp -f backupapp ../bin/.

uptdata: uptdata.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o uptdata uptdata.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f uptdata ../bin/.

rinetd: rinetd.cpp
	gcc -O2 -Wall -o rinetd rinetd.cpp -L. -lstdc++ -lm -lc
	cp -f rinetd ../bin/.

backuptables: backuptables.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o backuptables backuptables.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f backuptables ../bin/.

counttable: counttable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o counttable counttable.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f counttable ../bin/.

managesynonym: managesynonym.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o managesynonym managesynonym.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f managesynonym ../bin/.

crttable: crttable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o crttable crttable.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f crttable ../bin/.

synctest: synctest.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o synctest synctest.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f synctest ../bin/.

crtview: crtview.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o crtview crtview.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f crtview ../bin/.

managetable: managetable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o managetable managetable.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f managetable ../bin/.

crtsequence: crtsequence.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o crtsequence crtsequence.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f crtsequence ../bin/.

bissuedata: bissuedata.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o bissuedata bissuedata.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f bissuedata ../bin/.

gettabdata: gettabdata.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o gettabdata gettabdata.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f gettabdata ../bin/.

instabdata: instabdata.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o instabdata instabdata.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f instabdata ../bin/.

manageview: manageview.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o manageview manageview.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f manageview ../bin/.

migratetable: migratetable.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o migratetable migratetable.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f migratetable ../bin/.

dminingmysql: dminingmysql.cpp /htidc/htidc/c/_public.cpp /htidc/htidc/c/_public.h
	g++ $(CFLAGS) -o dminingmysql dminingmysql.cpp /htidc/htidc/c/_public.cpp $(MYSQLINCL) $(MYSQLLIB) \
	$(PUBLIC) -lmysqlclient
	cp -f dminingmysql /htidc/htidc/bin

drawcontour: drawcontour.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o drawcontour drawcontour.cpp _public.cpp -lm -lc
	cp -f drawcontour ../bin/.

drawwind: drawwind.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o drawwind drawwind.cpp _public.cpp -lm -lc
	cp -f drawwind ../bin/.

dminingpostgresql: dminingpostgresql.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o dminingpostgresql dminingpostgresql.cpp _public.cpp _pgdb.cpp \
	$(ORAINCL) $(ORALIB) -lpq -lm -lc
	cp -f dminingpostgresql ../bin/.

RenameFile: RenameFile.cpp
	g++ $(CFLAGS) -o RenameFile RenameFile.cpp /htidc/htidc/c/_public.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc -I/htidc/htidc/c
	cp -f RenameFile ../bin

pgzfile: pgzfile.cpp
	g++ $(CFLAGS) -o pgzfile pgzfile.cpp /htidc/htidc/c/_public.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc -I/htidc/htidc/c
	cp -f pgzfile ../bin

pcsvfiles: pcsvfiles.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o pcsvfiles pcsvfiles.cpp _public.cpp _oracle.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f pcsvfiles ../bin

extdataalarm: extdataalarm.cpp _public.h _public.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o extdataalarm extdataalarm.cpp _oracle.cpp _public.cpp idcapp.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f extdataalarm ../bin/.

syncoracletomysql: syncoracletomysql.cpp
	g++ $(CFLAGS) -o syncoracletomysql syncoracletomysql.cpp /htidc/htidc/c/_public.cpp /htidc/htidc/c/idcapp.cpp /htidc/htidc/c/_oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc -I/htidc/htidc/c \
	$(MYSQLINCL) $(MYSQLLIB) $(PUBLIC) -lmysqlclient -g
	cp -f syncoracletomysql ../bin/.

ftpdelfiles: ftpdelfiles.cpp _public.h _public.cpp ftp.h ftp.cpp
	g++ $(CFLAGS) -o ftpdelfiles ftpdelfiles.cpp _public.cpp ftp.cpp libftp.a -lm -lc
	cp -f ftpdelfiles ../bin/.

copyfile: copyfile.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o copyfile copyfile.cpp _public.cpp -lm -lc
	cp -f copyfile ../bin/.

ftpgetfiles2: ftpgetfiles2.cpp _public.h _public.cpp ftp.h ftp.cpp
	g++ $(CFLAGS) -o ftpgetfiles2 ftpgetfiles2.cpp _public.cpp ftp.cpp libftp.a -lm -lc
	cp -f ftpgetfiles2 ../bin/.

getfileinfo: getfileinfo.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o getfileinfo getfileinfo.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f getfileinfo ../bin/.

ftpputfiles1: ftpputfiles1.cpp _public.h _public.cpp ftp.h ftp.cpp
	g++ $(CFLAGS) -o ftpputfiles1 ftpputfiles1.cpp _public.cpp ftp.cpp libftp.a -lm -lc
	cp -f ftpputfiles1 ../bin/.

checkdirandalarm: checkdirandalarm.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o checkdirandalarm checkdirandalarm.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc
	cp -f checkdirandalarm ../bin/.

pgddpradarfiles: pgddpradarfiles.cpp
	g++ $(CFLAGS) -o pgddpradarfiles pgddpradarfiles.cpp /htidc/htidc/c/_public.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) -lm -lc -I/htidc/htidc/c
	cp -f pgddpradarfiles ../bin

ftpputforponds: ftpputforponds.cpp _public.h _public.cpp ftp.h ftp.cpp
	g++ $(CFLAGS) -o ftpputforponds ftpputforponds.cpp _public.cpp ftp.cpp libftp.a -lm -lc
	cp -f ftpputforponds ../bin/.

managetable_mysql: managetable_mysql.cpp _public.h _public.cpp
	g++ $(CFLAGS) -o managetable_mysql managetable_mysql.cpp _public.cpp _oracle.cpp \
	$(ORAINCL) $(ORALIB) $(ORALIBS) $(MYSQLINCL) $(MYSQLLIB) $(PUBLIC) -lmysqlclient -lm -lc -g
	cp -f managetable_mysql ../bin/.

clean:
	rm -f *.o *.a 
	rm -rf libftp.a procctl dbcp checkprogram deletefiles deletearchive gzipfiles synctable synctable_update\
	rm -rf tablecp1 tablecp2 tableupt cpdb managedict dfiletodb diskstat rinetd wfiletodb synctable_increment synctable_increment1\
	rm -rf backupdbuser backupapp ftpputfiles ftpgetfiles dminingoracle dminingtxt crtsynctrigger clearsynclog\
	rm -rf uptdata backuptables counttable managesynonym crtview managetable bissuedata manageview\
	rm -rf gettabdata instabdata crttable cpustat dbfilestat sessionstat alarmtotab migratetable\
	rm -rf fileclient fileserver drawcontour drawwind countformon montable\
	rm -rf dminingsqlserver dminingmysql crtsequence httpclient deletetable\
	rm -rf synctable_append gzipfiles1 pexptable expdbfiles pcsvfiles extdataalarm synctable_update3\
	rm -rf syncoracletomysql synctable_increment2 synctable_increment3 ftpdelfiles copyfile
