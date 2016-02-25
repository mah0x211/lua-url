package = "url"
version = "1.1.0-1"
source = {
    url = "git://github.com/mah0x211/lua-url.git",
    tag = "v1.1.0"
}
description = {
    summary = "url functions",
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
        ['url.codec'] = "codec.c"
    }
}

