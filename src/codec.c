/*
 *  Copyright (C) 2014 Masatoshi Teruya
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
 *
 *  src/codec.c
 *
 *  Created by Masatoshi Teruya on 14/04/11.
 *
 */

#include <errno.h>
#include <lauxhlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
    encodeURI   : 0-9 a-zA-Z !#$&'()*+,-./:;=?@_~

    uric        = reserved | unreserved | escaped
    reserved    = ";" | "," | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$"
    unreserved  = alphanum | mark
    mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"

    escaped     = "%" hex hex

    hex         = digit | "A" | "B" | "C" | "D" | "E" | "F" |
                            "a" | "b" | "c" | "d" | "e" | "f"

    digit       = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"

    alphanum    = alpha | digit
    alpha       = lowalpha | upalpha

    lowalpha    = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
                  "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
                  "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
    upalpha     = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
                  "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
                  "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
*/
static const unsigned char UNRESERVED_URI[256] = {
    //  ctrl-code: 0-32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    //  SP      "            %
    0, '!', 0, '#', '$', 0, '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    //                                                              <       >
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', 0, '=', 0, '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    //                                                              [  \  ]  ^
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0, 0, 0, 0, '_',
    //  `
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    //                                                              {  |  }
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0, 0, 0, '~'};

/*
    RFC 2396    : 0-9 a-zA-Z !'()*-._~

    unreserved  = alphanum | mark

    mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"

    alphanum    = alpha | digit

    alpha       = lowalpha | upalpha

    lowalpha    = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
                  "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
                  "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"

    upalpha     = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
                  "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
                  "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"

    digit       = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
*/
static const unsigned char UNRESERVED_2396[256] = {
    //  ctrl-code: 0-32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,

    //  SP      "  #  $  %  &                       +  ,            /
    0, '!', 0, 0, 0, 0, 0, '\'', '(', ')', '*', 0, 0, '-', '.', 0,

    //  digit
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

    //  :  ;  <  =  >  ?  @
    0, 0, 0, 0, 0, 0, 0,

    //  alpha-upper
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

    //  [  \  ]  ^       `
    0, 0, 0, 0, '_', 0,

    //  alpha-lower
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

    //  {  |  }
    0, 0, 0, '~'};

/*
    RFC 3986    : 0-9 a-zA-Z -._~

    unreserved  = alpha | digit | "-" | "." | "_" | "~"

    alpha       = lowalpha | upalpha

    lowalpha    = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
                  "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
                  "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"

    upalpha     = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
                  "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
                  "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"

    digit       = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
*/
static const unsigned char UNRESERVED_3986[256] = {
    //  ctrl-code: 0-32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,

    //  SP !  "  #  $  %  &  '  (  )  *  +  ,            /
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', '.', 0,

    //  digit
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

    //  :  ;  <  =  >  ?  @
    0, 0, 0, 0, 0, 0, 0,

    //  alpha-upper
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

    //  [  \  ]  ^       `
    0, 0, 0, 0, '_', 0,

    //  alpha-lower
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

    //  {  |  }
    0, 0, 0, '~'};

/*
    hex = 0-16
    '0' = 48
    '7' = 55
    'A' = 65
    'W' = 87
    'a' = 97
    uppercase:
        '7' = 'A' - 10
    lowercase:
        'W' = 'a' - 10
*/
static const unsigned char DEC2HEX[16] = "0123456789ABCDEF";

static const char HEX2DEC[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    //  0  1  2  3  4  5  6  7  8  9
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1,
    //  A   B   C   D   E   F
    10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    //  a   b   c   d   e   f
    10, 11, 12, 13, 14, 15};

// %[hex][hex]*4
#define CODEC_UTF8ENC_LEN 12
#define CODEC_UTF8DEC_LEN 4

static unsigned char *encode(char *str, size_t slen, const unsigned char *tbl,
                             size_t *len)
{
    unsigned char *dest = malloc(3 * slen + 1);

    if (dest) {
        unsigned char *src = (unsigned char *)str;
        ssize_t bytes      = 0;
        size_t i           = 0;

        for (; i < slen; i++) {
            if (tbl[*src]) {
                dest[bytes++] = *src;
            } else {
                dest[bytes]     = '%';
                // *src >> 4 = *src / 16
                dest[bytes + 1] = DEC2HEX[*src >> 4];
                // *src & 0xf = *src % 16
                dest[bytes + 2] = DEC2HEX[*src & 0xf];
                bytes += 3;
            }
            src++;
        }

        *len = bytes;
    }

    return dest;
}

