// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#pragma once

#include "lobject.h"

// GCObject size has to be at least 16 bytes, so a minimum of 2 floats is always reserved
#define sizevector(len) (offsetof(LVector, data) + ((len) < 2 ? 2 : (len)) * sizeof(float))

LUAI_FUNC LVector* luaV_newvector(lua_State* L, int n);
LUAI_FUNC void luaV_freevector(lua_State* L, LVector* v, struct lua_Page* page);
