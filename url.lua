--[[

  Copyright (C) 2013 Masatoshi Teruya
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

--]]

local uriparser = require('uriparser');
local codec = require('url.codec');


local function normalize( ... )
    local argv = {...};
    local path = argv[1];
    local seg = nil;
    local res = {};
    
    if #argv > 1 then
        path = table.concat( argv, '/' );
    end
    
    for seg in string.gmatch( path, '[^/]+' ) do
        if seg == '..' then
            table.remove( res );
        elseif seg ~= '.' then
            table.insert( res, seg );
        end
    end
    
    return '/' .. table.concat( res, '/' );
end

local function dirname( path )
    return string.match( path, '^(.+)/[^/]+$' );
end

local function basename( path, suffix )
    path = string.match( path, '^.+/([^/]+)$' );
    if suffix and suffix ~= path then
        return string.gsub( path, string.gsub( suffix, '%.', '%%.' ) .. '$', '' );
    end
    
    return path;
end

local function extname( path )
    return string.match( path, '%.[^/.]*$' );
end


return {
    encodeURI = codec.encodeURI,
    decodeURI = codec.decodeURI,
    encode2396 = codec.encode2396,
    decode2396 = codec.decode2396,
    encode3986 = codec.encode3986,
    decode3986 = codec.decode3986,
    normalize = normalize,
    dirname = dirname,
    basename = basename,
    extname = extname,
    parse = uriparser.parse,
    parseQuery = uriparser.parseQuery
};

