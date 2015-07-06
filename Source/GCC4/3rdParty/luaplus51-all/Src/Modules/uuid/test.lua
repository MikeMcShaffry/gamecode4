-- test uuid library

require"uuid"

print(uuid.version)
print""

u=uuid.new()
U="uuid generation"
U=U..string.rep(" ",#u-#U)
print(U,"type","time","","os.time")
print(u,"default",uuid.time(u))
u=uuid.new("default")
print(u,"default",uuid.time(u))
u=uuid.new("random")
print(u,"random",uuid.time(u))
u=uuid.new("time")
print(u,"time",uuid.time(u),os.time())
os.execute"sleep 2"
u=uuid.new("time")
print(u,"time",uuid.time(u),os.time())
print""

function test(x,expect)
 local isvalid=uuid.isvalid(x)
 print(x,isvalid,expect,isvalid==expect and "pass" or "FAIL")
end

U="uuid validation"
U=U..string.rep(" ",#u-#U)
print(U,"isvalid","expect")
-- from tst_uuid.c
test("84949cc5-4701-4a84-895b-354c584a981b", true)
test("84949CC5-4701-4A84-895B-354C584A981B", true)
test("84949cc5-4701-4a84-895b-354c584a981bc", false)
test("84949cc5-4701-4a84-895b-354c584a981", false)
test("84949cc5x4701-4a84-895b-354c584a981b", false)
test("84949cc504701-4a84-895b-354c584a981b", false)
test("84949cc5-470104a84-895b-354c584a981b", false)
test("84949cc5-4701-4a840895b-354c584a981b", false)
test("84949cc5-4701-4a84-895b0354c584a981b", false)
test("g4949cc5-4701-4a84-895b-354c584a981b", false)
test("84949cc5-4701-4a84-895b-354c584a981g", false)

print""
print(uuid.version)
