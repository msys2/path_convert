#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MSYS__
# include <sys/cygwin.h>
#endif
#include <ctype.h>

#include "path_conv.h"

#ifndef __MSYS__
static const char* ROOT_PATH = "C:/msys2";
#include <unistd.h>
#endif

typedef enum PATH_TYPE_E {
    NONE = 0,
    SIMPLE_WINDOWS_PATH,
    ESCAPE_WINDOWS_PATH,
    WINDOWS_PATH_LIST,
    UNC,
    ESCAPED_PATH,
    ROOTED_PATH,
    POSIX_PATH_LIST,
    URL
} path_type;

int is_special_posix_path(const char* from, const char* to, char** dst, const char* dstend);
void posix_to_win32_path(const char* from, const char* to, char** dst, const char* dstend);


path_type find_path_start_and_type(const char** src, int recurse, const char* end);
void copy_to_dst(const char* from, const char* to, char** dst, const char* dstend);
void convert_path(const char** from, const char* to, path_type type, char** dst, const char* dstend);

//Transformations
//SIMPLE_WINDOWS_PATH converter. Copy as is. Hold C:\Something\like\this
void swp_convert(const char** from, const char* to, char** dst, const char* dstend);
//ESCAPE_WINDOWS_PATH converter. Turn backslashes to slashes and skip first /. Hold /C:\Somethind\like\this
void ewp_convert(const char** from, const char* to, char** dst, const char* dstend);
//WINDOWS_PATH_LIST converter. Copy as is. Hold /something/like/this;
void wpl_convert(const char** from, const char* to, char** dst, const char* dstend);
//UNC convert converter. Copy as is. Hold //somethig/like/this
void unc_convert(const char** from, const char* to, char** dst, const char* dstend);
//ESCAPED_PATH converter. Turn backslashes to slashes and skip first /. Hold //something\like\this
void ep_convert(const char** from, const char* to, char** dst, const char* dstend);
//ROOTED_PATH converter. Prepend root dir to front. Hold /something/like/this
void rp_convert(const char** from, const char* to, char** dst, const char* dstend);
//URL converter. Copy as is.
void url_convert(const char** from, const char* to, char** dst, const char* dstend);
//POSIX_PATH_LIST. Hold x::x/y:z
void ppl_convert(const char** from, const char* to, char** dst, const char* dstend);


void find_end_of_posix_list(const char** to, int* in_string) {
    for (; **to != '\0' && (in_string ? (**to != *in_string) : **to != ' '); ++*to) {
    }

    if (**to == *in_string) {
        *in_string = 0;
    }
}

void find_end_of_rooted_path(const char** to, int* in_string) {
    for (; **to != '\0'; ++*to) {
        if (*in_string == 0 && **to == ' ') {
            return;
        }

        if (**to == *in_string) {
            *in_string = 0;
            return;
        }

        if (**to == '/') {
            if (*(*to - 1) == ' ') {
                *to -= 1;
                return;
            }
        }
    }
}

void sub_convert(const char** from, const char** to, char** dst, const char* dstend, int* in_string) {
    const char* copy_from = *from;
    path_type type = find_path_start_and_type(from, false, *to);

    if (type == POSIX_PATH_LIST) {
        find_end_of_posix_list(to, in_string);
    }

    if (type == ROOTED_PATH) {
        find_end_of_rooted_path(to, in_string);
    }

    copy_to_dst(copy_from, *from, dst, dstend);

    if (type != NONE) {
        convert_path(from, *to, type, dst, dstend);
    }

    if (*dst != dstend) {
        **dst = **to;
        *dst += 1;
    }
}

const char* convert(char *dst, size_t dstlen, const char *src) {
    if (dst == NULL || dstlen == 0 || src == NULL) {
        return dst;
    }

    int need_convert = false;
    for (const char* it = src; it != '\0'; ++it) {
        if (*it == '\\' || *it == '/') {
            need_convert = true;
            break;
        }
    }

    if (!need_convert) {
        return src;
    }

    const char* srcit = src;
    const char* srcbeg = src;
    char* dstit = dst;
    char* dstend = dst + dstlen;

    int prev_was_space = 0;
    int in_string = false;
    int in_string2;

    for (; *srcit != '\0'; ++srcit) {
        if (*srcit == '\'' || *srcit == '"') {
            if (in_string == *srcit) {
                in_string = 0;
            } else {
                in_string = *srcit;
            }
            continue;
        }

        if (isspace(*srcit)) {
            if (prev_was_space) {
                continue;
            }

            prev_was_space = true;
            in_string2 = in_string;
            sub_convert(&srcbeg, &srcit, &dstit, dstend, &in_string2);
            srcbeg = srcit + 1;
        }

        if (!isspace(*srcit) && prev_was_space) {
            prev_was_space = false;
            srcbeg = srcit;
            if (in_string != in_string2) {
                srcbeg += 1;
            }
            in_string = in_string2;
        }
    }

    sub_convert(&srcbeg, &srcit, &dstit, dstend, &in_string);

    if (dstit - dst < 2) {
        dstit = dst;
        copy_to_dst(src, NULL, &dstit, dstend);
        *dstit = '\0';
    }

    return dst;
}

