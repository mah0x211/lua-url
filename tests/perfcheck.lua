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
    
    if type(src) == 'string' then
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
    if type(src) == 'string' then
        return string.gsub( src, '%%([%x][%x])', function( hex )
            local dec = tonumber( hex, 16 );
            if tbl[dec] then
                return '%' .. hex;
            end
            return string.char( dec );
        end);
    end
    
    return '';
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


local url = require('../url');


local str =[[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]];

local unpack = unpack or table.unpack;
local num = tonumber( _G.arg[1] )
local ec,dc;

print(
    'usage: lua ' .. _G.arg[0] .. 
    ' <number of loops:default 10000> <1:use url module>'
);
if _G.arg[2] == '1' then
    print( 'use: url module' );
    encodeURI = url.encodeURI;
    encode2396 = url.encode2396;
    encode3986 = url.encode3986;
    decodeURI = url.decodeURI;
    decode2396 = url.decode2396;
    decode3986 = url.decode3986;
else
    print( 'use: pure lua implementation' );
end
print( 'loop: ', num );

for i = 0, num do
    ec = encodeURI( str );
    dc = decodeURI( ec );
    --[[
    print( 'encodeURI: ', ec, '\n',
        '-> decodeURI: ', assert( decodeURI( ec ) == str ), '\n',
        '-> decode2396: ', assert( decode2396( ec ) == str ), '\n',
        '-> decode3986: ', assert( decode3986( ec ) == str ), '\n',
        ''
    );
    --]]

    ec = encode2396( str );
    dc = decode2396( ec );
    --[[
    print( 'encode2396', ec, '\n',
        '-> decodeURI: ', assert( decodeURI( ec ) ~= str ), '\n',
        '-> decode2396: ', assert( decode2396( ec ) == str ), '\n',
        '-> decode3986: ', assert( decode3986( ec ) == str ), '\n',
        ''
    );
    --]]

    ec = encode3986( str );
    dc = decode3986( ec );
    --[[
    print( 'encode3986:', ec, '\n',
        '-> decodeURI: ', assert( decodeURI( ec ) ~= str ), '\n',
        '-> decode2396: ', assert( decode2396( ec ) ~= str ), '\n',
        '-> decode3986: ', assert( decode3986( ec ) == str ), '\n',
        ''
    );
    --]]
end

