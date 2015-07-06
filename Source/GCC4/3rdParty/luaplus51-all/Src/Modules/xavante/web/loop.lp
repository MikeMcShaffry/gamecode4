<html>
<head>
  <title>CGILua loop test</title>
</head>
<body>
<p>Executes a long loop, used by concurrency tests</p>
<% for i = 1, 30000 do %>
i =  $|i|$<br/>
<% end %>
</body>
</html>
