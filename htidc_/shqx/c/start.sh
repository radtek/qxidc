# �ýű����������Ϻ������������ĵķ������

# ����ȫ������վ���������ļ�
/htidc/shqx/bin/crtsurfdata1 /htidc/shqx/ini/stcode1.ini /data/shqx/ftp/surfdata /log/shqx/crtsurfdata_txt.log txt&
/htidc/shqx/bin/crtsurfdata1 /htidc/shqx/ini/stcode1.ini /data/shqx/ftp/surfdata /log/shqx/crtsurfdata_xml.log xml&
# ע�⣬Ŀ¼/data/shqx/ftp/surfdata��0.1��֮ǰ�������ļ��Զ�������crontab�е��ȣ��ű����£�
# */10 * * * *  /htidc/public/bin/deletefiles /data/shqx/ftp/surfdata 0.1 "*"

# �ɼ�ȫ������վ���������ļ�
/htidc/public/bin/ftpgetfile /log/shqx/ftpgetfile_surfdata.log "<host>118.89.50.198:21</host><port>21</port><mode>1</mode><username>oracle</username><password>te.st1234TES@T</password><localpath>/data/shqx/sdata/surfdata</localpath><remotepath>/data/shqx/ftp/surfdata</remotepath><matchname>SURF_*.TXT,SURF_*.XML</matchname><ptype>1</ptype><remotepathbak></remotepathbak><listfilename>/data/shqx/ftplist/ftpgetfile_surfdata.list</listfilename><okfilename>/data/shqx/ftplist/ftpgetfile_surfdata.xml</okfilename><timetvl>30</timetvl>" &

# ����ȫ������վ���������ļ������
/htidc/shqx/bin/psurfdata  /data/shqx/sdata/surfdata /log/shqx/psurfdata.log  shqx/pwdidc@snorcl11g_198 10 &
/htidc/shqx/bin/psurfdata1 /data/shqx/sdata/surfdata /log/shqx/psurfdata1.log shqx/pwdidc@snorcl11g_198 10 &

# ����T_SURFDATA��������֮ǰ������
/htidc/public/bin/deletetables "<logfilename>/log/shqx/deletetables_SURFDATA.log</logfilename><connstr>shqx/pwdidc@snorcl11g_198</connstr><tname>T_SURFDATA</tname><where>where ddatetime<sysdate-3</where><hourstr>00,02,04,08,10,12,14,16,18,20,22</hourstr>" &
