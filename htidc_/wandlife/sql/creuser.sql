--把数据库用户允许错误重试的次数改为不限制
alter profile DEFAULT limit FAILED_LOGIN_ATTEMPTS UNLIMITED;
alter profile DEFAULT limit PASSWORD_LIFE_TIME  UNLIMITED;

-- drop user wandlife cascade;
create user wandlife profile default identified by pwdidc default tablespace users temporary tablespace temp account unlock;
grant connect to wandlife;
grant dba to wandlife;


exit;
