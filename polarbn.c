#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "bignum.h"

#define MYNAME		"polarbn"
#define MYVERSION	MYNAME " library for " LUA_VERSION " / August 2014 / "\
"based on PolarSSL 1.3.7"
#define MYTYPE		MYNAME " bignumber"

#ifndef BN_is_negative
#define BN_is_negative(a) ((a)->s != 1)
#endif

static const char * error_msg(int e)
{
    switch(e) {
        case 1: return "memory allocation failed";
        case POLARSSL_ERR_MPI_BAD_INPUT_DATA: return "bad input data";
        case POLARSSL_ERR_MPI_BUFFER_TOO_SMALL: return "buffer too small";
        case POLARSSL_ERR_MPI_DIVISION_BY_ZERO: return "division by zero";
        case POLARSSL_ERR_MPI_FILE_IO_ERROR: return "file io error";
        case POLARSSL_ERR_MPI_INVALID_CHARACTER: return "invalid character";
        case POLARSSL_ERR_MPI_NEGATIVE_VALUE: return "negative value";
        case POLARSSL_ERR_MPI_NOT_ACCEPTABLE: return "not acceptable";
        default: return "internal error";
    }
}

static void mpi_check(lua_State *L, int e) {
    if (e != 0) {
        luaL_error(L, error_msg(e));
    }
}


static mpi *Bnew(lua_State *L)
{
    mpi *x = (mpi*)lua_newuserdata(L, sizeof(mpi));
    mpi_init(x);
    luaL_setmetatable(L, MYTYPE);
    return x;
}

static const mpi *Bget(lua_State *L, int i)
{
    switch (lua_type(L,i))
    {
        case LUA_TNUMBER:
        {
            const lua_Integer n = luaL_checkinteger(L, i);
            mpi *x=Bnew(L);
            mpi_check(L, mpi_lset(x, n));
            lua_replace(L,i);
            return x;
        }

        case LUA_TSTRING:
        {
            mpi *x=Bnew(L);
            const char *s = lua_tostring(L,i);
            int radix = 10;
            if(s[0]=='X' || s[0]=='x') {
                radix = 16;
                s++;
            }
            mpi_check(L, mpi_read_string(x, 16, s+1));
            lua_replace(L,i);
            return x;
        }

        default:
            return luaL_checkudata(L,i,MYTYPE);
    }
}

static int Bbits(lua_State *L)
{
    const mpi *a=Bget(L,1);
    lua_pushinteger(L, mpi_msb(a));
    return 1;
}

static int aux_tostring(lua_State *L, int radix)
{
    const mpi *a=Bget(L,1);
    size_t buffersize = 0;

    /* determine buffer size needed */
    mpi_write_string(a, radix, NULL, &buffersize);

    /* allocate and fill buffer */
    char *s = alloca(buffersize);
    mpi_check(L, mpi_write_string(a, radix, s, &buffersize));

    lua_pushlstring(L,s,buffersize-1);
    return 1;
}


static int Btostring(lua_State *L)
{
    return aux_tostring(L,10);
}

static int Btohex(lua_State *L)
{
    return aux_tostring(L,16);
}

static int Btotext(lua_State *L)
{
    const mpi *a = Bget(L,1);
    const int n = mpi_size(a);
    unsigned char *s = alloca(n);
    mpi_check(L, mpi_write_binary(a, s, n));
    lua_pushlstring(L, (const char *) s, n);
    return 1;
}

static int Btonumber(lua_State *L)
{
    Btostring(L);
    lua_pushnumber(L, lua_tonumber(L, -1));
    return 1;
}

static int Biszero(lua_State *L)
{
    const mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_cmp_int(a,0)==0);
    return 1;
}

static int Bisone(lua_State *L)
{
    const mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_cmp_int(a,1)==0);
    return 1;
}

static int Bisodd(lua_State *L)
{
    const mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_get_bit(a,0));
    return 1;
}

static int Bisneg(lua_State *L)
{
    const mpi *a=Bget(L,1);
    lua_pushboolean(L, a->s == -1);
    return 1;
}

static int Bnumber(lua_State *L)
{
    Bget(L,1);
    lua_settop(L,1);
    return 1;
}

static int Btext(lua_State *L)
{
    size_t l;
    const char *s=luaL_checklstring(L,1,&l);
    mpi *a=Bnew(L);
    mpi_check(L, mpi_read_binary(a, (unsigned char *) s, l));
    return 1;
}

static int Bcompare(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    lua_pushinteger(L,mpi_cmp_mpi(a,b));
    return 1;
}

static int Beq(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    lua_pushboolean(L,mpi_cmp_mpi(a,b)==0);
    return 1;
}

