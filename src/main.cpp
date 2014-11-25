/*
  The msys32 Path conversion source code is licensed under:

  CC0 1.0 Universal

  Official translations of this legal tool are available

  CREATIVE COMMONS CORPORATION IS NOT A LAW FIRM AND DOES NOT PROVIDE
  LEGAL SERVICES. DISTRIBUTION OF THIS DOCUMENT DOES NOT CREATE AN
  ATTORNEY-CLIENT RELATIONSHIP. CREATIVE COMMONS PROVIDES THIS
  INFORMATION ON AN "AS-IS" BASIS. CREATIVE COMMONS MAKES NO WARRANTIES
  REGARDING THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS
  PROVIDED HEREUNDER, AND DISCLAIMS LIABILITY FOR DAMAGES RESULTING FROM
  THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS PROVIDED
  HEREUNDER.

  Statement of Purpose

  The laws of most jurisdictions throughout the world automatically
  confer exclusive Copyright and Related Rights (defined below) upon the
  creator and subsequent owner(s) (each and all, an "owner") of an
  original work of authorship and/or a database (each, a "Work").

  Certain owners wish to permanently relinquish those rights to a Work
  for the purpose of contributing to a commons of creative, cultural and
  scientific works ("Commons") that the public can reliably and without
  fear of later claims of infringement build upon, modify, incorporate
  in other works, reuse and redistribute as freely as possible in any
  form whatsoever and for any purposes, including without limitation
  commercial purposes. These owners may contribute to the Commons to
  promote the ideal of a free culture and the further production of
  creative, cultural and scientific works, or to gain reputation or
  greater distribution for their Work in part through the use and
  efforts of others.

  For these and/or other purposes and motivations, and without any
  expectation of additional consideration or compensation, the person
  associating CC0 with a Work (the "Affirmer"), to the extent that he or
  she is an owner of Copyright and Related Rights in the Work,
  voluntarily elects to apply CC0 to the Work and publicly distribute
  the Work under its terms, with knowledge of his or her Copyright and
  Related Rights in the Work and the meaning and intended legal effect
  of CC0 on those rights.

  1. Copyright and Related Rights. A Work made available under CC0 may
  be protected by copyright and related or neighboring rights
  ("Copyright and Related Rights"). Copyright and Related Rights
  include, but are not limited to, the following:

  the right to reproduce, adapt, distribute, perform, display,
  communicate, and translate a Work;
  moral rights retained by the original author(s) and/or performer(s);
  publicity and privacy rights pertaining to a person's image or
  likeness depicted in a Work;
  rights protecting against unfair competition in regards to a Work,
  subject to the limitations in paragraph 4(a), below;
  rights protecting the extraction, dissemination, use and reuse of data
  in a Work;
  database rights (such as those arising under Directive 96/9/EC of the
  European Parliament and of the Council of 11 March 1996 on the legal
  protection of databases, and under any national implementation
  thereof, including any amended or successor version of such
  directive); and
  other similar, equivalent or corresponding rights throughout the world
  based on applicable law or treaty, and any national implementations
  thereof.

  2. Waiver. To the greatest extent permitted by, but not in
  contravention of, applicable law, Affirmer hereby overtly, fully,
  permanently, irrevocably and unconditionally waives, abandons, and
  surrenders all of Affirmer's Copyright and Related Rights and
  associated claims and causes of action, whether now known or unknown
  (including existing as well as future claims and causes of action), in
  the Work (i) in all territories worldwide, (ii) for the maximum
  duration provided by applicable law or treaty (including future time
  extensions), (iii) in any current or future medium and for any number
  of copies, and (iv) for any purpose whatsoever, including without
  limitation commercial, advertising or promotional purposes (the
  "Waiver"). Affirmer makes the Waiver for the benefit of each member of
  the public at large and to the detriment of Affirmer's heirs and
  successors, fully intending that such Waiver shall not be subject to
  revocation, rescission, cancellation, termination, or any other legal
  or equitable action to disrupt the quiet enjoyment of the Work by the
  public as contemplated by Affirmer's express Statement of Purpose.

  3. Public License Fallback. Should any part of the Waiver for any
  reason be judged legally invalid or ineffective under applicable law,
  then the Waiver shall be preserved to the maximum extent permitted
  taking into account Affirmer's express Statement of Purpose. In
  addition, to the extent the Waiver is so judged Affirmer hereby grants
  to each affected person a royalty-free, non transferable, non
  sublicensable, non exclusive, irrevocable and unconditional license to
  exercise Affirmer's Copyright and Related Rights in the Work (i) in
  all territories worldwide, (ii) for the maximum duration provided by
  applicable law or treaty (including future time extensions), (iii) in
  any current or future medium and for any number of copies, and (iv)
  for any purpose whatsoever, including without limitation commercial,
  advertising or promotional purposes (the "License"). The License shall
  be deemed effective as of the date CC0 was applied by Affirmer to the
  Work. Should any part of the License for any reason be judged legally
  invalid or ineffective under applicable law, such partial invalidity
  or ineffectiveness shall not invalidate the remainder of the License,
  and in such case Affirmer hereby affirms that he or she will not (i)
  exercise any of his or her remaining Copyright and Related Rights in
  the Work or (ii) assert any associated claims and causes of action
  with respect to the Work, in either case contrary to Affirmer's
  express Statement of Purpose.

  4. Limitations and Disclaimers.

  No trademark or patent rights held by Affirmer are waived, abandoned,
  surrendered, licensed or otherwise affected by this document.
  Affirmer offers the Work as-is and makes no representations or
  warranties of any kind concerning the Work, express, implied,
  statutory or otherwise, including without limitation warranties of
  title, merchantability, fitness for a particular purpose, non
  infringement, or the absence of latent or other defects, accuracy, or
  the present or absence of errors, whether or not discoverable, all to
  the greatest extent permissible under applicable law.
  Affirmer disclaims responsibility for clearing rights of other persons
  that may apply to the Work or any use thereof, including without
  limitation any person's Copyright and Related Rights in the Work.
  Further, Affirmer disclaims responsibility for obtaining any necessary
  consents, permissions or other rights required for any use of the
  Work.
  Affirmer understands and acknowledges that Creative Commons is not a
  party to this document and has no duty or obligation with respect to
  this CC0 or use of the Work.

  Contributions thanks to:
    niXman <i.nixman@autistici.org>
    Ely Arzhannikov <iarzhannikov@gmail.com>
    Alexey Pavlov <alexpux@gmail.com>
    Ray Donnelly <mingw.android@gmail.com>

*/

