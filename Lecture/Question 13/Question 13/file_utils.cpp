// file_utils.cpp
#include <iostream>
#include <fstream>
#include "file_utils.h"

using namespace std;

// ������ �о� ���۷� ��ȯ�ϴ� �Լ�
char* File_To_Buf(const char* file)
{
    ifstream in(file, ios_base::binary);

    if (!in) {
        cerr << file << " ������ ã�� �� �����ϴ�." << endl;
        exit(1);
    }

    in.seekg(0, ios_base::end);
    long len = in.tellg();
    char* buf = new char[len + 1];
    in.seekg(0, ios_base::beg);

    in.read(buf, len);
    buf[len] = '\0';

    return buf;
}