void copy_to_dst(const char* from, const char* to, char** dst, const char* dstend) {
    for (; (*from != '\0') && (from != to) && (*dst != dstend); ++from, ++(*dst)) {
        **dst = *from;
    }
}

const char** move(const char** p, int count) {
    *p += count;
    return p;
}

path_type find_path_start_and_type(const char** src, int recurse, const char* end) {
    const char* it = *src;

    if (*it == '\0' || it == end) return NONE;

    path_type result = NONE;

    if (isalpha(*it) && *(it + 1) == ':') {
        if (*(it + 2) == '\\') {
            return SIMPLE_WINDOWS_PATH;
        }

        if (*(it + 2) == '/' && memchr(it + 2, ':', end - (it + 2)) == NULL) {
            return SIMPLE_WINDOWS_PATH;
        }
    }

    if (*it == '/') {
        it += 1;

        if (isalpha(*it) && *(it + 1) == ':') {
            return ESCAPE_WINDOWS_PATH;
        }

        if (*it == '.' && *(it + 1) == '.') {
            return SIMPLE_WINDOWS_PATH;
        }

        int double_slashed = 0;
        if (*it == '/') {
            double_slashed = 1;
            it += 1;
        }

        if (double_slashed && *it == ':') {
            return URL;
        }


        for (; *it != '\0' && it != end; ++it) {
            switch(*it) {
                case ':': {char ch = *(it + 1); if (ch == '/' || ch == ':' || ch == '.') return POSIX_PATH_LIST;}
                case '/': result = (double_slashed) ? UNC : ROOTED_PATH; break;
                case ';': return WINDOWS_PATH_LIST;
            }
        }

        if (result != NONE) {
            return result;
        }

        return (double_slashed) ? ESCAPED_PATH : ROOTED_PATH;
    }

    if (*it == '@') {
        return find_path_start_and_type(move(src, 1), true, end);
    }

    if (*it == '"' || *it == '\'') {
        return find_path_start_and_type(move(src, 1), true, end);
    }

    int starts_with_minus = (*it == '-');

    if (starts_with_minus) {
        char n2 = *(it + 2);
        char n3 = *(it + 3);
        char n4 = *(it + 4);
        char n5 = *(it + 5);

        if (n2 == '/') {
            it += 2;
            result = ROOTED_PATH;
        } else if ((n2 == '\'' || n2 == '"') && n3 == '/') {
            it += 3;
            result = ROOTED_PATH;
        }

        if (isalpha(n2) && n3 == ':' && n4 == '/') {
            *src = it + 2;
            return SIMPLE_WINDOWS_PATH;
        }

        if ((n2 == '\'' || n2 == '"') && isalpha(n3) && n4 == ':' && n5 == '/') {
            *src = it + 3;
            return SIMPLE_WINDOWS_PATH;
        }
    }


    for (const char* it2 = it; *it2 != '\0' && it2 != end; ++it2) {
        char ch = *it2;
        if (ch == '=') {
            *src = it2 + 1;
            return find_path_start_and_type(src, true, end);
        }

        if (ch == ',' && starts_with_minus) {
            *src = it2 + 1;
            return find_path_start_and_type(src, true, end);
        }

        if (ch == ':') {
            it2 += 1;
            ch = *it2;
            if (ch == '/' || ch == ':' || ch == '.') {
                if (ch == '/' && *(it2 + 1) == '/') {
                    return URL;
                } else {
                    return POSIX_PATH_LIST;
                }
            } else {
                return SIMPLE_WINDOWS_PATH;
            }
        }
    }

    if (result != NONE) {
        *src = it;
        return result;
    }

    return SIMPLE_WINDOWS_PATH;
}

