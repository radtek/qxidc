################################################################################
# 文件传输服务
# /htidc/htidc/bin/procctl_wandlife 30 /htidc/htidc/bin/fileserver /log/wandlife/fileserver.log 5004

# 删除卫星云图最近1天的文件
/htidc/htidc/bin/procctl_wandlife 300 /htidc/htidc/bin/deletefiles /wandlife/star 1

# 获取台湾云图文件，分别为可见光、彩色云图、色调强化和黑白
/htidc/htidc/bin/procctl_wandlife 30 /htidc/wandlife/bin/wgettwstar /log/wandlife/wgettwstar_hsao.log "http://www.cwb.gov.tw/V7/observe/satellite/Data/HSAO/HSAO-yyyy-mm-dd-hh-mi.jpg" /wandlife/star/wget_hsao /wandlife/star/hsao 2 /htidc/wandlife/map/nobordwhite.png
/htidc/htidc/bin/procctl_wandlife 30 /htidc/wandlife/bin/wgettwstar /log/wandlife/wgettwstar_hs1p.log "http://www.cwb.gov.tw/V7/observe/satellite/Data/HS1P/HS1P-yyyy-mm-dd-hh-mi.jpg" /wandlife/star/wget_hs1p /wandlife/star/hs1p 2 /htidc/wandlife/map/nobordyellow.png
/htidc/htidc/bin/procctl_wandlife 30 /htidc/wandlife/bin/wgettwstar /log/wandlife/wgettwstar_hs1p.log "http://www.cwb.gov.tw/V7/observe/satellite/Data/HS1Q/HS1Q-yyyy-mm-dd-hh-mi.jpg" /wandlife/star/wget_hs1q /wandlife/star/hs1q 2 /htidc/wandlife/map/nobordwhite.png
/htidc/htidc/bin/procctl_wandlife 30 /htidc/wandlife/bin/wgettwstar /log/wandlife/wgettwstar_hs1o.log "http://www.cwb.gov.tw/V7/observe/satellite/Data/HS1O/HS1O-yyyy-mm-dd-hh-mi.jpg" /wandlife/star/wget_hs1o /wandlife/star/hs1o 2 /htidc/wandlife/map/nobordwhite.png

# 采集全国自动站数据，注意，原始文件名中的时间是世界时
# 采集的时候，需要精确匹配文件名的时间，减少对方服务器的压力
# 当前小时
/htidc/htidc/bin/procctl_wandlife  50 /htidc/htidc/bin/ftpgetfiles /log/wandlife/ftpgetfiles_10.148.1.104_zxt_zhzdz_1.log "<remoteip>10.148.1.104</remoteip><port>21</port><mode>pasv</mode><username>zxt</username><password>123456</password><localpath>/qxdata/wandlife/sdata/zhzdz</localpath><remotepath>/home/begz/data/pub0</remotepath><remotepathbak></remotepathbak><matchname>Z_*yyyymmdd-00.32hh24*_AWS_FTM.TXT</matchname><listfilename>/tmp/htidc/list/ftpgetfiles_10.148.1.104_zxt_zhzdz_wandlife_1.xml</listfilename><deleteremote>2</deleteremote><timeout>300</timeout>"
# 上一小时
/htidc/htidc/bin/procctl_wandlife 100 /htidc/htidc/bin/ftpgetfiles /log/wandlife/ftpgetfiles_10.148.1.104_zxt_zhzdz_2.log "<remoteip>10.148.1.104</remoteip><port>21</port><mode>pasv</mode><username>zxt</username><password>123456</password><localpath>/qxdata/wandlife/sdata/zhzdz</localpath><remotepath>/home/begz/data/pub0</remotepath><remotepathbak></remotepathbak><matchname>Z_*yyyymmdd-00.36hh24*_AWS_FTM.TXT</matchname><listfilename>/tmp/htidc/list/ftpgetfiles_10.148.1.104_zxt_zhzdz_wandlife_2.xml</listfilename><deleteremote>2</deleteremote><timeout>300</timeout>"

# 处理全国遥测站数据，转换为ALLAWSMIN_*.xml文件，用于入库到T_ALLAWSMIND表中。
/htidc/htidc/bin/procctl_wandlife 30 /htidc/qxidc/bin/pallawszfile /htidc/wandlife/ini/wandlife.xml "<logfilename>/log/wandlife/pallawszfile_zh.log</logfilename><isossmodata>true</isossmodata><zfilepath>/qxdata/wandlife/sdata/zhzdz</zfilepath><stdfilepath>/qxdata/wandlife/sdata/std</stdfilepath><allawsname>ALLAWSMIN</allawsname>"

# 城市天气预报，SPCC报文
/htidc/htidc/bin/procctl_wandlife 180 /htidc/htidc/bin/ftpgetfiles /log/wandlife/ftpgetfiles_172.22.1.17_zxt_sevpspcc.log "<remoteip>172.22.1.17</remoteip><port>21</port><mode>pasv</mode><username>zxt</username><password>123456</password><localpath>/qxdata/wandlife/sdata/sevp/spcc</localpath><remotepath>/bcgz/rffc</remotepath><matchname>Z_SEVP_C_*_P_RFFC-SPCC-*yyyymmdd*.TXT,Z_SEVP_C_*_P_RFFC-SPCC-*yyyymmdd-01.00*.TXT</matchname><listfilename>/tmp/htidc/list/ftpgetfiles_172.22.1.17_zxt_sevpspcc_wandlife.xml</listfilename><deleteremote>2</deleteremote><timeout>1800</timeout>"

