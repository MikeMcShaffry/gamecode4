local cosmo = require"cosmo"

values = { rank="Ace", suit="Spades" } 
template = "$rank of $suit"
result = cosmo.fill(template, values) 
assert(result== "Ace of Spades")
  
mycards = { {rank="Ace", suit="Spades"}, {rank="Queen", suit="Diamonds"}, {rank="10", suit="Hearts"} } 
template = "$do_cards[[$rank of $suit, ]]"
result = cosmo.fill(template, {do_cards = mycards})  
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, ")

result= cosmo.f(template){do_cards = mycards}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, ")

mycards = { {"Ace", "Spades"}, {"Queen", "Diamonds"}, {"10", "Hearts"} }
result = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
                 cosmo.yield{rank=v[1], suit=v[2]}
              end
           end
        }
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, ")

table.insert(mycards, {"2", "Clubs"})
template = "You have: $do_cards[[$rank of $suit]],[[, $rank of $suit]],[[, and $rank of $suit]]"
result = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
		 local template
                 if i == #mycards then -- for the last item use the third template (with "and")
                    template = 3
                 elseif i~=1 then -- use the second template for items 2...n-1
                    template = 2
                 end
                 cosmo.yield{rank=v[1], suit=v[2], _template=template}
              end
           end
        }
assert(result=="You have: Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs")

template = "$do_cards[[$1 of $2]]"
result= cosmo.f(template){do_cards = cosmo.make_concat(mycards)}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs")

template = "$concat{cards}[[$1 of $2]]"
result= cosmo.f(template){cards = mycards, concat = cosmo.concat}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs")
result= cosmo.f(template){cards = mycards, concat = cosmo.concat}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs")

template = "$do_cards[[$it]]"
result= cosmo.f(template){do_cards = cosmo.make_concat{ "Ace of Spades", "Queen of Diamonds", "10 of Hearts" } }
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts")
result= cosmo.f(template){do_cards = cosmo.make_concat{ "Ace of Spades", "Queen of Diamonds", "10 of Hearts" } }
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts")

template = "$do_cards[[$it]]"
result= cosmo.f(template){do_cards = cosmo.make_concat{ "Ace of Spades" } }
assert(result=="Ace of Spades")
result= cosmo.f(template){do_cards = cosmo.make_concat{ "Ace of Spades" } }
assert(result=="Ace of Spades")

template = "$concat{cards}[[$it]]"
result= cosmo.f(template){ cards = { "Ace of Spades", "Queen of Diamonds", "10 of Hearts" }, concat = cosmo.concat }
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts")
result= cosmo.f(template){ cards = { "Ace of Spades", "Queen of Diamonds", "10 of Hearts" }, concat = cosmo.concat }
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts")

template = "$concat{cards}[[$1 of $2]]"
result= cosmo.f(template){cards = mycards, concat = cosmo.concat}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs")
result= cosmo.f(template){cards = mycards, concat = cosmo.concat}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs")

template = "$do_cards{'; '}[[$1 of $2]]"
result= cosmo.f(template){do_cards = cosmo.make_concat(mycards)}
assert(result=="Ace of Spades; Queen of Diamonds; 10 of Hearts; 2 of Clubs")
result= cosmo.f(template){do_cards = cosmo.make_concat(mycards)}
assert(result=="Ace of Spades; Queen of Diamonds; 10 of Hearts; 2 of Clubs")

template = "$concat{cards, '; '}[[$1 of $2]]"
result= cosmo.f(template){cards = mycards, concat = cosmo.concat}
assert(result=="Ace of Spades; Queen of Diamonds; 10 of Hearts; 2 of Clubs")
result= cosmo.f(template){cards = mycards, concat = cosmo.concat}
assert(result=="Ace of Spades; Queen of Diamonds; 10 of Hearts; 2 of Clubs")

template = [====[You have: $do_cards[[$rank of $suit]],
[[, $rank of $suit]],
[[, and $rank of $suit]]]====]
result = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
		 local template
                 if i == #mycards then -- for the last item use the third template (with "and")
                    template = 3
                 elseif i~=1 then -- use the second template for items 2...n-1
                    template = 2
                 end
                 cosmo.yield{rank=v[1], suit=v[2], _template=template}
              end
           end
        }
assert(result=="You have: Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs")


players = {"John", "João"}
cards = {}
cards["John"] = mycards
cards["João"] = { {"Ace", "Diamonds"} }
template = "$do_players[=[$player has $do_cards[[$rank of $suit]],[[, $rank of $suit]],[[, and $rank of $suit]]\n]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          local template
                          if i == #mycards then -- for the last item use the third template (with "and")
                             template = 3
                          elseif i~=1 then -- use the second template for items 2...n-1
                             template = 2
                          end
                          cosmo.yield{rank=v[1], suit=v[2], _template=template}
                       end
                    end
                 }         
             end
          end
        }
