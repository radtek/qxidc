--�����ݿ��û�����������ԵĴ�����Ϊ������
alter profile DEFAULT limit FAILED_LOGIN_ATTEMPTS UNLIMITED;
alter profile DEFAULT limit PASSWORD_LIFE_TIME  UNLIMITED;

-- ���������û������û�
-- drop user shqx cascade;
create user shqx profile default identified by pwdidc default tablespace users account unlock;
grant connect to shqx;
grant dba to shqx;

exit;
