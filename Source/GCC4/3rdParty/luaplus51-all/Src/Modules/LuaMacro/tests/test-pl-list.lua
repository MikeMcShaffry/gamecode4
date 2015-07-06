if not pcall(require,'pl') then
    return print 'this example requires Penlight'
end

require_ 'macro.forall'

def_ LL List( _PUSH_('L',')') L

print(LL{i|i=1,3}..LL{i|i=10,20,5})

print( LL{List{k,v} for k,v in pairs{A=1,B=2}} )
