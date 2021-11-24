/**
 *  Copyright (C) 2017 Masatoshi Teruya
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *  src/parse.c
 *  lua-url
 *  Created by Masatoshi Teruya on 17/10/19.
 *
 */

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// lualib
#include "lauxhlib.h"

/**
 *  RFC 3986
 *
 *  pct-encoded     = "%" HEXDIG HEXDIG
 *  pchar           = "!" / "$" / "%" / "&" / "'" / "(" / ")" / "*" / "+" / ","
 *                  / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~"
 *                  / ALPHA / DIGIT /
 *
 *  userinfo        = *( pchar except "@" )
 *  hostname        = *( pchar except ":" / "@" )
 *  query           = *( pchar / "/" / "?" )
 *  fragment        = *( pchar / "/" / "?" )
 *
 *
 *  WHATWG-URL
 *
 *  URL-units           = pct-encoded / URL-code-points
 *  URL-code-points     = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / ","
 *                      / "-" / "." / "/" / ":" / ";" / "=" / "?" / "@" / "_"
 *                      / "~" / ALPHA / DIGIT
 *
 *  followings are jump-symbols;
 *      path-delimiter  = "/"
 *      port            = ":"
 *      userinfo        = "@"
 *      query           = "?"
 *      fragment        = "#"
 */
static const unsigned char URIC[256] = {
    //  ctrl-code: 0-32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //                       SP      "
    0, 0, 0, 0, 0, 0, 0, 0, '!', 0, '#', '$', '%', '&', '\'', '(', ')', '*',
    '+', ',', '-', '.',
    //  use query and fragment
    '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    //  use hostname
    ':',
    //       <       >
    ';', 0, '=', 0,
    //  use query and fragment
    '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N',
    //                                                              [  \  ]  ^
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0, 0, 0, 0, '_',
    //  `
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    //                                                              {  |  }
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0, 0, 0, '~'};

static const unsigned char HEXDIGIT[256] = {
    0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0, 0, 0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0,
    0, 0, 0, 0,   0,   'A', 'B', 'C', 'D', 'E', 'F', 0,   0,   0, 0,
    0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0, 0, 0, 0,   0,   0,   0,   'a', 'b', 'c', 'd', 'e', 'f'};

/**
 *  pct-encoded     = "%" HEXDIG HEXDIG
 *  HEXDIG          = "A" / "B" / "C" / "D" / "E" / "F"
 *                  / "a" / "b" / "c" / "d" / "e" / "f"
 *                  / DIGIT
 */
static inline int is_percentencoded(const unsigned char *str)
{
    return HEXDIGIT[str[0]] && HEXDIGIT[str[1]];
}

/**
 *  IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
 *  dec-octet   = DIGIT                 ; 0-9
 *              / %x31-39 DIGIT         ; 10-99
 *              / "1" 2DIGIT            ; 100-199
 *              / "2" %x30-34 DIGIT     ; 200-249
 *              / "25" %x30-35          ; 250-255
 */
static inline int parse_ipv4(unsigned char *url, size_t urllen, size_t *cur)
{
    size_t pos  = *cur;
    size_t head = pos;
    int nseg    = 0;
    int dec     = -1;

    for (; pos < urllen; pos++) {
        switch (url[pos]) {
        case '0' ... '9':
            if (pos - head < 4) {
                // convert to integer
                if (dec == -1) {
                    dec = url[pos] - '0';
                } else {
                    dec = (dec << 3) + (dec << 1) + (url[pos] - '0');
                }

                if (dec <= 0xFF) {
                    continue;
                }
            }
            break;

        case '.':
            if (pos - head && nseg < 3) {
                dec  = -1;
                head = pos + 1;
                nseg++;
                continue;
            }
            break;

        default:
            // done
            if (nseg == 3 && dec != -1) {
                *cur = pos;
                return url[pos];
            }
        }
        break;
    }

    // illegal byte sequence
    *cur = pos;
    return url[head];
}

/**
 * IPv6address  =                            6( h16 ":" ) ls32
 *              /                       "::" 5( h16 ":" ) ls32
 *              / [               h16 ] "::" 4( h16 ":" ) ls32
 *              / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
 *              / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
 *              / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
 *              / [ *4( h16 ":" ) h16 ] "::"              ls32
 *              / [ *5( h16 ":" ) h16 ] "::"              h16
 *              / [ *6( h16 ":" ) h16 ] "::"
 *
 * ls32         = ( h16 ":" h16 ) / IPv4address
 * h16         = 1*4HEXDIG
 */
