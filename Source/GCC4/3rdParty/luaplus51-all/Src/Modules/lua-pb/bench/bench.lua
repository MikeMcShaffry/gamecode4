#!/usr/bin/env lua

local socket = require"socket"
local time = socket.gettime
local clock = os.clock
local quiet = false
local enable_gc = false
local collectgarbage = collectgarbage
local assert = assert

if arg[1] == '-gc' then
	enable_gc = true
	table.remove(arg,1)
else
	print"GC is disabled so we can track memory usage better"
	print""
end

local function printf(fmt, ...)
	local res
	if not quiet then
		fmt = fmt or ''
		res = print(string.format(fmt, ...))
		io.stdout:flush()
	end
	return res
end

local function full_gc()
	-- make sure all free-able memory is freed
	collectgarbage"collect"
	collectgarbage"collect"
	collectgarbage"collect"
end

module(...)

_M.full_gc = full_gc

function bench(name, N, func, p1, p2, p3, p4, p5, p6)
	local start1,start2
	start1 = clock()
	start2 = time()
	for i=1,N do
		func(p1, p2, p3, p4, p5, p6)
	end
	local diff1 = (clock() - start1)
	local diff2 = (time() - start2)
	return diff1, diff2
end

function memtest(name, N, func, p1, p2, p3, p4, p5, p6)
	local start_mem, end_mem
	local rets = {}
	
	full_gc()
	for i=1,N do
		rets[i] = 0
	end
	printf('run garbage bench: %s', name)
	if not enable_gc then collectgarbage"stop" end
	start_mem = (collectgarbage"count" * 1024)

	for i=1,N do
		rets[i] = func(p1, p2, p3, p4, p5, p6)
	end

	end_mem = (collectgarbage"count" * 1024)
	printf('Total memory used: %10.3f Kbytes\n', (end_mem - start_mem) / 1024)

	collectgarbage"restart"
	full_gc()

	return ret, (end_mem - start_mem)
end

function speedtest(name, N, func, p1, p2, p3, p4, p5, p6)
	local start_mem, end_mem
	local secs1,secs2
 
	full_gc()
	if not enable_gc then collectgarbage"stop" end
	printf('run bench: %s', name)
	start_mem = (collectgarbage"count" * 1024)

	secs1,secs2 = bench(name, N, func, p1, p2, p3, p4, p5, p6)

	end_mem = (collectgarbage"count" * 1024)
	printf("total time: %10.6f (%10.6f) seconds", secs1, secs2)
	printf("time per: %10.6f (%10.6f) microseconds",
		(secs1 / N) * 1000 * 1000, (secs2 / N) * 1000 * 1000)
	printf('Total memory used: %10.3f Kbytes\n', (end_mem - start_mem) / 1024)

	collectgarbage"restart"
	full_gc()
end

function filter_memtest(name, N, filter, input, output)
	local start_mem, end_mem
	local ret

	full_gc()
	printf('run garbage bench: %s', name)
	if not enable_gc then collectgarbage"stop" end
	start_mem = (collectgarbage"count" * 1024)

	ret = filter(input)

	end_mem = (collectgarbage"count" * 1024)
	printf('Total memory used: %10.3f Kbytes\n', (end_mem - start_mem) / 1024)

	-- validate filter function's output
	assert(ret == output, "filter function failed.")
 
	collectgarbage"restart"
	full_gc()
end

function filter_speedtest(name, N, filter, input, output)
	local start_mem, end_mem
	local secs1,secs2
 
	full_gc()
	if not enable_gc then collectgarbage"stop" end
	printf('run bench: %s', name)
	start_mem = (collectgarbage"count" * 1024)

	secs1,secs2 = bench(name, N, filter, input)

	end_mem = (collectgarbage"count" * 1024)
	printf("total time: %10.6f (%10.6f) seconds", secs1, secs2)
	-- calculate Bytes per second.
	local len = #input
	if #output > len then len = #output end
	printf('Data speed: %10.3f MBytes per sec.', ((N * len) / (1024 * 1024) / secs1))
	printf('Total memory used: %10.3f Kbytes\n', (end_mem - start_mem) / 1024)

	collectgarbage"restart"
	full_gc()
end

function need_gc(enable)
	local old = enable_gc
	enable_gc = enable
	return old
end

