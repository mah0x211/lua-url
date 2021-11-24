local concat = table.concat
local assert = require('assertex')
local testcase = require('testcase')
local parse = require('url.parse')

function testcase.parse_full_url()
    -- test that parse url
    local segments = {
        'http://',
        'user:pswd@',
        'host.com',
        ':8080',
        '/p/a/t/h/',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        userinfo = 'user:pswd',
        user = 'user',
        password = 'pswd',
        host = 'host.com:8080',
        hostname = 'host.com',
        port = '8080',
        path = '/p/a/t/h/',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_without_password()
    -- test that parse url without userinfo
    local segments = {
        'http://',
        'user@',
        'host.com',
        ':8080',
        '/p/a/t/h/',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        userinfo = 'user',
        user = 'user',
        host = 'host.com:8080',
        hostname = 'host.com',
        port = '8080',
        path = '/p/a/t/h/',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_without_userinfo()
    -- test that parse url without userinfo
    local segments = {
        'http://',
        'host.com',
        ':8080',
        '/p/a/t/h/',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'host.com:8080',
        hostname = 'host.com',
        port = '8080',
        path = '/p/a/t/h/',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_without_userinfo_hostname()
    -- test that parse url without userinfo
    local segments = {
        'http://',
        ':8080',
        '/p/a/t/h/',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = ':8080',
        hostname = '',
        port = '8080',
        path = '/p/a/t/h/',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_without_userinfo_port()
    -- test that parse url without userinfo and port
    local segments = {
        'http://',
        'host.com',
        '/p/a/t/h/',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'host.com',
        hostname = 'host.com',
        path = '/p/a/t/h/',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_without_userinfo_port_pathname()
    -- test that parse url without userinfo, port and pathname
    local segments = {
        'http://',
        'host.com',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'host.com',
        hostname = 'host.com',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_without_userinfo_port_pathname_query()
    -- test that parse url without userinfo, port, pathname and query
    local segments = {
        'http://',
        'host.com',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'host.com',
        hostname = 'host.com',
        fragment = 'hash',
    })
end

function testcase.parse_without_userinfo_port_pathname_query_fragment()
    -- test that parse url without userinfo, port, pathname, query and fragment
    local segments = {
        'http://',
        'host.com',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'host.com',
        hostname = 'host.com',
    })
end

function testcase.parse_without_authority()
    -- test that parse file scheme
    local segments = {
        'file://',
        '/p/a/t/h/',
        '?q1=v1-1&q1=v1-1&q2=v2',
        '#hash',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'file',
        path = '/p/a/t/h/',
        query = '?q1=v1-1&q1=v1-1&q2=v2',
        fragment = 'hash',
    })
end

function testcase.parse_query()
    -- test that parse query
    local s = '?q1=v1-1&q1=v1-1&q2=v2'
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = s,
    })

    -- test that parse query params
    s = '?q1=v1-1&q1=v1-1&q2=v2&q3=&q4='
    u, cur, err = parse(s, true)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = s,
        queryParams = {
            q1 = 'v1-1',
            q2 = 'v2',
            q3 = '',
            q4 = '',
        },
    })

    -- test that parse query params as array
    s = '?q1=v1-1&q1=v1-2&q2=v2&=v3&q3&q3=v4&=v5&q2='
    u, cur, err = parse(s, true, nil, true)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = s,
        queryParams = {
            'v3',
            'v5',
            q1 = {
                'v1-1',
                'v1-2',
            },
            q2 = {
                'v2',
                '',
            },
            q3 = {
                '',
                'v4',
            },
        },
    })
end