static inline int parse_ipv6(unsigned char *url, size_t urllen, size_t *cur)
{
    size_t pos  = *cur;
    size_t head = 0;
    int zerogrp = 0;
    int nbit    = 0;

    if (url[pos] == ':') {
        zerogrp = url[pos + 1] == ':';
        // not zero group
        if (!zerogrp) {
            return url[pos + 1];
        }
        pos += 2;
        nbit += 16;
    }

    for (; pos < urllen; pos++) {
        switch (url[pos]) {
        // found finish
        case ']':
            *cur = pos;
            return ']';

        // zero-group
        case ':':
            // illegal byte sequence
            // zero group already defined
            if (zerogrp) {
                *cur = pos;
                return url[pos];
            }
            nbit += 16;
            zerogrp = 1;
            continue;

        // h16
        case '0' ... '9':
        case 'A' ... 'F':
        case 'a' ... 'f':
            if (nbit < 128) {
                nbit += 16;
                head = pos;
                if (HEXDIGIT[url[++pos]] && HEXDIGIT[url[++pos]] &&
                    HEXDIGIT[url[++pos]]) {
                    pos++;
                }
                switch (url[pos]) {
                case ':':
                    if (url[pos + 1] != ']') {
                        continue;
                    }
                    break;

                // found finish
                case ']':
                    if (nbit == 128) {
                        *cur = pos;
                        return ']';
                    }
                    break;

                // embed ipv4
                case '.':
                    // if nbit is less than or equal to 112, can be
                    // embedded ipv4 address (32 bit).
                    //
                    //  max nbit = 128 bit(IPv6) - 32 bit(IPv4)
                    //           = 96
                    if (nbit <= 112) {
                        *cur = head;
                        return parse_ipv4(url, urllen, cur);
                    }
                    break;
                }
            }
        }

        break;
    }

    // illegal byte sequence
    *cur = pos;
    return url[pos];
}

typedef int (*query_parser_t)(lua_State *L, unsigned char *url, size_t urllen,
                              size_t *cur);

static inline int parse_querystring(lua_State *L, unsigned char *url,
                                    size_t urllen, size_t *cur)
{
    size_t pos  = *cur;
    size_t head = pos;

    for (; pos < urllen; pos++) {
        switch (URIC[url[pos]]) {
        // fragment
        case '#':
            // add query field
            if (pos - head - 1) {
                lauxh_pushlstr2tbl(L, "query", (const char *)url + head,
                                   pos - head);
            }
            // paththrough

        // illegal byte sequence
        case 0:
            *cur = pos;
            return url[pos];

        // key-value separator
        case '=':
        case '&':
            break;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + pos + 1)) {
                *cur = pos;
                return '%';
            }
            // skip "%<HEX>"
            pos += 2;
            break;
        }
    }

    // add query field
    if (pos - head - 1) {
        lauxh_pushlstr2tbl(L, "query", (const char *)url + head, pos - head);
    }
    *cur = pos;

    return 0;
}

