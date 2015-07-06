## Orbit Tutorial

This tutorial shows how to create a simple blog application in Orbit, backed by
a database. It's very simple because it does not include any "admin" pages; you have
to add posts directly to the database (though you can do it through a Lua console, and
this tutorial will show how), but there is an interface for commenting on posts.

The tutorial assumes you have already installed Orbit (preferrably as part of [Kepler](http://www.keplerproject.org),
or via [LuaRocks](http://luarocks.org), and already have a web server that supports WSAPI set up (the Xavante web 
server that comes with Kepler is a good choice).

Complete source code for this blog is in the `samples` folder of Orbit's distribution.
If you have installed Orbit via Kepler or LuaRocks look inside the `rocks` folder
of your installation.

## Initialization

You should create a `blog.lua` file, which will be the main source file for our application.
The first thing you should put in this file is the code to load Orbit and other libraries you
are going to use in your app:

<pre>
require "orbit"
require "orbit.cache"
require "markdown"
</pre>

In this example we are going to use Orbit's page cache, and the Markdown parser for marking
up posts.

All Orbit applications are Lua modules, so we include this line:

<pre>
module("blog", package.seeall, orbit.app)
</pre>

This sets up the `blog` module and initializes it as an Orbit application.

`orbit.app` injects quite a lot of stuff in the `blog` module's namespace.
The most important of these are the `dispatch_get`, `dispatch_post`,
and `model` methods that let you define the main functionality of the
application. It also defines a `mapper` variable that Orbit uses to create
the models (Orbit initializes this variable to its default OR mapper). Finally,
it defines default controllers for 404 and 500 HTTP error codes as the
`not_found` and `server_error` variables, respectively. Override those if you
want custom pages for your application.

Let's load a configuration script for the blog (a common pattern in applications).
You can get this script from [here](blog_config.lua).

<pre>
require "blog_config"
</pre>

The next few lines load one of LuaSQL's database driver (defined in the configuration),
and sets up Orbit's OR mapper.

<pre>
require("luasql." .. database.driver)
local env = luasql[database.driver]()
mapper.conn = env:connect(unpack(database.conn_data))
mapper.driver = database.driver
</pre>

Orbit's mapper needs a database connection to use, and which driver you are
using (currently only "sqlite3" and "mysql" are supported).

You need to initialize the mapper before creating your application's models because
Orbit's mapper hits the database on model creation to get the schema. Speaking of
schema, now is a good time to create your blogs' database. I will assume you are
using SQLite3. Create a `blog.db` database with the following SQL script:

<pre>
CREATE TABLE blog_post
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "title" VARCHAR(255) DEFAULT NULL,
       "body" TEXT DEFAULT NULL,
       "n_comments" INTEGER DEFAULT NULL,
       "published_at" DATETIME DEFAULT NULL);

CREATE TABLE blog_comment
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "post_id" INTEGER DEFAULT NULL,
       "author" VARCHAR(255) DEFAULT NULL,
       "email" VARCHAR(255) DEFAULT NULL,
       "url" VARCHAR(255) DEFAULT NULL,
       "body" TEXT DEFAULT NULL,
       "created_at" DATETIME DEFAULT NULL);

CREATE TABLE blog_page
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "title" VARCHAR(30) DEFAULT NULL,
       "body" TEXT DEFAULT NULL);
</pre>

Orbit's mapper uses the `id` field to identify objects in the
database, so you need it for every kind of object you are mapping.

Finally, let's initialize Orbit's page cache before creating our models:

<pre>
local cache = orbit.cache.new(blog, cache_path)
</pre>

The page cache greatly speeds access to any page that you cache, but you need
to be careful and invalidate the cache for a page when any content on that page
changes. We will see how to cache and invalidate pages in the controller
section of this tutorial.

## Creating Models

Our blog application has three kinds of objects: posts, comments, and
"static" pages (things like an "About" page for the blog, for example). It's no
coincidence that we also have three tables in the database, each table maps
to a kind of object our application handles, and for each kind we will
create a model. We first create a model object for posts:

<pre>
posts = blog:model "post"
</pre>