#include "path_conv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************************/

typedef struct test_data_t {
    const char *src;
    const char *dst;
    bool fail;
} test_data;

static const test_data datas[] = {
    {"-LIBPATH:../lib", "-LIBPATH:../lib", false}
    ,{"-LIBPATH:../lib:/tmp", "-LIBPATH:..\\lib;C:\\msys32\\tmp", false}
    ,{"//Collection:http://tfsserver", "//Collection:http://tfsserver", false}
    ,{"/Collection:http://tfsserver", "/Collection:http://tfsserver", false}
    ,{"-L'/foo bar/boo' PREFIX='/foo bar/boo'", "-L'C:/msys32/foo bar/boo' PREFIX='/foo bar/boo'", false}
    ,{"-L'/foo /bar/boo' PREFIX='/foo /bar/boo'", "-L'C:/msys32/foo /bar/boo' PREFIX='/foo /bar/boo'", false}
    ,{"C:\\foo\\bar", "C:\\foo\\bar", false} // 0
    ,{"/foo/bar;", "/foo/bar;", false} // 1
    ,{"//foobar", "/foobar", false} // 2
    ,{"//foo\\bar", "/foo/bar", false} // 3
    ,{"//foo/bar", "//foo/bar", false} // 4
    ,{"/c:\\foo\\bar", "c:/foo/bar", false} // 5
    ,{"foo=/bar", "foo=C:/msys32/bar", false} // 6
    ,{"-foo,/bar", "-foo,C:/msys32/bar", false} // 7
    ,{"-I/foo,/bar", "-I/foo,C:/msys32/bar", false} // 8
    ,{"-I/foo", "-IC:/msys32/foo", false} // 9
    ,{"-L/foo", "-LC:/msys32/foo", false} // 9
    ,{"-L'/foo /bar'", "-L'C:/msys32/foo /bar'", false} // 9
    ,{"-L'/foo bar'", "-L'C:/msys32/foo bar'", false} // 9
    ,{"'/opt /bin'", "'C:/msys32/opt /bin'", false}
    ,{"'/opt files/bin'", "'C:/msys32/opt files/bin'", false}
    ,{"/", "C:/msys32/", false} // 10
    ,{"/..", "/..", false} // 11
    ,{"x:x:/x", "x:x:/x", false} // 12
    ,{"x::x:/xx", "x;x;C:\\msys32\\xx", false} // 13
    ,{"x::x/z:x", "x;x\\z;x", false} // 14
    ,{"x::/x z:x", "x;C:\\msys32\\x z;x", false} // 14
    ,{"'x::/x z:x'", "'x;C:\\msys32\\x z;x'", false} // 14
    ,{"/dev/null", "nul", false} // 14
    ,{"'/dev/null'", "'nul'", false} // 14
    ,{"/tmp:/tmp", "C:\\msys32\\tmp;C:\\msys32\\tmp", false} // 14
    ,{"'/tmp:/tmp'", "'C:\\msys32\\tmp;C:\\msys32\\tmp'", false} // 14
    ,{"-L'/tmp:/tmp'", "-L'C:\\msys32\\tmp;C:\\msys32\\tmp'", false} // 14
    ,{"-L/tmp:/tmp", "-LC:\\msys32\\tmp;C:\\msys32\\tmp", false} // 14
    ,{"'/bin:/Program Files:/lib'", "'C:\\msys32\\usr\\bin;C:\\msys32\\Program Files;C:\\msys32\\lib'", false}
    ,{"'-L/opt /bin'", "'-LC:/msys32/opt /bin'", false}
    ,{"-w -- INSTALL_ROOT=C:/Test/ports64", "-w -- INSTALL_ROOT=C:/Test/ports64", false} // 15
    ,{"-w -- INSTALL_ROOT=C:\\Test\\ports64", "-w -- INSTALL_ROOT=C:\\Test\\ports64", false} // 16
    ,{"-IC:/Test/ports64", "-IC:/Test/ports64", false} // 17
    //,{"-g -O2 -I/foo -L/foo PREFIX=/foo", "-g -O2 -IC:/msys32/foo -LC:/msys32/foo PREFIX=C:/msys32/foo", false}
    //,{"-g -O2 -I/foo -L'/foo bar/boo' PREFIX='/foo bar/boo'", "-g -O2 -IC:/msys32/foo -L'C:/msys32/foo bar/boo' PREFIX='C:/msys32/foo bar/boo'", false}
    ,{"'C:\\foo\\bar'", "'C:\\foo\\bar'", false} // 0
    ,{"'/foo/bar;'", "'/foo/bar;'", false} // 1
    ,{"'//foobar'", "'/foobar'", false} // 2
    ,{"'//foo\\bar'", "'/foo/bar'", false} // 3
    ,{"'//foo/bar'", "'//foo/bar'", false} // 4
    ,{"'/c:\\foo\\bar'", "'c:/foo/bar'", false} // 5
    ,{"'foo=/bar'", "'foo=C:/msys32/bar'", false} // 6
    ,{"'-foo,/bar'", "'-foo,C:/msys32/bar'", false} // 7
    ,{"'-I/foo,/bar'", "'-I/foo,C:/msys32/bar'", false} // 8
    ,{"'-I/foo'", "'-IC:/msys32/foo'", false} // 9
    ,{"'/'", "'C:/msys32/'", false} // 10
    ,{"'/..'", "'/..'", false} // 11
    ,{"'x:x:/x'", "'x:x:/x'", false} // 12
    ,{"'x::x:/x'", "'x;x;X:\\'", false} // 13
    ,{"'-w -- INSTALL_ROOT=C:/Test/ports64'", "'-w -- INSTALL_ROOT=C:/Test/ports64'", false} // 15
    ,{"'-w -- INSTALL_ROOT=C:\\Test\\ports64'", "'-w -- INSTALL_ROOT=C:\\Test\\ports64'", false} // 16
    ,{"'-IC:/Test/ports64'", "'-IC:/Test/ports64'", false} // 17
    ,{"http://google.ru", "http://google.ru", false}
    ,{"'http://google.ru'", "'http://google.ru'", false}
    ,{"'-I/foo,http://google.ru'", "'-I/foo,http://google.ru'", false} // 8
    ,{"'x::http://google.ru:x'", "'x;http://google.ru;x'", false} // 8
    ,{"", "", false}
    ,{"''", "''", false}
    ,{"/usr/local/info:/usr/share/info:/usr/info:", "C:\\msys32\\usr\\local\\info;C:\\msys32\\usr\\share\\info;C:\\msys32\\usr\\info", false}
    ,{"as_nl=\r", "as_nl=\r", false}
    ,{"as_nl=\n", "as_nl=\n", false}
    ,{"as_nl= ", "as_nl= ", false}
    ,{"as_nl='\r'", "as_nl='\r'", false}
    ,{"as_nl='\n'", "as_nl='\n'", false}
    ,{"as_nl=' '", "as_nl=' '", false}
    ,{"C:/Test/ports64", "C:/Test/ports64", false}
    ,{"'C:/Test/ports64'", "'C:/Test/ports64'", false}
    ,{"('C:/msys32')", "('C:/msys32')", false}
    ,{"--implib=./libblah.a", "--implib=./libblah.a", false}
    ,{"'--implib=./libblah.a'", "'--implib=./libblah.a'", false}
    ,{"--implib=../lib/libblah.a", "--implib=../lib/libblah.a", false}
    ,{"'--implib=../lib/libblah.a'", "'--implib=../lib/libblah.a'", false}
    //,{"    /foo", "    C:/msys32/foo", false}
    //,{"'    /foo'", "'    C:/msys32/foo'", false}
    //,{"files = '''__init__.py z/codegen.py b/codegen_main.py codegen_docbook.py config.py dbustypes.py parser.py utils.py''' \n", "files = '''__init__.py z/codegen.py b/codegen_main.py codegen_docbook.py config.py dbustypes.py parser.py utils.py''' \n", false}
    ,{
"import sys\n"
"import os\n"
"\n"
"def main():\n"
"    print sys.argv\n"
"    print os.path.exists('some/path')\n"
"    print os.path.exists('/some/path')\n"
"\n"
"if __name__ == '__main__':\n"
"    main()\n",
"import sys\n"
"import os\n"
"\n"
"def main():\n"
"    print sys.argv\n"
"    print os.path.exists('some/path')\n"
"    print os.path.exists('/some/path')\n"
"\n"
"if __name__ == '__main__':\n"
"    main()\n", false}
    ,{
"import sys\n"
"import os\n"
"\n"
"def main():\n"
"    print sys.argv\n"
"    print os.path.exists('some/path')\n"
"    print os.path.exists('/some/path')\n"
"\n"
"if __name__ == '__main__':\n"
"    main()\n",
"import sys\n"
"import os\n"
"\n"
"def main():\n"
"    print sys.argv\n"
"    print os.path.exists('some/path')\n"
"    print os.path.exists('/some/path')\n"
"\n"
"if __name__ == '__main__':\n"
"    main()\n", true}
    ,{"E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.vert;E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.frag", "E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.vert;E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.frag", false}
    ,{"'E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.vert;E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.frag'", "'E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.vert;E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.frag'", false}
    ,{"-IC:/msys64/mingw64/include", "-IC:/msys64/mingw64/include", false}
    ,{"E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.vert", "E:/msys64/home/Wayne/src/kicad/product/common/gal/opengl/shader.vert", false}
    ,{"-//OASIS//DTD", "-//OASIS//DTD", false}
    ,{"-DCMAKE_INSTALL_PREFIX:PATH=/bb/pkg/mingw", "-DCMAKE_INSTALL_PREFIX:PATH=C:/msys32/bb/pkg/mingw", false}
    ,{"-DCMAKE_INSTALL_PREFIX=/bb/pkg/mingw", "-DCMAKE_INSTALL_PREFIX=C:/msys32/bb/pkg/mingw", false}
    ,{"foo:echo /bar/baz", "foo:echo /bar/baz", false}
    ,{"@/foo/bar", "@C:/msys32/foo/bar", false}
    ,{"@/foo/bar@", "@C:/msys32/foo/bar@", false}
    ,{"'@/foo/bar'", "'@C:/msys32/foo/bar'", false}
    ,{"///foo/bar", "//foo/bar", false}
    ,{".:./", ".;.\\", false}
    ,{"..:./", "..;.\\", false}
    ,{"../:./", "..\\;.\\", false}
    ,{"../:./", "..\\;.\\", false}
    ,{"../aa/:./", "..\\aa\\;.\\", false}
    ,{"../", "../", false}
    ,{"/foo/bar/", "C:/msys32/foo/bar/", false}
    ,{"-MExtUtils::ParseXS=process_file", "-MExtUtils::ParseXS=process_file", false}
    ,{"/foo/bin/../libs", "C:/msys32/foo/bin/../libs", false}
    ,{"'/foo/bin/../libs'", "'C:/msys32/foo/bin/../libs'", false}
    ,{"ExtUtils::ParseXS::process_file(filename => \"$<\", output => \"$@\", typemap => \"$(PURPLE_PERL_TOP)/common/typemap\");",
      "ExtUtils::ParseXS::process_file(filename => \"$<\", output => \"$@\", typemap => \"$(PURPLE_PERL_TOP)/common/typemap\");", false}
    ,{"-Wl,test/path", "-Wl,test/path", false}
    ,{"-Wl,/test/path", "-Wl,C:/msys32/test/path", false}
    ,{"-Wl,--out-implib,64/shlib/libgcc_s.a.tmp", "-Wl,--out-implib,64/shlib/libgcc_s.a.tmp", false}
    ,{"-Wl,--out-implib,_64/shlib/libgcc_s.a.tmp", "-Wl,--out-implib,_64/shlib/libgcc_s.a.tmp", false}
    ,{"/64/shlib/libgcc_s.a.tmp", "C:/msys32/64/shlib/libgcc_s.a.tmp", false}
    ,{0, 0, false}
};

