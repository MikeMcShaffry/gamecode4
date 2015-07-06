lua-pb
=======

Lua Protocol Buffers.

Supports dynamic loading of Protocol Buffer message definition files `.proto`

Installing
----------

	$ sudo luarocks install "https://raw.github.com/Neopallium/lua-pb/master/lua-pb-scm-0.rockspec"

Design
------

Frontend `.proto` definition file parser:

* pb/proto/scanner.lua -- LPeg lexer for `.proto` files.
* pb/proto/util.lua    -- some utility functions.
* pb/proto/grammar.lua -- LPeg grammar for `.proto` files.
* pb/proto/parser.lua  -- LPeg based `.proto` -> AST tree parser.

There can be multiple Backend message definition compilers.  An optimized backend for LuaJIT
is planned.

Standard backend compiler

* pb/standard.lua          -- main compiler code.
* pb/handlers.lua          -- a helper object for managing registered encode/decode formats.
* pb/standard/message.lua  -- defines message interface.
* pb/standard/repeated.lua -- defines repeated field interface.
* pb/standard/pack.lua     -- binary format encoding (Uses modules luabitops & struct)
* pb/standard/unpack.lua   -- binary format decoding (Uses modules luabitops & struct)
* pb/standard/buffer.lua   -- encoding buffer
* pb/standard/unknown.lua  -- object for hold unknown fields.
* pb/standard/dump.lua     -- message dumping code.

Finished
--------
* .proto definition parser
* Message encoding/decoding
* Dumping messages to text format.
* Support for packing/unpacking unknown fields.

TODO
----

* rename pb/handlers.lua to pb/formats.lua

Missing features:

* custom options:

	import "google/protobuf/descriptor.proto";
	
	extend google.protobuf.MessageOptions {
	  optional string my_option = 51234;
	}
	
	message MyMessage {
	  option (my_option) = "Hello world!";
	}

* services


Improvements:

* store unknown fields as raw binary, only fully decode when accessed.
* LuaJIT optimized backend compiler.

