module('signalslot', package.seeall)

local print_slot_h = fprint(assert(io.open(output_path..module_name..'_slot.hpp', 'w')))
local print_slot_c = fprint(assert(io.open(output_path..module_name..'_slot.cpp', 'w')))

local signals = {}
local slots = {}

function copy_signals(functions)
	for f in pairs(functions) do
		if f.xarg.signal=='1' then
			signals[f] = true
		end
	end
end

function slots_for_signals()
	for sig in pairs(signals) do
		local args, comma = '(', ''
		for i, a in ipairs(sig.arguments) do
			args = args .. comma .. a.xarg.type_name .. ' arg'..i
			comma = ', '
		end
		args = args .. ')'
		local pushlines, stack = make_pushlines(sig.arguments)
		if pushlines then
			if not slots['void __slot '..args] then
				slots['void __slot '..args] = 'void LqtSlotAcceptor::__slot '..args..[[ {
  //int oldtop = lua_gettop(L);
  //lua_getfield(L, -1, "__slot]]..string.gsub(args, ' arg.', '')..[[");
  //if (lua_isnil(L, -1)) {
    //lua_pop(L, 1);
    //lua_getfield(L, -1, "__slot");
  //}
  //if (!lua_isfunction(L, -1)) {
    //lua_settop(L, oldtop);
    //return;
  //}
  lua_pushvalue(L, -2);
]] .. pushlines .. [[
  if (lqtL_pcall(L, ]]..stack..[[+1, 0, 0)) {
    //lua_error(L);
    qDebug() << lua_tostring(L, -1);
    lua_pop(L, 1);
  }
  //lua_settop(L, oldtop);
}
]]
			end
		else
			ignore(sig.xarg.fullname, 'slot', stack)
		end
	end
end

function print_slots(s)
	print_slot_h'class LqtSlotAcceptor : public QObject {'
	print_slot_h'  Q_OBJECT'
	print_slot_h'  lua_State *L;'
	print_slot_h'  public:'
	print_slot_h('  LqtSlotAcceptor(lua_State *l, QObject *p=NULL) : QObject(p), L(l) { setObjectName("'..module_name..'"); lqtL_register(L, this); }')
	print_slot_h'  virtual ~LqtSlotAcceptor() { lqtL_unregister(L, this); }'
	print_slot_h'  public slots:'
	for p, b in pairs(s) do
		print_slot_h('  '..p..';')
		print_slot_c(b)
	end
	print_slot_h'};\n'
	print_slot_h('\nextern LqtSlotAcceptor *lqtSlotAcceptor_'..module_name..';')
	print_slot_c('\nLqtSlotAcceptor *lqtSlotAcceptor_'..module_name..';')
end


-------------------------------------------------------------------------------

function process(functions)
	copy_signals(functions)
	slots_for_signals()
end

function output()
	print_slot_h('#ifndef LQT_SLOT_'..module_name)
	print_slot_h('#define LQT_SLOT_'..module_name)
	print_slot_h(output_includes)
	print_slot_c('#include "'..module_name..'_slot.hpp'..'"\n\n')
	
	print_slots(slots)
	
	print_slot_h('#endif')
end

