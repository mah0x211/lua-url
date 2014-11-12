local url = require('url');

local uri = 'http://user:pass@host.com:8080/p/a/t/h/?query1=string1&query2=string2#hash';
local res = ifNil( url.parse( uri ) );

for k, v in pairs({
    host = 'host.com',
    port = '8080',
    absolutePath = false,
    query = 'query1=string1&query2=string2',
    userinfo = 'user:pass',
    fragment = 'hash',
    scheme = 'http'
}) do
    ifNotEqual( v, res[k] );
end


res = ifNil( url.parseQuery( res.query ) );
for k, v in pairs({
    query1 = 'string1',
    query2 = 'string2'
}) do
    ifNotEqual( v, res[k] );
end


res = ifNil( url.parse( uri, true ) );
for k, v in pairs({
    host = 'host.com',
    port = '8080',
    absolutePath = false,
    userinfo = 'user:pass',
    fragment = 'hash',
    scheme = 'http'
}) do
    ifNotEqual( v, res[k] );
end

res = res.query;
for k, v in pairs({
    query1 = 'string1',
    query2 = 'string2'
}) do
    ifNotEqual( v, res[k] );
end