/***************************************************************************/

const char* escape(const char* str, char* dst) {
    char* tmp = dst;
    for (; str != NULL && *str != '\0'; ++str) {
        switch(*str) {
            case '\r': *(tmp++) = '\\'; *(tmp++)='r'; break;
            case '\n': *(tmp++) = '\\'; *(tmp++)='n'; break;
            case '\b': *(tmp++) = '\\'; *(tmp++)='b'; break;
            case '\t': *(tmp++) = '\\'; *(tmp++)='t'; break;
            default:
                    *(tmp++) = *str;
        }
    }
    *tmp = '\0';
    return dst;
}

void litter_buffer(char* buf, size_t len) {
    for (size_t p = 0; p != len; ++p) {
        buf[p] = p + 1;
    }
}

int main() {
    int passed = 0;
    int total = 0;
    for ( const test_data *it = &datas[0]; it && it->src; ++it ) {
        total += 1;
        const char *path = it->src;
        const size_t blen = strlen(it->dst)*2 + 10;
        char *buf = (char*)malloc(blen);
        litter_buffer(buf, blen);

        const char *res = convert(buf, blen, path);
        if ( 0 != strcmp(res, it->dst)) {
            char epath[1024];
            char eres[1024];
            char edst[1024];
            printf("test %ld failed: src=\"%s\", dst=\"%s\" expect=\"%s\"\n", (it - &datas[0]), escape(path, epath), escape(res, eres), escape(it->dst, edst));
            if (it->fail) {
                passed += 1;
            }
        } else {
            char epath[1024];
            char eres[1024];
            passed += 1;
            printf("test %ld passed: src=\"%s\", dst=\"%s\"\n", (it - &datas[0]), escape(path, epath), escape(res, eres));
        }

        free(buf);
    }

    printf("%f\n", (float)passed / total);

    return total != passed;
}

/***************************************************************************/
