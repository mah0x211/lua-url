/**
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

#include <lauxhlib.h>

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
    //  ctrl-code: 0-31
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,

    // SP      "            %
    0, 0, '!', 0, '#', '$', 0, '&', '\'', '(', ')', '*', '+', ',', '-', '.',
    '/',

    // digit
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

    //        <       >
    ':', ';', 0, '=', 0, '?', '@',

    // alpha-upper
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
    //   [  \  ]  ^       `
    'Z', 0, 0, 0, 0, '_', 0,

    // alpha-lower
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    //   {  |  }
    'z', 0, 0, 0, '~'};

/*
    encode_form  : 0-9 a-zA-Z *-._~
    https://url.spec.whatwg.org/#application-x-www-form-urlencoded-percent-encode-set

    application/x-www-form-urlencoded = percent-encode
                                        except: alpha | digit | "*" | "-" |
                                                "." | "_"

    parcent-encode              = component-percent-encode |
                                  "!" | "'" | "(" | ")" | "~"

    component-percent-encode    = userinfo-percent-encode |
                                  "$" | "%" | "&" | "+" | ","

    userinfo-percent-encode     = path-percent-encode |
                                  "/" | ":" | ";" | "=" | "@" | "[" | "\" |
                                  "]" | "^" | "|"

    path-percent-encode         = query-percent-encode |
                                  "?" | "`" | "{" | "}"

    query-percent-encode        = c0-control-percent-encode |
                                  code-points-gt-7e-encode |
                                  " " | '"' | "#" | "<" | ">"

    c0-control-percent-encode   = 0x0 to 0x1F

    code-points-gt-7e-encode    = greater than "~"


    unreserved  = alpha | digit | mark

    alpha       = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
                  "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
                  "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
                  "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
                  "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
                  "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"

    digit       = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"

    mark        = *-._~
*/
static const unsigned char UNRESERVED_FORM[256] = {
    //  ctrl-code: 0-31
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,

    // SP   !  "  #  $  %  &  \  (  )       +  ,            /
    0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, '*', 0, 0, '-', '.', 0,

    // digit                                          :  ;  <  =  >  ?  @
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0, 0, 0, 0, 0, 0,

    // alpha-upper
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
    //   [  \  ]  ^       `
    'Z', 0, 0, 0, 0, '_', 0,

    // alpha-lower
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    //   {  |  }
    'z', 0, 0, 0, '~'};

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
    //  ctrl-code: 0-31
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,

    // SP      "  #  $  %  &                       +  ,            /
    0, 0, '!', 0, 0, 0, 0, 0, '\'', '(', ')', '*', 0, 0, '-', '.', 0,

    // digit                                          :  ;  <  =  >  ?  @
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0, 0, 0, 0, 0, 0,

    // alpha-upper
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
    //   [  \  ]  ^       `
    'Z', 0, 0, 0, 0, '_', 0,

    // alpha-lower
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    //   {  |  }
    'z', 0, 0, 0, '~'};

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
    //  ctrl-code: 0-31
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,

    // SP !  "  #  $  %  &  '  (  )  *  +  ,            /
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', '.', 0,

    // digit
    //                                                :  ;  <  =  >  ?  @
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0, 0, 0, 0, 0, 0, 0,

    // alpha-upper
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
    //   [  \  ]  ^       `
    'Z', 0, 0, 0, 0, '_', 0,

    // alpha-lower
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    //   {  |  }
    'z', 0, 0, 0, '~'};

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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //  0  1  2  3  4  5  6  7  8  9
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0, 0, 0,
    //  A   B   C   D   E   F
    11, 12, 13, 14, 15, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    //  a   b   c   d   e   f
    11, 12, 13, 14, 15, 16};

// %[hex][hex]*4
#define CODEC_UTF8ENC_LEN 12
#define CODEC_UTF8DEC_LEN 4

