/*
 *  Copyright (C) 2013 Masatoshi Teruya
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */
/*
 *  uriparser_bind.c
 *  Created by Masatoshi Teruya on 13/05/17.
 */

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <lauxlib.h>
#include <lualib.h>
#include <uriparser/Uri.h>

#define lstate_tbl2tbl_start(L,k) do{ \
    lua_pushstring(L,k); \
    lua_newtable(L); \
}while(0)

#define lstate_tbl2tbl_end(L)    lua_rawset(L,-3);

#define lstate_str2tbl(L,k,v) do{ \
    lua_pushstring(L,k); \
    lua_pushstring(L,v); \
    lua_rawset(L,-3); \
}while(0)

#define lstate_strn2tbl(L,k,v,n) do{ \
    lua_pushstring(L,k); \
    lua_pushlstring(L,v,n); \
    lua_rawset(L,-3); \
}while(0)

#define lstate_bool2tbl(L,k,v) do{ \
    lua_pushstring(L,k); \
    lua_pushboolean(L,v); \
    lua_rawset(L,-3); \
}while(0)

#define lstate_pusherr(L,c) do{ \
    lua_settop(L,0); \
    lua_pushnil(L); \
    switch(c){ \
        case URI_ERROR_SYNTAX: \
            lua_pushstring(L,"Parsed text violates expected format"); \
        break; \
        case URI_ERROR_NULL: \
            lua_pushstring(L,"One of the params passed was NULL"); \
        break; \
        case URI_ERROR_MALLOC: \
            lua_pushstring(L,"Requested memory could not be allocated"); \
        break; \
        case URI_ERROR_OUTPUT_TOO_LARGE: \
            lua_pushstring(L,"Some output is to large for the receiving buffer"); \
        break; \
        case URI_ERROR_NOT_IMPLEMENTED: \
            lua_pushstring(L,"The called function is not implemented yet"); \
        break; \
        case URI_ERROR_RANGE_INVALID: \
            lua_pushstring(L,"The parameters passed contained invalid ranges"); \
        break; \
        case URI_ERROR_ADDBASE_REL_BASE: \
            lua_pushstring(L,"Given base is not absolute"); \
        break; \
        case URI_ERROR_REMOVEBASE_REL_BASE: \
            lua_pushstring(L,"Given base is not absolute"); \
        break; \
        case URI_ERROR_REMOVEBASE_REL_SOURCE: \
            lua_pushstring(L,"Given base is not absolute"); \
        break; \
    } \
}while(0)


static int parse_query( lua_State *L, const char *str, size_t len )
{
    UriQueryListA *qry = NULL;
    int nqry = 0;
    int rc = uriDissectQueryMallocA( &qry, &nqry, str, str + len );
    
    if( rc == URI_SUCCESS )
    {
        UriQueryListA *ptr = qry;
        while( ptr )
        {
            if( ptr->key ){
                lstate_str2tbl( L, ptr->key, ptr->value ? ptr->value : "" );
            }
            ptr = ptr->next;
        }
        uriFreeQueryListA( qry );
    }
    
    return URI_SUCCESS;
}


static int parse_lua( lua_State *L )
{
    int rc = 0;
    int argc = lua_gettop( L );
    size_t len = 0;
    const char *url = luaL_checklstring( L, 1, &len );
    const char *pathTail = url + len;
    int parseQry = 0;
    UriParserStateA state;
    UriUriA uri;
    
    if( argc > 1 ){
        luaL_argcheck( L, lua_isboolean( L, 2 ), 2, "must be boolean" );
        parseQry = lua_toboolean( L, 2 );
    }
    
    // verify request-url
    state.uri = &uri;
    rc = uriParseUriA( &state, url );
    if( rc != URI_SUCCESS ){
        lstate_pusherr( L, rc );
    }
    else
    {
        // create table
        lua_newtable( L );
        // set absolutePath
        lstate_bool2tbl( L, "absolutePath", uri.absolutePath );
        // set scheme
        if( uri.scheme.first ){
            lstate_strn2tbl( L, "scheme", uri.scheme.first, 
                             uri.scheme.afterLast - uri.scheme.first );
        }
        // set userInfo
        if( uri.userInfo.first ){
            lstate_strn2tbl( L, "userinfo", uri.userInfo.first, 
                             uri.userInfo.afterLast - uri.userInfo.first );
        }
        // set hostText
        if( uri.hostText.first ){
            lstate_strn2tbl( L, "host", uri.hostText.first, 
                             uri.hostText.afterLast - uri.hostText.first );
        }
        // set portText
        if( uri.portText.first ){
            lstate_strn2tbl( L, "port", uri.portText.first, 
                             uri.portText.afterLast - uri.portText.first );
        }
        
        // set fragment
        if( uri.fragment.first ){
            pathTail = uri.fragment.first - 1;
            lstate_strn2tbl( L, "fragment", uri.fragment.first, 
                             uri.fragment.afterLast - uri.fragment.first );
        }
        
        // set query
        if( uri.query.first )
        {
            pathTail = uri.query.first - 1;
            // no query parse
            if( !parseQry ){
                lstate_strn2tbl( L, "query", uri.query.first, 
                                 uri.query.afterLast - uri.query.first );
            }
            else
            {
                lstate_tbl2tbl_start( L, "query" );
                rc = parse_query( L, uri.query.first, 
                                  uri.query.afterLast - uri.query.first );
                if( rc == URI_SUCCESS ){
                    lstate_tbl2tbl_end( L );
                }
                else {
                    lstate_pusherr( L, rc );
                    goto CLEANUP;
                }
            }
        }
        
        // set path
        if( uri.pathHead ){
            const char *pathHead = uri.pathHead->text.first - 1;
            lstate_strn2tbl( L, "path", pathHead, pathTail - pathHead - 1 );
        }
        else {
            lstate_strn2tbl( L, "path", "/", 1 );
        }
        lua_pushnil(L);
    }
    
    CLEANUP:
        uriFreeUriMembersA( &uri );
    
    return 2;
}


static int parse_query_lua( lua_State *L )
{
    size_t len = 0;
    const char *qry = luaL_checklstring( L, 1, &len );
    int rc = 0;
    
    // create table
    lua_newtable( L );
    rc = parse_query( L, qry, len );
    if( rc == URI_SUCCESS ){
        lua_pushnil(L);
    }
    else {
        lstate_pusherr( L, rc );
    }
    
    return 2;
}

// make error
static int const_newindex( lua_State *L ){
    return luaL_error( L, "attempting to change protected module" );
}

LUALIB_API int luaopen_uriparser( lua_State *L )
{
    struct luaL_Reg funcs[] = {
        { "parse", parse_lua },
        { "parseQuery", parse_query_lua },
        { NULL, NULL }
    };
    int i = 0;
    
    // create protected-table
    lua_newtable( L );
    // create __metatable
    lua_newtable( L );
    // create substance
    lua_pushstring( L, "__index" );
    lua_newtable( L );
    
    // set functions
    for( i = 0; funcs[i].name; i++ ){ 
        lua_pushstring( L, funcs[i].name );
        lua_pushcfunction( L, funcs[i].func );
        lua_rawset( L, -3 );
    }
    
    // set substance to __metable.__index field
    lua_rawset( L, -3 );
    // set __newindex function to __metable.__newindex filed
    lua_pushstring( L, "__newindex" );
    lua_pushcfunction( L, const_newindex );
    lua_rawset( L, -3 );
    // convert protected-table to metatable
    lua_setmetatable( L, -2 );
    
    return 1;
}

