###################################################
# 在251的T_ALLAWSDATA表上创建同步日志表
/htidc/htidc/bin/crtsynctrigger "<logfilename>/log/sqxj/crtsynctrigger_T_ALLAWSDATA.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><tname>T_ALLAWSDATA</tname><logtname>T_ALLAWSDATA_LOG</logtname><logidxname1>IDX_ALLAWSDATA_LOG_1</logidxname1><logidxname2>IDX_ALLAWSDATA_LOG_2</logidxname2><tablespace>USERS</tablespace><logseqname>SEQ_ALLAWSDATA_LOG</logseqname><triggername>TR_ALLAWSDATA_SYNC</triggername><syncinsert>true</syncinsert><syncupdate>true</syncupdate><syncdelete>false</syncdelete><initdata>true</initdata>"

# 从核心数据库的T_ALLAWSDATA到251的T_ALLAWSDATA
/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_increment "<logfilename>/log/sqxj/synctable_increment_T_ALLAWSDATA1.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_ALLAWSDATA@SZIDC</srctname><srcfields></srcfields><srclogtname>T_ALLAWSDATA@SZIDC</srclogtname><dsttname>T_ALLAWSDATA</dsttname><dstfields></dstfields><timetvl>10</timetvl>"

# 从251的T_ALLAWSDATA到251的T_ALLAWSDATA_TEST
/htidc/htidc/bin/procctl 30 /htidc/htidc/bin/synctable_increment "<logfilename>/log/sqxj/synctable_increment_T_ALLAWSDATA2.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.97.251</connstr><srctname>T_ALLAWSDATA</srctname><srcfields></srcfields><srclogtname>T_ALLAWSDATA_SYNC</srclogtname><dsttname>T_ALLAWSDATA_TEST</dsttname><dstfields></dstfields><timetvl>10</timetvl><and>and ossmocode like '5%%'</and>"