void convert_path(const char** from, const char* to, path_type type, char** dst, const char* dstend) {
    switch(type) {
        case SIMPLE_WINDOWS_PATH: swp_convert(from, to, dst, dstend); break;
        case ESCAPE_WINDOWS_PATH: ewp_convert(from, to, dst, dstend); break;
        case WINDOWS_PATH_LIST: wpl_convert(from, to, dst, dstend); break;
        case UNC: unc_convert(from, to, dst, dstend); break;
        case ESCAPED_PATH: ep_convert(from, to, dst, dstend); break;
        case ROOTED_PATH: rp_convert(from, to, dst, dstend); break;
        case URL: url_convert(from, to, dst, dstend); break;
        case POSIX_PATH_LIST: ppl_convert(from, to, dst, dstend); break;
        case NONE: // prevent warnings;
        default:
                return;
    }
}

void swp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    copy_to_dst(*from, to, dst, dstend);
}

void ewp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    *from += 1;
    unc_convert(from, to, dst, dstend);
}

void wpl_convert(const char** from, const char* to, char** dst, const char* dstend) {
    swp_convert(from, to, dst, dstend);
}

void unc_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
        if (*it == '\\') {
            **dst = '/';
        } else {
            **dst = *it;
        }
    }
}

void ep_convert(const char** from, const char* to, char** dst, const char* dstend) {
    ewp_convert(from, to, dst, dstend);
}

void rp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    const char* real_to = to;

    if (*real_to == '\0')  {
        real_to -= 1;
        if (*real_to != '\'' && *real_to != '"') {
            real_to += 1;
        }
    }

    if (!is_special_posix_path(*from, real_to, dst, dstend)) {
        posix_to_win32_path(it, real_to, dst, dstend);
    }

    if (real_to != to) {
        **dst = *real_to;
        *dst += 1;
    }
}

void url_convert(const char** from, const char* to, char** dst, const char* dstend) {
    unc_convert(from, to, dst, dstend);
}

void subp_convert(const char** from, const char* end, int is_url, char** dst, const char* dstend) {
    const char* begin = *from;
    path_type type = find_path_start_and_type(from, 0, end);
    copy_to_dst(begin, *from, dst, dstend);

    if (type == NONE) {
        return;
    }

    char* start = *dst;
    convert_path(from, end, type, dst, dstend);

    if (!is_url) {
        for (; start != *dst; ++start) {
            if (*start == '/') {
                *start = '\\';
            }
        }
    }
}

void ppl_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    const char* beg = it;
    int prev_was_simc = 0;
    int is_url = 0;
    for (; (*it != '\0' && it != to) && (*dst != dstend); ++it) {
        if (*it == ':') {
            if (prev_was_simc) {
                continue;
            }
            if (*(it + 1) == '/' && *(it + 2) == '/') {
                is_url = 1;
                continue;
            }
            prev_was_simc = 1;
            subp_convert(&beg, it, is_url, dst, dstend);
            is_url = 0;

            **dst = ';';
            *dst += 1;
        }

        if (*it != ':' && prev_was_simc) {
            prev_was_simc = 0;
            beg = it;
        }
    }

    if (!prev_was_simc) {
        subp_convert(&beg, it, is_url, dst, dstend);
    } else {
        *dst -= 1;
    }
}

int is_special_posix_path(const char* from, const char* to, char** dst, const char* dstend) {
    const char dev_null[] = "/dev/null";

    if ((to - from) == (sizeof(dev_null) - 1) && strncmp(from, "/dev/null", to - from) == 0) {
        copy_to_dst("nul", NULL, dst, dstend);
        return true;
    }
    return false;
}

#ifdef __MSYS__

void posix_to_win32_path(const char* from, const char* to, char** dst, const char* dstend) {
    if ( from != to ) {
        char *one_path = (char*)alloca(to-from+1);
        strncpy(one_path, from, to-from);
        one_path[to-from] = '\0';
        char win32_path1[PATH_MAX + 1];
        ssize_t result = cygwin_conv_path(CCP_POSIX_TO_WIN_A|CCP_ABSOLUTE, one_path, win32_path1, PATH_MAX+1);
        printf("called cygwin_conv_path(CCP_POSIX_TO_WIN_A,%s -> %s, in-size %d, result = %zd\n", one_path, win32_path1, PATH_MAX+1, result);
        if( result !=0 ) {
            copy_to_dst(one_path, NULL, dst, dstend);
        } else {
            char *win32_path=win32_path1;
            for (; (*win32_path != '\0') && (*dst != dstend); ++win32_path, ++(*dst)) {
                **dst = (*win32_path == '\\') ? '/' : *win32_path;
            }
        }
    }
}

#else

void posix_to_win32_path(const char* from, const char* to, char** dst, const char* dstend) {
    copy_to_dst(ROOT_PATH, NULL, dst, dstend);

    for (; (*from != '\0' && from != to) && (*dst != dstend); ++from, ++(*dst)) {
        if (*from == '\\') {
            **dst = '/';
        } else {
            **dst = *from;
        }
    }
}

#endif
