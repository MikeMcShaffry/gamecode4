#!/usr/bin/env wsapi.cgi

require "orbit"
require "orbit.cache"
require "orbit.model"
require "markdown"

--
-- Declares that this is module is an Orbit app
--
module("blog", package.seeall, orbit.new)

--
-- Loads configuration data
--
require "blog_config"

--
-- Initializes DB connection for Orbit's default model mapper
--
require("luasql." .. database.driver)
local env = luasql[database.driver]()
mapper.conn = env:connect(unpack(database.conn_data))
mapper.driver = orbit.model.drivers[database.driver]
mapper.schema = blog_schema

-- Initializes page cache
local cache = orbit.cache.new(blog, cache_path)

--
-- Models for this application. Orbit calls mapper:new for each model,
-- so if you want to replace Orbit's default ORM mapper by another
-- one (file-based, for example) just redefine the mapper global variable
--

posts = blog:model "post"

function posts:find_comments()
   return comments:find_all_by_post_id{ self.id }
end

function posts:find_recent()
   return self:find_all("published_at is not null",
			{ order = "published_at desc",
			   count = recent_count })
end

function posts:find_by_month_and_year(month, year)
   local s = os.time({ year = year, month = month, day = 1 })
   local e = os.time({ year = year + math.floor(month / 12),
			month = (month % 12) + 1,
			day = 1 })
   return self:find_all("published_at >= ? and published_at < ?",
			{ s, e, order = "published_at desc" })
end

