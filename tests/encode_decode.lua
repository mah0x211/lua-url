local inspect = require('util').inspect;
local url = require('url');

local str =[[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]];
local ec,dc;
print( 'str', str );

ec = url.encodeURI( str );
print( '\nencodeURI', ec );
print( 'decodeURI  ==', assert( str == url.decodeURI( ec ) ) );
print( 'decode2396 ==', assert( str == url.decode2396( ec ) ) );
print( 'decode3986 ==', assert( str == url.decode3986( ec ) ) );


ec = url.encode2396( str );
print( '\nencode2396', ec );
print( 'decodeURI  ~=', assert( str ~= url.decodeURI( ec ) ) );
print( 'decode2396 ==', assert( str == url.decode2396( ec ) ) );
print( 'decode3986 ==', assert( str == url.decode3986( ec ) ) );


ec = url.encode3986( str );
print( '\nencode3986', ec );
print( 'decodeURI  ~=', assert( str ~= url.decodeURI( ec ) ) );
print( 'decode2396 ~=', assert( str ~= url.decode2396( ec ) ) );
print( 'decode3986 ==', assert( str == url.decode3986( ec ) ) );
