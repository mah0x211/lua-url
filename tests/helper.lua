require('process').chdir( (arg[0]):match( '^(.+[/])[^/]+%.lua$' ) );

_G.inspect = require('util').inspect;

local function printUsage( method, ... )
    local args = {};
    local _, v, t;
    
    for _, v in ipairs({...}) do
        t = type(v);
        if t == 'table' then
            args[#args+1] = inspect( v );
        elseif t == 'string' then
            args[#args+1] = ('%q'):format( v );
        elseif t == 'number' then
            args[#args+1] = ('%d'):format( v );
        else
            args[#args+1] = ('%s'):format( tostring(v) );
        end
    end
    
    print( '\n' );
    print( ('%s( %s )'):format( method, table.concat( args, ', ' ) ) );
end
_G.printUsage = printUsage;

return true;

