
theme = "wordpress"

home_page = "/home"

blocks = {
  title = { "title", args = { "Test Website" } },
  javascript = { "javascript" },
  css = { "css" },
  banner = { "banner", args = { title = "Test Website", tagline = "It is simple!" } },
  copyright = { "copyright", args = { "2010" } },
  about = { "generic", args = { title = "About Website", text = "Lorem ipsum." } },
  links = { 
    "links", args = { title = "Useful Links", 
		      links = {
			{ "Lua", "http://www.lua.org" },
			{ "Kepler", "http://www.keplerproject.org" }
		      }
		    }
  },
  show_latest = { "show_latest_body", args = { node = "post", count = 7 } },
  recent_links = { "show_latest", args = { node = "post", title = "Recent Posts", count = 7 } },
  powered_by = { "generic", args = { title = "Powered by", text = "Orbit and Kepler toolkit." } },
  post = { "node_info" },
  poll = { "latest_poll" },
  poll_result = { "poll_total" },
  new_node = { "form_new_node" },
  edit_node = { "form_edit_node" }
}

database = {
  driver = "mysql",
  connection = { "toycms-devel", "root", "password" }
}

plugins = {
  "nodes.lua",
  "poll.lua"
}