static int encode_lua(lua_State *L, const unsigned char *tbl)
{
    size_t len            = 0;
    unsigned char *src    = (unsigned char *)lauxh_checklstring(L, 1, &len);
    unsigned char dest[3] = {'%', 0};
    luaL_Buffer b         = {0};

    lua_settop(L, 1);
    luaL_buffinit(L, &b);
    for (size_t i = 0; i < len; i++) {
        unsigned char c          = *src;
        unsigned char unreserved = tbl[c];
        if (unreserved) {
            luaL_addchar(&b, unreserved);
        } else {
            // *src >> 4 = *src / 16
            dest[1] = DEC2HEX[c >> 4];
            // *src & 0xf = *src % 16
            dest[2] = DEC2HEX[c & 0xf];
            luaL_addlstring(&b, (char *)dest, 3);
        }
        src++;
    }

    luaL_pushresult(&b);
    return 1;
}

static int encode_uri_lua(lua_State *L)
{
    return encode_lua(L, UNRESERVED_URI);
}
static int encode_form_lua(lua_State *L)
{
    return encode_lua(L, UNRESERVED_FORM);
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
static int unicode_pt2utf8(luaL_Buffer *b, uint32_t cp)
{
    unsigned char dest[4] = {0};

    // range: u+0000 ... u+007f
    //   bit: 0xxx xxxx
    if (cp < 0x80) {
        luaL_addchar(b, cp);
    }
    // range: u+0080 ... u+07ff
    //   bit: [110y yyyx]:0xc0
    //        [10xx xxxx]:0x80
    else if (cp < 0x800) {
        dest[0] = 0xc0 | (cp >> 6);
        dest[1] = 0x80 | (cp & 0x3f);
        luaL_addlstring(b, (char *)dest, 2);
    }
    // range: u+d800 ... u+dfff use for surrogate pairs
    else if (cp > 0xD7FF && cp < 0xE000) {
        return -2;
    }
    // range: u+0800 ... u+ffff
    //   bit: [1110 yyyy]:0xe0
    //        [10yx xxxx]:0x80
    //        [10xx xxxx]:0x80
    else if (cp < 0x10000) {
        dest[0] = 0xe0 | (cp >> 12);
        dest[1] = 0x80 | ((cp >> 6) & 0x3f);
        dest[2] = 0x80 | (cp & 0x3f);
        luaL_addlstring(b, (char *)dest, 3);
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
        luaL_addlstring(b, (char *)dest, 4);
    }
    /* UTF-8 now max 4 bytes
    // range: u+200000 ... u+3FFFFFF
    //   bit: [1111 10yy]:0xf8
    //        [10yy xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    else if( cp < 0x400000 ){
        dest[0] = 0xf8 | ( cp >> 24 );
        dest[1] = 0x80 | ( ( cp >> 18 ) & 0x3f );
        dest[2] = 0x80 | ( ( cp >> 12 ) & 0x3f );
        dest[3] = 0x80 | ( ( cp >> 6 ) & 0x3f );
        dest[4] = 0x80 | ( cp & 0x3f );
        luaL_addlstring(b, (char *)dest, 5);
    }
    // range: u+4000000 ... u+7FFFFFFF
    //   bit: [1111 110y]:0xfc
    //        [10yy xxxx]:0x80
    //        [10yy xxxx]:0x80
    //        [10yy xxxx]:0x80
    //        [10xx xxxx]:0x80
    //        [10xx xxxx]:0x80
    else if( cp < 0x800000 ){
        dest[0] = 0xfc | ( cp >> 30 );
        dest[1] = 0x80 | ( ( cp >> 24 ) & 0x3f );
        dest[2] = 0x80 | ( ( cp >> 18 ) & 0x3f );
        dest[3] = 0x80 | ( ( cp >> 12 ) & 0x3f );
        dest[4] = 0x80 | ( ( cp >> 6 ) & 0x3f );
        dest[5] = 0x80 | ( cp & 0x3f );
        luaL_addlstring(b, (char *)dest, 6);
    }
    //*/
    // invalid: code-point > 0x7FFFFFFF
    else {
        errno = ERANGE;
        return -1;
    }

    return 0;
}

typedef enum {
    DECODE_ALL  = 0,
    DECODE_URI  = 1,
    DECODE_FORM = 2
} decode_type_e;

/*
                   hex: 0xf                 = 0-15  = 4bit
    unicode code-point: u+0000 ... u+10ffff = 21bit
                 ascii: u+0000 ... u+007f   = 0-127 = 7bit
*/
static int decode(lua_State *L, char *str, size_t slen, decode_type_e dectype)
{
    luaL_Buffer b = {0};

    luaL_buffinit(L, &b);

    for (size_t i = 0; i < slen; i++) {
        unsigned char *src = (unsigned char *)str + i;
        if (*src != '%') {
            if (dectype == DECODE_FORM && *src == '+') {
                luaL_addchar(&b, ' ');
            } else {
                luaL_addchar(&b, *src);
            }
            continue;
        }
        // percent-encoding(%hex) must have more than 2 byte strings after '%'.
        else if (slen < i + 2) {
            lua_pushnil(L);
            lua_pushinteger(L, i + 1);
            return 2;
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
        else if (HEX2DEC[src[1]] && HEX2DEC[src[2]]) {
            /*
                hi = HEX2DEC( src[1] )-1  << 4;
                lo = HEX2DEC( src[2] )-1;
                hl = hi | lo;
            */
            uint32_t hl = ((HEX2DEC[src[1]] - 1) << 4) | (HEX2DEC[src[2]] - 1);

            // decodeURI did not decode the following characters: '#$&+,/:;=?@'
            if (dectype == DECODE_URI) {
                switch (hl) {
                case '#':
                case '$':
                case '&':
                case '+':
                case ',':
                case '/':
                case ':':
                case ';':
                case '=':
                case '?':
                case '@':
                    luaL_addlstring(&b, (char *)src, 3);
                    i += 2;
                    continue;
                }
            }
            luaL_addchar(&b, hl);
            i += 2;
            continue;
        }
        // %u[hex]*4
        else if (src[1] == 'u' && HEX2DEC[src[2]] && HEX2DEC[src[3]] &&
                 HEX2DEC[src[4]] && HEX2DEC[src[5]]) {
            uint32_t hi = (HEX2DEC[src[2]] - 1) << 4 | (HEX2DEC[src[3]] - 1);
            uint32_t lo = (HEX2DEC[src[4]] - 1) << 4 | (HEX2DEC[src[5]] - 1);
            uint32_t hl = (hi << 8) | lo;

            switch (unicode_pt2utf8(&b, hl)) {
            case 0:
                i += 5;
                continue;
            case -2:
                // surrogate pairs
                if (src[6] == '%' && src[7] == 'u' && HEX2DEC[src[8]] &&
                    HEX2DEC[src[9]] && HEX2DEC[src[10]] && HEX2DEC[src[11]]) {
                    size_t surp = 0x10000 + (hl - 0xD800) * 0x400;

                    hi = (HEX2DEC[src[8]] - 1) << 4 | (HEX2DEC[src[9]] - 1);
                    lo = (HEX2DEC[src[10]] - 1) << 4 | (HEX2DEC[src[11]] - 1);
                    surp += ((hi << 8) | lo) - 0xDC00;
                    if (unicode_pt2utf8(&b, surp) == 0) {
                        i += 11;
                        continue;
                    }
                }
            }
        }
        lua_pushnil(L);
        lua_pushinteger(L, i + 1);
        return 2;
    }

    luaL_pushresult(&b);
    return 1;
}

static int decode_uri_lua(lua_State *L)
{
    size_t len      = 0;
    const char *src = lauxh_checklstring(L, 1, &len);

    lua_settop(L, 1);
    return decode(L, (char *)src, len, DECODE_URI);
}

static int decode_form_lua(lua_State *L)
{
    size_t len      = 0;
    const char *src = lauxh_checklstring(L, 1, &len);

    lua_settop(L, 1);
    return decode(L, (char *)src, len, DECODE_FORM);
}

static int decode_lua(lua_State *L)
{
    size_t len      = 0;
    const char *src = lauxh_checklstring(L, 1, &len);

    lua_settop(L, 1);
    return decode(L, (char *)src, len, DECODE_ALL);
}

LUALIB_API int luaopen_url_codec(lua_State *L)
{
    struct luaL_Reg method[] = {
        {"encode_uri",  encode_uri_lua },
        {"encode_form", encode_form_lua},
        {"encode2396",  encode2396_lua },
        {"encode3986",  encode3986_lua },
        {"decode_uri",  decode_uri_lua },
        {"decode_form", decode_form_lua},
        {"decode",      decode_lua     },
        {NULL,          NULL           }
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
