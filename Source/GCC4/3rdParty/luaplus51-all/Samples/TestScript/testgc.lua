iters = 1 --1000
listlen = 2 --100
master = {}

function mklist(n,tail)
  if n == 0 then return tail end;
  local head = {}
  local t = tail
  local m = n - 1
  t2 = mklist(m,t) -- deliberately not tail rec
  local node = {}
  node.link = t2
  node.data = n
  return node
end

-- First test only
--for i = 1,iters do master[1]=mklist(listlen,nil) end

for i = 1,iters do
  local j = i
  if j > 100 then j = 0 end
  master[j]=mklist(listlen,nil)
  master[j]=nil
end

t2 = nil
