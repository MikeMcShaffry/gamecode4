
local schema = require "orbit.schema"

-- Puts all the definitions below in blog's namespace
module("blog", package.seeall)

blog_title = "Blog"

cache_path = "page_cache"

copyright_notice = "Copyright 2007 Foobar"

about_blurb = [[This is an example of a blog built using Orbit. You
can browse posts and add comments, but to add new posts you have
to go directly to the database. This will be fixed in the future.]] 

blogroll = {
  { "http://slashdot.org", "Slashdot"},
  { "http://news.google.com", "Google News" },
  { "http://www.wikipedia.org", "Wikipedia" },
}

-- Uncomment this to send static files through X-Sendfile
-- use_xsendfile = true

database = {
--  driver = "mysql",
--  conn_data = { "blog", "root", "password" }
  driver = "sqlite3",
  conn_data = { blog.real_path .. "/blog.db" }
}

blog_schema = schema.loadstring([[
  table_prefix = "blog_"
  post = entity {
    fields = {
      id = key(),
      title = text(),
      body = long_text(),
      n_comments = integer(),
      published_at = timestamp()
    }
  }
  comment = entity {
    fields = {
      id = key(),
      post_id = integer(),
      author = text(),
      email = text(),
      url = text(),
      body = long_text(),
      created_at = timestamp()
    }
  }
  page = entity {
    fields = {
      id = key(),
      title = text(),
      body = long_text(),
    }
  }
]], "@blog_schema.lua")

recent_count = 7

strings = {}

strings.pt = {
  home_page_name = "Página Inicial",
  about_title = "Sobre o Blog",
  last_posts = "Últimos Posts",
  blogroll_title = "Links",
  archive_title = "Arquivo",
  anonymous_author = "Anônimo",
  no_posts = "No há posts publicados.",
  published_at = "Publicado às",
  comments = "Comentários",
  written_by = "Escrito por",
  on_date = "em",
  new_comment = "Novo comentário",
  no_comment = "Você esqueceu o comentário!",
  form_name = "Nome:",
  form_email = "Email:",
  form_url = "Site:",
  italics = "itálico",
  bold = "negrito",
  link = "link",
  send = "Enviar"
}

strings.en = {
  home_page_name = "Home Page",
  about_title = "About this Blog",
  last_posts = "Recent Posts",
  blogroll_title = "Links",
  archive_title = "Archives",
  anonymous_author = "Anonymous",
  no_posts = "No published posts.",
  published_at = "Published at",
  comments = "Comments",
  written_by = "Written by",
  on_date = "at",
  new_comment = "New comment",
  no_comment = "You forgot the comment!",
  form_name = "Name:",
  form_email = "Email:",
  form_url = "Site:",
  italics = "italics",
  bold = "bold",
  link = "link",
  send = "Send"
}

language = "en"

strings = strings[language]

months = {}

months.pt = { "Janeiro", "Fevereiro", "Março", "Abril",
    "Maio", "Junho", "Julho", "Agosto", "Setembro", "Outubro",
    "Novembro", "Dezembro" }

months.en = { "January", "February", "March", "April",
    "May", "June", "July", "August", "September", "October",
    "November", "December" }

weekdays = {}

weekdays.pt = { "Domingo", "Segunda", "Terça", "Quarta",
    "Quinta", "Sexta", "Sábado" }

weekdays.en = { "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday" }

-- Utility functions

time = {}
date = {}
month = {}

local datetime_mt = { __call = function (tab, date) return tab[language](date) end }

setmetatable(time, datetime_mt)
setmetatable(date, datetime_mt)
setmetatable(month, datetime_mt)

function time.pt(date)
  local time = os.date("%H:%M", date)
  date = os.date("*t", date)
  return date.day .. " de "
    .. months.pt[date.month] .. " de " .. date.year .. " às " .. time
end

function date.pt(date)
  date = os.date("*t", date)
  return weekdays.pt[date.wday] .. ", " .. date.day .. " de "
    .. months.pt[date.month] .. " de " .. date.year
end

function month.pt(month)
  return months.pt[month.month] .. " de " .. month.year
end

local function ordinalize(number)
  if number == 1 then
    return "1st"
  elseif number == 2 then
    return "2nd"
  elseif number == 3 then
    return "3rd"
  else
    return tostring(number) .. "th"
  end
end

function time.en(date)
  local time = os.date("%H:%M", date)
  date = os.date("*t", date)
  return months.en[date.month] .. " " .. ordinalize(date.day) .. " " ..
     date.year .. " at " .. time
end

function date.en(date)
  date = os.date("*t", date)
  return weekdays.en[date.wday] .. ", " .. months.en[date.month] .. " " ..
     ordinalize(date.day) .. " " .. date.year 
end

function month.en(month)
  return months.en[month.month] .. " " .. month.year
end

