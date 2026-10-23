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
            if tbl and tbl[dec] then
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

local url = require('url')
local timer = require('testcase.timer')

local str = [[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]]

local num = arg[1] == nil and 10000 or tonumber(arg[1])
assert(num and num > 0 and num < math.huge and num % 1 == 0,
       'number of loops must be a positive integer')

print('usage: lua ' .. arg[0] .. ' [number of loops:default 10000]')
print('loop: ' .. num)

-- Check that both implementations perform the same conversions before timing.
local function verify(encode_fn, decode_fn, native_encode, native_decode)
    local encoded = encode_fn(str)
    assert(encoded == native_encode(str), 'encoded output differs from url module')
    local decoded = decode_fn(encoded)
    assert(decoded == native_decode(encoded), 'decoded output differs from url module')
    assert(decoded == str, 'round-trip conversion failed')
end
verify(encodeURI, decodeURI, url.encode_uri, url.decode_uri)
verify(encode2396, decode, url.encode2396, url.decode)
verify(encode3986, decode, url.encode3986, url.decode)

local function measure(name, encode_uri, decode_uri, encode_rfc2396, encode_rfc3986, decode_all)
    local t = timer.new()
    assert(t:start())
    for _ = 1, num do
        decode_uri(encode_uri(str))
        decode_all(encode_rfc2396(str))
        decode_all(encode_rfc3986(str))
    end
    local elapsed, fmt = t:stop()
    assert(elapsed, fmt)
    print(name .. ': ' .. string.format(fmt, elapsed))
end

measure('pure Lua', encodeURI, decodeURI, encode2396, encode3986, decode)
measure('url module', url.encode_uri, url.decode_uri, url.encode2396, url.encode3986, url.decode)
