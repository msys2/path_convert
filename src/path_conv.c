#include "path_conv.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char* ROOT_PATH = "C:/msys2";

typedef enum PATH_TYPE_E {
    NONE = 0,
    SIMPLE_WINDOWS_PATH,
    ESCAPE_WINDOWS_PATH,
    WINDOWS_PATH_LIST,
    UNC,
    ESCAPED_PATH,
    ROOTED_PATH,
    POSIX_PATH_LIST,
    URL,
    ESCAPED_ROOTED_PATH,
} path_type;

path_type find_path_start_and_type(const char** src, int recurse, const char* end);
void copy_to_dst(const char* from, const char* to, char** dst, const char* dstend);
void convert_path(const char** from, const char* to, path_type type, char** dst, const char* dstend);

void swp_convert(const char** from, const char* to, char** dst, const char* dstend);
void ewp_convert(const char** from, const char* to, char** dst, const char* dstend);
void wpl_convert(const char** from, const char* to, char** dst, const char* dstend);
void unc_convert(const char** from, const char* to, char** dst, const char* dstend);
void ep_convert(const char** from, const char* to, char** dst, const char* dstend);
void rp_convert(const char** from, const char* to, char** dst, const char* dstend);
void erp_convert(const char** from, const char* to, char** dst, const char* dstend);
void url_convert(const char** from, const char* to, char** dst, const char* dstend);
void ppl_convert(const char** from, const char* to, char** dst, const char* dstend);

const char* convert(char *dst, size_t dstlen, const char *src) {
    path_type type;
    const char* srcit = src;
    const char* srcbeg = src;
    char* dstit = dst;
    char* dstend = dst + dstlen;
    type = find_path_start_and_type(&srcit, 0, NULL);

    if (type != NONE) {
        printf("%d\n", type);
        copy_to_dst(srcbeg, srcit, &dstit, dstend);
        convert_path(&srcit, NULL, type, &dstit, dstend);
    }

    if (dstit != dstend) {
        *dstit = '\0';
    }

    return dst;
}

void copy_to_dst(const char* from, const char* to, char** dst, const char* dstend) {
    for (; (from != to) && (*dst != dstend); ++from, ++(*dst)) {
        **dst = *from;
    }
}

path_type find_path_start_and_type(const char** src, int recurse, const char* end) {
    const char* it = *src;

    if (*it == '\0' || it == end) return NONE;

    if (isalpha(*it) && *(it + 1) == ':') {
        if ((recurse && *(it + 2) == '/') ||
            (*(it + 2) == '\\')) {
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

        path_type result = NONE;

        for (; *it != '\0' && it != end; ++it) {
            switch(*it) {
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
        *src = it + 1;
        return find_path_start_and_type(src, 1, end);
    }

    if (*it == '"' || *it == '\'') {
        *src = it + 1;
        return find_path_start_and_type(src, 1, end);
    }

    if (*it == '-' && *(it + 1) == '/') {
        *src = it + 1;
        return find_path_start_and_type(src, 1, end);
    }


    int starts_with_minus = (*it == '-');
    int not_starte_with_spec = !starts_with_minus &&
                               (*it != '"') &&
                               (*it != '\'') &&
                               (*it != '@');

    if (recurse != 0) {
        const char* it2 = it - 1;
        not_starte_with_spec = (*it2 != '-') &&
                               (*it2 != '"') &&
                               (*it2 != '\'') &&
                               (*it2 != '@');
    }

    for (const char* it2 = it; *it2 != '\0' && it2 != end; ++it2) {
        if (*it2 == '=') {
            *src = it2 + 1;
            return find_path_start_and_type(src, 1, end);
        }

        if (*it2 == ',' && starts_with_minus) {
            *src = it2 + 1;
            return find_path_start_and_type(src, 1, end);
        }

        if (*it2 == ':' && not_starte_with_spec) {
            it2 += 1;
            if (*it2 == '/' || *it2 == ':' || *it2 == '.') {
                return POSIX_PATH_LIST;
            } else {
                return SIMPLE_WINDOWS_PATH;
            }
        }
    }

    if (*it == '-' && *(it + 1) == 'I') {
        *src = it + 2;
        return find_path_start_and_type(src, 1, end);
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
        case ESCAPED_ROOTED_PATH: erp_convert(from, to, dst, dstend); break;
        case NONE: // prevent warnings;
        default:
                return;
    }
}

void swp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    for (; (it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
        **dst = *it;
    }
    *from = it;
}

void ewp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    it += 1;
    for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
        if (*it == '\\') {
            **dst = '/';
        } else {
            **dst = *it;
        }
    }
    *from = it;
}

void wpl_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
        **dst = *it;
    }
    *from = it;
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
    *from = it;
}

