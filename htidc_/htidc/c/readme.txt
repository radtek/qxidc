1. 清理旧的程序
# cd /htidc/htidc/c
# mkdir ../bin
# make clean

2. 编译安装FreeTDS软件包，用root用户执行以下的脚本。
# cd /htidc/freetds-0.82
# ./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
# make
# make install

3. 把/htidc目录的属主改为oracle
# chown -R oracle:dba /htidc
注意，oracle用户的组不一定是dba，也可能是oinstall，视oracle安装的实际情况而定。

4. 增加动态链接库的搜索路径，把/usr/local/freetds/lib增加LD_LIBRARY_PATH中去。
# vi /etc/profile.d/sysenv.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_HOME/lib:$ORACLE_HOME/lib32:/usr/local/freetds/lib:.

注意：第5、6步是用于在shell下测试与SQL Server数据库的连接用的，我们的数据采集程序并不需要这些配置。

5. 用/usr/local/freetds/bin/tsql工具可以连上数据库，可以执行任何SQL语句。
# export LANG=zh_CN.GB18030
# /usr/local/freetds/bin/tsql -H 10.0.65.111 -p 1433 -U sa -P sapwd
locale is "zh_CN.GB18030"
locale charset is "GB18030"
1 >use 数据库名
2 >go
1 >sp_help 表名
2 >go
看看显示结果是否正常，exit退出tsql

6. 切换到oracle用户，编译后台程序
# su - oracle
$ cd /htidc/htidc/c
$ make
注意编译的过程中有没有错误。

7. 删除源程序
$ cd /htidc/htidc/c
$ rm -rf *.h *.cpp *.c makefile
注意：一定要删除掉源程序，这些程序非常重要，不能留在服务器上，否则月底考核不及格。

8. 在每个运行汉唐后台程序的服务器上必须启动后台服务监控程序，用root用户启动。
在/etc/rc.d/rc.local中增加以下内容
/htidc/htidc/bin/procctl  30 /htidc/htidc/bin/checkprogram

9. oracle的归档日志文件清理程序，用root用户启动。
在/etc/rc.d/rc.local中增加以下内容
/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletearchive  /oracle/archive 20
/oracle/archive是归档日志文件存放的目录，20表示只保留最近20个归档文件，之前的将清除。
注意：在每个运行了oracle数据库并且启用了归档日志方式的服务器上一定要启动这个程序。

10. 其它文件清理程序，用root用户启动。
在/etc/rc.d/rc.local中增加以下内容
/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /htidc/sqxt/data/wfile/stdbak 0.5
/htidc/sqxt/data/wfile/stdbak是待清理的目录，0.5表示清理0.5天之前的全部文件
注意，汉唐数据中心的很多文件是存放在/tmp/htidc目录下的，所以这个目录必须定时清理，一般用如下参数
/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /tmp/htidc 5

11. 其它程序的说明
backupapp 备份本服务器全部应用程序和文件的程序，用root用户启动。
backupdbuser 采用exp备份数据库用户的程序，用oracle用户启动。
dminingtxt 从文本文件数据源采集数据的程序，用oracle用户启动。
dminingoracle 从oracle数据库数据源表采集数据的程序，用oracle用户启动。
dminingsqlserver 从sqlserver数据库数据源表采集数据的程序，用oracle用户启动。
ftpputfiles 把文件用ftp协议从本服务器发送到其它服务器的程序，用root用户启动。
ftpgetfiles 把文件用ftp协议从其它服务器采集到本服务器的程序，用root用户启动。

以上程序的用法就不说了，在shell下输入程序名获取程序详细的帮助信息，非常详尽，一定要认真看。
例如，在shell下输入

$ /htidc/htidc/bin/ftpputfiles

显示将如下

Using:/htidc/htidc/bin/ftpputfiles logfilename xmlbuffer

Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpputfiles /tmp/htidc/log/ftpputfiles_OBTDATA_HYCZ.log "<remoteip>10.148.11.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>passwd</password><localpath>/tmp/htidc/ftpfiles</localpath><localpathbak>/tmp/htidc/ftpfiles</localpathbak><remotepathtmp>/oracle/data/recvtmp</remotepathtmp><remotepath>/oracle/data/recv</remotepath><matchname>T_OBTHOURD_*_*_HYCZ.xml.gz</matchname><timeout>300</timeout>"


本程序是数据中心的公共功能模块，用于把本地文件发送到远程的FTP服务器。
logfilename是本程序运行的日志文件。
xmlbuffer为文件传输的参数，如下：
<remoteip>10.148.11.23</remoteip> 远程服务器的IP。
<port>21</port> 远程服务器FTP的端口。
<mode>pasv</mode> 传输模式，port和pasv，可选字段，如果为空，缺省采用pasv模式。
<username>oracle</username> 远程服务器FTP的用户名。
<password>passwd</password> 远程服务器FTP的密码。
<localpath>/tmp/htidc/ftpfiles</localpath> 本地文件存放的目录。
<localpathbak></localpathbak>
本地文件发送成功后，存放的备份目录，如果该目录为空，文件发送成功后直接删除，如果localpath等于localpathbak，文件发送成功后将保留源文件不变。
<remotepathtmp>/oracle/data/recvtmp</remotepathtmp>
远程服务器存放文件的临时目录。
<remotepath>/oracle/data/recv</remotepath>
远程服务器存放文件的最终目录，remotepathtmp和remotepath的取值可以相同。
<matchname>T_OBTHOURD_*_*_HYCZ.XML.GZ,T_OBTHOURD_*_*_HYMS.XML.GZ</matchname>
待发送文件匹配的文件名，采用大写匹配，不匹配的文件不会被发送，注意，为了保证临时文件不被发送，本字段尽可能精确匹配。
<timeout>300</timeout>
FTP发送文件的超时时间，单位：秒，注意，必须确保传输某单个文件的时间小于timeout的取值，否则会造成传输失败的情况。
以上的参数只有mode和localpathbak为可选参数，其它的都必填。


port 模式是建立从服务器高端口连到客户端20端口数据连接
pasv 模式是建立客户高端口连到服务器返回的数据端口的数据连接

port（主动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，服务器接受连接，建立一条命令链路。
当需要传送数据时，服务器从20端口向客户端的空闲端口发送连接请求，建立一条数据链路来传送数据。

pasv（被动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，服务器接受连接，建立一条命令链路。
当需要传送数据时，客户端向服务器的空闲端口发送连接请求，建立一条数据链路来传送数据。