static int Blt(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    lua_pushboolean(L, mpi_cmp_mpi(a,b)<0);
    return 1;
}

static int Bneg(lua_State *L)
{
    const mpi *b=Bget(L,1);
    mpi *a=Bnew(L);
    mpi_check(L, mpi_copy(a,b));
    a->s = -b->s;
    return 1;
}

static int Babs(lua_State *L)
{
    const mpi *b=Bget(L,1);
    if (b->s != 1) {
        mpi *a=Bnew(L);
        mpi_check(L, mpi_copy(a,b));
        a->s = 1;
    } else {
        lua_settop(L,1);
    }
    return 1;
}

static int Badd(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_check(L, mpi_add_mpi(c,a,b));
    return 1;
}

static int Bsub(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_check(L, mpi_sub_mpi(c,a,b));
    return 1;
}

static int Bmul(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_check(L, mpi_mul_mpi(c,a,b));
    return 1;
}

static int Bdiv(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *q=Bnew(L);
    mpi_check(L, mpi_div_mpi(q,NULL,a,b));
    return 1;
}

static int Bmod(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *r=Bnew(L);
    mpi_check(L, mpi_div_mpi(NULL,r,a,b));
    return 1;
}

/* Remove references to rmod, because there is no comparable PolarSSL function */

static int Bdivmod(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *q=Bnew(L);
    mpi *r=Bnew(L);
    mpi_check(L, mpi_div_mpi(q,r,a,b));
    return 2;
}

static int Bgcd(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_check(L, mpi_gcd(c,a,b));
    return 1;
}

/* Remove references to pow.  PolarSSL only has powmod */

/* Remove addmmod, submod and mulmod.  They are unnecessary */

static int Bpowmod(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *b=Bget(L,2);
    const mpi *m=Bget(L,3);
    mpi *c=Bnew(L);
    mpi_check(L, mpi_exp_mod(c,a,b,m,NULL));
    return 1;
}

/* Sqrmod and sqrtmod don't exist */

static int Binvmod(lua_State *L)
{
    const mpi *a=Bget(L,1);
    const mpi *m=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_check(L, mpi_inv_mod(c,a,m));
    return 1;
}

/* TODO: Add the random methods later.  First test the basic bignum stuff */
/*
static int Brandom(lua_State *L)
{

}

static int Baprime(lua_State *L)
{

}

static Bisprime(lua_State *L)
{

}
*/

static int Bgc(lua_State *L)
{
    mpi *a = luaL_checkudata(L,1,MYTYPE);
    mpi_free(a);
    return 0;
}


static const luaL_Reg R[] =
{
	{ "__add",	Badd	},		/** __add(x,y) */
	{ "__div",	Bdiv	},		/** __div(x,y) */
	{ "__eq",	Beq	},		/** __eq(x,y) */
	{ "__gc",	Bgc	},
	{ "__lt",	Blt	},		/** __lt(x,y) */
	{ "__mod",	Bmod	},		/** __mod(x,y) */
	{ "__mul",	Bmul	},		/** __mul(x,y) */
	{ "__sub",	Bsub	},		/** __sub(x,y) */
	{ "__tostring",	Btostring},		/** __tostring(x) */
	{ "__unm",	Bneg	},		/** __unm(x) */
	{ "abs",	Babs	},
	{ "add",	Badd	},
	{ "bits",	Bbits	},
	{ "compare",	Bcompare},
	{ "div",	Bdiv	},
    { "divmod", Bdivmod },
	{ "gcd",	Bgcd	},
	{ "invmod",	Binvmod	},
	{ "isneg",	Bisneg	},
	{ "isodd",	Bisodd	},
	{ "isone",	Bisone	},
	{ "iszero",	Biszero	},
	{ "mod",	Bmod	},
	{ "mul",	Bmul	},
	{ "neg",	Bneg	},
	{ "number",	Bnumber	},
	{ "powmod",	Bpowmod	},
	{ "sub",	Bsub	},
	{ "text",	Btext	},
	{ "tohex",	Btohex	},
	{ "totext",	Btotext	},
	{ "tonumber",	Btonumber},
	{ "tostring",	Btostring},
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_polarbn(lua_State *L)
{
    luaL_newmetatable(L,MYTYPE);
    luaL_setfuncs(L,R,0);
    lua_pushliteral(L,"version");			/** version */
    lua_pushliteral(L,MYVERSION);
    lua_settable(L,-3);
    lua_pushliteral(L,"__index");
    lua_pushvalue(L,-2);
    lua_settable(L,-3);
    return 1;
}
