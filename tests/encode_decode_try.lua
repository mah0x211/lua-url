local url = require('url');

local str =[[ !"#$%&\'()*+,-./\n
0123456789
:;<=>?@
ABCDEFGHIJKLMNOPQRSTUVWXYZ
[\]^_`
abcdefghijklmnopqrstuvwxyz
{|}~]];
local ec,dc;

ec = ifNil( url.encodeURI( str ) );
ifNotEqual( str, url.decodeURI( ec ) );
ifNotEqual( str, url.decode2396( ec ) );
ifNotEqual( str, url.decode3986( ec ) );


ec = ifNil( url.encode2396( str ) );
ifEqual( str, url.decodeURI( ec ) );
ifNotEqual( str, url.decode2396( ec ) );
ifNotEqual( str, url.decode3986( ec ) );


ec = ifNil( url.encode3986( str ) );
ifEqual( str, url.decodeURI( ec ) );
ifEqual( str, url.decode2396( ec ) );
ifNotEqual( str, url.decode3986( ec ) );
