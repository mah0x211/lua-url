rockspec_format = "3.0"
package = "url"
version = "scm-1"
source = {
    url = "git+https://github.com/mah0x211/lua-url.git"
}
description = {
    summary = "url functions",
    homepage = "https://github.com/mah0x211/lua-url",
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1",
}
build = {
    type = "builtin",
    modules = {
        url = "url.lua",
        ["url.codec"] = "src/codec.c",
        ["url.parse"] = "src/parse.c"
    }
}
