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

-- conversion table
--[[ 
    0-9 
    a-zA-Z
    !#$&'()*+,-./:;=?@_~
]]
local TBL_ENCURI = {
--  ctrl-code: 0-32
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 
    nil, 
--  SP         "              %
    nil, '!', nil, '#', '$', nil, '&', '\'', '(', ')', '*', '+', ',', '-', '.', 
    '/', 
--                                                               <         >
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', nil, '=', nil, 
    '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
--                                                                    [    \
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', nil, nil, 
--   ]    ^         `
    nil, nil, '_', nil, 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
--                                                          {    |    }
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', nil, nil, nil, '~'
};

--[[ 
    0-9
    a-zA-Z
    !'()*-._~
]]
local TBL_ENC2396 = {
--  ctrl-code: 0-32
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 
    nil,  
--  SP         "    #    $    %    &                         +    ,
    nil, '!', nil, nil, nil, nil, nil, '\'', '(', ')', '*', nil, nil, '-', '.', 
--   /                                                      :    ;    <    =
    nil, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', nil, nil, nil, nil, 
--   >    ?    @
    nil, nil, nil,
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
--                                                          [    \    ]    ^
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', nil, nil, nil, nil, 
--        `
    '_', nil, 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
--                                                          {    |    }
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', nil, nil, nil, '~'
};

--[[ 
    0-9
    a-zA-Z
    -._~
]]
local TBL_ENC3986 = {
--  ctrl-code: 0-32
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 
    nil, 
--  SP    !    "    #    $    %    &    '    (    )    *    +    ,
    nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, '-', '.', 
--   /                                                      :    ;    <    =
    nil, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', nil, nil, nil, nil, 
--   >    ?    @
    nil, nil, nil,
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
--                                                          [    \    ]    ^
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', nil, nil, nil, nil, 
--        `
    '_', nil, 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
--                                                          {    |    }
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', nil, nil, nil, '~'
};


local function encode( src, tbl )
    local dest = '';
    local len = #src;
    local code;
    
    for i = 1, len, 1 do
        code = string.byte( src, i );
        if tbl[code] then
            dest = dest .. string.char(code);
        else
            dest = dest .. '%' .. string.format( '%02X', code );
        end
    end
    
    return dest;
end

local function encodeURI( src )
    return encode( src, TBL_ENCURI );
end

local function encode2396( src )
    return encode( src, TBL_ENC2396 );
end

local function encode3986( src )
    return encode( src, TBL_ENC3986 );
end


local function decode( src, tbl )
    return string.gsub( src, '%%([%x][%x])', function( hex )
        local dec = tonumber( hex, 16 );
        if tbl[dec] then
            return '%' .. hex;
        end
        return string.char( dec );
    end);
end

local function decodeURI( src )
    return decode( src, TBL_ENCURI );
end

local function decode2396( src )
    return decode( src, TBL_ENC2396 );
end

local function decode3986( src )
    return decode( src, TBL_ENC3986 );
end


return {
    encodeURI = encodeURI,
    decodeURI = decodeURI,
    encode2396 = encode2396,
    decode2396 = decode2396,
    encode3986 = encode3986,
    decode3986 = decode3986
};

