lua-url
====

url string utility.

## Dependencies

- uriparser: <https://github.com/mah0x211/lua-uriparser>


## Installation

```sh
luarocks install --from=http://mah0x211.github.io/rocks/ url
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

### res, err = parse( url [, parseQuery] )

returns the table of parsed url.

**Parameters**

- `url:string`: url string.
- `parseQuery:boolean`: parse query-string if `true`.

**Returns**

1. `res:table`: url info table.
2. `err:string`: error string.

**Example**

```lua
local url = require('url');
local uri = url.parse('http://user:pass@host.com:8080/p/a/t/h/?query=string#hash');

--[[
following items is included in the result table;
{
    host = "host.com"
    path = "/p/a/t/h/"
    scheme = "http"
    userinfo = "user:pass"
    query = "query=string"
    fragment = "hash"
    port = "8080"
}
--]]
```

### res, err = parseQuery( qry )

returns the table of parsed query-string.

**Parameters**

- `qry:string`: query string.

**Returns**

1. `res:table`: query info table.
2. `err:string`: error string.

**Example**

```lua
local url = require('url');
local qry = url.parseQuery('query1=string1&query2=string2');

--[[
following items is included in the result table;
{
    query1 = "string1",
    query2 = ""string2"
}
--]]
```
