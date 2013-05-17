url = require('../url');

local str =[[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]];
local ec,dc;

print( 'str -> ', str );

ec = url.encodeURI( str );
dc = url.decodeURI( ec );
print( '\n',
    '----------------------------\n',
    'str -> encodeURI -> decodeURI\n',
    ec, '\n',
    dc == str, '\n',
    '----------------------------\n',
    'encodeURI -> decode2396\n',
    url.decode2396( ec ) == str, '\n',
    '----------------------------\n',
    'encodeURI -> decode3986\n',
    url.decode3986( ec ) == str, '\n',
    ''
);

ec = url.encode2396( str );
dc = url.decode2396( ec );
print( '\n',
    '----------------------------\n',
    'str -> encode2396 -> decode2396\n',
    ec, '\n',
    dc == str, '\n',
    '----------------------------\n',
    'encode2396 -> decodeURI\n',
    url.decodeURI( ec ) == str, '\n',
    '----------------------------\n',
    'encode2396 -> decode3986\n',
    url.decode3986( ec ) == str, '\n',
    ''
);


ec = url.encode3986( str );
dc = url.decode3986( ec );
print( '\n',
    '----------------------------\n',
    'str -> encode3986 -> decode3986\n',
    ec, '\n',
    dc == str, '\n',
    '----------------------------\n',
    'encode3986 -> decodeURI\n',
    url.decodeURI( ec ) == str, '\n',
    '----------------------------\n',
    'encode3986 -> decode2396\n',
    url.decode2396( ec ) == str, '\n',
    ''
);


