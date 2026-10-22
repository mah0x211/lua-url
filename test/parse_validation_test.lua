local testcase = require('testcase')
local parse = require('url.parse')

local function success(s, expected, params, init, query)
    local result, cur, err = parse(s, params, init, query)
    assert.is_nil(err)
    assert.equal(cur, #s)
    assert.equal(result, expected)
end

function testcase.initial_cursor_bounds()
    -- Fail safely on the old parser before trying a nonempty out-of-range input.
    local ok = pcall(parse, '', false, 1)
    assert.is_false(ok)
    for _, query in ipairs({ false, true }) do
        assert.is_false(pcall(parse, 'x', false, 2, query))
        success('x', {}, false, 1, query)
        success('', {}, false, 0, query)
    end
    success('x/path', { path = '/path' }, false, 1)
end

function testcase.first_equals_separates_query_value()
    for _, input in ipairs({ '?x=a=b', '?x=a%3Db' }) do
        success(input, { query = input, query_params = { x = { 'a=b' } } }, true)
    end
    success('?x==&x=a+b=c&=v=z&y&z=%3D=', {
        query = '?x==&x=a+b=c&=v=z&y&z=%3D=',
        query_params = { x = { '=', 'a b=c' }, [''] = { 'v=z' },
                         y = { '' }, z = { '==' } },
    }, true)
end

function testcase.ipv6_end_and_fragment()
    for _, suffix in ipairs({ '', '#fragment', '?x=y', '/path' }) do
        local expected = { scheme = 'http', host = '[::1]', hostname = '[::1]' }
        if suffix == '#fragment' then expected.fragment = 'fragment'
        elseif suffix == '?x=y' then expected.query = '?x=y'
        elseif suffix == '/path' then expected.path = '/path' end
        success('http://[::1]' .. suffix, expected)
    end
end

function testcase.ipv6_address_validation()
    for _, ip in ipairs({ '', '1', '1:2', '::ffff:192.0.2.', '::ffff:256.0.2.1',
                          '::ffff:01.0.2.1', '1:2:3:4:5:6:7',
                          '1:2:3:4:5:6:7:8:9', '1::2::3', '12345::',
                          '1:2:3:4:5:6:7:8::', '1:2:3:4:5:6:7::8',
                          '::ffff:192.0.2.1.1', ':', '1:2:3:4:5:6:7:8:',
                          '1:2:3:4:5:6:7:192.0.2.1', '::ffff:192.0..1',
                          '::ffff:192.0.2a.1', '::ffff:192.0.2.0001', '::g' }) do
        local s = 'http://[' .. ip .. ']/'
        local _, cur, err = parse(s)
        assert.is_true(err ~= nil)
        assert.equal(err, s:sub(cur + 1, cur + 1))
    end
    for _, ip in ipairs({ '::', '::1', '1::', '1:2:3:4:5:6:7:8',
                          '1:2:3:4:5:6:7::', '::2:3:4:5:6:7:8',
                          '::ffff:192.0.2.1', '1:2:3:4:5:6:192.0.2.1',
                          '::192.0.2.1' }) do
        local host = '[' .. ip .. ']'
        success('http://' .. host .. '/', {
            scheme = 'http', host = host, hostname = host, path = '/',
        })
    end
    for _, s in ipairs({ 'http://[', 'http://[::', 'http://[::ffff:192.0.2.1' }) do
        local _, cur, err = parse(s)
        assert.equal(cur, #s)
        assert.equal(err, string.char(0))
    end
end

function testcase.short_host_and_port()
    success('http://a:', { scheme = 'http', host = 'a', hostname = 'a' })
    success('http://:1', { scheme = 'http', host = ':1', hostname = '', port = '1' })
    success('http://:?x=y', { scheme = 'http', query = '?x=y' })
end

function testcase.userinfo_is_not_a_port()
    for _, password in ipairs({ '65536', '123456789012345678901234567890',
                                'pa:ss', ':', '', 'pa%3Ass' }) do
        local userinfo = 'user:' .. password
        success('http://' .. userinfo .. '@example.com:65535/', {
            scheme = 'http', userinfo = userinfo, user = 'user', password = password,
            hostname = 'example.com', host = 'example.com:65535', port = '65535', path = '/',
        })
    end
    for _, prefix in ipairs({ 'http://host:', 'http://u:p@host:', 'http://[::1]:' }) do
        local s = prefix .. '65536/'
        local _, cur, err = parse(s)
        assert.equal(cur, #prefix + 4)
        assert.equal(err, '6')
    end
end

function testcase.scheme_grammar()
    success('foo.bar://example.com/path', {
        scheme = 'foo.bar', hostname = 'example.com', host = 'example.com', path = '/path',
    })
    local _, _, err = parse('1http://example.com/path')
    assert.is_true(err ~= nil)
    success('host.com:8080', { path = 'host.com:8080' })
    local _, cur, invalid = parse('http:/localhost')
    assert.equal(cur, 4)
    assert.equal(invalid, ':')
end

function testcase.reg_name_initial_characters()
    for _, first in ipairs({ '_', '~', '-', '!', '$', '&', "'", '(', ')', '*', '+', ',', ';', '=' }) do
        local host = first .. 'host.example'
        success('http://' .. host .. '/', {
            scheme = 'http', host = host, hostname = host, path = '/',
        })
    end
end

function testcase.ipv6_nul_is_not_end_of_input()
    local s = 'http://[::1]' .. string.char(0) .. 'tail'
    local _, cur, err = parse(s)
    assert.equal(cur, 12)
    assert.equal(err, string.char(0))
end

function testcase.schemeless_path_with_multiple_colons()
    success('host.com:8080://host', { path = 'host.com:8080://host' })
end
