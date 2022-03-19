lua-url
====

[![test](https://github.com/mah0x211/lua-url/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-url/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-url/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-url)


url string utility.

## Installation

```sh
luarocks install url
```


## Encoding

```
str = encodeURI( str )
str = encode2396( str )
str = encode3986( str )
```

encode a string to a percent-encoded string.

- `encodeURI` encodes characters except `ALPHA_DIGIT (a-zA-Z0-9)` and `!#$&'()*+,./:;=?@_~-`.
  - based on ECMAScript. please see [developer.mozilla.org](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURI) for more details.
- `encode2396` encodes characters except `ALPHA_DIGIT` and `!'()*._~-`.
  - based on RFC 2396.
- `encode3986` encodes characters except `ALPHA_DIGIT` and `._~-`.
  - based on RFC 3986.


**Parameters**

- `str:string`: a string.

**Returns**

- `str:string`: a encoded string.


## Decoding

```
str, err = decodeURI( str )
str, err = decode( str )
```

decode a percent-encoded string.

- `decodeURI` decodes percent-encoded characters except 
  - based on ECMAScript. please see [developer.mozilla.org](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/decodeURI) for more details.
- `decode` decodes all percent-encoded characters.

**Parameters**

- `str:string`: encoded uri string.

**Returns**

- `str:string`: decoded string on success, or nil on failure.
- `err:integer`: position at where the illegal character was found.


## Parser

### res, cur, err = parse( url [, parseQuery [, init [, param_array]]] )

returns the table of parsed url.

**Parameters**

- `url:string`: url string.
- `parseQuery:boolean`: parse query-string if `true`.
- `init:integer`: where to cursor start position. (default `0`)
- `param_array:boolean`: store the query parameters as an array.. (default `false`)

**Returns**

1. `res:table`: url info table.
2. `cur:number`: cursor stop position.
3. `err:string`: error character.


**Example**

```lua
local dump = require('dump')
local url = require('url')

local res, cur, err = url.parse(
                          'head http://user:pass@host.com:8080/p/a/t/h/?query=string&query=value#hash tail',
                          true, 5, true)
print(dump({
    res = res,
    cur = cur,
    err = err,
}))
--[[
{
    cur = 74,
    err = " ",
    res = {
        fragment = "hash",
        host = "host.com:8080",
        hostname = "host.com",
        password = "pass",
        path = "/p/a/t/h/",
        port = "8080",
        query = "?query=string&query=value",
        queryParams = {
            query = {
                [1] = "string",
                [2] = "value"
            }
        },
        scheme = "http",
        user = "user",
        userinfo = "user:pass"
    }
}
--]]

res, cur, err = url.parse(
                    'head http://user:pass@host.com:8080/p/a/t/h/?query=string&query=value#hash tail',
                    true, 5)
print(dump({
    res = res,
    cur = cur,
    err = err,
}))
--[[
{
    cur = 74,
    err = " ",
    res = {
        fragment = "hash",
        host = "host.com:8080",
        hostname = "host.com",
        password = "pass",
        path = "/p/a/t/h/",
        port = "8080",
        query = "?query=string&query=value",
        queryParams = {
            query = "value"
        },
        scheme = "http",
        user = "user",
        userinfo = "user:pass"
    }
}
--]]

res, cur, err = url.parse(
                    'head http://user:pass@host.com:8080/p/a/t/h/?query=string&query=value#hash tail',
                    false, 5)
print(dump({
    res = res,
    cur = cur,
    err = err,
}))

--[[
{
    cur = 74,
    err = " ",
    res = {
        fragment = "hash",
        host = "host.com:8080",
        hostname = "host.com",
        password = "pass",
        path = "/p/a/t/h/",
        port = "8080",
        query = "?query=string&query=value",
        scheme = "http",
        user = "user",
        userinfo = "user:pass"
    }
}
--]]

-- parse query
res, cur, err = url.parse('head ?query=string&query=value#hash tail', false, 5)
print(dump({
    res = res,
    cur = cur,
    err = err,
}))

--[[
{
    cur = 35,
    err = " ",
    res = {
        fragment = "hash",
        query = "?query=string&query=value"
    }
}
--]]
```
