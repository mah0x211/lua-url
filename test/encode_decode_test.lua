local testcase = require('testcase')
local url = require('url')

local ALPHA_LO = 'abcdefghijklmnopqrstuvwxyz'
local ALPHA_UP = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
local DIGIT = '0123456789'
local ALPHADIGIT = ALPHA_LO .. ALPHA_UP .. DIGIT
local TESTSTR = [[ !"#$%&\'()*+,-./
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]]

function testcase.encode_uri()
    -- test that encodeURL
    local s = url.encode_uri(TESTSTR)
    s = string.gsub(s, '%%[a-fA-F0-9][a-fA-F0-9]', '')

    local mark = "!#$&'()*+,./:;=?@_~-"
    local unescaped = ALPHADIGIT .. mark
    assert.equal(#s, #unescaped)
    assert.re_match(s, '[' .. unescaped .. ']')
    assert.not_re_match(s, '[^' .. unescaped .. ']')
end

function testcase.encode2396()
    -- test that encode2396
    local s = url.encode2396(TESTSTR)
    s = string.gsub(s, '%%[a-fA-F0-9][a-fA-F0-9]', '')

    local mark = "!'()*._~-"
    local unescaped = ALPHADIGIT .. mark
    assert.equal(#s, #unescaped)
    assert.re_match(s, '[' .. unescaped .. ']')
    assert.not_re_match(s, '[^' .. unescaped .. ']')
end

function testcase.encode3986()
    -- test that encode3986
    local s = url.encode3986(TESTSTR)
    s = string.gsub(s, '%%[a-fA-F0-9][a-fA-F0-9]', '')

    local mark = '._~-'
    local unescaped = ALPHADIGIT .. mark
    assert.equal(#s, #unescaped)
    assert.re_match(s, '[' .. unescaped .. ']')
    assert.not_re_match(s, '[^' .. unescaped .. ']')
end

function testcase.decode_uri()
    local escaped = ''
    for i = 1, 0x7E do
        escaped = escaped .. string.format('%%%02X', i)
    end

    -- test that decodeURL did not decode '#$&+,/:;=?@' characters
    local decoded = assert(url.decode_uri(escaped))
    local s = ''
    for c in string.gmatch(decoded, '%%[a-fA-F0-9][a-fA-F0-9]') do
        local n = tonumber(string.sub(c, 2), 16)
        s = s .. string.char(n)
    end
    local mark = '#$&+,/:;=?@'
    local undecoded = mark
    assert.equal(#s, #undecoded)
    assert.re_match(s, '[' .. undecoded .. ']')
    assert.not_re_match(s, '[^' .. undecoded .. ']')
end

function testcase.decode()
    local escaped = ''
    for i = 1, 0x7E do
        escaped = escaped .. string.format('%%%02X', i)
    end

    -- test that decode all escaped characters
    local decoded = assert(url.decode(escaped))
    local s = ''
    for c in string.gmatch(decoded, '%%[a-fA-F0-9][a-fA-F0-9]') do
        local n = tonumber(string.sub(c, 2), 16)
        s = s .. string.char(n)
    end
    assert.equal(#s, 0)
end

function testcase.decode_unicode_point()
    -- test that decode unicode point
    assert.equal(url.decode_uri(
                     '%u0041 %u00E8 %u3042 %uD869%uDEB2 %u0041 %u00E8 %u3042 %uD869%uDEB2'),
                 'A è あ 𪚲 A è あ 𪚲')

    -- test that returns err if invalid code point
    local cp = '%20%u4'
    local s, err = url.decode_uri(cp)
    assert.is_nil(s)
    assert.equal(string.sub(cp, 1, err), '%20%')
end
