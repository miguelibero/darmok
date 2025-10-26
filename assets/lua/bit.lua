
local bit = bit or bit32

-- If running Lua 5.3+, define our own table using native ops
if not bit then
  bit = {}
  bit.band   = function(a, b) return a & b end
  bit.bor    = function(a, b) return a | b end
  bit.bxor   = function(a, b) return a ~ b end
  bit.lshift = function(a, b) return a << b end
  bit.rshift = function(a, b) return a >> b end
  bit.bnot   = function(a) return ~a end
end

return bit