The parameter for the `model` method is the name of a table in the database.
The `posts` object that this method creates represents the collection of
posts, and at the same time is a prototype for all posts (we will see the
implications of that shortly). Orbit's mapper creates a rather functional
object by itself: you can do `posts:find(3)`, for example, and get the
post with `id` 3, or `posts:find_all("n_comments < ?", { 3, order = "published_at desc" })`
and get a list of all posts with less than three comments, ordered by most recent to least.

You can use the predefined `find` methods for all queries to the database, but it
helps to abstract common queries in your own methods. You can do that by
adding methods to the `posts` object:

<pre>
function posts:find_recent()
   return self:find_all("published_at is not null",
			{ order = "published_at desc",
			   count = recent_count })
end
</pre>

The lines above add a `find_recent` method to the `posts` object that returns
a list of the most recent published posts (the number is in the configuration script),
ordered from most recent to least. The application is going to use this method
to generate the list of posts in the home page, as well as the "Recent Posts" section
of the blog's sidebar.

Another feature of our blog is going to be archive pages that show all posts of
a certain month and year. We will define a method for that too:

<pre>
function posts:find_by_month_and_year(month, year)
   local s = os.time({ year = year, month = month, day = 1 })
   local e = os.time({ year = year + math.floor(month / 12),
			month = (month % 12) + 1,
			day = 1 })
   return self:find_all("published_at >= ? and published_at < ?",
			{ s, e, order = "published_at desc" })
end
</pre>

This is a more complicated method, as we have to convert from a simple month and
year to start and end dates in the standard Lua format. Finally, we will also
define a method that returns all months (and years) that have posts, to later
generate the links for the "Archive" section in the sidebar:

<pre>
function posts:find_months()
   local months = {}
   local previous_month = {}
   local posts = self:find_all("published_at is not null", 
                               { order = "published_at desc" })
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
</pre>

This method gets all posts in the database, ordered by date, and iterates over them storing
the each unique pair of month and year in a list.

We can also define methods for individual post objects by defining methods
in the `posts` object, the only difference is how they are used (you use `find_recent`
by doing `posts:find_recent()`, but you will use `find_comments` by doing `p:find_comments()`,
where `p` is a particular post object. We will define a method to retrieve all comments of
a post:

<pre>
function posts:find_comments()
   return comments:find_all_by_post_id{ self.id }
end
</pre>

This method uses a predefined method of the `comments` object (which we will create
shortly) that gets all comments with field `post_id` equal to the id of the current
post (`self.id`). This method establishes a relation between posts and comments;
a future version of Orbit's mapper will let you define these declaratively.

Creating the `comments` object is simple:

<pre>
comments = blog:model "comment"
</pre>

Let's just add a convenience method for comments that build the comment's link
from the its data:

<pre>
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
</pre>

The `pages` object is even simpler, the default functionality provided
by Orbit's mapper is enough, so we just create it with `model`:

<pre>
pages = blog:model "pages"
</pre>

This concludes the "model" part of our application. We can now move on
to defining the applications' page flow, by defining *controllers* and
mapping them to URLs.

## Defining Controllers

Controllers are the interface between the web and your application. With Orbit
you can map the path part of your application's URLs (in http://myserver.com/myapp.ws/foo/bar
the path is /foo/bar, for example) to controllers. In Lua terms, an Orbit
controller is a function that receives a request/response object (usually
called `web`) plus parameters extracted from the path, and returns text that
is sent to the client (usually HTML, but can be XML, or even an image).

You map paths to controllers with the `dispatch_get` and `dispatch_post` methods,
for GET and POST requests, respectively. The first parameter to these methods
is the controller, a Lua function, and all the other parameters are *mapping patterns*,
written using Lua's string matching syntax, so one controller can answer to multiple mappings.

Below is the controller for the main page of the blog:

<pre>
function index(web)
   local ps = posts:find_recent()
   local ms = posts:find_months()
   local pgs = pgs or pages:find_all()
   return render_index(web, { posts = ps, months = ms,
			  recent = ps, pages = pgs })
end

blog:dispatch_get(cache(index), "/", "/index") 
</pre>

