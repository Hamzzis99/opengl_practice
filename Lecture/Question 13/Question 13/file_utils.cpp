// file_utils.cpp
#include <iostream>
#include <fstream>
#include "file_utils.h"

using namespace std;

// 파일을 읽어 버퍼로 반환하는 함수
char* File_To_Buf(const char* file)
{
    ifstream in(file, ios_base::binary);

    if (!in) {
        cerr << file << " 파일을 찾을 수 없습니다." << endl;
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
