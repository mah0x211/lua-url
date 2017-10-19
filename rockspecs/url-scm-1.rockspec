package = "url"
version = "scm-1"
source = {
    url = "git://github.com/mah0x211/lua-url.git"
}
description = {
    summary = "url functions",
    detailed = [[]],
    homepage = "https://github.com/mah0x211/lua-url",
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1",
    "uriparser >= 0.8.4"
}
build = {
    type = "builtin",
    modules = {
        url = "url.lua",
        ["url.codec"] = "src/codec.c"
    }
}

