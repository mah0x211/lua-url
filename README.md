#lua-url
url string utility.

## Dependencies

- liburiparser - http://uriparser.sourceforge.net/


## Installation

```sh
luarocks install --from=http://mah0x211.github.io/rocks/ url
```

or 

```sh
git clone https://github.com/mah0x211/lua-url.git
cd lua-url
luarocks make
```

## String Codecs

### Encoding

returns the encoded string.

- encodeURI( str ): based on ECMAScript. please see [developer.mozilla.org](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURI) for more details.
- encode2396( str ): based on RFC 2396.
- encode3986( str ): based on RFC 3986.

**Parameters**

- str: string.

**Returns**

1. str: encoded string.
2. err: error number.


### Decoding

returns the decoded string.

- decodeURI( str ): based on ECMAScript. please see [developer.mozilla.org](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/decodeURI) for more details.
- decode2396( str ): based on RFC 2396.
- decode3986( str ): based on RFC 3986.

**Parameters**

- str: string.

**Returns**

1. str: decoded string on success, or nil on failure.
2. err: error number.


## Parser

### parse( str [, parseQuery] )

returns the table of parsed url.

**Parameters**

- str: url string.
- parseQuery: parse query-string if true.

**Returns**

1. res: url info table.
2. err: error string.

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
    absolutePath = "false"
    fragment = "hash"
    port = "8080"
}
--]]
```

### parseQuery( str )

returns the table of parsed query-string.

**Parameters**

- str: query string.

**Returns**

1. res: query info table.
2. err: error string.

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
