lua-url
====

[![test](https://github.com/mah0x211/lua-url/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-url/actions/workflows/test.yml)

url string utility.

## Installation

```sh
luarocks install url
```


## String Codecs

### Encoding

returns the encoded string.

- str, err = encodeURI( uri ): based on ECMAScript. please see [developer.mozilla.org](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURI) for more details.
- str, err = encode2396( uri ): based on RFC 2396.
- str, err = encode3986( uri ): based on RFC 3986.

**Parameters**

- `uri:string`: uri string.

**Returns**

1. `str:string`: encoded string.
2. `err:number`: error number.


### Decoding

returns the decoded string.

- str, err = decodeURI( uri ): based on ECMAScript. please see [developer.mozilla.org](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/decodeURI) for more details.
- str, err = decode2396( uri ): based on RFC 2396.
- str, err = decode3986( uri ): based on RFC 3986.

**Parameters**

- `uri:string`: encoded uri string.

**Returns**

1. `str:string`: decoded string on success, or nil on failure.
2. `err:number`: error number.


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
local url = require('url');

local res, cur, err = url.parse('head http://user:pass@host.com:8080/p/a/t/h/?query=string#hash tail', true, 5)

--[[
cur = 62,
err = " ",
res = {
    fragment = "hash",
    host = "host.com:8080",
    hostname = "host.com",
    password = "pass",
    path = "/p/a/t/h/",
    port = "8080",
    query = "?query=string",
    queryParams = {
        query = "string"
    },
    scheme = "http",
    user = "user",
    userinfo = "user:pass"
}
--]]


-- parse query
res, cur, err = url.parse('head ?query=string#hash tail', false, 5);

--[[
cur = 23,
err = " ",
res = {
    fragment = "hash",
    query = "?query=string"
}
--]]
```
