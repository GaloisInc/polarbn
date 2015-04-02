#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "bignum.h"

/*using namespace std;*/

#define lua_boxpointer(L,u) \
(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))

#define MYNAME		"polarbn"
#define MYVERSION	MYNAME " library for " LUA_VERSION " / August 2014 / "\
"based on PolarSSL 1.3.7"
#define MYTYPE		MYNAME " bignumber"

#ifndef BN_is_negative
#define BN_is_negative(a) ((a)->s != 1)
#endif

static mpi *Bnew(lua_State *L)
{
    mpi **X = (mpi **) lua_newuserdata(L, sizeof(mpi));
    mpi *x = malloc(sizeof(mpi));
    *X = x;
    mpi_init(x);
    luaL_setmetatable(L, MYTYPE);
    return x;
}

static mpi *Bget(lua_State *L, int i)
{
    switch (lua_type(L,i))
    {
        case LUA_TNUMBER:
        case LUA_TSTRING:
        {
            mpi *x=Bnew(L);
            const char *s=lua_tostring(L,i);
            if(s[0]=='X' || s[0]=='x') mpi_read_string(x, 16, s+1); else mpi_read_string(x,10,s);
            lua_replace(L,i);
            return x;
        }
        default:
            return *((mpi**)luaL_checkudata(L,i,MYTYPE));
    }
    return NULL;
}

static int Bbits(lua_State *L)
{
    mpi *a=Bget(L,1);
    lua_pushinteger(L, mpi_msb(a));
    return 1;
}

static int Btostring(lua_State *L)
{
    mpi *a=Bget(L,1);
    int n = mpi_msb(a);
    size_t numChars = 3 + n/2;
    char *s = (char *) malloc(numChars); /*for radix 10, we are safe with one char for every 3 bits with one extra for the terminating 0*/
    mpi_write_string(a, 10, s, &numChars);
    lua_pushstring(L,s);
    free(s);
    return 1;
}

static int Btohex(lua_State *L)
{
    mpi *a=Bget(L,1);
    int n = mpi_msb(a);
    size_t numChars = 3 + n/4;
    char *s = (char *) malloc(numChars); /*for radix 16, we are safe with one char for every 4 bits with one extra for the terminating 0*/
    mpi_write_string(a, 16, s, &numChars);
    lua_pushstring(L,s);
    free(s);
    return 1;
}

static int Btotext(lua_State *L)
{
    mpi *a = Bget(L,1);
    int n = mpi_size(a);
    unsigned char *s = (unsigned char *) malloc(n);
    if (s == NULL) return 0;
    mpi_write_binary(a, s, n);
    lua_pushlstring(L, (const char *) s, n);
    free(s);
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
    mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_cmp_int(a,0)==0);
    return 1;
}

static int Bisone(lua_State *L)
{
    mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_cmp_int(a,1)==0);
    return 1;
}

static int Bisodd(lua_State *L)
{
    mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_get_bit(a,0));
    return 1;
}

static int Bisneg(lua_State *L)
{
    mpi *a=Bget(L,1);
    lua_pushboolean(L, mpi_cmp_int(a,0)<0);
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
    mpi_read_binary(a, (unsigned char *) s, l);
    return 1;
}

static int Bcompare(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    lua_pushinteger(L,mpi_cmp_mpi(a,b));
    return 1;
}

static int Beq(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    lua_pushboolean(L,mpi_cmp_mpi(a,b)==0);
    return 1;
}

static int Blt(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    lua_pushboolean(L, mpi_cmp_mpi(a,b)<0);
    return 1;
}

static int Bneg(lua_State *L)
{
    mpi A;
    mpi *a = &A;
    mpi *b=Bget(L,1);
    mpi *c=Bnew(L);
    mpi_init(a);
    mpi_sub_mpi(c, a, b);
    mpi_free(a);
    return 1;
}

static int Babs(lua_State *L)
{
    mpi *b=Bget(L,1);
    if (mpi_cmp_int(b,0)<0) {
        mpi A;
        mpi *a=&A;
        mpi *c=Bnew(L);
        mpi_init(a);
        mpi_sub_mpi(c,a,b);
        mpi_free(a);
    }
    else lua_settop(L,1);
    return 1;
}

static int Badd(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_add_mpi(c,a,b);
    return 1;
}

static int Bsub(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_sub_mpi(c,a,b);
    return 1;
}

static int Bmul(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_mul_mpi(c,a,b);
    return 1;
}

static int Bdiv(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *q=Bnew(L);
    mpi *r=NULL;
    mpi_div_mpi(q,r,a,b);
    return 1;
}

static int Bmod(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *r=Bnew(L);
    mpi *q=NULL;
    mpi_div_mpi(q,r,a,b);
    return 1;
}

/* Remove references to rmod, because there is no comparable PolarSSL function */

static int Bdivmod(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *q=Bnew(L);
    mpi *r=Bnew(L);
    mpi_div_mpi(q,r,a,b);
    return 2;
}

static int Bgcd(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_gcd(c,a,b);
    return 1;
}

/* Remove references to pow.  PolarSSL only has powmod */

/* Remove addmmod, submod and mulmod.  They are unnecessary */

static int Bpowmod(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *b=Bget(L,2);
    mpi *m=Bget(L,3);
    mpi RR;
    mpi *rr=&RR;
    mpi_init(rr);
    mpi *c=Bnew(L);
    mpi_exp_mod(c,a,b,m,rr);
    mpi_free(rr);
    return 1;
}

/* Sqrmod and sqrtmod don't exist */

static int Binvmod(lua_State *L)
{
    mpi *a=Bget(L,1);
    mpi *m=Bget(L,2);
    mpi *c=Bnew(L);
    mpi_inv_mod(c,a,m);
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
    mpi **a = (mpi **) lua_touserdata(L, 1);
    mpi_free(*a);
    free(*a);
    lua_pushnil(L);
    lua_setmetatable(L,1);
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








