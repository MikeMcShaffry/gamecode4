
local model = require "orbit.model" 

local function build_query(sql, values)
  return model.condition_parser:match(sql, 1, function (field, op)
						return field .. op .. values[field]
					      end)
end

local queries = {
  { 
    from = [[node_term.term = term.id and term.vocabulary = vocabulary.id and term.name = ? and 
             vocabulary.name = ?]],
    to = [[node_term.term = term.id and term.vocabulary = vocabulary.id and term.name = visibility and vocabulary.name = home]],
    values = { ["vocabulary.name"] = "home", ["term.name"] = "visibility" }
  },
  { 
    from = [[node_term.term = term.id and term.vocabulary = vocabulary.id and term.name = ? and 
             vocabulary.name = ? and term.display_name is not null]],
    to = [[node_term.term = term.id and term.vocabulary = vocabulary.id and term.name = visibility and vocabulary.name = home and term.display_name is not null]],
    values = { ["vocabulary.name"] = "home", ["term.name"] = "visibility" }
  },
  {
    from = [[node in ?]],
    to = [[node in foo]],
    values = { node = "foo" }
  },
  { 
    from = [[node_term.term!=term and term.vocabulary = vocabulary.id and term.name = ? and 
             vocabulary.name = ? and term.display_name is not null]],
    to = [[node_term.term!=term and term.vocabulary = vocabulary.id and term.name = visibility and vocabulary.name = home and term.display_name is not null]],
    values = { ["vocabulary.name"] = "home", ["term.name"] = "visibility" }
  },
  { 
    from = [[node_term.term!=term and term.vocabulary = vocabulary.id and term.name =? and 
             vocabulary.name = ? and term.display_name is not null]],
    to = [[node_term.term!=term and term.vocabulary = vocabulary.id and term.name =visibility and vocabulary.name = home and term.display_name is not null]],
    values = { ["vocabulary.name"] = "home", ["term.name"] = "visibility" }
  },
}

for _, q in ipairs(queries) do
  if not (build_query(q.from, q.values) == q.to) then
    print(build_query(q.from, q.values))
  end
end

