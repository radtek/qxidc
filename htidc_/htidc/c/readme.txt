1. ����ɵĳ���
# cd /htidc/htidc/c
# mkdir ../bin
# make clean

2. ���밲װFreeTDS���������root�û�ִ�����µĽű���
# cd /htidc/freetds-0.82
# ./configure --prefix=/usr/local/freetds --with-tdsver=8.0 --enable-msdblib
# make
# make install

3. ��/htidcĿ¼��������Ϊoracle
# chown -R oracle:dba /htidc
ע�⣬oracle�û����鲻һ����dba��Ҳ������oinstall����oracle��װ��ʵ�����������

4. ���Ӷ�̬���ӿ������·������/usr/local/freetds/lib����LD_LIBRARY_PATH��ȥ��
# vi /etc/profile.d/sysenv.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_HOME/lib:$ORACLE_HOME/lib32:/usr/local/freetds/lib:.

ע�⣺��5��6����������shell�²�����SQL Server���ݿ�������õģ����ǵ����ݲɼ����򲢲���Ҫ��Щ���á�

5. ��/usr/local/freetds/bin/tsql���߿����������ݿ⣬����ִ���κ�SQL��䡣
# export LANG=zh_CN.GB18030
# /usr/local/freetds/bin/tsql -H 10.0.65.111 -p 1433 -U sa -P sapwd
locale is "zh_CN.GB18030"
locale charset is "GB18030"
1 >use ���ݿ���
2 >go
1 >sp_help ����
2 >go
������ʾ����Ƿ�������exit�˳�tsql

6. �л���oracle�û��������̨����
# su - oracle
$ cd /htidc/htidc/c
$ make
ע�����Ĺ�������û�д���

7. ɾ��Դ����
$ cd /htidc/htidc/c
$ rm -rf *.h *.cpp *.c makefile
ע�⣺һ��Ҫɾ����Դ������Щ����ǳ���Ҫ���������ڷ������ϣ������µ׿��˲�����

8. ��ÿ�����к��ƺ�̨����ķ������ϱ���������̨�����س�����root�û�������
��/etc/rc.d/rc.local��������������
/htidc/htidc/bin/procctl  30 /htidc/htidc/bin/checkprogram

9. oracle�Ĺ鵵��־�ļ����������root�û�������
��/etc/rc.d/rc.local��������������
/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletearchive  /oracle/archive 20
/oracle/archive�ǹ鵵��־�ļ���ŵ�Ŀ¼��20��ʾֻ�������20���鵵�ļ���֮ǰ�Ľ������
ע�⣺��ÿ��������oracle���ݿⲢ�������˹鵵��־��ʽ�ķ�������һ��Ҫ�����������

10. �����ļ����������root�û�������
��/etc/rc.d/rc.local��������������
/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /htidc/sqxt/data/wfile/stdbak 0.5
/htidc/sqxt/data/wfile/stdbak�Ǵ������Ŀ¼��0.5��ʾ����0.5��֮ǰ��ȫ���ļ�
ע�⣬�����������ĵĺܶ��ļ��Ǵ����/tmp/htidcĿ¼�µģ��������Ŀ¼���붨ʱ����һ�������²���
/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/deletefiles /tmp/htidc 5

11. ���������˵��
backupapp ���ݱ�������ȫ��Ӧ�ó�����ļ��ĳ�����root�û�������
backupdbuser ����exp�������ݿ��û��ĳ�����oracle�û�������
dminingtxt ���ı��ļ�����Դ�ɼ����ݵĳ�����oracle�û�������
dminingoracle ��oracle���ݿ�����Դ��ɼ����ݵĳ�����oracle�û�������
dminingsqlserver ��sqlserver���ݿ�����Դ��ɼ����ݵĳ�����oracle�û�������
ftpputfiles ���ļ���ftpЭ��ӱ����������͵������������ĳ�����root�û�������
ftpgetfiles ���ļ���ftpЭ��������������ɼ������������ĳ�����root�û�������

