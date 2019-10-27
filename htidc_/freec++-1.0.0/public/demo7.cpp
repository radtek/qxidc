//
// ��������ʾ�ļ�������CFile�����ļ���д�����ݣ���û�в�����ʱ�ļ�����ķ���
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./demo7 xmlfilename\n\n");

    printf("Example:./demo7 demo7.xml\n\n");

    printf("��������ʾ�ļ�������CFile�����ļ���д�����ݣ���û�в�����ʱ�ļ�����ķ�����\n");

    return -1;
  }

  CFile File;

  // ���ļ�
  if (File.OpenForWrite(argv[1],"w")==FALSE)
  {
    printf("File.OpenForWrite(%s) failed.\n",argv[1]); return -1;
  }
  
  // ���ļ�д������
  File.Fprintf(\
     "<?xml version='1.0' encoding='gbk'?>\n"\
     "<data>\n"\
     "<id>1001</id><name>��һ</name><sex>��</sex><age>21</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1002</id><name>�Ŷ�</name><sex>��</sex><age>22</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1003</id><name>����</name><sex>��</sex><age>23</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1004</id><name>����</name><sex>��</sex><age>24</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1005</id><name>����</name><sex>��</sex><age>25</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1006</id><name>����</name><sex>��</sex><age>26</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1007</id><name>����</name><sex>��</sex><age>27</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "<id>1008</id><name>�Ű�</name><sex>Ů</sex><age>28</age><memo>��ע��\n"\
     "1���������Ĵ���һ��ũ�壻\n"\
     "2�������ڳ���ģ�\n"\
     "3���������ڹ��ݹ�����</memo><endl/>\n"\
     "<id>1009</id><name>�ž�</name><sex>��</sex><age>29</age><memo>��ע��Ϣ��û�л��С�</memo><endl/>\n"\
     "</data>\n");

  // �ȴ�30�룬��Ŀ¼�п�����demo7.xml.tmp����ʱ�ļ�
  sleep(30);

  // �ر��ļ�
  File.CloseOnly();

  return 0;
}