# 城市天气预报，SNWFD报文
/htidc/htidc/bin/procctl_wandlife 180 /htidc/htidc/bin/ftpgetfiles /log/wandlife/ftpgetfiles_172.22.1.17_zxt_sevpsnwfd.log "<remoteip>172.22.1.17</remoteip><port>21</port><mode>pasv</mode><username>zxt</username><password>123456</password><localpath>/qxdata/wandlife/sdata/sevp/snwfd</localpath><remotepath>/bcgz/rffc</remotepath><matchname>Z_SEVP_C_*_P_RFFC-SNWFD-*yyyymmdd*.TXT,Z_SEVP_C_*_P_RFFC-SNWFD-*yyyymmdd-01.00*.TXT</matchname><listfilename>/tmp/htidc/list/ftpgetfiles_172.22.1.17_zxt_sevpsnwfd_wandlife.xml</listfilename><deleteremote>2</deleteremote><timeout>1800</timeout>"

# 处理精细化天气预报（中央台订正产品）
/htidc/htidc/bin/procctl_wandlife 30 /htidc/qxidc/bin/pcitywfiletxt /htidc/wandlife/ini/wandlife.xml /qxdata/wandlife/sdata/sevp/spcc  SPCC /qxdata/wandlife/sdata/std
/htidc/htidc/bin/procctl_wandlife 30 /htidc/qxidc/bin/pcitywfiletxt /htidc/wandlife/ini/wandlife.xml /qxdata/wandlife/sdata/sevp/snwfd  SNWFD /qxdata/wandlife/sdata/std

# 从服务器上获取全国预警信号发布的文件
/htidc/htidc/bin/procctl_wandlife 10 /htidc/htidc/bin/fileclient /log/wandlife/fileclient_get_alert.log "<localrootpath>/qxdata/wandlife/sdata/alert</localrootpath><dstrootpath>/home/alert</dstrootpath><url>223.4.5.165,3306</url><matchname>alert.xml</matchname><andchild>FALSE</andchild><timetvl>20</timetvl><checkcount>1</checkcount><conntype>2</conntype>"

# 处理预警信号
/htidc/htidc/bin/procctl_wandlife 30 /htidc/wandlife/bin/palertfile /log/wandlife/palertfile.log wandlife/pwdidc /qxdata/wandlife/sdata/alert/alert.xml

# 从天气网上获取生活指数
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_0.log wandlife/pwdidc /qxdata/wandlife/sdata/index 0 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_1.log wandlife/pwdidc /qxdata/wandlife/sdata/index 1 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_2.log wandlife/pwdidc /qxdata/wandlife/sdata/index 2 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_3.log wandlife/pwdidc /qxdata/wandlife/sdata/index 3 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_4.log wandlife/pwdidc /qxdata/wandlife/sdata/index 4 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_5.log wandlife/pwdidc /qxdata/wandlife/sdata/index 5 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_6.log wandlife/pwdidc /qxdata/wandlife/sdata/index 6 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_7.log wandlife/pwdidc /qxdata/wandlife/sdata/index 7 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_8.log wandlife/pwdidc /qxdata/wandlife/sdata/index 8 &
/htidc/htidc/bin/procctl_wandlife 1200 /htidc/wandlife/bin/pindexfile /log/wandlife/pindexfile_9.log wandlife/pwdidc /qxdata/wandlife/sdata/index 9 &

# 二维表格数据入库的主程序
/htidc/htidc/bin/procctl_wandlife 10 /htidc/htidc/bin/dfiletodb /htidc/wandlife/ini/wandlife.xml /qxdata/wandlife/sdata/std std

# 运行在数据源服务器上
# /htidc/htidc/bin/proc_wandlife 80 /htidc/htidc/bin/dminingoracle /tmp/htidc/log/dminingoracle_OBTHOURD_wandlife.log "<charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc</connstr><selectsql>select ddatetime,ossmocode obtid,latitude,longitude,height,wd2dd,wd2df/10 wd2df,wd10dd,wd10df/10 wd10df,hourr/10 hourrf,t/10 t,u rh,p/10 p,st/10 st,p0/10 p0,v2,ph,keyid from T_ALLAWSMIND where ddatetime>sysdate-0.05 and ossmocode in (select obtid from T_OBTCODE_DMI)</selectsql><fieldstr>ddatetime,obtid,lat,lon,height,wd2dd,wd2df,wd10dd,wd10df,hourrf,t,rh,p,st,p0,v2,ph,keyid</fieldstr><fieldlen>20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20</fieldlen><bfilename>ALLAWSMIN</bfilename><efilename>wandlife</efilename><outpathtmp>/qxdata/wandlife/sdata/std</outpathtmp><outpath>/qxdata/wandlife/sdata/std</outpath><starttime></starttime><incfield>keyid</incfield><incfilename>/tmp/htidc/list/dminingoracle_ALLAWSMIND_wandlife.list</incfilename>"

# 
# 运行在数据源服务器上
# /htidc/htidc/bin/proc_wandlife 30 /htidc/htidc/bin/fileclient /log/wandlife/fileclient_allawsmind.log "<localrootpath>/qxdata/wandlife/sdata/std</localrootpath><dstrootpath>/qxdata/wandlife/sdata/std</dstrootpath><url>10.148.11.23,5005</url><matchname>*</matchname><andchild>FALSE</andchild><timetvl>30</timetvl><checkcount>50</checkcount><conntype>1</conntype>"