The last line sets up the mapping between the `index` function and the root
of the application. The call to `cache` sets up page caching for this controller,
using the cache we created earlier (this is another common Lua idiom, functions
as decorators).

The `index` controller shows all recent posts, and is pretty straightforward. It
just fetches the required model data from the database, then calls an auxiliary
function (called a *view* in MVC terminology) to render the actual HTML code.

Another important controller is the one that shows single posts:

<pre>
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
</pre>

Here we map all paths like /post/53 to the `view_post` controller. The pattern
captures the number, and this is passed along to the controller by Orbit. For
/post/53, the controller receives the string "53" as `post_id` and uses this
to fetch the corresponding post. Again, HTML rendering is factored out to
another function, and this controller is cached.

If no post with that id is found then the default controller for missing
pages gets called, `blog.not_found` (`orbit.app` put it in the `blog` namespace).
Orbit also calls this controller is it does not find a valid match for the
path. Another default controller is `server_error`, called when any unprotected
error happens in controller/view code.

Archives and pages are similar in structure:

<pre>
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
</pre>

The archives use the same layout as the index, so they reuse its HTML
generator. Archives also extract two parameters from the path, the month and
the year, so paths are like /archive/2008/05.

Finally, you can also set up Orbit to serve static files with the `dispatch_static`
convenience method:

<pre>
blog:dispatch_static("/head%.jpg", "/style%.css")
</pre>

These are also patterns, so the dots are escaped. You can set up a whole folder in
your application as static with `blog:dispatch_static("/templates/.+")`. Orbit always
looks for the files in your application's folder. Of course you are free to let
your application only handle dynamic content and let your web server serve static
content; `dispatch_static` is just a convenience to have "zero-configuration" applications.

There is one controller left, for adding comments. This one will answer to POST
instead of GET:

<pre>
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
</pre>

The `add_comment` controller first validates the input, delegating
to `view_post` if the comment field is empty (which will show an error
message in the page). You access POST parameters via the `web.input` table,
which is conveniently aliased to an `input` local variable.

The controller creates a new comment object and fills it with data, then
saves it to the database. It also updates the post object to increase the
number of comments the post has by one, and also saves it. It then proceeds
to invalidate (in the cache) all pages that may show this information: the index, the post's page,
and the archives for this particular post. Finally, it redirects to the post's page,
which will show the new comment. This is a common idiom in web programming
called POST-REDIRECT-GET, where every POST is followed by a redirect to a GET.
This avoids double posting in case the user hits reload.

The only thing left now is the HTML generation itself. This is the topic of the next section.

## Views: Generating HTML

Views are the last component of the MVC triad. For Orbit views are just simple functions that
generate content (usually HTML), and are strictly optional, meaning you can return content
directly from the controller. But it's still good programming practice to separate controllers
and views.

