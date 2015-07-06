
local pb = require"pb"

local utils = require"utils"

-- load .proto file.
local media = require"protos.media"

--print(utils.dump(media))

local MediaContent = media.MediaContent

local data = {
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
}

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

local msg = MediaContent(data)

check_MediaContent(msg)

pb.print(msg)

local bin = msg:Serialize()

print("--- encoded message: bytes", #bin)

local file = assert(io.open(arg[1] or 'media.bin', 'w'))
assert(file:write(bin))
assert(file:close())

print("--- decode message")
local msg1, off = MediaContent():Parse(bin)

check_MediaContent(msg1)
print(utils.dump(msg1))
pb.print(msg1)

print("Valid .proto file")

