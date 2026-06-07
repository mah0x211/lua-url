rockspec_format = "3.0"
package = "url"
version = "scm-1"
source = {
    url = "git+https://github.com/mah0x211/lua-url.git",
}
description = {
    summary = "url functions",
    homepage = "https://github.com/mah0x211/lua-url",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga",
}
dependencies = {
    "lua >= 5.1",
    "lauxhlib >= 0.3.1",
}
build_dependencies = {
    "luarocks-build-hooks >= 0.8.0",
}
build = {
    type = "hooks",
    before_build = "$(extra-vars)",
    extra_variables = {
        CFLAGS = "-Wall -Wno-trigraphs -Wmissing-field-initializers -Wreturn-type -Wmissing-braces -Wparentheses -Wno-switch -Wunused-function -Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value -Wuninitialized -Wunknown-pragmas -Wshadow -Wsign-compare",
    },
    conditional_variables = {
        URL_COVERAGE = {
            CFLAGS = "--coverage",
            LIBFLAG = "--coverage",
        },
    },
    modules = {
        ["url"] = "url.lua",
        ["url.codec"] = {
            sources = "src/codec.c",
            incdirs = {
                "$(DEP_LAUXHLIB_INCDIR)",
            },
        },
        ["url.parse"] = {
            sources = "src/parse.c",
            incdirs = {
                "$(DEP_LAUXHLIB_INCDIR)",
            },
        },
    },
}
