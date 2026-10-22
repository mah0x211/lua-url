local testcase = require('testcase')
local parse = require('url.parse')
local codec = require('url.codec')

local function parsed(s, expected)
    local actual, cur, err = parse(s)
    assert.is_nil(err)
    assert.equal(cur, #s)
    assert.equal(actual, expected)
end

local function rejected(s, cur)
    local _, actual, err = parse(s)
    assert.equal(actual, cur)
    assert.equal(err, cur == #s and string.char(0) or s:sub(cur + 1, cur + 1))
end

function testcase.ip_literal_invalid_and_incomplete_boundaries()
    for _, prefix in ipairs({
        'http://[:', 'http://[::1', 'http://[::1:',
        'http://[::ffff:192.', 'http://[::ffff:192.0',
        'http://[::ffff:192.0.2.', 'http://[::ffff:192.0.2.1',
    }) do
        rejected(prefix, #prefix)
    end
    for _, case in ipairs({
        { 'http://[::1g]/', 'g' },
        { 'http://[::ffff:1234.1.2.3]/', '4' },
        { 'http://[::ffff:1.2.:.4]/', ':' },
    }) do
        local s, bad = case[1], case[2]
        local start = bad == ':' and #('http://[::ffff:1.2.') + 1 or 1
        rejected(s, assert(string.find(s, bad, start, true)) - 1)
    end
end

function testcase.scheme_and_path_boundaries()
    for _, s in ipairs({ 'a', '-', '+', '0', 'a.b:', 'a.b:/x', 'a.b:x',
                          'path_:x', 'a.b://host:1/path:tail' }) do
        if s == 'a.b://host:1/path:tail' then
            parsed(s, { scheme = 'a.b', host = 'host:1', hostname = 'host',
                        port = '1', path = '/path:tail' })
        else
            parsed(s, { path = s })
        end
    end
    parsed('HTTP://host', { scheme = 'HTTP', host = 'host', hostname = 'host' })
    parsed('aZ09+.-://host', { scheme = 'aZ09+.-', host = 'host', hostname = 'host' })
    for _, s in ipairs({ 'a:', 'a:/', 'a:x/', 'a:/x' }) do
        rejected(s, 1)
    end
    rejected('+://host', 0)
    rejected('-://host', 0)
end

function testcase.authority_port_delimiter_boundaries()
    for _, delimiter in ipairs({ '', '/', '?q=x', '#f' }) do
        for _, authority in ipairs({ 'a:', ':1', ':', 'a:1' }) do
            local expected = { scheme = 'http' }
            if authority == 'a:' then
                expected.host, expected.hostname = 'a', 'a'
            elseif authority == ':1' then
                expected.host, expected.hostname, expected.port = ':1', '', '1'
            elseif authority == 'a:1' then
                expected.host, expected.hostname, expected.port = 'a:1', 'a', '1'
            end
            if delimiter == '/' then expected.path = '/'
            elseif delimiter == '?q=x' then expected.query = '?q=x'
            elseif delimiter == '#f' then expected.fragment = 'f' end
            parsed('http://' .. authority .. delimiter, expected)
        end
    end
    rejected('http://:x', 8)
    rejected('http://?q=x', 7)
    rejected('http://#f', 7)
    parsed('http://:p@host:1', { scheme = 'http', userinfo = ':p', user = '',
                               password = 'p', host = 'host:1', hostname = 'host', port = '1' })
end

function testcase.query_percent_and_hex_boundaries()
    local s = '?x=%2f%2F%AF%af%00&y=%09%20%3A%3a'
    local result, cur, err = parse(s, true)
    assert.is_nil(err)
    assert.equal(cur, #s)
    assert.equal(result.query_params, { x = { '//' .. string.char(175, 175, 0) },
        y = { string.char(9) .. ' ::' } })
end

function testcase.large_query_buffers()
    -- Exercise Lua's buffer growth for each unescape output path.
    for _, case in ipairs({ { '+', ' ' }, { '%41', 'A' }, { 'a', 'a' } }) do
        local raw = string.rep(case[1], 65536)
        if case[1] == 'a' then raw = '%41' .. raw end
        local expected = string.rep(case[2], 65536)
        if case[1] == 'a' then expected = 'A' .. expected end
        local s = '?x=' .. raw
        local result, cur, err = parse(s, true)
        assert.is_nil(err)
        assert.equal(cur, #s)
        assert.equal(result.query_params, { x = { expected } })
    end
end

function testcase.large_codec_buffers()
    local text = string.rep('a', 65536)
    assert.equal(codec.encode_uri(text), text)
    assert.equal(codec.decode_form(string.rep('+', 65536)), string.rep(' ', 65536))
    assert.equal(codec.decode(text), text)
    assert.equal(codec.decode(string.rep('%41', 65536)), string.rep('A', 65536))
end

function testcase.percent_triplet_validation_boundaries()
    for _, prefix in ipairs({ '/x', '?x=', '#x', 'http://host' }) do
        for _, escape in ipairs({ '%', '%0', '%g0', '%0g', '%gg' }) do
            local input = prefix .. escape
            rejected(input, #prefix)
        end
    end
end

function testcase.path_character_classes()
    local s = "/!$&'()*+,-./0123456789:;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~%41"
    parsed(s, { path = s })
end
