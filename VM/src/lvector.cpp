// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#include "lvector.h"

#include "lgc.h"
#include "lmem.h"
#include "ldo.h"

#include <string.h>

LVector* luaV_newvector(lua_State* L, int n)
{
    if (n < 2)
        luaG_runerror(L, "vector must have at least 2 components");
    if (n > 1024)
        luaG_runerror(L, "vector dimension too large");

    LVector* v = luaM_newgco(L, LVector, sizevector(n), L->activememcat);
    luaC_init(L, v, LUA_TVECTOR_N);
    v->len = unsigned(n);
    memset(v->data, 0, v->len * sizeof(float));
    return v;
}

void luaV_freevector(lua_State* L, LVector* v, lua_Page* page)
{
    luaM_freegco(L, v, sizevector(v->len), v->memcat, page);
}
