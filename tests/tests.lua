require('process').chdir( (arg[0]):match( '^(.+[/])[^/]+%.lua$' ) );
require('./helper');
local cl = require('ansicolors');
local gettimeofday = require('process').gettimeofday;

local printOrg = print;
local function hookPrint()
    _G.print = function() end;
end
local function unhookPrint()
    _G.print = printOrg;
end

local function noop()
end

local PRINT_HOOK = hookPrint;
if _G.arg[1] then
    PRINT_HOOK = noop;
end


local CWD = require('process').getcwd();
local SEACHPATH = CWD .. '/?.lua;' .. package.path;
local EXTENSION = '.lua';
local TESTS = {
    'encode_decode',
    'parse',
};

local fmtSuccess, fmtFailure;
-- setup output format
do
    local fmt = '%%{yellow}TEST %-%ds cost %20f sec %%{reset}| ';
    local len = 0;
    for _, v in ipairs( TESTS ) do
        if ( #v + #EXTENSION ) > len then
            len = #v + 4;
        end
    end

    fmt = '%%{yellow}TEST %-' .. len .. 's %%{cyan}%12f sec %%{reset} ';
    fmtSuccess = fmt .. '%%{green}SUCCESS';
    fmtFailure = fmt .. '%%{red}FAILURE - %s';
end

local _, v, file, ok, err, sec, cost, costAll;
local success = {};
local failure = {};

-- set search path
package.path = SEACHPATH;

print( cl( ('%%{magenta}START %d TESTS'):format( #TESTS ) ) );
print( '====================================================================' );

costAll = 0;
sec = gettimeofday();
for _, v in ipairs( TESTS ) do
    -- append file extension
    file = ('%s%s'):format( v, EXTENSION );
    
    -- run test
    PRINT_HOOK();
    cost = gettimeofday();
    ok, err = pcall( require, './' .. v );
    unhookPrint();
    cost = gettimeofday() - cost;
    costAll = costAll + cost;
    
    if ok then
        success[#success+1] = {
            file = file,
            cost = cost
        };
        print( cl( fmtSuccess:format( file, cost ) ) );
    else
        -- remove cwd path
        err = err:gsub( '/+', '/' ):sub( #CWD + 2 );
        failure[#failure+1] = {
            file = file,
            err = err
        };
        print( cl( fmtFailure:format( file, cost, err:match('^[^\n]+') ) ) );
    end
    if PRINT_HOOK ~= hookPrint then
        print( '--------------------------------------------------------------------' );
    end
end
sec = gettimeofday() - sec;

print( '====================================================================' );
print( cl( ('%%{magenta}TIME: %f sec, TOTAL COST: %f sec\n'):format( sec, costAll ) ) );
print( cl( ('%%{green}SUCCESS: %d'):format( #success ) ) );
print( cl( ('%%{red}FAILURE: %d'):format( #failure ) ) );

for _, v in ipairs( failure ) do
    print( cl( ('  %%{yellow}%s:%%{reset}\n\t%s'):format( v.file, v.err ) ) );
end

-- time rank
if #success > 0 then
    print( '--------------------------------------------------------------------' );
    table.sort( success, function(a,b)
        return a.cost > b.cost;
    end);
    for _, v in ipairs( success ) do
        print( cl( ('%%{green}%-10f sec %%{reset}| %%{yellow}%s'):format( v.cost, v.file ) ) );
    end
end
print( '--------------------------------------------------------------------' );
print( '\n' );


