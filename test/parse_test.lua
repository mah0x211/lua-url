local concat = table.concat
local testcase = require('testcase')
local parse = require('url.parse')

function testcase.parse_empty_url()
    -- test that parse empty-url
    local u, cur, err = parse('')
    assert.equal(cur, 0)
    assert.is_nil(err)
    assert.equal(u, {})
end

function testcase.parse_illegal_url()
    -- test that parse empty-url
    local u, cur, err = parse(string.char(0))
    assert.equal(cur, 0)
    assert.is_nil(err)
    assert.equal(u, {})
end

function testcase.parse_full_url()
    -- test that parse full url
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

function testcase.parse_without_scheme()
    -- test that parse url without scheme, userinfo, port, pathname, query and fragment
    local segments = {
        'host.com:8080',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        path = 'host.com:8080',
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

function testcase.parse_pathname()
    -- test that parse path
    local s = '/foo/bar/baz%20qux'
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        path = '/foo/bar/baz%20qux',
    })

    -- test that parse path with frament
    s = '/foo/bar/baz%20qux#fragment-value'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        path = '/foo/bar/baz%20qux',
        fragment = 'fragment-value',
    })

    -- test that return an error if contains a invalid character
    s = '/foo/bar|baz%20qux'
    u, cur, err = parse(s, true)
    assert.equal(cur, 8)
    assert.equal(err, '|')
    assert.equal(u, {
        path = '/foo/bar',
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '/foo/bar/baz%2qux'
    u, cur, err = parse(s, true)
    assert.equal(cur, 12)
    assert.equal(err, '%')
    assert.equal(u, {})
end

function testcase.parse_query_string()
    -- test that parse query
    local s = '?q1=v1-1%20&q1=v1-1&q2=v2'
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = s,
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '?q1=v1-1%2&q2=v2'
    u, cur, err = parse(s)
    assert.equal(cur, 8)
    assert.equal(err, '%')
    assert.equal(u, {})
end

function testcase.parse_query_params()
    -- test that parse query
    local s = '?q1=v1-1&q1=v1-1%20&q2=v2'
    local u, cur, err = parse(s, true)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = s,
        queryParams = {
            q1 = 'v1-1%20',
            q2 = 'v2',
        },
    })

    -- test that return an error if contains a invalid character
    s = '?q1=v1-1&q2=v2|'
    u, cur, err = parse(s, true)
    assert.equal(cur, 14)
    assert.equal(err, '|')
    assert.equal(u, {
        query = '?q1=v1-1&q2=v2',
        queryParams = {
            q1 = 'v1-1',
            q2 = 'v2',
        },
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '?q1=v1-1&q2=%2v2'
    u, cur, err = parse(s, true)
    assert.equal(cur, 12)
    assert.equal(err, '%')
    assert.equal(u, {
        query = '?q1=v1-1&q2=',
        queryParams = {
            q1 = 'v1-1',
            q2 = '',
        },
    })
end

function testcase.parse_query_params_as_array()
    -- test that parse query
    local s = '?q1=v1-1&q1=v1-1%20&q2=v2'
    local u, cur, err = parse(s, true, nil, true)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = s,
        queryParams = {
            q1 = {
                'v1-1',
                'v1-1%20',
            },
            q2 = {
                'v2',
            },
        },
    })

    -- test that return an error if contains a invalid character
    s = '?q1=v1-1&q2=v2|'
    u, cur, err = parse(s, true, nil, true)
    assert.equal(cur, 14)
    assert.equal(err, '|')
    assert.equal(u, {
        query = '?q1=v1-1&q2=v2',
        queryParams = {
            q1 = {
                'v1-1',
            },
            q2 = {
                'v2',
            },
        },
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '?q1=v1-1&q2=v2-1&q2=%2v2-2'
    u, cur, err = parse(s, true, nil, true)
    assert.equal(cur, 20)
    assert.equal(err, '%')
    assert.equal(u, {
        query = '?q1=v1-1&q2=v2-1&q2=',
        queryParams = {
            q1 = {
                'v1-1',
            },
            q2 = {
                'v2-1',
                '',
            },
        },
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

function testcase.parse_fragment()
    -- test that parse fragment
    local segments = {
        '#foo?bar#baz%20/qux',
    }
    local s = concat(segments)
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        fragment = 'foo?bar#baz%20/qux',
    })

    -- test that parse empty fragment
    u, cur, err = parse('#')
    assert.equal(cur, 1)
    assert.is_nil(err)
    assert.equal(u, {
        fragment = '',
    })

    -- test that return an error if contains a invalid percent-encoded string
    u, cur, err = parse('#foo%1')
    assert.equal(cur, 4)
    assert.equal(err, '%')
    assert.equal(u, {})

    -- test that return an error if contains a invalid character
    u, cur, err = parse('#fo|o')
    assert.equal(cur, 3)
    assert.equal(err, '|')
    assert.equal(u, {
        fragment = 'fo',
    })
end

