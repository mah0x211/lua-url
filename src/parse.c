/*
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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
// lualib
#include "../deps/lauxhlib/lauxhlib.h"


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
    '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
//  use hostname
    ':',
//       <       >
    ';', 0, '=', 0,
//  use query and fragment
    '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
//                                                              [  \  ]  ^
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0, 0, 0, 0, '_',
//  `
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
//                                                              {  |  }
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0, 0, 0, '~'
};


static const unsigned char HEXDIGIT[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    0, 0, 0, 0, 0, 0, 0,
    'A', 'B', 'C', 'D', 'E', 'F',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    'a', 'b', 'c', 'd', 'e', 'f'
};


/**
 *  pct-encoded     = "%" HEXDIG HEXDIG
 *  HEXDIG          = "A" / "B" / "C" / "D" / "E" / "F"
 *                  / "a" / "b" / "c" / "d" / "e" / "f"
 *                  / DIGIT
 */
static inline int is_percentencoded( const unsigned char *str )
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
static inline int parse_ipv4( unsigned char *url, size_t urllen, size_t *cur )
{
    size_t pos = *cur;
    size_t head = pos;
    int nseg = 0;
    int dec = -1;

    for(; pos < urllen; pos++ )
    {
        switch( url[pos] )
        {
            case '0' ... '9':
                if( pos - head < 4 )
                {
                    // convert to integer
                    if( dec == -1 ){
                        dec = url[pos] - '0';
                    }
                    else {
                        dec = ( dec << 3 ) + ( dec << 1 ) + ( url[pos] - '0' );
                    }

                    if( dec <= 0xFF ){
                        continue;
                    }
                }
                break;

            case '.':
                if( pos - head && nseg < 3 ){
                    dec = -1;
                    head = pos + 1;
                    nseg++;
                    continue;
                }
                break;


            default:
                if( nseg == 3 && dec != -1 ){
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
 *              ; アドレスの下位 32 ビット
 *
 * h16         = 1*4HEXDIG
 * ; 16 進数字で表現される 16 ビットのアドレス
 */
static inline int parse_ipv6( unsigned char *url, size_t urllen, size_t *cur )
{
    size_t pos = *cur;
    size_t head = 0;
    int zerogrp = 0;
    int ngrp = 0;

    if( url[pos] == ':' )
    {
        zerogrp = url[pos + 1] == ':';
        // not zero group
        if( !zerogrp ){
            return url[pos + 1];
        }
        pos += 2;
        ngrp++;
    }

    for(; pos < urllen; pos++ )
    {
        switch( url[pos] )
        {
            // found finish
            case ']':
                *cur = pos;
                return ']';

            // zero-group
            case ':':
                // illegal byte sequence
                // zero group already defined
                if( zerogrp ){
                    *cur = pos;
                    return url[pos];
                }
                ngrp++;
                zerogrp = 1;
                continue;

            // h16
            case '0' ... '9':
            case 'A' ... 'F':
            case 'a' ... 'f':
                if( ngrp < 8 )
                {
                    ngrp++;
                    head = pos;
                    if( HEXDIGIT[url[++pos]] && HEXDIGIT[url[++pos]] &&
                        HEXDIGIT[url[++pos]] ){
                        pos++;
                    }
                    switch( url[pos] )
                    {
                        case ':':
                            if( url[pos + 1] != ']' ){
                                continue;
                            }
                            break;

                        // found finish
                        case ']':
                            if( ngrp == 8 ){
                                *cur = pos;
                                return ']';
                            }
                            break;

                        // embed ipv4
                        case '.':
                            if( ngrp < 7 ){
                                *cur = head;
                                return parse_ipv4( url, urllen, cur );
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


typedef int (*query_parser_t)( lua_State *L, unsigned char *url,
                               size_t urllen, size_t *cur );

static inline int parse_querystring( lua_State *L, unsigned char *url,
                                     size_t urllen, size_t *cur )
{
    size_t pos = *cur;
    size_t head = pos;

    for(; pos < urllen; pos++ )
    {
        switch( URIC[url[pos]] )
        {
            // fragment
            case '#':
                lauxh_pushlstr2tblat( L, "query", (const char*)url + head,
                                      pos - head, 1 );
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
                if( !is_percentencoded( url + pos + 1 ) ){
                    *cur = pos;
                    return '%';
                }
                // skip "%<HEX>"
                pos += 2;
                break;
        }
    }

    *cur = pos;

    return 0;
}


static inline int parse_queryparams( lua_State *L, unsigned char *url,
                                     size_t urllen, size_t *cur )
{
    size_t pos = *cur + 1;
    size_t head = pos;
    size_t tail = -1;
    size_t phead = 0;
    size_t len = 0;
    int idx = 0;

    lua_pushstring( L, "queryParams" );
    lua_newtable( L );

#define push_param() do{                                                \
    if( ( len = pos - head ) ){                                         \
        switch( tail ){                                                 \
            case -1:                                                    \
                lua_pushlstring( L, (const char*)url + head, len );     \
                lua_pushboolean( L, 1 );                                \
                break;                                                  \
            case 0:                                                     \
                lua_pushinteger( L, ++idx );                            \
                lua_pushlstring( L, (const char*)url + head, len );     \
                break;                                                  \
            default:                                                    \
                lua_pushlstring( L, (const char*)url + phead, tail );   \
                lua_pushlstring( L, (const char*)url + head, len );     \
        }                                                               \
        lua_rawset( L, -3 );                                            \
    }                                                                   \
}while(0)


    for(; pos < urllen; pos++ )
    {
        switch( URIC[url[pos]] )
        {
            // fragment
            case '#':
                lauxh_pushlstr2tblat( L, "query", (const char*)url + *cur,
                                      pos - *cur, 1 );
                push_param();
                // paththrough

            // illegal byte sequence
            case 0:
                lua_rawset( L, 1 );
                *cur = pos;
                return url[pos];

            // key-value separator
            case '=':
                phead = head;
                tail = pos - head;
                head = pos + 1;
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
                if( !is_percentencoded( url + pos + 1 ) ){
                    lua_rawset( L, 1 );
                    *cur = pos;
                    return '%';
                }
                // skip "%<HEX>"
                pos += 2;
                break;
        }
    }

    lauxh_pushlstr2tblat( L, "query", (const char*)url + *cur,
                          pos - *cur, 1 );
    push_param();
    lua_rawset( L, 1 );
    *cur = pos;

    return 0;

#undef push_param

}




static inline int parse_fragment( lua_State *L, unsigned char *url, size_t urllen,
                                  size_t *cur )
{
    size_t pos = *cur;
    size_t head = pos;

    // parse fragment
    for(; pos < urllen ; pos++ )
    {
        switch( URIC[url[pos]] )
        {
            // illegal byte sequence
            case 0:
            case '#':
                lauxh_pushlstr2tblat( L, "fragment", (const char*)url + head,
                                      pos - head, 1 );
                *cur = pos;
                return url[pos];

            // percent-encoded
            case '%':
                // invalid percent-encoded format
                if( !is_percentencoded( url + pos + 1 ) ){
                    *cur = pos;
                    return '%';
                }
                // skip "%<HEX>"
                cur += 2;
        }
    }

    lauxh_pushlstr2tblat( L, "fragment", (const char*)url + head, pos - head, 1);
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
static int parse_lua( lua_State *L )
{
    size_t urllen = 0;
    const char *src = luaL_checklstring( L, 2, &urllen );
    unsigned char *url = (unsigned char*)src;
    unsigned char c = 0;
    size_t head = 0;
    size_t tail = 0;
    size_t phead = 0;
    size_t cur = 0;
    size_t userinfo = 0;
    size_t portnum = 0;
    int chk_scheme = 1;
    query_parser_t parseqry = parse_querystring;

    // check arguments
    luaL_checktype( L, 1, LUA_TTABLE );
    if( lua_gettop( L ) > 2 )
    {
        luaL_checktype( L, 3, LUA_TBOOLEAN );
        if( lua_toboolean( L, 3 ) ){
            parseqry = parse_queryparams;
        }
    }

    if( !urllen ){
        lua_pushinteger( L, 0 );
        return 1;
    }

    // check first byte
    switch( *url ){
        // illegal byte sequence
        case 0:
            lua_pushinteger( L, cur );
            return 1;

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
    for(; cur < urllen; cur++ )
    {
        switch( URIC[url[cur]] )
        {
            // illegal byte sequence
            case 0:
                lauxh_pushlstr2tblat( L, "path", src + head, cur - head, 1 );
                lua_pushinteger( L, cur );
                lua_pushlstring( L, src + cur, 1 );
                return 2;

            // query-string
            case '?':
                lauxh_pushlstr2tblat( L, "path", src + head, cur - head, 1 );
                goto PARSE_QUERY;

            // fragment
            case '#':
                lauxh_pushlstr2tblat( L, "path", src + head, cur - head, 1 );
                cur++;
                goto PARSE_FRAGMENT;

            // percent-encoded
            case '%':
                // invalid percent-encoded format
                if( !is_percentencoded( url + cur + 1 ) ){
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
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
                if( chk_scheme ){
                    chk_scheme = 0;
                    goto PARSE_SCHEME;
                }
                break;
        }
    }

    // set path
    lauxh_pushlstr2tblat( L, "path", src + head, cur - head, 1 );
    lua_pushinteger( L, cur );
    return 1;


PARSE_QUERY:
    switch( parseqry( L, url, urllen, &cur ) ){
        // done
        case 0:
            lua_pushinteger( L, cur );
            return 1;

        // fragment
        case '#':
            cur++;
            goto PARSE_FRAGMENT;

        // illegal byte sequence
        default:
            lua_pushinteger( L, cur );
            lua_pushlstring( L, src + cur, 1 );
            return 2;
    }



PARSE_FRAGMENT:
    // parse fragment
    head = cur;
    switch( parse_fragment( L, url, urllen, &cur ) ){
        // done
        case 0:
            lua_pushinteger( L, cur );
            return 1;

        // illegal byte sequence
        default:
            lua_pushinteger( L, cur );
            lua_pushlstring( L, src + cur, 1 );
            return 2;
    }


PARSE_SCHEME:
    // set "scheme" to scheme field
    lauxh_pushlstr2tblat( L, "scheme", src + head, cur - head, 1 );
    // skip ":"
    cur++;
    // must be double-slash
    // skip "//"
    if( ( cur + 1 ) >= urllen || url[cur++] != '/' || url[cur++] != '/' ){
        lua_pushinteger( L, cur - 1 );
        lua_pushlstring( L, src + cur - 1, 1 );
        return 2;
    }


PARSE_HOST:
    // parse host
    head = cur;
    // check first byte
    switch( url[cur] )
    {
        // parse ipv6
        case '[':
            goto PARSE_IPV6;

        // illegal byte sequence
        case '.':
            lua_pushinteger( L, cur - 1 );
            lua_pushlstring( L, src + cur - 1, 1 );
            return 2;

        default:
            // illegal byte sequence
            if( url[cur] != '%' && !isalnum( url[cur] ) ){
                lua_pushinteger( L, cur - 1 );
                lua_pushlstring( L, src + cur - 1, 1 );
                return 2;
            }
    }


#define push_host() do {                                                \
    lauxh_pushlstr2tblat( L, "host", src + head, cur - head, 1 );       \
    lauxh_pushlstr2tblat( L, "hostname", src + head, cur - head, 1 );   \
}while(0)


    for(; cur < urllen; cur++ )
    {
        switch( URIC[url[cur]] )
        {
            // illegal byte sequence
            case 0:
                push_host();
                lua_pushinteger( L, cur );
                lua_pushlstring( L, src + cur, 1 );
                return 2;

            case '.':
                continue;

            case '@':
                // illegal byte sequence
                // userinfo already parsed
                if( userinfo ){
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
                }
                lauxh_pushlstr2tbl( L, "userinfo", src + head, cur - head );
                lauxh_pushlstr2tbl( L, "user", src + head, cur - head );
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
                if( !is_percentencoded( url + cur + 1 ) ){
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
                }
                // skip "%<HEX>"
                cur += 2;
        }
    }

    push_host();
    lua_pushinteger( L, cur );
    return 1;



PARSE_IPV6:
    // parse ipv6
    head = cur;
    cur++;
    switch( parse_ipv6( url, urllen, &cur ) )
    {
        // found delemiter
        case ']':
            cur++;
            switch( url[cur] )
            {
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
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
            }
            break;

        // illegal byte sequence
        default:
            lua_pushinteger( L, cur );
            lua_pushlstring( L, src + cur, 1 );
            return 2;
    }

#undef push_host



PARSE_PORT:
    // parse port
    phead = cur;
    portnum = 0;

#define push_hostport() do {                                            \
    lauxh_pushlstr2tblat( L, "host", src + head, cur - head, 1 );       \
    lauxh_pushlstr2tblat( L, "hostname", src + head, tail - head, 1 );  \
    lauxh_pushlstr2tblat( L, "port", src + phead, cur - phead, 1 );     \
}while(0)


    for(; cur < urllen; cur++ )
    {
        c = url[cur];
        switch( c )
        {
            // convert to integer
            case '0' ... '9':
                portnum = ( portnum << 3 ) + ( portnum << 1 ) + ( c - '0' );
                // illegal byte sequence
                // invalid port range
                if( portnum > 0xFFFF ){
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
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
                // userinfo already parsed
                if( userinfo ){
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
                }
                // this byte sequences to use as username
                goto PARSE_PASSWORD;
        }
    }

    push_hostport();
    lua_pushinteger( L, cur );
    return 1;

#undef push_hostport



PARSE_PASSWORD:
    for(; cur < urllen; cur++ )
    {
        switch( URIC[url[cur]] )
        {
            // illegal byte sequence
            case 0:
            case ':':
            case '/':
            case '?':
            case '#':
                lua_pushinteger( L, cur );
                lua_pushlstring( L, src + cur, 1 );
                return 2;

            case '@':
                lauxh_pushlstr2tblat( L, "userinfo", src + head, cur - head, 1 );
                lauxh_pushlstr2tblat( L, "user", src + head, tail - head, 1 );
                lauxh_pushlstr2tblat( L, "password", src + phead, cur - phead, 1 );
                userinfo = c;
                cur++;
                goto PARSE_HOST;

            // percent-encoded
            case '%':
                // invalid percent-encoded format
                if( !is_percentencoded( url + cur + 1 ) ){
                    lua_pushinteger( L, cur );
                    lua_pushlstring( L, src + cur, 1 );
                    return 2;
                }
                // skip "%<HEX>"
                cur += 2;
                // paththrough
        }
    }

    // invalid userinfo format
    lua_pushinteger( L, cur );
    lua_pushlstring( L, src + cur, 1 );
    return 2;
}


LUALIB_API int luaopen_url_parse( lua_State *L )
{
    lua_pushcfunction( L, parse_lua );
    return 1;
}