static inline int parse_queryparams_as_array(lua_State *L, unsigned char *url,
                                             size_t urllen, size_t *cur)
{
    size_t pos   = *cur + 1;
    size_t head  = pos;
    size_t tail  = -1;
    size_t phead = 0;
    size_t len   = 0;
    int nparam   = 0;

    lua_pushstring(L, "queryParams");
    lua_newtable(L);

#define push_v2tbl(L, v, vl)                                                   \
 do {                                                                          \
  lua_pushlstring(L, (v), (vl));                                               \
  lua_rawseti(L, -2, lauxh_rawlen(L, -2) + 1);                                 \
 } while (0)

#define get_tbl(L, k, kl)                                                      \
 do {                                                                          \
  lua_pushlstring(L, (k), (kl));                                               \
  lua_rawget(L, -2);                                                           \
  if (lua_type(L, -1) != LUA_TTABLE) {                                         \
   int ref = LUA_NOREF;                                                        \
   lua_pop(L, 1);                                                              \
   lua_pushlstring(L, (k), (kl));                                              \
   lua_newtable(L);                                                            \
   ref = lauxh_ref(L);                                                         \
   lauxh_pushref(L, ref);                                                      \
   lua_rawset(L, -3);                                                          \
   lauxh_pushref(L, ref);                                                      \
   lauxh_unref(L, ref);                                                        \
  }                                                                            \
 } while (0)

#define push_k2tbl(L, k, kl)                                                   \
 do {                                                                          \
  get_tbl(L, k, kl);                                                           \
  push_v2tbl(L, "", 0);                                                        \
  lua_pop(L, 1);                                                               \
 } while (0)

#define push_kv2tbl(L, k, kl, v, vl)                                           \
 do {                                                                          \
  get_tbl(L, (k), (kl));                                                       \
  push_v2tbl(L, v, vl);                                                        \
  lua_pop(L, 1);                                                               \
 } while (0)

#define push_param()                                                           \
 do {                                                                          \
  if ((len = pos - head)) {                                                    \
   switch (tail) {                                                             \
   case -1:                                                                    \
    push_k2tbl(L, (const char *)url + head, len);                              \
    break;                                                                     \
   case 0:                                                                     \
    push_v2tbl(L, (const char *)url + head, len);                              \
    break;                                                                     \
   default:                                                                    \
    push_kv2tbl(L, (const char *)url + phead, tail, (const char *)url + head,  \
                len);                                                          \
    phead = 0;                                                                 \
   }                                                                           \
   nparam++;                                                                   \
  } else if (phead) {                                                          \
   push_k2tbl(L, (const char *)url + phead, pos - phead - 1);                  \
   nparam++;                                                                   \
  }                                                                            \
 } while (0)

    for (; pos < urllen; pos++) {
        switch (URIC[url[pos]]) {
        // fragment
        case '#':
            // add query field
            if (pos - *cur - 1) {
                lauxh_pushlstr2tblat(L, "query", (const char *)url + *cur,
                                     pos - *cur, 2);
                push_param();
            }
            // paththrough

        // illegal byte sequence
        case 0:
            // add queryParams field
            if (nparam) {
                lua_rawset(L, -3);
            } else {
                lua_pop(L, 2);
            }
            *cur = pos;
            return url[pos];

        // key-value separator
        case '=':
            phead = head;
            tail  = pos - head;
            head  = pos + 1;
            break;

        // next key-value pair
        case '&':
            push_param();
            head = pos + 1;
            tail = -1;
            break;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + pos + 1)) {
                lua_rawset(L, -3);
                *cur = pos;
                return '%';
            }
            // skip "%<HEX>"
            pos += 2;
            break;
        }
    }

    // add queryParams and query field
    if (pos - *cur - 1) {
        push_param();
        lua_rawset(L, -3);
        lauxh_pushlstr2tbl(L, "query", (const char *)url + *cur, pos - *cur);
    } else {
        lua_pop(L, 2);
    }
    *cur = pos;

    return 0;

#undef push_param
}