static int encode_lua(lua_State *L, const unsigned char *tbl)
{
    size_t len      = 0;
    const char *src = lauxh_checklstring(L, 1, &len);
    char *dest      = (char *)encode((char *)src, len, tbl, &len);

    if (dest) {
        lua_pushlstring(L, dest, len);
        return 1;
    }

    // got error
    lua_pushnil(L);
    lua_pushinteger(L, errno);

    return 2;
}

static int encodeuri_lua(lua_State *L)
{
    return encode_lua(L, UNRESERVED_URI);
}
static int encode2396_lua(lua_State *L)
{
    return encode_lua(L, UNRESERVED_2396);
}
static int encode3986_lua(lua_State *L)
{
    return encode_lua(L, UNRESERVED_3986);
}

/*
                hex: 0xf                 = 0-15      = 4bit
    utf8 code-point: u+0000 ... u+10ffff = 0-1114111 = 21bit
              ascii: u+0000 ... u+007f   = 0-127     = 7bit
*/
static int unicode_pt2utf8(unsigned char *dest, size_t *len, uint32_t cp)
{
    // range: u+0000 ... u+007f
    //   bit: 0xxx xxxx
    if (cp < 0x80) {
        dest[0] = cp;
        *len += 1;
    }
    // range: u+0080 ... u+07ff
    //   bit: [110y yyyx]:0xc0
    //        [10xx xxxx]:0x80
    else if (cp < 0x800) {
        dest[0] = 0xc0 | (cp >> 6);
        dest[1] = 0x80 | (cp & 0x3f);
        *len += 2;
    }
    // invalid code-point
    // range: u+d800 ... u+dfff use for surrogate pairs
    else if (cp > 0xD7FF && cp < 0xE000) {
        errno = ERANGE;
        return -1;
    }
    // range: u+0800 ... u+ffff
    //   bit: [1110 yyyy]:0xe0
    //        [10yx xxxx]:0x80
    //        [10xx xxxx]:0x80
    else if (cp < 0x10000) {
        dest[0] = 0xe0 | (cp >> 12);
        dest[1] = 0x80 | ((cp >> 6) & 0x3f);
        dest[2] = 0x80 | (cp & 0x3f);
        *len += 3;
    }
    // range: u+10000 ... u+1FFFFF
    //   bit: [1111 0yyy]:0xf0
    //        [10yy xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    //
    // RFC 3629: UTF-8, characters from the U+0000..U+10FFFF
    // else if( cp < 0x200000 ){
    else if (cp < 0x110000) {
        dest[0] = 0xf0 | (cp >> 18);
        dest[1] = 0x80 | ((cp >> 12) & 0x3f);
        dest[2] = 0x80 | ((cp >> 6) & 0x3f);
        dest[3] = 0x80 | (cp & 0x3f);
        *len += 4;
    }
    /* UTF-8 now max 4 bytes
    // range: u+200000 ... u+3FFFFFF
    //   bit: [1111 10yy]:0xf8
    //        [10yy xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    else if( cp < 0x400000 ){
        dest[decLen] = 0xf8 | ( cp >> 24 );
        dest[decLen+1] = 0x80 | ( ( cp >> 18 ) & 0x3f );
        dest[decLen+2] = 0x80 | ( ( cp >> 12 ) & 0x3f );
        dest[decLen+3] = 0x80 | ( ( cp >> 6 ) & 0x3f );
        dest[decLen+4] = 0x80 | ( cp & 0x3f );
        decLen += 5;
    }
    // range: u+4000000 ... u+7FFFFFFF
    //   bit: [1111 110y]:0xfc
    //        [10yy xxxx]:0x80
    //        [10yy xxxx]:0x80
    //        [10yy xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    else if( cp < 0x800000 ){
        dest[decLen] = 0xfc | ( cp >> 30 );
        dest[decLen+1] = 0x80 | ( ( cp >> 24 ) & 0x3f );
        dest[decLen+2] = 0x80 | ( ( cp >> 18 ) & 0x3f );
        dest[decLen+3] = 0x80 | ( ( cp >> 12 ) & 0x3f );
        dest[decLen+4] = 0x80 | ( ( cp >> 6 ) & 0x3f );
        dest[decLen+5] = 0x80 | ( cp & 0x3f );
        decLen += 6;
    }
    //*/
    // invalid: code-point > 0x7FFFFFFF
    else {
        errno = ERANGE;
        return -1;
    }

    return 0;
}

