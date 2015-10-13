# makefile for bc library for Lua

# change these to reflect your Lua installation
#LUA= /tmp/lhf/lua-5.2.0
#LUAINC= $(LUA)/src
#LUALIB= $(LUA)/src
#LUABIN= $(LUA)/src

# these will probably work if Lua has been installed globally
# if installed on MacOSX with Homebrew, it should look something like this
LUA= /usr/local/Cellar/lua52/5.2.3
LUAINC= $(LUA)/include
LUALIB= $(LUA)/lib
LUABIN= $(LUA)/bin

# probably no need to change anything below here
CC= gcc
#CFLAGS= $(INCS) $(WARN) -O2 $G
CFLAGS= $(INCS) $(WARN) -g $G
WARN= -pedantic -Wall -Wextra
#WARN == -ansi
INCS= -I$(LUAINC) -I.
# comment in the first line for linux and the second for Mac
#MAKESO= $(CC) -shared
MAKESO= $(CC) -bundle -undefined dynamic_lookup

MYNAME= polarbn
MYLIB= $(MYNAME)
T= $(MYNAME).so
OBJS= $(MYLIB).o bignum.o
TEST= test.lua

all:	test

test:	$T
	$(LUABIN)/lua $(TEST)

o:	$(MYLIB).o

so:	$T

$T:	$(OBJS)
	$(MAKESO) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $T core core.*

doc:
	@echo "$(MYNAME) library:"
	@fgrep '/**' $(MYLIB).c | cut -f2 -d/ | tr -d '*' | sort | column

# eof