static inline int parse_queryparams(lua_State *L, unsigned char *url,
                                    size_t urllen, size_t *cur)
{
    size_t pos   = *cur + 1;
    size_t head  = pos;
    size_t tail  = -1;
    size_t phead = 0;
    size_t len   = 0;
    int idx      = 0;
    int nparam   = 0;

    lua_pushstring(L, "queryParams");
    lua_newtable(L);

#define push_param()                                                           \
 do {                                                                          \
  if ((len = pos - head)) {                                                    \
   switch (tail) {                                                             \
   case -1:                                                                    \
    lua_pushlstring(L, (const char *)url + head, len);                         \
    lua_pushliteral(L, "");                                                    \
    break;                                                                     \
   case 0:                                                                     \
    lua_pushinteger(L, ++idx);                                                 \
    lua_pushlstring(L, (const char *)url + head, len);                         \
    break;                                                                     \
   default:                                                                    \
    lua_pushlstring(L, (const char *)url + phead, tail);                       \
    lua_pushlstring(L, (const char *)url + head, len);                         \
    phead = 0;                                                                 \
   }                                                                           \
   lua_rawset(L, -3);                                                          \
   nparam++;                                                                   \
  }                                                                            \
 } while (0)

    for (; pos < urllen; pos++) {
        switch (URIC[url[pos]]) {
        // fragment
        case '#':
            // add query field
            if (pos - *cur - 1) {
                lauxh_pushlstr2tblat(L, "query", (const char *)url + *cur,
                                     pos - *cur, 2);
                push_param();
            }
            // paththrough

        // illegal byte sequence
        case 0:
            // add queryParams field
            if (nparam) {
                lua_rawset(L, -3);
            } else {
                lua_pop(L, 2);
            }
            *cur = pos;
            return url[pos];

        // key-value separator
        case '=':
            phead = head;
            tail  = pos - head;
            head  = pos + 1;
            break;

        // next key-value pair
        case '&':
            push_param();
            head = pos + 1;
            tail = -1;
            break;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + pos + 1)) {
                lua_rawset(L, -3);
                *cur = pos;
                return '%';
            }
            // skip "%<HEX>"
            pos += 2;
            break;
        }
    }

    // add queryParams and query field
    if (pos - *cur - 1) {
        push_param();
        lua_rawset(L, -3);
        lauxh_pushlstr2tbl(L, "query", (const char *)url + *cur, pos - *cur);
    } else {
        lua_pop(L, 2);
    }
    *cur = pos;

    return 0;

#undef push_param
}

static inline int parse_fragment(lua_State *L, unsigned char *url,
                                 size_t urllen, size_t *cur)
{
    size_t pos  = *cur;
    size_t head = pos;

    // parse fragment
    for (; pos < urllen; pos++) {
        switch (URIC[url[pos]]) {
        // illegal byte sequence
        case 0:
        case '#':
            lauxh_pushlstr2tbl(L, "fragment", (const char *)url + head,
                               pos - head);
            *cur = pos;
            return url[pos];

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + pos + 1)) {
                *cur = pos;
                return '%';
            }
            // skip "%<HEX>"
            cur += 2;
        }
    }

    lauxh_pushlstr2tbl(L, "fragment", (const char *)url + head, pos - head);
    *cur = pos;

    return 0;
}

/*
 URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]

 host          = IP-literal / IPv4address / reg-name

 IP-literal    = "[" ( IPv6address / IPvFuture  ) "]"

 IPvFuture     = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

 IPv6address   =                            6( h16 ":" ) ls32
               /                       "::" 5( h16 ":" ) ls32
               / [               h16 ] "::" 4( h16 ":" ) ls32
               / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
               / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
               / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
               / [ *4( h16 ":" ) h16 ] "::"              ls32
               / [ *5( h16 ":" ) h16 ] "::"              h16
               / [ *6( h16 ":" ) h16 ] "::"

 h16           = 1*4HEXDIG
 ls32          = ( h16 ":" h16 ) / IPv4address

 IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
 dec-octet     = DIGIT                 ; 0-9
               / %x31-39 DIGIT         ; 10-99
               / "1" 2DIGIT            ; 100-199
               / "2" %x30-34 DIGIT     ; 200-249
               / "25" %x30-35          ; 250-255

*/
static int parse_lua(lua_State *L)
{
    int argc                = lua_gettop(L);
    size_t urllen           = 0;
    const char *src         = luaL_checklstring(L, 1, &urllen);
    unsigned char *url      = (unsigned char *)src;
    unsigned char c         = 0;
    size_t head             = 0;
    size_t tail             = 0;
    size_t phead            = 0;
    size_t cur              = 0;
    size_t userinfo         = 0;
    size_t portnum          = 0;
    int chk_scheme          = 1;
    int omit_hostname       = 0;
    query_parser_t parseqry = parse_querystring;
    int param_array         = 0;

    // check arguments
    if (argc > 4) {
        lua_settop(L, 4);
    }
    switch (argc) {
    case 4:
        // store the query parameters as an array
        param_array = lauxh_optboolean(L, 4, param_array);
    case 3:
        // initial cursor option
        cur = lauxh_optuint64(L, 3, cur);
    case 2:
        // parse query-params option
        if (lauxh_optboolean(L, 2, 0)) {
            if (param_array) {
                parseqry = parse_queryparams_as_array;
            } else {
                parseqry = parse_queryparams;
            }
        }
    }

    lua_settop(L, 1);
    lua_newtable(L);
    if (!urllen) {
        lua_pushinteger(L, 0);
        return 2;
    }

    // check first byte
    switch (url[cur]) {
    // illegal byte sequence
    case 0:
        lua_pushinteger(L, cur);
        return 2;

    // query-string
    case '?':
        goto PARSE_QUERY;

    // fragment
    case '#':
        cur++;
        goto PARSE_FRAGMENT;
    }

PARSE_PATHNAME:
    // pathname
    head = cur;
    for (; cur < urllen; cur++) {
        switch (URIC[url[cur]]) {
        // illegal byte sequence
        case 0:
            lauxh_pushlstr2tbl(L, "path", src + head, cur - head);
            lua_pushinteger(L, cur);
            lua_pushlstring(L, src + cur, 1);
            return 3;

        // query-string
        case '?':
            lauxh_pushlstr2tbl(L, "path", src + head, cur - head);
            goto PARSE_QUERY;

        // fragment
        case '#':
            lauxh_pushlstr2tbl(L, "path", src + head, cur - head);
            cur++;
            goto PARSE_FRAGMENT;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + cur + 1)) {
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            // skip "%<HEX>"
            cur += 2;
            // paththrough

        // set chk_scheme to 0 if not scheme characters
        case '!':
        case '$':
        // 0x24-2A = "&" "'" "(" ")" "*"
        case '&' ... '*':
        case ',':
        case '/':
        case ';':
        case '=':
        case '@':
        case '_':
        case '~':
            chk_scheme = 0;
            break;

        case ':':
            // use as scheme separator
            if (chk_scheme) {
                chk_scheme = 0;
                goto PARSE_SCHEME;
            }
            break;
        }
    }

    // set path
    lauxh_pushlstr2tbl(L, "path", src + head, cur - head);
    lua_pushinteger(L, cur);
    return 2;