assert(result=="John has Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs\nJoão has Ace of Diamonds\n")

players = {"John", "João"}
cards = {}
cards["John"] = mycards
cards["João"] = { {"Ace", "Diamonds"} }
template = "$do_players[[$player has $do_cards[[$rank of $suit]],[=[, $rank of $suit]=],[[, and $rank of $suit]]\n]]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          local template
                          if i == #mycards then -- for the last item use the third template (with "and")
                             template = 3
                          elseif i~=1 then -- use the second template for items 2...n-1
                             template = 2
                          end
                          cosmo.yield{rank=v[1], suit=v[2], _template=template}
                       end
                    end
                 }         
             end
          end
        }
assert(result=="John has Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs\nJoão has Ace of Diamonds\n")

template = "$do_players[=[$player$if_john[[$mark]] has $do_cards[[$rank of $suit, ]]\n]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          cosmo.yield{rank=v[1], suit=v[2]}
                       end
                    end,
                    if_john = cosmo.c(p=="John"){mark="*"}
                 }         
             end
          end
        }

assert(result=="John* has Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs, \nJoão has Ace of Diamonds, \n")

template = "$do_players[=[$do_cards[[$rank of $suit ($player), ]]]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          cosmo.yield{rank=v[1], suit=v[2]}
                       end
                    end,
                 }         
             end
          end
        }

assert(result=="Ace of Spades (John), Queen of Diamonds (John), 10 of Hearts (John), 2 of Clubs (John), Ace of Diamonds (João), ")

template = "$do_players[=[$player: $n card$if_plural[[s]] $if_needs_more[[(needs $n more)]]\n]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    n = #cards[p],
                    if_plural = cosmo.cond(#cards[p] > 1, {}),
                    if_needs_more = cosmo.cond(#cards[p] < 3, { n = 3-#cards[p] })
                 }         
             end
          end
        }
assert(result=="John: 4 cards \nJoão: 1 card (needs 2 more)\n")

result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    n = #cards[p],
                    if_plural = cosmo.c(#cards[p] > 1){},
                    if_needs_more = cosmo.c(#cards[p] < 3){ n = 3-#cards[p] }
                 }         
             end
          end
        }
assert(result=="John: 4 cards \nJoão: 1 card (needs 2 more)\n")

template = " $foo|bar $foo|1|baz "
result = cosmo.fill(template, { foo = { { baz = "World!" }, bar = "Hello" } })
assert(result==" Hello World! ")

template = " $(foo.bar) $(foo[1].baz) "
result = cosmo.fill(template, { foo = { { baz = "World!" }, bar = "Hello" } })
assert(result==" Hello World! ")

template = " $(foo.bar) $(foo[1].baz) "
result = cosmo.fill(template, { foo = { { baz = "World!" }, bar = "Hello" } })
assert(result==" Hello World! ")

