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
#include <lauxhlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    0, 0, 0, 0, 0, 0,
    // SP      "
    0, 0, '!', 0, '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.',
    //  use query and fragment
    '/',
    //  DIGIT
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    //  use hostname
    ':',
    //   <       >
    ';', 0, '=', 0,
    //  use query and fragment
    '?', '@',
    // ALPHA
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
    //   [  \  ]  ^       `
    'Z', 0, 0, 0, 0, '_', 0,
    // ALPHA
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    //   {  |  }
    'z', 0, 0, 0, '~'};

/**
 *  pct-encoded     = "%" HEXDIG HEXDIG
 *  HEXDIG          = "A" / "B" / "C" / "D" / "E" / "F"
 *                  / "a" / "b" / "c" / "d" / "e" / "f"
 *                  / DIGIT
 */
static inline int is_percentencoded(const unsigned char *str)
{
    return isxdigit(str[0]) && isxdigit(str[1]);
}

static inline int unhex(unsigned char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0;
}

static inline void unescape(lua_State *L, const char *str, size_t len)
{
    luaL_Buffer b = {0};

    luaL_buffinit(L, &b);
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
        case '+':
            luaL_addchar(&b, ' ');
            continue;

        case '%':
            luaL_addchar(&b, (unhex(str[i + 1]) << 4) | unhex(str[i + 2]));
            i += 2;
            continue;

        default:
            luaL_addchar(&b, str[i]);
        }
    }
    luaL_pushresult(&b);
}

/**
 *  IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
 *  dec-octet   = DIGIT                 ; 0-9
 *              / %x31-39 DIGIT         ; 10-99
 *              / "1" 2DIGIT            ; 100-199
 *              / "2" %x30-34 DIGIT     ; 200-249
 *              / "25" %x30-35          ; 250-255
 */
