#include "path_conv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************************/

typedef struct test_data_t {
    const char *src;
    const char *dst;
} test_data;

static const test_data datas[] = {
    {"C:\\foo\\bar", "C:\\foo\\bar"} // 0
    ,{"/foo/bar;", "/foo/bar;"} // 1
    ,{"//foobar", "/foobar"} // 2
    ,{"//foo\\bar", "/foo/bar"} // 3
    ,{"//foo/bar", "//foo/bar"} // 4
    ,{"/c:\\foo\\bar", "c:/foo/bar"} // 5
    ,{"foo=/bar", "foo=C:/msys2/bar"} // 6
    ,{"-foo,/bar", "-foo,C:/msys2/bar"} // 7
    ,{"-I/foo,/bar", "-I/foo,C:/msys2/bar"} // 8
    ,{"-I/foo", "-IC:/msys2/foo"} // 9
    ,{"-L/foo", "-LC:/msys2/foo"} // 9
    ,{"/", "C:/msys2"} // 10
    ,{"/..", "/.."} // 11
    ,{"x:x:/x", "x:x:/x"} // 12
    ,{"x::x:/x", "x;x;C:\\msys2\\x"} // 13
    ,{"x::x/z:x", "x;x\\z;x"} // 14
    ,{"x::x//:z:x", "x;x//:z;x"} // 14
    ,{"/tmp:/tmp", "C:\\msys2\\tmp;C:\\msys2\\tmp"} // 14
    ,{"-w -- INSTALL_ROOT=C:/Test/ports64", "-w -- INSTALL_ROOT=C:/Test/ports64"} // 15
    ,{"-w -- INSTALL_ROOT=C:\\Test\\ports64", "-w -- INSTALL_ROOT=C:\\Test\\ports64"} // 16
    ,{"-IC:/Test/ports64", "-IC:/Test/ports64"} // 17
    ,{"-g -O2 -I/foo -L/foo PREFIX=/foo", "-g -O2 -IC:/msys2/foo -LC:/msys2/foo PREFIX=C:/msys2/foo"}
    ,{"-g -O2 -I/foo -L'/foo bar/boo' PREFIX='/foo bar/boo'", "-g -O2 -IC:/msys2/foo -L'C:/msys2/foo bar/boo' PREFIX='C:/msys2/foo bar/boo'"}
    ,{"'C:\\foo\\bar'", "'C:\\foo\\bar'"} // 0
    ,{"'/foo/bar;'", "'/foo/bar;'"} // 1
    ,{"'//foobar'", "'/foobar'"} // 2
    ,{"'//foo\\bar'", "'/foo/bar'"} // 3
    ,{"'//foo/bar'", "'//foo/bar'"} // 4
    ,{"'/c:\\foo\\bar'", "'c:/foo/bar'"} // 5
    ,{"'foo=/bar'", "'foo=C:/msys2/bar'"} // 6
    ,{"'-foo,/bar'", "'-foo,C:/msys2/bar'"} // 7
    ,{"'-I/foo,/bar'", "'-I/foo,C:/msys2/bar'"} // 8
    ,{"'-I/foo'", "'-IC:/msys2/foo'"} // 9
    ,{"'/'", "'C:/msys2'"} // 10
    ,{"'/..'", "'/..'"} // 11
    ,{"'x:x:/x'", "'x:x:/x'"} // 12
    ,{"'x::x:/x'", "'x::x:/x'"} // 13
    ,{"'-w -- INSTALL_ROOT=C:/Test/ports64'", "'-w -- INSTALL_ROOT=C:/Test/ports64'"} // 15
    ,{"'-w -- INSTALL_ROOT=C:\\Test\\ports64'", "'-w -- INSTALL_ROOT=C:\\Test\\ports64'"} // 16
    ,{"'-IC:/Test/ports64'", "'-IC:/Test/ports64'"} // 17
    ,{0, 0}
};

/***************************************************************************/

int main() {
    for ( const test_data *it = &datas[0]; it && it->src; ++it ) {
        const char *path = it->src;
        const size_t blen = strlen(it->dst)*2;
        char *buf = (char*)malloc(blen);

        const char *res = convert(buf, blen, path);
        if ( 0 != strcmp(res, it->dst) ) {
            printf("test %ld failed: src=\"%s\", dst=\"%s\" expect=\"%s\"\n", (it - &datas[0]), path, res, it->dst);
        } else {
            printf("test %ld passed\n", (it - &datas[0]));
        }

        free(buf);
    }
}

/***************************************************************************/
