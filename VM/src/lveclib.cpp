// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#include "lualib.h"

#include "lcommon.h"
#include "lnumutils.h"
#include "lapi.h"

#include <math.h>
#include <vector>
#include <string.h>
static int vector_create(lua_State* L)
{
    int count = lua_gettop(L);
    if (count < 2)
        luaL_error(L, "vector.create requires at least 2 arguments");

    float* v = (float*)lua_newbuffer(L, count * sizeof(float));
    for (int i = 0; i < count; ++i)
        v[i] = (float)luaL_checknumber(L, i + 1);

    lua_pushvectorn(L, v, count);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_dim(lua_State* L)
{
    int n;
    luaL_checkvectorn(L, 1, &n);
    lua_pushnumber(L, n);
    return 1;
}

static int vector_magnitude(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += (double)v[i] * v[i];

    lua_pushnumber(L, sqrt(sum));
    return 1;
}

static int vector_normalize(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += (double)v[i] * v[i];

    float invSqrt = (float)(1.0 / sqrt(sum));

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    for (int i = 0; i < n; ++i)
        res[i] = v[i] * invSqrt;

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_cross(lua_State* L)
{
    int na, nb;
    const float* a = luaL_checkvectorn(L, 1, &na);
    const float* b = luaL_checkvectorn(L, 2, &nb);

    if (na != 3 || nb != 3)
        luaL_error(L, "vector.cross is only supported for 3D vectors");

    lua_pushvector(L, a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
    return 1;
}

static int vector_dot(lua_State* L)
{
    int na, nb;
    const float* a = luaL_checkvectorn(L, 1, &na);
    const float* b = luaL_checkvectorn(L, 2, &nb);

    if (na != nb)
        luaL_error(L, "vector.dot requires vectors of the same dimension");

    double dot = 0.0;
    for (int i = 0; i < na; ++i)
        dot += (double)a[i] * b[i];

    lua_pushnumber(L, dot);
    return 1;
}

static int vector_angle(lua_State* L)
{
    int na, nb;
    const float* a = luaL_checkvectorn(L, 1, &na);
    const float* b = luaL_checkvectorn(L, 2, &nb);
    const float* axis = luaL_optvector(L, 3, nullptr);

    if (na != 3 || nb != 3)
        luaL_error(L, "vector.angle is only supported for 3D vectors");

    // cross(a, b)
    float cross[] = {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};

    double sinA = sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]);
    double cosA = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    double angle = atan2(sinA, cosA);

    if (axis)
    {
        if (cross[0] * axis[0] + cross[1] * axis[1] + cross[2] * axis[2] < 0.0f)
            angle = -angle;
    }

    lua_pushnumber(L, angle);
    return 1;
}

static int vector_floor(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    for (int i = 0; i < n; ++i)
        res[i] = floorf(v[i]);

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_ceil(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    for (int i = 0; i < n; ++i)
        res[i] = ceilf(v[i]);

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_abs(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    for (int i = 0; i < n; ++i)
        res[i] = fabsf(v[i]);

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_sign(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    for (int i = 0; i < n; ++i)
        res[i] = luaui_signf(v[i]);

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_clamp(lua_State* L)
{
    int nv, nmin, nmax;
    const float* v = luaL_checkvectorn(L, 1, &nv);
    const float* min = luaL_checkvectorn(L, 2, &nmin);
    const float* max = luaL_checkvectorn(L, 3, &nmax);

    if (nv != nmin || nv != nmax)
        luaL_error(L, "vector.clamp requires vectors of the same dimension");

    float* res = (float*)lua_newbuffer(L, nv * sizeof(float));
    for (int i = 0; i < nv; ++i)
    {
        luaL_argcheck(L, min[i] <= max[i], 3, "max component must be greater than or equal to min component");
        res[i] = luaui_clampf(v[i], min[i], max[i]);
    }

    lua_pushvectorn(L, res, nv);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_min(lua_State* L)
{
    int numArgs = lua_gettop(L);
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    memcpy(res, v, n * sizeof(float));

    for (int i = 2; i <= numArgs; i++)
    {
        int nb;
        const float* b = luaL_checkvectorn(L, i, &nb);
        if (nb != n)
            luaL_error(L, "vector.min requires vectors of the same dimension");

        for (int j = 0; j < n; ++j)
        {
            if (b[j] < res[j])
                res[j] = b[j];
        }
    }

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_max(lua_State* L)
{
    int numArgs = lua_gettop(L);
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    float* res = (float*)lua_newbuffer(L, n * sizeof(float));
    memcpy(res, v, n * sizeof(float));

    for (int i = 2; i <= numArgs; i++)
    {
        int nb;
        const float* b = luaL_checkvectorn(L, i, &nb);
        if (nb != n)
            luaL_error(L, "vector.max requires vectors of the same dimension");

        for (int j = 0; j < n; ++j)
        {
            if (b[j] > res[j])
                res[j] = b[j];
        }
    }

    lua_pushvectorn(L, res, n);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static int vector_index(lua_State* L)
{
    int n;
    const float* v = luaL_checkvectorn(L, 1, &n);

    if (lua_isnumber(L, 2))
    {
        int idx = lua_tointeger(L, 2);
        if (idx >= 1 && idx <= n)
        {
            lua_pushnumber(L, v[idx - 1]);
            return 1;
        }
        return 0;
    }

    size_t namelen = 0;
    const char* name = luaL_checklstring(L, 2, &namelen);

    if (namelen == 1)
    {
        int ic = (name[0] | ' ') - 'x';

        // 'w' is before 'x' in ascii, so ic is -1 when indexing with 'w'
        if (ic == -1)
            ic = 3;

        if (unsigned(ic) < 4 && ic < n)
        {
            lua_pushnumber(L, v[ic]);
            return 1;
        }
    }

    luaL_error(L, "attempt to index %dD vector with '%s'", n, name);
}

static int vector_lerp(lua_State* L)
{
    int na, nb;
    const float* a = luaL_checkvectorn(L, 1, &na);
    const float* b = luaL_checkvectorn(L, 2, &nb);
    const float t = static_cast<float>(luaL_checknumber(L, 3));

    if (na != nb)
        luaL_error(L, "vector.lerp requires vectors of the same dimension");

    float* res = (float*)lua_newbuffer(L, na * sizeof(float));
    for (int i = 0; i < na; ++i)
        res[i] = luai_lerpf(a[i], b[i], t);

    lua_pushvectorn(L, res, na);
    lua_remove(L, -2); // remove buffer
    return 1;
}

static const luaL_Reg vectorlib[] = {
    {"create", vector_create},
    {"dim", vector_dim},
    {"magnitude", vector_magnitude},
    {"normalize", vector_normalize},
    {"cross", vector_cross},
    {"dot", vector_dot},
    {"angle", vector_angle},
    {"floor", vector_floor},
    {"ceil", vector_ceil},
    {"abs", vector_abs},
    {"sign", vector_sign},
    {"clamp", vector_clamp},
    {"max", vector_max},
    {"min", vector_min},
    {"lerp", vector_lerp},
    {NULL, NULL},
};

static void createmetatable(lua_State* L)
{
    lua_createtable(L, 0, 1); // create metatable for vectors

    // push dummy vector
    lua_pushvector(L, 0.0f, 0.0f, 0.0f);

    lua_pushvalue(L, -2);
    lua_setmetatable(L, -2); // set vector metatable
    lua_pop(L, 1);           // pop dummy vector

    lua_pushcfunction(L, vector_index, nullptr);
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);
    lua_pop(L, 1); // pop the metatable
}

int luaopen_vector(lua_State* L)
{
    luaL_register(L, LUA_VECLIBNAME, vectorlib);

    lua_pushvector(L, 0.0f, 0.0f, 0.0f);
    lua_setfield(L, -2, "zero");
    lua_pushvector(L, 1.0f, 1.0f, 1.0f);
    lua_setfield(L, -2, "one");

    createmetatable(L);

    return 1;
}
