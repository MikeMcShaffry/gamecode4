module (arg and arg[1])


local timer_mt = { __index = {} }
local timer = timer_mt.__index

-- create new event list
function new ()
	return setmetatable ({jobs={}}, timer_mt)
end

-- add event
-- f() is called no sooner than time t
function timer:add (t, f)
	local job={time=t, func=f}
	local ins = -1

	for i,v in ipairs (self.jobs) do
		if v.time > t then
			ins = i
			break
		end
	end

	if ins > 0 then
		table.insert (self.jobs, ins, job)
	else
		table.insert (self.jobs, job)
	end
end

-- add relatike event
-- f() is called no sooner than time now+t
-- (t is seconds in most systems)
function timer:add_rel (t, f)
	return self:add (os.time() + t, f)
end

-- add repeatig event
-- f() is called no more often than once each t
-- (seconds in most systems)
function timer:add_rep (t, f)
	local function w ()
		f ()
		return self:add_rel (t, w)
	end
	return self:add_rel (t, w)
end

-- called periodically
function timer:tick ()
	while self.jobs[1] and os.time() >= self.jobs[1].time do
		local job = table.remove (self.jobs, 1)
		job.func()
	end
end