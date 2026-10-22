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

function testcase.encode_form()
    -- test that encodeURL
    local s = url.encode_form(TESTSTR)
    s = string.gsub(s, '%%[a-fA-F0-9][a-fA-F0-9]', '')

    local mark = "+*-._~"
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

function testcase.decode_form()
    local escaped = ''
    for i = 1, 0x7E do
        escaped = escaped .. string.format('%%%02X', i)
    end

    -- test that decode all escaped characters
    local decoded = assert(url.decode_form(escaped))
    local s = ''
    for c in string.gmatch(decoded, '%%[a-fA-F0-9][a-fA-F0-9]') do
        local n = tonumber(string.sub(c, 2), 16)
        s = s .. string.char(n)
    end
    assert.equal(#s, 0)

    -- test that decode_form is decode '+' to ' '
    assert.equal(url.decode_form('h+ello+++world!'), 'h ello   world!')
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

function testcase.decode_all_byte_values()
    local upper = {}
    local lower = {}
    local bytes = {}
    for i = 0, 255 do
        upper[#upper + 1] = string.format('%%%02X', i)
        lower[#lower + 1] = string.format('%%%02x', i)
        bytes[#bytes + 1] = string.char(i)
    end

    local expected = table.concat(bytes)
    for _, name in ipairs({ 'decode', 'decode_form' }) do
        assert.equal(url[name](table.concat(upper)), expected)
        assert.equal(url[name](table.concat(lower)), expected)
    end
end

function testcase.decode_utf8_percent_bytes()
    local escaped = '%41%20%C3%A8%20%E3%81%82%20%F0%AA%9A%B2'
    for _, name in ipairs({ 'decode_uri', 'decode_form', 'decode' }) do
        assert.equal(url[name](escaped), 'A è あ 𪚲')
    end
end

function testcase.decode_uri_binary_bytes()
    assert.equal(url.decode_uri('%00%7F%80%FF'), '\0\127\128\255')
end

function testcase.decode_uri_preserves_reserved_escapes()
    local escaped = '%23%24%26%2B%2b%2C%2c%2F%2f%3A%3a%3B%3b%3D%3d%3F%3f%40'
    assert.equal(url.decode_uri(escaped), escaped)
end

function testcase.decode_preserves_percent_encoded_text()
    for _, name in ipairs({ 'decode_uri', 'decode_form', 'decode' }) do
        assert.equal(url[name]('%25u0041'), '%u0041')
        assert.equal(url[name]('%2541'), '%41')
        assert.equal(url[name]('%41tail'), 'Atail')
        assert.equal(url[name]('raw\0bytes'), 'raw\0bytes')
    end
end

function testcase.decode_form_literal_and_encoded_plus()
    assert.equal(url.decode_form('a+b%2Bc'), 'a b+c')
    assert.equal(url.decode('a+b%2Bc'), 'a+b+c')
    assert.equal(url.decode_uri('a+b%2Bc'), 'a+b%2Bc')
end

function testcase.decode_empty_string()
    for _, name in ipairs({ 'decode_uri', 'decode_form', 'decode' }) do
        local s, err = url[name]('')
        assert.equal(s, '')
        assert.is_nil(err)
    end
end

function testcase.decode_rejects_percent_u()
    local cases = {
        { '%u0041', 1 },
        { '%u0000', 1 },
        { '%u00E8', 1 },
        { '%u3042', 1 },
        { '%uD869%uDEB2', 1 },
        { '%uD800', 1 },
        { '%uDC00', 1 },
        { '%uFFFF', 1 },
        { '%U0041', 1 },
        { '%u', 1 },
        { '%u0', 1 },
        { '%u00', 1 },
        { '%u000', 1 },
        { '%uZZZZ', 1 },
        { 'prefix%u0041', 7 },
        { '%41%u0042', 4 },
    }
    for _, name in ipairs({ 'decode_uri', 'decode_form', 'decode' }) do
        for _, case in ipairs(cases) do
            local s, err = url[name](case[1])
            assert.is_nil(s)
            assert.equal(err, case[2])
        end
    end
end

function testcase.decode_rejects_malformed_percent_escapes()
    local cases = {
        { '%', 1 },
        { '%0', 1 },
        { '%GG', 1 },
        { '%G0', 1 },
        { '%0G', 1 },
        { '%aZ', 1 },
        { '%%41', 1 },
        { '%\0', 1 },
        { '%0\0', 1 },
        { '%\2550', 1 },
        { '%0\255', 1 },
        { 'abc%', 4 },
        { '%41%0', 4 },
        { 'a\0b%G0', 4 },
    }
    for _, name in ipairs({ 'decode_uri', 'decode_form', 'decode' }) do
        for _, case in ipairs(cases) do
            local s, err = url[name](case[1])
            assert.is_nil(s)
            assert.equal(err, case[2])
        end
    end
end
