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

function testcase.parse_scheme()
    -- test that parse scheme with host
    local s = 'http://localhost'
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'localhost',
        hostname = 'localhost',
    })

    -- test that parse scheme with ipv6 host
    s = 'https://[2001:db8:85a3:8d3:1319:8a2e:370:7348]:80'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'https',
        host = '[2001:db8:85a3:8d3:1319:8a2e:370:7348]:80',
        hostname = '[2001:db8:85a3:8d3:1319:8a2e:370:7348]',
        port = '80',
    })

    -- test that parse scheme with port
    s = 'https://:80'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'https',
        host = ':80',
        hostname = '',
        port = '80',
    })

    -- test that parse scheme with pathname
    s = 'https://./foo/bar'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'https',
        path = './foo/bar',
    })

    -- test that return an error if invalid scheme format
    s = 'http:/localhost'
    u, cur, err = parse(s)
    assert.equal(cur, 4)
    assert.equal(err, ':')
    assert.equal(u, {})
end

function testcase.parse_host()
    -- test that parse host
    local s = 'http://example.com'
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'example.com',
        hostname = 'example.com',
    })

    -- test that parse host that contains percent-encoded string
    s = 'http://ex%45mple.com'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'ex%45mple.com',
        hostname = 'ex%45mple.com',
    })

    -- test that parse ipv4 host
    s = 'http://127.0.0.1'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = '127.0.0.1',
        hostname = '127.0.0.1',
    })

    -- test that parse host with empty-userinfo
    s = 'http://@example.com'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'example.com',
        hostname = 'example.com',
    })

    -- test that return an error if userinfo delimiter is declared more than once
    s = 'http://@@example.com'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://@')
    assert.equal(err, '@')
    assert.equal(u, {
        scheme = 'http',
    })

    -- test that return an error if found invalid character
    s = 'http://example.com|'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://example.com')
    assert.equal(err, '|')
    assert.equal(u, {
        scheme = 'http',
        host = 'example.com',
        hostname = 'example.com',
    })

    -- test that return an error if invalid host format
    s = 'http://$localhost'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://')
    assert.equal(err, '$')
    assert.equal(u, {
        scheme = 'http',
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = 'http://exa%2mple.com/foo/bar'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://exa')
    assert.equal(err, '%')
    assert.equal(u, {
        scheme = 'http',
    })

    -- test that return an error if no host after userinfo
    s = 'http://user:pswd@/foo/bar'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://user:pswd@')
    assert.equal(err, '/')
    assert.equal(u, {
        scheme = 'http',
        password = 'pswd',
        user = 'user',
        userinfo = 'user:pswd',
    })

    -- test that return an error if userinfo is declared more than once
    s = 'http://user:pswd@example.com@example.com/foo/bar'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://user:pswd@example.com')
    assert.equal(err, '@')
    assert.equal(u, {
        scheme = 'http',
        password = 'pswd',
        user = 'user',
        userinfo = 'user:pswd',
    })
end

function testcase.parse_port()
    -- test that parse port
    local s = 'http://example.com:80'
    local u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = 'example.com:80',
        hostname = 'example.com',
        port = '80',
    })

    -- test that parse port without host
    s = 'http://:80'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = ':80',
        hostname = '',
        port = '80',
    })

    -- test that parse port with query
    s = 'http://:80?foo=bar'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = ':80',
        hostname = '',
        port = '80',
        query = '?foo=bar',
    })

    -- test that parse port with fragment
    s = 'http://:65535#foo=bar'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        scheme = 'http',
        host = ':65535',
        hostname = '',
        port = '65535',
        fragment = 'foo=bar',
    })

    -- test that can be omit the port after ':'
    for _, v in ipairs({
        {
            s = 'http://example.com:?foo=bar',
            exp = {
                scheme = 'http',
                host = 'example.com',
                hostname = 'example.com',
                query = '?foo=bar',
            },
        },
        {
            s = 'http://:?foo=bar',
            exp = {
                scheme = 'http',
                query = '?foo=bar',
            },
        },
    }) do
        u, cur, err = parse(v.s)
        assert.equal(cur, #v.s)
        assert.is_nil(err)
        assert.equal(u, v.exp)
    end

    -- test that return an error if port greater than 65535
    s = 'http://example.com:65536/foo/bar'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://example.com:6553')
    assert.equal(err, '6')
    assert.equal(u, {
        scheme = 'http',
    })

    -- test that return an error if port contains non-digit character
    s = 'http://user:pswd@example:80pswd:?foo'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), 'http://user:pswd@example:80')
    assert.equal(err, 'p')
    assert.equal(u, {
        scheme = 'http',
        userinfo = 'user:pswd',
        user = 'user',
        password = 'pswd',
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

    -- test that parse path with fragment
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
    assert.equal(string.sub(s, 1, cur), '/foo/bar')
    assert.equal(err, '|')
    assert.equal(u, {
        path = '/foo/bar',
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '/foo/bar/baz%2qux'
    u, cur, err = parse(s, true)
    assert.equal(string.sub(s, 1, cur), '/foo/bar/baz')
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

    -- test that parse query string with fragment
    s = '?q1=v1-1#&q2=v2'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        query = '?q1=v1-1',
        fragment = '&q2=v2',
    })

    -- test that return an error if contains a invalid character
    s = '?q1=v1-1|#&q2=v2'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), '?q1=v1-1')
    assert.equal(err, '|')
    assert.equal(u, {
        query = '?q1=v1-1',
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '?q1=v1-1&%2q2=v2'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), '?q1=v1-1&')
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
    assert.equal(string.sub(s, 1, cur), '?q1=v1-1&q2=v2')
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
    assert.equal(string.sub(s, 1, cur), '?q1=v1-1&q2=')
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
    assert.equal(string.sub(s, 1, cur), '?q1=v1-1&q2=v2')
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
    assert.equal(string.sub(s, 1, cur), '?q1=v1-1&q2=v2-1&q2=')
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
    s = '#'
    u, cur, err = parse(s)
    assert.equal(cur, #s)
    assert.is_nil(err)
    assert.equal(u, {
        fragment = '',
    })

    -- test that return an error if contains a invalid percent-encoded string
    s = '#foo%1'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), '#foo')
    assert.equal(err, '%')
    assert.equal(u, {})

    -- test that return an error if contains a invalid character
    s = '#fo|o'
    u, cur, err = parse(s)
    assert.equal(string.sub(s, 1, cur), '#fo')
    assert.equal(err, '|')
    assert.equal(u, {
        fragment = 'fo',
    })
end

