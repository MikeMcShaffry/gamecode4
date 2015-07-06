
local pb = require"pb"

local function encode_new_msg(create, ...)
	local msg =create(...)
	return msg:Serialize()
end

local function decode_new_msg(create, data)
	local msg =create()
	return msg:Parse(data)
end

local bench = require"bench.bench"

-- load .proto file.
local media = require"protos.media"

local MediaContent = media.MediaContent

local function create_media()
	return MediaContent({
		media = {
			uri = "http://javaone.com/keynote.mpg",
			title = "Javaone Keynote",
			width = 640,
			height = 480,
			format = "video/mpg4",
			duration = 18000000,    -- half hour in milliseconds
			size = 58982400,        -- bitrate * duration in seconds / 8 bits per byte
			bitrate = 262144,       -- 256k
			person = {"Bill Gates", "Steve Jobs"},
			player = 'JAVA',
			copyright = nil,
		},
		image = {
			{
				uri = "http://javaone.com/keynote_large.jpg",
				title = "Javaone Keynote",
				width = 1024,
				height = 768,
				size = 'LARGE',
			},
			{
				uri = "http://javaone.com/keynote_small.jpg",
				title = "Javaone Keynote",
				width = 320,
				height = 240,
				size = 'SMALL',
			},
		},
	})
end

--
-- Check MediaContent record
--
local function check_MediaContent_media(media)
	assert(media ~= nil)
	assert(media.uri == "http://javaone.com/keynote.mpg")
	assert(media.title == "Javaone Keynote")
	assert(media.width == 640)
	assert(media.height == 480)
	assert(media.format == "video/mpg4")
	assert(media.duration == 18000000)    -- half hour in milliseconds
	assert(media.size == 58982400)        -- bitrate * duration in seconds / 8 bits per byte
	assert(media.bitrate == 262144)       -- 256k
	local person = media.person
	assert(person[1] == "Bill Gates")
	assert(person[2] == "Steve Jobs")
	assert(media.player == 'JAVA')
	assert(media.copyright == nil)
end

local function check_MediaContent(content)
	assert(content ~= nil)
	-- check media record
	check_MediaContent_media(content.media)
	-- check image records.
	local image = content.image
	local img
	img = image[1]
	assert(img ~= nil)
	assert(img.uri == "http://javaone.com/keynote_large.jpg")
	assert(img.title == "Javaone Keynote")
	assert(img.width == 1024)
	assert(img.height == 768)
	assert(img.size == 'LARGE')
	img = image[2]
	assert(img ~= nil)
	assert(img.uri == "http://javaone.com/keynote_small.jpg")
	assert(img.title == "Javaone Keynote")
	assert(img.width == 320)
	assert(img.height == 240)
	assert(img.size == 'SMALL')
end

local loop = tonumber(arg[1] or 100000)

local msg = create_media()
---[[

check_MediaContent(msg)

print("------------ create empty benchmark.")
bench.memtest("create empty Media", 100, MediaContent)
bench.speedtest("create empty Media", loop, MediaContent)

print("Memory usage:", collectgarbage"count")

print("------------ create full benchmark.")
bench.memtest("create full Media", 100, create_media)
bench.speedtest("create full Media", loop, create_media)

print("Memory usage:", collectgarbage"count")

print("------------ check benchmark.")
bench.memtest("check Media", 100, check_MediaContent, msg)
bench.speedtest("check Media", loop, check_MediaContent, msg)

print("Memory usage:", collectgarbage"count")
--]]

print("------------ encode same benchmark.")
local function encode_msg(msg)
	return msg:Serialize()
end
bench.memtest("encode same Media", 100, encode_msg, msg)
bench.speedtest("encode same Media", loop, encode_msg, msg)

print("Memory usage:", collectgarbage"count")

---[[
print("------------ encode different benchmark.")
bench.memtest("encode different Media", 100, encode_new_msg, create_media)
bench.speedtest("encode different Media", loop, encode_new_msg, create_media)

print("Memory usage:", collectgarbage"count")

print("------------ decode benchmark.")
local bin = msg:Serialize()
local msg1, off = MediaContent():Parse(bin)
check_MediaContent(msg1)
bench.memtest("decode Media", 100, decode_new_msg, MediaContent, bin)
bench.speedtest("decode Media", loop, decode_new_msg, MediaContent, bin)

print("Memory usage:", collectgarbage"count")
--]]

