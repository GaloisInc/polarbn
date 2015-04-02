The code that is relevant to the LUA wrapper is availible under an MIT license pursuant to the FOSS license exemption for PolarSSL: https://polarssl.org/foss-license-exception

PolarSSL itself, including the files bignum.h, bignum.c, bn_mul.h, and check_config.h that are included in this distribution are under the GPL license.  If you own a commercial license to PolarSSL, you can use that version instead.  The copies of these PolarSSL files are provided only as a convenience, and it shoudl be trivial to include copies from the standard releases.

This is a multi precision integer library for Lua based on the PolarSSL bignum library. The functions are more or less the same ones in the lbn and lbc Lua bignum libraries.  Performance is considerably better than lbc, and it does not require an OpenSSL install in order to run, as does lbn. 

To compile, simply modify the Makefile to point at the appropriate directories for your lua executable, lib, and include directories.  This will build the relevant PolarSSL bignum files as well as create polarbn.so, which exports the appropriate bignum functions to lua.

Metatable functions are included so the usual arithmetic operators will work on bignums.  In addition there is automatic coercion to bignums for string and number types when necessary.

Functions:

	__add
	__div
	__eq
	__gc
	__lt
	__mod
	__mul
	__sub
	__tostring
	__unm
	abs
	add
	bits
	compare
	div
    divmod
	gcd
	invmod
	isneg
	isodd
	isone
	iszero
	mod
	mul
	neg
	number
	powmod
	sub
	text
	tohex
	totext
	tonumber
	tostring

Crypto caveat: While I have made some efforts to have the __gc function zero out bignums before deallocating them, there are many places where Lua may leave traces of your keys in memory.  Be very careful.
	
Future work:  It would be nice to add type information so that a tool like TypedLua or TidalLock could perform some sort of type-checking on bignums. 
	