���ϳ�����÷��Ͳ�˵�ˣ���shell�������������ȡ������ϸ�İ�����Ϣ���ǳ��꾡��һ��Ҫ���濴��
���磬��shell������

$ /htidc/htidc/bin/ftpputfiles

��ʾ������

Using:/htidc/htidc/bin/ftpputfiles logfilename xmlbuffer

Sample:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/ftpputfiles /tmp/htidc/log/ftpputfiles_OBTDATA_HYCZ.log "<remoteip>10.148.11.23</remoteip><port>21</port><mode>pasv</mode><username>oracle</username><password>passwd</password><localpath>/tmp/htidc/ftpfiles</localpath><localpathbak>/tmp/htidc/ftpfiles</localpathbak><remotepathtmp>/oracle/data/recvtmp</remotepathtmp><remotepath>/oracle/data/recv</remotepath><matchname>T_OBTHOURD_*_*_HYCZ.xml.gz</matchname><timeout>300</timeout>"


���������������ĵĹ�������ģ�飬���ڰѱ����ļ����͵�Զ�̵�FTP��������
logfilename�Ǳ��������е���־�ļ���
xmlbufferΪ�ļ�����Ĳ��������£�
<remoteip>10.148.11.23</remoteip> Զ�̷�������IP��
<port>21</port> Զ�̷�����FTP�Ķ˿ڡ�
<mode>pasv</mode> ����ģʽ��port��pasv����ѡ�ֶΣ����Ϊ�գ�ȱʡ����pasvģʽ��
<username>oracle</username> Զ�̷�����FTP���û�����
<password>passwd</password> Զ�̷�����FTP�����롣
<localpath>/tmp/htidc/ftpfiles</localpath> �����ļ���ŵ�Ŀ¼��
<localpathbak></localpathbak>
�����ļ����ͳɹ��󣬴�ŵı���Ŀ¼�������Ŀ¼Ϊ�գ��ļ����ͳɹ���ֱ��ɾ�������localpath����localpathbak���ļ����ͳɹ��󽫱���Դ�ļ����䡣
<remotepathtmp>/oracle/data/recvtmp</remotepathtmp>
Զ�̷���������ļ�����ʱĿ¼��
<remotepath>/oracle/data/recv</remotepath>
Զ�̷���������ļ�������Ŀ¼��remotepathtmp��remotepath��ȡֵ������ͬ��
<matchname>T_OBTHOURD_*_*_HYCZ.XML.GZ,T_OBTHOURD_*_*_HYMS.XML.GZ</matchname>
�������ļ�ƥ����ļ��������ô�дƥ�䣬��ƥ����ļ����ᱻ���ͣ�ע�⣬Ϊ�˱�֤��ʱ�ļ��������ͣ����ֶξ����ܾ�ȷƥ�䡣
<timeout>300</timeout>
FTP�����ļ��ĳ�ʱʱ�䣬��λ���룬ע�⣬����ȷ������ĳ�����ļ���ʱ��С��timeout��ȡֵ���������ɴ���ʧ�ܵ������
���ϵĲ���ֻ��mode��localpathbakΪ��ѡ�����������Ķ����


port ģʽ�ǽ����ӷ������߶˿������ͻ���20�˿���������
pasv ģʽ�ǽ����ͻ��߶˿��������������ص����ݶ˿ڵ���������

port����������ʽ�����ӹ����ǣ��ͻ������������FTP�˿ڣ�Ĭ����21�������������󣬷������������ӣ�����һ��������·��
����Ҫ��������ʱ����������20�˿���ͻ��˵Ŀ��ж˿ڷ����������󣬽���һ��������·���������ݡ�

pasv����������ʽ�����ӹ����ǣ��ͻ������������FTP�˿ڣ�Ĭ����21�������������󣬷������������ӣ�����һ��������·��
����Ҫ��������ʱ���ͻ�����������Ŀ��ж˿ڷ����������󣬽���һ��������·���������ݡ�

