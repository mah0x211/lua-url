local inspect = require('util').inspect;
local url = require('../url');

local uri = 'http://user:pass@host.com:8080/p/a/t/h/?query1=string1&query2=string2#hash';
local res,k,v;

print( 'parse: ', uri );
res = url.parse( uri );
print( inspect( res ) );
for k, v in pairs({
    host = 'host.com',
    port = '8080',
    absolutePath = false,
    query = 'query1=string1&query2=string2',
    userinfo = 'user:pass',
    fragment = 'hash',
    scheme = 'http'
}) do
    assert( v == res[k] );
end


print( 'parseQuery: ', res.query );
res = url.parseQuery( res.query );
print( inspect( res ) );
for k, v in pairs({
    query1 = 'string1',
    query2 = 'string2'
}) do
    assert( v == res[k] );
end

print( 'parse full' );
print( inspect( url.parse( uri, true ) ) );

print( inspect( {url.parse( 'hello', true )} ) );