function posts:find_months()
   local months = {}
   local previous_month = {}
   local posts = self:find_all({ order = "published_at desc" })
   for _, post in ipairs(posts) do
      local date = os.date("*t", post.published_at)
      if previous_month.month ~= date.month or
	 previous_month.year ~= date.year then
	 previous_month = { month = date.month, year = date.year,
	    date_str = os.date("%Y/%m", post.published_at) }
	 months[#months + 1] = previous_month
      end
   end
   return months
end

comments = blog:model "comment"

function comments:make_link()
   local author = self.author or strings.anonymous_author
   if self.url and self.url ~= "" then
      return "<a href=\"" .. self.url .. "\">" .. author .. "</a>"
   elseif self.email and self.email ~= "" then
      return "<a href=\"mailto:" .. self.email .. "\">" .. author .. "</a>"
   else
      return author
   end
end

pages = blog:model "page"

--
-- Controllers for this application
--

function index(web)
   local ps = posts:find_recent()
   local ms = posts:find_months()
   local pgs = pgs or pages:find_all()
   return render_index(web, { posts = ps, months = ms,
			  recent = ps, pages = pgs })
end

blog:dispatch_get(cache(index), "/", "/index") 

function view_post(web, post_id, comment_missing)
   local post = posts:find(tonumber(post_id))
   if post then
      local recent = posts:find_recent()
      local pgs = pages:find_all()
      post.comments = post:find_comments()
      local months = posts:find_months()
      return render_post(web, { post = post, months = months,
			    recent = recent, pages = pgs,
			    comment_missing = comment_missing })
   else
      return not_found(web)
   end
end

blog:dispatch_get(cache(view_post), "/post/(%d+)")

function add_comment(web, post_id)
   local input = web.input
   if string.find(input.comment, "^%s*$") then
      return view_post(web, post_id, true)
   else
      local comment = comments:new()
      comment.post_id = tonumber(post_id)
      comment.body = markdown(input.comment)
      if not string.find(input.author, "^%s*$") then
	 comment.author = input.author
      end
      if not string.find(input.email, "^%s*$") then
	 comment.email = input.email
      end
      if not string.find(input.url, "^%s*$") then
	 comment.url = input.url
      end
      comment:save()
      local post = posts:find(tonumber(post_id))
      post.n_comments = (post.n_comments or 0) + 1
      post:save()
      cache:invalidate("/")
      cache:invalidate("/post/" .. post_id)
      cache:invalidate("/archive/" .. os.date("%Y/%m", post.published_at))
      return web:redirect(web:link("/post/" .. post_id))
   end
end

blog:dispatch_post(add_comment, "/post/(%d+)/addcomment")

function view_archive(web, year, month)
   local ps = posts:find_by_month_and_year(tonumber(month),
					   tonumber(year))
   local months = posts:find_months()
   local recent = posts:find_recent()
   local pgs = pages:find_all()
   return render_index(web, { posts = ps, months = months,
			  recent = recent, pages = pgs })
end

blog:dispatch_get(cache(view_archive), "/archive/(%d%d%d%d)/(%d%d)")

blog:dispatch_static("/head%.jpg", "/style%.css")

function view_page(web, page_id)
   local page = pages:find(tonumber(page_id))
   if page then
      local recent = posts:find_recent()
      local months = posts:find_months()
      local pgs = pages:find_all()
      return render_page(web, { page = page, months = months,
		     recent = recent, pages = pgs })
   else
      not_found(web)
   end
end

blog:dispatch_get(cache(view_page), "/page/(%d+)")

--
-- Views for this application
--

function layout(web, args, inner_html)
   return html{
      head{
	 title(blog_title),
	 meta{ ["http-equiv"] = "Content-Type",
	    content = "text/html; charset=utf-8" },
	 link{ rel = 'stylesheet', type = 'text/css', 
	    href = web:static_link('/style.css'), media = 'screen' }
      },
      body{
	 div{ id = "container",
	    div{ id = "header", title = "sitename" },
	    div{ id = "mainnav",
	       _menu(web, args)
	    }, 
            div{ id = "menu",
	       _sidebar(web, args)
	    },  
	    div{ id = "contents", inner_html },
	    div{ id = "footer", copyright_notice }
	 }
      }
   } 
end

function _menu(web, args)
   local res = { li(a{ href= web:link("/"), strings.home_page_name }) }
   for _, page in pairs(args.pages) do
      res[#res + 1] = li(a{ href = web:link("/page/" .. page.id), page.title })
   end
   return ul(res)
end

function _blogroll(web, blogroll)
   local res = {}
   for _, blog_link in ipairs(blogroll) do
      res[#res + 1] = li(a{ href=blog_link[1], blog_link[2] })
   end
   return ul(res)
end

function _sidebar(web, args)
   return {
      h3(strings.about_title),
      ul(li(about_blurb)),
      h3(strings.last_posts),
      _recent(web, args),
      h3(strings.blogroll_title),
      _blogroll(web, blogroll),
      h3(strings.archive_title),
      _archives(web, args)
   }
end

function _recent(web, args)
   local res = {}
   for _, post in ipairs(args.recent) do
      res[#res + 1] = li(a{ href=web:link("/post/" .. post.id), post.title })
   end
   return ul(res)
end

function _archives(web, args)
   local res = {}
   for _, month in ipairs(args.months) do
      res[#res + 1] = li(a{ href=web:link("/archive/" .. month.date_str), 
			    blog.month(month) })
   end
   return ul(res)
end

function render_index(web, args)
   if #args.posts == 0 then
      return layout(web, args, p(strings.no_posts))
   else
      local res = {}
      local cur_time
      for _, post in pairs(args.posts) do
	 local str_time = date(post.published_at)
	 if cur_time ~= str_time then
	    cur_time = str_time
	    res[#res + 1] = h2(str_time)
	 end
	 res[#res + 1] = h3(post.title)
	 res[#res + 1] = _post(web, post)
      end
      return layout(web, args, div.blogentry(res))
   end
end

function _post(web, post)
   return {
      markdown(post.body),
      p.posted{ 
	 strings.published_at .. " " .. 
	    os.date("%H:%M", post.published_at), " | ",
	 a{ href = web:link("/post/" .. post.id .. "#comments"), strings.comments ..
	    " (" .. (post.n_comments or "0") .. ")" }
      }
   }
end

function _comment(web, comment)
   return { p(comment.body),
      p.posted{
	 strings.written_by .. " " .. comment:make_link(),
	 " " .. strings.on_date .. " " ..
	    time(comment.created_at) 
      }
   }
end

function render_page(web, args)
   return layout(web, args, div.blogentry(markdown(args.page.body)))
end

function render_post(web, args)
   local res = { 
      h2(span{ style="position: relative; float:left", args.post.title }
	 .. "&nbsp;"),
      h3(date(args.post.published_at)),
      _post(web, args.post)
   }
   res[#res + 1] = a{ name = "comments" }
   if #args.post.comments > 0 then
      res[#res + 1] = h2(strings.comments)
      for _, comment in pairs(args.post.comments) do
	 res[#res + 1 ] = _comment(web, comment)
      end
   end
   res[#res + 1] = h2(strings.new_comment)
   local err_msg = ""
   if args.comment_missing then
      err_msg = span{ style="color: red", strings.no_comment }
   end
   res[#res + 1] = form{
      method = "post",
      action = web:link("/post/" .. args.post.id .. "/addcomment"),
      p{ strings.form_name, br(), input{ type="text", name="author",
	 value = web.input.author },
         br(), br(),
         strings.form_email, br(), input{ type="text", name="email",
	 value = web.input.email },
         br(), br(),
         strings.form_url, br(), input{ type="text", name="url",
	 value = web.input.url },
         br(), br(),
         strings.comments .. ":", br(), err_msg,
         textarea{ name="comment", rows="10", cols="60", web.input.comment },
	 br(),
         em(" *" .. strings.italics .. "* "),
         strong(" **" .. strings.bold .. "** "), 
         " [" .. a{ href="/url", strings.link } .. "](http://url) ",
         br(), br(),
         input.button{ type="submit", value=strings.send }
      }
   }
   return layout(web, args, div.blogentry(res))
end

-- Adds html functions to the view functions

orbit.htmlify(blog, "layout", "_.+", "render_.+")