How you generate content is up to you: concatenate Lua strings, use `table.concat`, use
a third-party template library... Orbit provides programmatic HTML/XML generation
through `orbit.htmlify`, but you are free to use any method you want. In this tutorial
we will stick with programmatic generation, though, as the other methods (straight
strings, [Cosmo](http://cosmo.luaforge.net), etc.) are thoroughly documented elsewhere.

When you htmlify a function, Orbit changes the function's environment to let you generate
HTML by calling the tags as functions. It's better to show how it works than to explain, so
here is the basic view of the blog application, `layout`:

<pre>
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
</pre>

This view is a decorator for other views, and generates the boilerplate for each
page in the blog (header, footer, sidebar). You can see the HTML-generating functions
througout the code, such as `title`, `html`, `head`, `div`. Each takes either a string
or a table, and generates the corresponding HTML. If you pass a table, the array part
is concatenated and used as the content, while the hash part os used as HTML attributes
for that tag. A tag with no content generates a self-closing tag (`meta` and `link` in
the code above).

Of note in the code above are the calls to `web:static_link` and to the `_menu` and
`_sidebar` functions. The `static_link` method generates a link to a static resource
of the application, stripping out the SCRIPT_NAME from the URL (for example,
if the URL is http://myserver.com/myblog/blog.ws/index it will return /myblog/style.css
as the link).

The `_menu` and `_sidebar` functions are just helper views to generate the blog's menubar
and sidebar:

<pre>
function _menu(web, args)
   local res = { li(a{ href= web:link("/"), strings.home_page_name }) }
   for _, page in pairs(args.pages) do
      res[#res + 1] = li(a{ href = web:link("/page/" .. page.id), page.title })
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
</pre>

Here you see a mixture of standard Lua idioms (filling a table and passing it
to a concatenation function) and Orbit's programmatic HTML. They also use
the `web:link` method, which generates intra-application links. The `sidebar`
function uses a few more convenience functions, for better factoring:

<pre>
function _blogroll(web, blogroll)
   local res = {}
   for _, blog_link in ipairs(blogroll) do
      res[#res + 1] = li(a{ href=blog_link[1], blog_link[2] })
   end
   return ul(res)
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
</pre>

Notice how these functions do not call anything in the model, just using
whichever data was passed by them (all the way from the controller).

We can now get to the main view functions. Let's start with the easiest,
and smallest, one, to render pages:

<pre>
function render_page(web, args)
   return layout(web, args, div.blogentry(markdown(args.page.body)))
end
</pre>

This is a straightforward call to `layout`, passing the body
of the page inside a `div`. The only thing of note
is the `div.blogentry` syntax, which generates a `div` with
a `class` attribute equal to "blogentry", instead of a straight
`div`.

Moving on, we will now write the view for index pages (and archive
pages):

<pre>
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
</pre>

Again we mix Lua with programmatic generation, and factor part
of the output (the HTML for the body of the posts themselves) to another function
(we will be able to reuse this function for the single-post view). The
only unusual piece of logic is to implement fancier dates, the code
only prints a date when it changes, so several posts made in the same
day appear under the same date.

The `_post` helper is pretty straightforward:

<pre>
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
</pre>

Now we can finally move to the piece-de-resistance, the view that renders
single posts, along with their comments, and the "post a comment" form:

<pre>
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
</pre>

This is a lot of code to digest at once, so let's go piece by piece. The first few lines
generate the body of the post, using the `_post` helper. Then we have the list of comments,
again with the body of each comment generated by a helper, `_comment`. In the middle we
have an error message that is generated if the user tried to post an empty comment, and
then the "add a comment" form. A form needs a lot of HTML, so there's quite a lot of
code, but it should be self-explanatory and is pretty basic HTML (making it pretty is
the responsibility of the style sheet).

The `_comment` helper is pretty simple:

<pre>
function _comment(web, comment)
   return { p(comment.body),
      p.posted{
	 strings.written_by .. " " .. comment:make_link(),
	 " " .. strings.on_date .. " " ..
	    time(comment.created_at) 
      }
   }
end
</pre>

Finally, we need to set all of these view functions up for programmatic HTML generation:

<pre>
orbit.htmlify(blog, "layout", "_.+", "render_.+")
</pre>

The `orbit.htmlify` function takes a table and a list of patterns, and sets all
functions in that table with names that match one of the patterns up for
HTML generation. Here we set the `layout` function, all the `render_` functions,
and all the helpers (the functions starting with `_`).

## Deployment

For this part of the tutorial it is better if you go to the `samples/blog` folder
of Orbit's distribution (again, look inside the `rocks` folder if you installed
with Kepler or LuaRocks). An Orbit application is a WSAPI application, so
deployment is very easy, you can just copy all the application's files (`blog.lua`,
`blog_config.lua`, `blog.db`, `head.jpg`, and `style.css`) to a folder in your
web server's docroot (if you installed Kepler, to a folder inside `kepler/htdocs`),
and create a launcher script in this folder. The launcher script is simple (call 
it `blog.ws`):

<pre>
#!/usr/bin/env wsapi.cgi
require "blog"
return blog
</pre>

Depending on your configuration, you might need to install the `luasql-sqlite3` and
`markdown` rocks before running the application. Now just start Xavante, and
point your browser to blog.ws, and you should see the index page of the blog. If you
created a blog.db from scratch you are not going to see any posts, though. The blog
application in `samples/blog' includes a blog.db filled with random posts and comments.

