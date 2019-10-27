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

# 通用框架头文件路径
PUBINCL = -I/root/htidc/public/c

# 通用框架库文件路径
PUBCPP = /root/htidc/public/c/_public.cpp 
FTPCPP = /root/htidc/public/c/_ftp.cpp 
OCICPP = /root/htidc/public/c/_ooci.cpp

CC=g++

# CFLAGS = -O2
# CFLAGS = -O2 -Wall

CFLAGS = -g -Wno-write-strings -Wno-unused-variable


all: crtsurfdata crtsurfdata1 psurfdata_old psurfdata psurfdata1 psignallog

crtsurfdata:crtsurfdata.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o crtsurfdata crtsurfdata.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp crtsurfdata ../bin/.

crtsurfdata1:crtsurfdata1.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o crtsurfdata1 crtsurfdata1.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp crtsurfdata1 ../bin/.

psurfdata_old: psurfdata_old.cpp 
	$(CC) $(CFLAGS) -o psurfdata_old psurfdata_old.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) -lm -lc
	cp psurfdata_old ../bin/.

psurfdata: psurfdata.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o psurfdata psurfdata.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp psurfdata ../bin/.

psurfdata1: psurfdata1.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o psurfdata1 psurfdata1.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp psurfdata1 ../bin/.

psignallog: psignallog.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o psignallog psignallog.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp psignallog ../bin/.

tt: tt.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o tt tt.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc


clean:
	rm -rf crtsurfdata crtsurfdata1 psurfdata_old psurfdata psurfdata1 psignallog