/*
                   hex: 0xf                 = 0-15  = 4bit
    unicode code-point: u+0000 ... u+10ffff = 21bit
                 ascii: u+0000 ... u+007f   = 0-127 = 7bit
*/
static unsigned char *decode(char *str, size_t slen, const unsigned char *tbl,
                             size_t *len)
{
    unsigned char *dest = malloc(slen + 1);

    if (dest) {
        unsigned char *src = (unsigned char *)str;
        uint32_t hl, hi, lo;
        size_t bytes = 0;
        size_t i     = 0;

        for (; i < slen; i++) {
            if (*src != '%') {
                dest[bytes++] = *src;
                src++;
            }
            // percent-encoding(%hex) must have more than 2 byte strings after
            // '%'.
            else if (i + 2 > slen) {
                goto INVALID_ENCODING;
            }
            /*
                hex(8bit) to decimal
                e.g.
                    hex:'%41'
                    '4' to hex:0x04[0000 0100]
                    '1' to hex:0x01[0000 0001]
                    0x04[0000 0100] << 4bit
                    0x40[0100 0000] | 0x01[0000 0001]
                    0x41[0100 0001]
                    0x41 = 65 = 'A'

                    hex:'%7a'
                    '7' to hex:0x07[0000 0111]
                    'a' to hex:0x0a[0000 1010]
                    0x07[0000 0111] << 4bit
                    0x70[0111 0000] | 0x0a[0000 1010]
                    0x7a[0111 1010]
                    0x7a:122 = 'z'

                    hex:'%7a4'
                    '7' to hex:0x07[0000 0111]
                    'a' to hex:0x0a[0000 1010]
                    '4' to hex:0x04[0000 0100]
                    0x007[0000 0000 0111] << 8bit
                    0x00a[0000 0000 1010] << 4bit
                    0x70[0111 0000 0000] | [0000 1010 0000] | 0x04[0000 0100]
                    0x7a4[0111 1010 0100] = 1956
            */
            // %[hex]*2
            else if (HEX2DEC[src[1]] > -1 && HEX2DEC[src[2]] > -1) {
                /*
                    hi = HEX2DEC( src[1] ) << 4;
                    lo = HEX2DEC( src[2] );
                    hl = hi | lo;
                */
                hl = (HEX2DEC[src[1]] << 4) | HEX2DEC[src[2]];
                if (!tbl[hl]) {
                    dest[bytes++] = hl;
                } else {
                    dest[bytes]     = src[0];
                    dest[bytes + 1] = src[1];
                    dest[bytes + 2] = src[2];
                    bytes += 3;
                }
                i += 2;
                src += 3;
            }
            // %u[hex]*4
            else if (src[1] == 'u' && HEX2DEC[src[2]] > -1 &&
                     HEX2DEC[src[3]] > -1 && HEX2DEC[src[4]] > -1 &&
                     HEX2DEC[src[5]] > -1) {
                hi = HEX2DEC[src[2]] << 4 | HEX2DEC[src[3]];
                lo = HEX2DEC[src[4]] << 4 | HEX2DEC[src[5]];
                hl = (hi << 8) | lo;
                if (unicode_pt2utf8(dest + bytes, &bytes, hl) == -1) {
                    goto INVALID_ENCODING;
                }
                i += 4;
                src += 6;
            } else {
                goto INVALID_ENCODING;
            }
        }

        *len = bytes;

        return dest;

INVALID_ENCODING:
        errno = EINVAL;
        free(dest);
        dest = NULL;
    }

    return dest;
}

static int decode_lua(lua_State *L, const unsigned char *tbl)
{
    size_t len      = 0;
    const char *src = lauxh_checklstring(L, 1, &len);
    char *dest      = (char *)decode((char *)src, len, tbl, &len);

    if (dest) {
        lua_pushlstring(L, dest, len);
        free(dest);
        return 1;
    }

    // got error
    lua_pushnil(L);
    lua_pushinteger(L, errno);

    return 2;
}

static int decodeuri_lua(lua_State *L)
{
    return decode_lua(L, UNRESERVED_URI);
}
static int decode2396_lua(lua_State *L)
{
    return decode_lua(L, UNRESERVED_2396);
}
static int decode3986_lua(lua_State *L)
{
    return decode_lua(L, UNRESERVED_3986);
}

LUALIB_API int luaopen_url_codec(lua_State *L)
{
    struct luaL_Reg method[] = {
        {"encodeURI",  encodeuri_lua },
        {"decodeURI",  decodeuri_lua },
        {"encode2396", encode2396_lua},
        {"decode2396", decode2396_lua},
        {"encode3986", encode3986_lua},
        {"decode3986", decode3986_lua},
        {NULL,         NULL          }
    };
    int i;

    // method
    lua_newtable(L);
    i = 0;
    while (method[i].name) {
        lauxh_pushfn2tbl(L, method[i].name, method[i].func);
        i++;
    }

    return 1;
}
