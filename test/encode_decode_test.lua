local testcase = require('testcase')
local url = require('url');

local TESTSTR = [[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]]

function testcase.encode_decode()
    local ec

    -- test that encodeURL
    ec = assert(url.encodeURI(TESTSTR))
    assert.not_equal(ec, TESTSTR)
    -- the encoded string can be decoded with the decode* funcs
    assert.equal(TESTSTR, url.decodeURI(ec))
    assert.equal(TESTSTR, url.decode2396(ec))
    assert.equal(TESTSTR, url.decode3986(ec))

    -- test that encode2396
    ec = assert(url.encode2396(TESTSTR))
    assert.not_equal(ec, TESTSTR)
    -- the encoded string can be decoded with the decode2396/3986 funcs
    assert.not_equal(TESTSTR, url.decodeURI(ec))
    assert.equal(TESTSTR, url.decode2396(ec))
    assert.equal(TESTSTR, url.decode3986(ec))

    -- test that encode3986
    ec = assert(url.encode3986(TESTSTR))
    assert.not_equal(ec, TESTSTR)
    -- the encoded string can be decoded with the decode3986 func
    assert.not_equal(TESTSTR, url.decodeURI(ec))
    assert.not_equal(TESTSTR, url.decode2396(ec))
    assert.equal(TESTSTR, url.decode3986(ec))
end

function testcase.decode_unicode_point()
    -- test that decode unicode point
    assert.equal(url.decodeURI(
                     '%u0041 %u00E8 %u3042 %uD869%uDEB2 %u0041 %u00E8 %u3042 %uD869%uDEB2'),
                 'A è あ 𪚲 A è あ 𪚲')

    -- test that returns err if invalid code point
    local s, err = url.decodeURI('%4')
    assert.is_nil(s)
    assert.equal(err, 22)
end
