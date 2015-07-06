#!/usr/bin/env lua

package	= 'lua-pb'
version	= 'scm-0'
source	= {
	url	= 'git://github.com/Neopallium/lua-pb.git'
}
description	= {
	summary	= "Lua Protocol Buffers",
	detailed	= '',
	homepage	= 'https://github.com/Neopallium/lua-pb',
	license	= 'MIT',
	maintainer = "Robert G. Jakabosky",
}
dependencies = {
	'lua >= 5.1',
	'lpeg >= 0.10',
	'luabitop >= 1.0.1',
	'struct >= 1.2',
}
build	= {
	type		= 'none',
	install = {
		lua = {
			['pb']                  = "pb.lua",
			['pb.proto.scanner']    = "pb/proto/scanner.lua",
			['pb.proto.util']       = "pb/proto/util.lua",
			['pb.proto.grammar']    = "pb/proto/grammar.lua",
			['pb.proto.parser']     = "pb/proto/parser.lua",
			['pb.standard']         = "pb/standard.lua",
			['pb.standard.pack']    = "pb/standard/pack.lua",
			['pb.standard.unpack']  = "pb/standard/unpack.lua",
			['pb.standard.buffer']  = "pb/standard/buffer.lua",
			['pb.standard.unknown'] = "pb/standard/unknown.lua",
			['pb.standard.dump']    = "pb/standard/dump.lua",
		}
	}
}