void ep_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    it += 1;
    for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
        if (*it == '\\') {
            **dst = '/';
        } else {
            **dst = *it;
        }
    }
    *from = it;
}

void rp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = ROOT_PATH;

    for (; *it != '\0' && *dst != dstend; ++it, ++(*dst)) {
        **dst = *it;
    }

    it = *from;
    if (*(it + 1) != '\0' && *(it + 1) != '\'' && *(it + 1) != '"') {
        for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
            if (*it == '\\') {
                **dst = '/';
            } else {
                **dst = *it;
            }
        }
    } else {
        it += 1;
    }


    if ((*dst != dstend) && (*it != '\0' && it != to)) {
        char ch = *it;
        if (ch == '\'' || ch == '"') {
            **dst = ch;
            *dst += 1;
            it += 1;
        }
    }

    *from = it;
}

void erp_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = ROOT_PATH;

    for (; *it != '\0' && *dst != dstend; ++it, ++(*dst)) {
        if (*it == '/') {
            **dst = '\\';
        } else {
            **dst = *it;
        }
    }

    it = *from;
    if (*(it + 1) != '\0' && *(it + 1) != '\'' && *(it + 1) != '"') {
        for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
            if (*it == '/') {
                **dst = '\\';
            } else {
                **dst = *it;
            }
        }
    } else {
        it += 1;
    }


    if ((*dst != dstend) && (*it != '\0' && it != to)) {
        char ch = *it;
        if (ch == '\'' || ch == '"') {
            **dst = ch;
            *dst += 1;
            it += 1;
        }
    }

    *from = it;
}

void url_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    for (; (*it != '\0' && it != to) && (*dst != dstend); ++it, ++(*dst)) {
        if (*it == '\\') {
            **dst = '/';
        } else {
            **dst = *it;
        }
    }
    *from = it;
}

void ppl_convert(const char** from, const char* to, char** dst, const char* dstend) {
    const char* it = *from;
    const char* beg = it;
    int prev_was_simc = 0;
    int url = 0;
    for (; (*it != '\0') && (*dst != dstend); ++it) {
        if (*it == ':') {
            if (prev_was_simc) {
                continue;
            }
            if (*from + 2 < it && *(it - 2) == '/' && *(it - 1) == '/') {
                url = 1;
                continue;
            }
            prev_was_simc = 1;
            path_type type = find_path_start_and_type(&beg, 0, it);

            if (type == NONE) {
                return;
            }

            char* start = *dst;
            convert_path(&beg, it, type, dst, dstend);

            if (!url) {
                for (; start != *dst; ++start) {
                    if (*start == '/') {
                        *start = '\\';
                    }
                }
            }
            url = 0;

            **dst = ';';
            *dst += 1;
        }

        if (*it != ':' && prev_was_simc) {
            prev_was_simc = 0;
            beg = it;
        }
    }

    path_type type = find_path_start_and_type(&beg, 0, it);

    if (type == NONE) {
        return;
    }

    char* start = *dst;
    convert_path(&beg, it, type, dst, dstend);

    if (!url) {
        for (; start != *dst; ++start) {
            if (*start == '/') {
                *start = '\\';
            }
        }
    }

}
