#!/usr/bin/env wsapi.cgi

local orbit = require"orbit"

local songs = orbit.new()

songs.config = {
  theme = {
    name = "default", config = {}
  }
}

songs.templates = {
  ["/default/layout.html"] = [=[
    <html>
      <head><title>Song List</title>
      <body>
        $inner
      </body>
    </html>
  ]=],
  ["/default/index.html"] = [=[
    <h1>Songs</h1>
    <table>
      $songs[[<tr><td>$it</td></tr>]]
    </table>
  ]=],
}

songs.song_list = {
  "Sgt. Pepper's Lonely Hearts Club Band",
  "With a Little Help from My Friends",
  "Lucy in the Sky with Diamonds",
  "Getting Better",
  "Fixing a Hole",
  "She's Leaving Home",
  "Being for the Benefit of Mr. Kite!",
  "Within You Without You",
  "When I'm Sixty-Four",
  "Lovely Rita",
  "Good Morning Good Morning",
  "Sgt. Pepper's Lonely Hearts Club Band (Reprise)",
  "A Day in the Life"
}

function songs:index(req, res)
  res:write(self:layout(req, res,
                        self.theme:load("index.html"):render(req, res,
                                                             { songs = self.song_list })))
end

-- Builds the application's dispatch table

songs.routes = {
  { pattern = "/", name = "index", method = "get" },
}

return songs:load()
