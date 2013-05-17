url = require('../url');

local str =[[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]];
local ec,dc;

print( 'str {' );
print( str );
print( '}\n' );

ec = url.encodeURI( str );
dc = url.decodeURI( ec );
print( 'encodeURI: ', ec, '\n',
    '-> decodeURI: ', dc == str, '\n',
    '-> decode2396: ', url.decode2396( ec ) == str, '\n',
    '-> decode3986: ', url.decode3986( ec ) == str, '\n',
    ''
);

ec = url.encode2396( str );
dc = url.decode2396( ec );
print( 'encode2396', ec, '\n',
    '-> decode2396:', dc == str, '\n',
    '-> decodeURI: ', url.decodeURI( ec ) == str, '\n',
    '-> decode3986: ', url.decode3986( ec ) == str, '\n',
    ''
);


ec = url.encode3986( str );
dc = url.decode3986( ec );
print( 'encode3986:', ec, '\n',
    '-> decode3986: ', dc == str, '\n',
    '-> decodeURI: ', url.decodeURI( ec ) == str, '\n',
    '-> decode2396: ', url.decode2396( ec ) == str, '\n',
    ''
);


local function showResult( tbl, err )
    if err then
        print( err );
    else
        local k,v = next(tbl);
        print( '{' );
        while k do
            if k == 'query' then
                local qk,qv = next(v);
                print( '', k .. ' {' );
                while qk do
                    print( '', '', qk .. ':', qv );
                    qk,qv = next( v, qk );
                end
                print( '', '}' );
            else
                print( '', k .. ':', v );
            end
            k,v = next( tbl, k );
            
        end
        print( '}' );
    end
end

local uri = 'http://user:pass@host.com:8080/p/a/t/h/?query=string#hash';
print( 'parse: ', uri );
showResult( url.parse( url.encodeURI( uri ) ) );


uri = '/p/a/t/h/?query=string#hash';
print( 'parse: ', uri );
showResult( url.parse( url.encodeURI( uri ) ) );

