rockspec_format = "3.0"
package = "url"
version = "1.3.0-1"
source = {
    url = "git+https://github.com/mah0x211/lua-url.git",
    tag = "v1.3.0",
}
description = {
    summary = "url functions",
    homepage = "https://github.com/mah0x211/lua-url",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga"
}
dependencies = {
    "lua >= 5.1",
}
build = {
    type = "builtin",
    modules = {
        url = "url.lua",
        ["url.codec"] = {
            sources = { "src/codec.c" },
        },
        ["url.parse"] = {
            incdirs = { "deps/lauxhlib" },
            sources = { "src/parse.c" },
        },
    }
}