PARSE_QUERY:
    switch (parseqry(L, url, urllen, &cur)) {
    // done
    case 0:
        lua_pushinteger(L, cur);
        return 2;

    // fragment
    case '#':
        cur++;
        goto PARSE_FRAGMENT;

    // illegal byte sequence
    default:
        lua_pushinteger(L, cur);
        lua_pushlstring(L, src + cur, 1);
        return 3;
    }

PARSE_FRAGMENT:
    // parse fragment
    head = cur;
    switch (parse_fragment(L, url, urllen, &cur)) {
    // done
    case 0:
        lua_pushinteger(L, cur);
        return 2;

    // illegal byte sequence
    default:
        lua_pushinteger(L, cur);
        lua_pushlstring(L, src + cur, 1);
        return 3;
    }

PARSE_SCHEME:
    // set "scheme" to scheme field
    lauxh_pushlstr2tbl(L, "scheme", src + head, cur - head);
    // skip ":"
    cur++;
    // must be double-slash
    // skip "//"
    if ((cur + 1) >= urllen || url[cur++] != '/' || url[cur++] != '/') {
        lua_pushinteger(L, cur - 1);
        lua_pushlstring(L, src + cur - 1, 1);
        return 3;
    }

PARSE_HOST:
    // parse host
    head = cur;
    // check first byte
    switch (url[cur]) {
    // parse ipv6
    case '[':
        goto PARSE_IPV6;

    case '/':
    case '.':
        // host required if userinfo is defined
        if (!userinfo) {
            // some scheme (e.g. file) can be omit parsing the authority
            goto PARSE_PATHNAME;
        }
        // illegal byte sequence
        lua_pushinteger(L, cur - 1);
        lua_pushlstring(L, src + cur - 1, 1);
        return 3;

    case ':':
        omit_hostname = 1;
        tail          = cur;
        cur++;
        goto PARSE_PORT;

    default:
        // illegal byte sequence
        if (url[cur] != '%' && !isalnum(url[cur])) {
            lua_pushinteger(L, cur - 1);
            lua_pushlstring(L, src + cur - 1, 1);
            return 3;
        }
    }

