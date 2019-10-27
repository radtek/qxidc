-- pg数据库
create or replace function to_null(varchar) returns numeric as $$
begin
if (length($1)=0) then
  return null;
else
  return $1;
end if;
end
$$ LANGUAGE plpgsql;

-- oracle数据库
create or replace function to_null(in_value in varchar2) return varchar2
is
begin
  return in_value;
end;
/

-- mysql数据库，但是没有效果
delimiter $$
DROP FUNCTION IF EXISTS to_null;

create function to_null(in_value varchar(10)) returns decimal
begin
if (length(in_value)=0) then
  return null;
else
  return in_value;
end if;
end;
$$



