print("importing polarbn")
local polarbn = require('polarbn')

local function hex2bignum(s)
  local x=polarbn.number(0)
  for i=1,#s do
    x=16*x+tonumber(s:sub(i,i),16)
  end
  return x
end

print("polarbn", polarbn)
for i,v in pairs(polarbn) do print(i,v) end
print("Assigning a bignum")
local x = polarbn.number(2)
print ("x has been assigned:", x)
local y = hex2bignum("DE2BCBF6955817183995497CEA956AE515D2261898FA0510")
print("assigning exponent:",  polarbn.tohex(y))

local group5prime =
    [[FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1
      29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD
      EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245
      E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED
      EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D
      C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F
      83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D
      670C354E 4ABC9804 F1746C08 CA237327 FFFFFFFF FFFFFFFF]]
local prime5  = string.gsub(group5prime, "%s+", "")
local m = hex2bignum(prime5)
print("assigning modulus", polarbn.tohex(m))
local z = polarbn.powmod(x,y,m)
print("x^e mod m = ", polarbn.tohex(z))
collectgarbage()