#define push_host()                                                            \
 do {                                                                          \
  lauxh_pushlstr2tbl(L, "host", src + head, cur - head);                       \
  lauxh_pushlstr2tbl(L, "hostname", src + head, cur - head);                   \
 } while (0)

    for (; cur < urllen; cur++) {
        switch (URIC[url[cur]]) {
        // illegal byte sequence
        case 0:
            push_host();
            lua_pushinteger(L, cur);
            lua_pushlstring(L, src + cur, 1);
            return 3;

        case '.':
            continue;

        case '@':
            // illegal byte sequence
            // userinfo already parsed
            if (userinfo) {
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            lauxh_pushlstr2tbl(L, "userinfo", src + head, cur - head);
            lauxh_pushlstr2tbl(L, "user", src + head, cur - head);
            userinfo = cur;
            cur++;
            goto PARSE_HOST;

        case ':':
            tail = cur;
            cur++;
            goto PARSE_PORT;

        case '/':
            push_host();
            goto PARSE_PATHNAME;

        case '?':
            push_host();
            goto PARSE_QUERY;

        case '#':
            push_host();
            cur++;
            goto PARSE_FRAGMENT;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + cur + 1)) {
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            // skip "%<HEX>"
            cur += 2;
        }
    }

    push_host();
    lua_pushinteger(L, cur);
    return 2;

PARSE_IPV6:
    // parse ipv6
    head = cur;
    cur++;
    switch (parse_ipv6(url, urllen, &cur)) {
    // found delemiter
    case ']':
        cur++;
        switch (url[cur]) {
        case ':':
            tail = cur;
            cur++;
            goto PARSE_PORT;

        case '/':
            push_host();
            goto PARSE_PATHNAME;

        case '?':
            push_host();
            goto PARSE_QUERY;

        // illegal byte sequence
        default:
            push_host();
            lua_pushinteger(L, cur);
            lua_pushlstring(L, src + cur, 1);
            return 3;
        }
        break;

    // illegal byte sequence
    default:
        lua_pushinteger(L, cur);
        lua_pushlstring(L, src + cur, 1);
        return 3;
    }

#undef push_host

PARSE_PORT:
    // parse port
    phead   = cur;
    portnum = 0;

#define push_hostport()                                                        \
 do {                                                                          \
  lauxh_pushlstr2tbl(L, "host", src + head, cur - head);                       \
  lauxh_pushlstr2tbl(L, "hostname", src + head, tail - head);                  \
  lauxh_pushlstr2tbl(L, "port", src + phead, cur - phead);                     \
 } while (0)

    for (; cur < urllen; cur++) {
        c = url[cur];
        switch (c) {
        // convert to integer
        case '0' ... '9':
            portnum = (portnum << 3) + (portnum << 1) + (c - '0');
            // illegal byte sequence
            // invalid port range
            if (portnum > 0xFFFF) {
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            continue;

        case '/':
            // set "hostname", "host" and "port" fields
            push_hostport();
            goto PARSE_PATHNAME;

        case '?':
            push_hostport();
            goto PARSE_QUERY;

        case '#':
            push_hostport();
            cur++;
            goto PARSE_FRAGMENT;

        default:
            // illegal byte sequence
            // userinfo already parsed or hostname ommited
            if (userinfo || omit_hostname) {
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            // this byte sequences to use as username
            goto PARSE_PASSWORD;
        }
    }

    push_hostport();
    lua_pushinteger(L, cur);
    return 2;

#undef push_hostport

PARSE_PASSWORD:
    for (; cur < urllen; cur++) {
        switch (URIC[url[cur]]) {
        // illegal byte sequence
        case 0:
        case ':':
        case '/':
        case '?':
        case '#':
            lua_pushinteger(L, cur);
            lua_pushlstring(L, src + cur, 1);
            return 3;

        case '@':
            lauxh_pushlstr2tbl(L, "userinfo", src + head, cur - head);
            lauxh_pushlstr2tbl(L, "user", src + head, tail - head);
            lauxh_pushlstr2tbl(L, "password", src + phead, cur - phead);
            userinfo = c;
            cur++;
            goto PARSE_HOST;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + cur + 1)) {
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            // skip "%<HEX>"
            cur += 2;
            // paththrough
        }
    }

    // invalid userinfo format
    lua_pushinteger(L, cur);
    lua_pushlstring(L, src + cur, 1);
    return 3;
}

LUALIB_API int luaopen_url_parse(lua_State *L)
{
    lua_pushcfunction(L, parse_lua);
    return 1;
}