static int parse_ipv4(unsigned char *url, size_t urllen, size_t *cur)
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
static int parse_ipv6(unsigned char *url, size_t urllen, size_t *cur)
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
                if (isxdigit(url[++pos]) && isxdigit(url[++pos]) &&
                    isxdigit(url[++pos])) {
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

typedef struct {
    lua_State *L;
    const char *s;
    size_t head;
    const char *key;
    size_t klen;
    int is_key_encoded;
} query_param_t;

static inline void query_param_init(query_param_t *p, lua_State *L,
                                    const char *s, size_t head)
{
    *p = (query_param_t){L, s, head, NULL, 0, 0};
}

static inline void query_param_set_keytail(query_param_t *p, size_t pos,
                                           int is_encoded)
{
    p->key            = p->s + p->head;
    p->klen           = pos - p->head;
    p->head           = pos + 1;
    p->is_key_encoded = is_encoded;
}

static int query_param_set_tail(query_param_t *p, size_t pos, int is_encoded)
{
    lua_State *L       = p->L;
    const char *key    = p->key;
    size_t klen        = p->klen;
    const char *val    = "";
    size_t vlen        = 0;
    size_t len         = pos - p->head;
    int is_key_encoded = p->is_key_encoded;
    int is_val_encoded = 0;

    if (key) {
        if (len) {
            // key=val
            val            = p->s + p->head;
            vlen           = len;
            is_val_encoded = is_encoded;
        }
    } else if (len) {
        // key=""
        key            = p->s + p->head;
        klen           = len;
        is_key_encoded = is_encoded;
    } else {
        return 0;
    }

    // get value table
    if (is_key_encoded) {
        unescape(L, key, klen);
    } else {
        lua_pushlstring(L, key, klen);
    }
    lua_pushvalue(L, -1);
    lua_rawget(L, -3);
    if (lua_istable(L, -1)) {
        // value table exists
        lua_replace(L, -2);
    } else {
        // create value table
        int ref = LUA_NOREF;
        lua_pop(L, 1);
        lua_createtable(L, 1, 0);
        ref = lauxh_ref(L);
        lauxh_pushref(L, ref);
        lua_rawset(L, -3);
        lauxh_pushref(L, ref);
        lauxh_unref(L, ref);
    }

    // push value to value table
    if (is_val_encoded) {
        unescape(L, val, vlen);
    } else {
        lua_pushlstring(L, val, vlen);
    }
    lua_rawseti(L, -2, lauxh_rawlen(L, -2) + 1);
    lua_pop(L, 1);

    p->head           = pos + 1;
    p->key            = NULL;
    p->klen           = 0;
    p->is_key_encoded = 0;

    return 1;
}

static inline int parse_query(lua_State *L, unsigned char *url, size_t urllen,
                              size_t *cur, int parse_params)
{
    size_t head     = *cur;
    size_t pos      = 0;
    query_param_t p = {0};
    int nparam      = 0;
    int is_encoded  = 0;

    // skip query delimiter
    if (url[head] == '?') {
        head++;
    }
    // skip param separator
    while (url[head] == '&') {
        head++;
    }
    pos = head;

    if (parse_params) {
        query_param_init(&p, L, (const char *)url, pos);
        lua_pushstring(L, "queryParams");
        lua_newtable(L);
    }

    for (; pos < urllen; pos++) {
        switch (URIC[url[pos]]) {
        // illegal byte sequence
        case 0:
            // fallthrough

        // fragment
        case '#':
            goto PARSE_DONE;

        // percent-encoded
        case '%':
            // invalid percent-encoded format
            if (!is_percentencoded(url + pos + 1)) {
                goto PARSE_DONE;
            }
            // skip "%<HEX>"
            pos += 2;
            // fallthrough

        case '+':
            is_encoded = 1;
            break;

        // key-value separator
        case '=':
            if (parse_params) {
                query_param_set_keytail(&p, pos, is_encoded);
                is_encoded = 0;
            }
            break;

        // next key-value pair
        case '&':
            if (parse_params) {
                nparam += query_param_set_tail(&p, pos, is_encoded);
                // skip param separator
                while (url[pos + 1] == '&') {
                    pos++;
                }
                if (url[pos] == '&') {
                    p.head = pos + 1;
                }
            }

            break;
        }
    }

PARSE_DONE:
    // add query_params and query field
    if (parse_params) {
        nparam += query_param_set_tail(&p, pos, is_encoded);
        if (nparam) {
            lua_rawset(L, -3);
        } else {
            lua_pop(L, 2);
        }
    }
    if (pos > head) {
        lauxh_pushlstr2tbl(L, "query", (const char *)url + *cur, pos - *cur);
    }
    *cur = pos;
    return url[pos];
}

/**
 * fragment      = *( pchar / "/" / "?" )
 * pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
 * unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
 * pct-encoded   = "%" HEXDIG HEXDIG
 * sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
 *               / "*" / "+" / "," / ";" / "="
 */
static int parse_fragment(lua_State *L, unsigned char *url, size_t urllen,
                          size_t *cur)
{
    size_t pos  = *cur;
    size_t head = pos;

    // parse fragment
    for (; pos < urllen; pos++) {
        switch (URIC[url[pos]]) {
        // illegal byte sequence
        case 0:
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
            pos += 2;
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
    int argc           = lua_gettop(L);
    size_t urllen      = 0;
    const char *src    = luaL_checklstring(L, 1, &urllen);
    unsigned char *url = (unsigned char *)src;
    unsigned char c    = 0;
    size_t head        = 0;
    size_t tail        = 0;
    size_t phead       = 0;
    size_t cur         = 0;
    size_t userinfo    = 0;
    size_t portnum     = 0;
    int chk_scheme     = 1;
    int omit_hostname  = 0;
    int parse_params   = 0;
    int is_querystring = 0;

    // check arguments
    if (argc > 4) {
        lua_settop(L, 4);
    }
    switch (argc) {
    case 4:
        // url is query-string
        is_querystring = lauxh_optboolean(L, 4, 0);
    case 3:
        // initial cursor option
        cur = lauxh_optuint64(L, 3, cur);
    case 2:
        // parse query-params option
        parse_params = lauxh_optboolean(L, 2, 0);
    }

    lua_settop(L, 1);
    lua_newtable(L);
    if (!urllen) {
        lua_pushinteger(L, 0);
        return 2;
    } else if (is_querystring) {
        goto PARSE_QUERY;
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
            // fallthrough to disable chk_scheme

        // set chk_scheme to 0 if not scheme characters
        case '!':
        case '$':
        // 0x24-2A = & ' ( ) *
        case '&' ... '*':
        case ',':
        case '.':
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
    switch (parse_query(L, url, urllen, &cur, parse_params)) {
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
    // must be double-slash
    if ((cur + 2) >= urllen || url[cur + 1] != '/' || url[cur + 2] != '/') {
        lua_pushinteger(L, cur);
        lua_pushlstring(L, src + cur, 1);
        return 3;
    }
    // set "scheme" to scheme field
    lauxh_pushlstr2tbl(L, "scheme", src + head, cur - head);
    // skip "://"
    cur += 3;

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
        lua_pushinteger(L, cur);
        lua_pushlstring(L, src + cur, 1);
        return 3;

    case ':':
        omit_hostname = 1;
        tail          = cur;
        cur++;
        goto PARSE_PORT;

    case '@':
CHECK_USERINFO:
        // userinfo already parsed
        if (userinfo) {
            // illegal byte sequence
            lua_pushinteger(L, cur);
            lua_pushlstring(L, src + cur, 1);
            return 3;
        }
        // previous string is treated as userinfo
        if (cur - head) {
            lauxh_pushlstr2tbl(L, "userinfo", src + head, cur - head);
            lauxh_pushlstr2tbl(L, "user", src + head, cur - head);
        }
        userinfo = cur;
        cur++;
        goto PARSE_HOST;

    default:
        // host must be started with ALPHA / DIGIT / '%' (percent-encoded)
        if (url[cur] != '%' && !isalnum(url[cur])) {
            // illegal byte sequence
            lua_pushinteger(L, cur);
            lua_pushlstring(L, src + cur, 1);
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
            goto CHECK_USERINFO;

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
  if (cur - head > 2) {                                                        \
   lauxh_pushlstr2tbl(L, "hostname", src + head, tail - head);                 \
   if (cur - phead) {                                                          \
    lauxh_pushlstr2tbl(L, "host", src + head, cur - head);                     \
    lauxh_pushlstr2tbl(L, "port", src + phead, cur - phead);                   \
   } else {                                                                    \
    lauxh_pushlstr2tbl(L, "host", src + head, tail - head);                    \
   }                                                                           \
  }                                                                            \
 } while (0)

    for (; cur < urllen; cur++) {
        c = url[cur];
        switch (c) {
        // convert to integer
        case '0' ... '9':
            portnum = (portnum << 3) + (portnum << 1) + (c - '0');
            // invalid port range
            if (portnum > 0xFFFF) {
                // illegal byte sequence
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
            // userinfo already parsed or hostname ommited
            if (userinfo || omit_hostname) {
                // illegal byte sequence
                lua_pushinteger(L, cur);
                lua_pushlstring(L, src + cur, 1);
                return 3;
            }
            // previsous string is treated as username
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
            // fallthrough
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