template = " Hello $message{ 'World!' } "
result = cosmo.fill(template, { message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $(message){ 'World!' } "
result = cosmo.fill(template, { message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $(message){ 'World!' } "
result = cosmo.fill(template, { message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $message{ $msg } "
result = cosmo.fill(template, { msg = "World!", message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $message{ $(msg) } "
result = cosmo.fill(template, { msg = "World!", message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $message{ $(msg) } "
result = cosmo.fill(template, { msg = "World!", message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.fill(template, { msg = "World!", 
		       message = function (arg) cosmo.yield{ it = arg[1] } end })
assert(result==" Hello World! ")

template = " $foo|bar $foo|1|baz "
result = cosmo.f(template){ foo = { { baz = "World!" }, bar = "Hello" } }
assert(result==" Hello World! ")

template = " Hello $message{ 'World!' } "
result = cosmo.f(template){ message = function (arg) return arg[1] end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg } "
result = cosmo.f(template){ msg = "World!", message = function (arg) return arg[1] end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.f(template){ msg = "World!", 
		       message = function (arg) cosmo.yield{ it = arg[1] } end }
assert(result==" Hello World! ")


template = " Hello $message{ $msg } "
result = cosmo.f(template){ msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.f(template){ msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg } "
result = cosmo.fill(template, { msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end })
assert(result==" Hello World! ")

template = " Hello $message{ msg } "
result = cosmo.fill(template, { msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end })
assert(result==" Hello World! ")

template = " Hello $message{ msg } "
result = cosmo.fill(template, { msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end })
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.fill(template, { msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end })
assert(result==" Hello World! ")

template = " $message{ greeting = 'Hello', target = 'World' } "
result = cosmo.fill(template, { message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end })
assert(result==" Hello World! ")

template = " $message{ greeting = 'Hello', target = 'World' } "
result = cosmo.f(template){ message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end }
assert(result==" Hello World! ")

template = " $message{ greeting = 'Hello', target = 'World' }[[$grt $tgt]] "
result = cosmo.fill(template, { message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end })
assert(result==" Hello World ")

template = " $message{ greeting = 'Hello', target = 'World'}[[$grt $tgt]] "
result = cosmo.f(template){ message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end }
assert(result==" Hello World ")

local env = {}
setmetatable(env, { __index = { text = "Hello World!" } })
template = " $show[[$text]] "
result = cosmo.fill(template, { show = function () cosmo.yield(env) end })
assert(result == " Hello World! ")
result = cosmo.f(template){ show = function () cosmo.yield(env) end }
assert(result == " Hello World! ")

template = " $map{ 1, 2, 3, 4, 5}[[$it]] "
result = cosmo.fill(template, { map = cosmo.map })
assert(result == " 12345 ")
result = cosmo.f(template){ map = cosmo.map }
assert(result == " 12345 ")

template = " $map{ 1, [[foo]], 3, 4, 5}[[$it]] "
result = cosmo.fill(template, { map = cosmo.map })
assert(result == " 1foo345 ")
result = cosmo.f(template){ map = cosmo.map }
assert(result == " 1foo345 ")

template = " $map{ 1, 2, 3, 4, 5} "
result = cosmo.fill(template, { map = cosmo.map })
assert(result == " 12345 ")
result = cosmo.f(template){ map = cosmo.map }
assert(result == " 12345 ")

template = "$inject{ msg = 'Hello', target = 'World' }[[ $msg $target! ]]"
result = cosmo.fill(template, { inject = cosmo.inject })
assert(result == " Hello World! ")
result = cosmo.f(template){ inject = cosmo.inject }
assert(result == " Hello World! ")

template = "$if{ $x + 3 > 4, target = 'World' }[[ Hello $target! ]],[[ Hi $target! ]]"
result = cosmo.fill(template, { x = 0, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ x = 0, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { x = 2, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ x = 2, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ $x + 3 > 4, target = 'World' }[[ Hello $target! ]]"
result = cosmo.fill(template, { x = 0, ["if"] = cosmo.cif })
assert(result == "")
result = cosmo.f(template){ x = 0, ["if"] = cosmo.cif }
assert(result == "")
result = cosmo.fill(template, { x = 2, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ x = 2, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ $x + 3 > 4, target = 'World' }[[ Hello $target! ]][[ Hi $target! ]]"
result = cosmo.fill(template, { x = 0, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ x = 0, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { x = 2, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ x = 2, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ $x + 3 > 4, target = 'World' }[[ Hello $target! ]][[ Hi $target! ]]"
result = cosmo.fill(template, { x = 0, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ x = 0, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { x = 2, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ x = 2, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ x + 3 > 4, target = 'World' }[[ Hello $target! ]],[[ Hi $target! ]]"
result = cosmo.fill(template, { x = 0, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ x = 0, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { x = 2, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ x = 2, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ $mod($x, 4) == 0, target = 'World' }[[ Hello $target! ]],[[ Hi $target! ]]"
result = cosmo.fill(template, { mod = math.fmod, x = 2, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ mod = math.fmod, x = 2, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { mod = math.fmod, x = 4, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ mod = math.fmod, x = 4, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ math.fmod(x, 4) == 0, target = 'World' }[[ Hello $target! ]],[[ Hi $target! ]]"
result = cosmo.fill(template, { math = math, x = 2, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ math = math, x = 2, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { math = math, x = 4, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ math = math, x = 4, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ x == 0, target = 'World' }[[ Hello $target! ]],[[ Hi $target! ]]"
result = cosmo.fill(template, { math = math, x = 4, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ math = math, x = 4, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { math = math, x = 0, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ math = math, x = 0, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$if{ x(), target = 'World' }[[ Hello $target! ]],[[ Hi $target! ]]"
result = cosmo.fill(template, { math = math, x = function () return false end, ["if"] = cosmo.cif })
assert(result == " Hi World! ")
result = cosmo.f(template){ math = math, x = function () return false end, ["if"] = cosmo.cif }
assert(result == " Hi World! ")
result = cosmo.fill(template, { math = math, x = function () return true end, ["if"] = cosmo.cif })
assert(result == " Hello World! ")
result = cosmo.f(template){ math = math, x = function () return true end, ["if"] = cosmo.cif }
assert(result == " Hello World! ")

template = "$lit[[ $msg]]"
result = cosmo.fill(template, { lit = function ()
					cosmo.yield("Hello", true)
					cosmo.yield({ msg = "World" })
					cosmo.yield("!", true)
				      end })
assert(result == "Hello World!")
result = cosmo.fill(template, { lit = function ()
					cosmo.yield("Hello", true)
					cosmo.yield({ msg = "World" })
					cosmo.yield("!", true)
				      end })
assert(result == "Hello World!")
						    