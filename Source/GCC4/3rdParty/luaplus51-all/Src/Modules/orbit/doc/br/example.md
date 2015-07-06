## Tutorial Orbit

Este tutorial mostra como criar uma aplicação blog simples, apoiada por uma base de dados. Ela é muito simples porque não inclui nenhuma página de administração ("admin"); você tem que adicionar posts diretamente na base de dados (embora você possa postar através de um console Lua, e este tutorial mostrará como), mas tem uma interface para comentários nos posts.

O tutorial parte do princípio que você já tem o Orbit instalado (preferencialmente como parte do [Kepler](http://www.keplerproject.org), ou pelo [LuaRocks](http://luarocks.org), e já tem um servidor web que aceita configuração WSAPI (o servidor web Xavante que vem com o Kepler é uma boa escolha).

Os códigos fontes para este blog estão nas pastas ´samples´ da distribuição do Orbit. Se você instalou o Orbit pelo Kepler ou LuaRocks, veja dentro da pasta ´rocks´ da sua instalação.

## Inicialização

Você deve criar um arquivo ´blog.lua´, que será o arquivo principal de códigos fontes da nossa aplicação. A primeira coisa que você deve por no arquivo é o código para carregar o Orbit e outras bibliotecas que você usará no seu aplicativo.

<pre> 
require "orbit" 
require "orbit.cache" 
require "markdown" 
</pre> 

Neste exemplo usaremos a página de  cache do Orbit, e o parser Markdown para marcar os posts.

Todas as aplicações Orbit são módulos Lua, portanto incluiremos esta linha:

<pre> 
module("blog", package.seeall, orbit.app) 
</pre> 

Isso configura o módulo ´blog´ e o inicializa como uma aplicação Orbit.

´orbit.app´ coloca muitas coisas no namespace do módulo ´blog´. Os mais importantes são os metódos ´dispatch_get´, ´dispatch_post´ e ´model´ que permitem que você defina a funcionalidade principal da sua aplicação. Eles também definem a variável ´mapper´ que o Orbit usa para criar os modelos (Orbit inicializa essa variável para o seu mapeador objeto-relacional padrão). Por último, eles também definem os controles padrões para os códigos de erros 404 e 500 HTTP como as variáveis ´not_found´ e ´server_error´, respectivamente. Redefina essas variáveis se quiser páginas customizadas para a sua aplicação.

Vamos carregar um script de configuração para o blog (um modelo comum em aplicações). Você pode pegar este script [aqui](blog_config.lua).

<pre> 
require "blog_config" 
</pre> 

As próximas linhas carregam um driver de base de dados LuaSQL (definido na configuração), e configura o mapeador objeto relacional do Orbit.

<pre> 
require("luasql." .. database.driver) 
local env = luasql[database.driver]() 
mapper.conn = env:connect(unpack(database.conn_data)) 
mapper.driver = database.driver 
</pre> 

O mapeador do Orbit precisa usar uma conexão de base de dados, e de qual driver você estiver usando (no momento apenas o "sqlite3" e "mysql" são aceitos).

Você precisa iniciar o mapeador antes de criar os modelos de sua aplicação porque o mapeador do Orbit consulta a base de dados durante a criação de modelos para pegar o esquema. Falando em esquema, agora é uma boa hora para criar a base de dados do seu blog. Parto do princípio que você está usando o SQLite3. Crie uma base de dados ´blog.db´ com o script SQL abaixo:

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

O mapeador do Orbit usa o campo ´id´ para identificar objetos na base de dados, portanto você precisará de um para cada um dos tipos de objetos que estiver mapeando.

Por último, vamos iniciar o cache de páginas do Orbit antes de criar nossos modelos:

<pre> 
local cache = orbit.cache.new(blog, cache_path) 
</pre> 

O cache de páginas acelera o acesso a qualquer página que você cacheie, mas você precisará ser cuidadoso e limpar o cache para uma página quando qualquer conteúdo nela mudar. Veremos como cachear e invalidar páginas na seção de controle deste tutorial.

## Criando Modelos

Nossa aplicação de blog tem três tipos de objetos: posts, comentários e páginas "estáticas" (como a página de "Sobre" do blog, por exemplo). Não é coincidência que também temos três tipos de tabelas na base de dados, cada tabela mapeia um tipo de objeto que a nossa aplicação reconhece, e para cada tipo criaremos um modelo. Primeiro criaremos um objeto modelo para posts:

<pre> 
posts = blog:model "post" 
</pre> 

O parâmetro para o método ´model´ é o nome de uma tabela na base de dados. O objeto ´posts´ que esse método cria representa a coleção de posts, e ao mesmo tempo é um protótipo para todos os posts (veremos as implicação disso em breve). O mapeador do Orbit cria um objeto funcional por conta própria: você pode fazer ´post:find(3)´, por exemplo, e pegar o post com ´id´ 3, ou ´post:find_all("n_comments < ?", { 3, order = "published_at desc"})´ e ter uma lista de todos os posts com menos de três comentários, do mais recente ao mais antigo.

Você pode usar o método ´find´ pré-definido para todas as buscas na base de dados, mas ajuda simplificar buscas comuns nos seus métodos. Você pode fazer isso adicionando métodos no objeto ´posts´:

<pre> 
function posts:find_recent() 
return self:find_all("published_at is not null", 
{ order = "published_at desc", 
count = recent_count }) 
end 
</pre> 

As linhas acima adicionam um método ´find_recent´ no objeto ´posts´, retornando uma lista dos posts publicados mais recentementes (o número está no script de configuração), do mais recente ao mais antigo. A aplicação irá usar este método para gerar a lista de posts na home page, assim como a seção "Posts recentes" na lateral do blog.

Outra característica do nosso blog será a página de arquivo que mostra todos os posts de um certo mês e ano. Definiremos um método para isto também:

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

Este é o método mais complicado, já que temos que converter de um mês e ano simples para data de começo e fim no formato Lua padrão. Por último, também definiremos um método para retornar todos os meses (e anos) que tem posts, para mais tarde gerar os links para a seção "Arquivo" no sidebar:

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

Este método pega todos os posts na base de dados, ordenados por data, e iterates over them armazenando cada par de mês e ano numa lista.

We can also define methods for individual post objects by defining methods 
in the `posts` object, the only difference is how they are used (you use `find_recent` 
by doing `posts:find_recent()`, but you will use `find_comments` by doing `p:find_comments()`, 
where `p` is a particular post object. We will define a method to retrieve all comments of 
a post: 

Também podemos definir métodos para objetos de posts individuais definindo métodos no objeto ´posts´, a única diferença é como eles são usados (você usa ´find_recent´ criando ´posts:find_recent()´, mas você usará ´find_comments´ criando ´p:find_comments()´, onde ´p´ é um objeto post específico. Definiremos um método para recuperar todos os comentários de um post:

<pre> 
function posts:find_comments() 
return comments:find_all_by_post_id{ self.id } 
end 
</pre> 

Este método usa um método pré-definido do objeto ´comments´ (que criaremos em breve) que pega todos os comentários com o campo ´post_id´ iguais ao id do post atual (´self.id´). Este método cria uma relação entre os posts e os comentários; uma versão futura do mapeador Orbit permitirá que você defina isso declaradamente.

Criar o objeto ´comments´ é simples:

<pre> 
comments = blog:model "comment" 
</pre> 

Vamos adicionar um método de conveniência para comentários que constróem o link de comentário a partir de seus dados:

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

O objeto ´pages´ é mais simples ainda, a funcionalidade padrão fornecida pelo mapeador Orbit é suficiente, então criaremos com o ´model´:

<pre> 
pages = blog:model "pages" 
</pre> 

Isso conclui a parte "model" da nossa aplicação. Agora podemos seguir para definir a flow página da aplicação, definindo *controllers* e mapeando-os para URLs.

## Definindo controladores

Controladores são a interface entre a web e sua aplicação. Com o Orbit você pode 
mapear a parte path das URLs de sua aplicação (pore exemplo, em http://myserver.com/myapp.ws/foo/bar 
o path é /foo/bar) em controladores. Em termos Lua, um controlador Orbit é uma função que recebe 
um objeto request/response (chamado `web`) e parâmetros obtidos do path, e retorna um texto 
que é enviado para o cliente (geralmente HTML, mas pode ser XML ou mesmo uma imagem).

Você mapeia paths em controladores com os métodos `dispatch_get` e `dispatch_post` 
para requisições GET e POST respectivamente. O primeiro parâmetro destes métodos é o 
controlador, uma função Lua, e todos os outros parâmetros são os *padrões de mapeamento*, 
escritos na sintaxe de padrões de strings de Lua, de forma que um controlador pode responder 
a mapeamentos diversos.

Abaixo esta o controlador para a pagina principal do blog:

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

A última linha estabelece o mapeamento entre a função `index` e root da aplicação.
O chamado do `cache` define o *caching* para esse controlador,
usando o *cache* que criamos anteriormente (este é outro idioma comum do Lua,
funciona como"decorators").

O controlador `index`mostra todos os posts recentes, e é bem direto. Ele somente chama os dados modelo solicitados apartir do banco de dados, depois chama uma função auxiliar (chamada *view* na tecnologia MVC) para  renderizar o código HTML atual.

Outro importante controlador é o que mostra posts únicos:

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

Aqui nós mapeamos todos os paths como /post/53 para o controlador `view_post`. O pattern captura os números, e é passado por todo controlador pelo Orbit. Para /post/53, o controlador recebe a string ''53'' como `post_id` e usa isto para chamar o post correspondente. Novamente, a renderização do HTML esta factored out para outra função, e este controlador esta cached. 

Se nenhum post com este id for encontrado, o controlador default de páginas perdidas será chamado `blog.not_found` (`orbit.app` coloque isto no  namespace do `blog`) 

Arquivos e paginas tem estruturas similares:


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

Os arquivos utilizam o mesmo *layout* que o index, logo ele reutiliza seu gerador de HTML. Os arquivos também extraem dois parametros do *path*, o mês e o ano, logo os *paths* são como /archive/2008/05. 

Finalmente você pode também pode definir arquivos estáticos com o método de conveniência `dispatch_static`.

<pre> 
blog:dispatch_static("/head%.jpg", "/style%.css") 
</pre> 

Esses também são *patterns*, logo os pontos são escapados. Você pode definir em sua aplicação, uma pasta como estática com `blog:dispatch_static("/templates/.+")`. O Orbit sempre procura pelos arquivos nas pastas das aplicações. Claro que você pode deixar sua aplicação comportar somente conteúdos dinâmicos e deixar seu servidor web servir conteúdo estátic; `dispatch_static` é só uma conveniência para ter aplicações "zero-configuration" 

Tem um controlador para adicionar comentários. Este irá responder ao POST em vez de receber:

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

O controlador `add_comment` primeiro valida o input, delegando ao `view_post` se o campo de comentário estiver vazio (o qual irá mostrar uma menssagem de erro na página). Você acessa o parâmetro POST pela tabela `web.input`, que é convenientemente  aliased para um `input` local variável.  

O controlador cria um novo objeto comment, preenche com dados e depois salva no banco de dados. Ele também atualiza o objeto post para aumentar o número de *comment* o post tem por um, alem de salvar. Depois continua para invalidar (em *cache*) todas as páginas que talvez mostre esta informação: o index, a página de postagem e os arquivos para este post em particular. Finalmente, ele redireciona para a página de postagem, que irá mostrar o novo comentário. Este é um idioma comum na programação web chamada POST-REDIRECT-GET, onde todo POST é seguido por um redirecionamente para um GET. Isso evita a dupla postagem no caso do usuário carregar a página novamente. 

A única coisa que resta agora é a geração de HTML. Está é um tópico da próxima sessão. 

## Visualizações: Gerando HTML

Visualizações é o último componente do trio MVC. Para o Orbit, visualizações são funções simples que geram conteúdo (geralmente HTML), e são estritamente opcionais, o que significa que você pode devolver conteúdo diretamente do seu controle. Mas ainda é bom ter prática em programação para separar controles e visualizações.

Como você gera conteúdo é escolha sua: concatene correntes Lua, use ´table.concat´, use um template de biblioteca de terceiros... Orbit fornece geração de HTML/XML programático através de órbit.htmlify´, mas você está livre para usar qualquer método que preferir. Neste tutorial manteremos a geração programática, embora, assim como outros métodos (strings retas, [Cosmo](http://cosmo.luaforge.net), etc.) são inteiramente documentadas em outro lugar.

Quando você htmlify uma função, o Orbit muda o ambiente da função permitindo que você gere HTML chamando as tags de funções. É melhor mostrar como funciona do que explicar, então aqui vai a visualização básica da aplicação do blog, ´layout´:

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

Esta visualização é um decorador para outras visualizações, e gera o boilerplate para cada página do blog (cabeçalho, rodapé, sidebar). Você pode ver as funções de gerador HTML por todo o código, como ´title´, ´html´, ´head´, ´div´. Cada um tem ou uma string ou uma tabela, e gera o HTML correspondente. Se você dispensar uma tabela, a parte de banco de dados é concatenada e usada como conteúdo, enquanto a parte hash é usada como atributos HTML para aquela tag. Uma tag sem conteúdo gera um tag self-closing (´meta´ e ´link´ no código acima).

Digno de nota no código acima são as chamadas ´web:static_link´ e as funções ´_menu´ e ´_sidebar´.O método ´static_link´ gera um link para um recurso estático da aplicação, tirando o SCRIPT_NAME da URL (por exemplo, se a URL é http://myserver.com/myblog/blog.ws/index irá voltar como /myblog/style.css como o link).

As funções ´_menu´ e ´_sidebar´ são apenas visualizações de ajuda para gerar a barra de menu e sidebar do blog:

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

Aqui você vê uma mistura de idiomas básicos do Lua (preenchendo uma tabela e passando para uma função concatenada) e o HTML programático do Orbit. Eles também usam o método ´web:link´, que gera links intra-aplicação. A função ´sidebar´ usa mais funções de conveniência, para fatorar melhor:



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

Note como essas funções não chamam nada no modelo, apenas usam qualquer dado que foi passado (desde o controle).

Agora podemos ir para as funções. de visualização principal. Começaremos com a mais fácil e menor, para páginas renderizadas.

<pre> 
function render_page(web, args) 
return layout(web, args, div.blogentry(markdown(args.page.body))) 
end 
</pre> 

Isto é uma chamada direta para o ´layout´, passando o corpo da página dentro de um ´div´. A única coisa importante é a sintaxe ´div.blogentryp´, que gera um ´div´ com um atributo ´class´ igual ao "blogentry", ao invés de um ´div´ direto.

Seguindo em frente, escreveremos a visualização para páginas index (e páginas de arquivo):

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

Novamente misturamos Lua com gerador programático, e parte fatoral do emissor (o próprio HTML para o corpo dos posts) para outra função (poderemos reutilizar esta função para visualização de apenas um post). O único pedaço incomum de lógica é o de implementar datas especiais, o código só publica quando a data muda, portanto muitos posts do mesmo dia aparecem com a mesma data.

A ajuda do ´_post´ é bem simples:

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

Agora podemos seguir para a piece-de-resistance, a visualização que renderiza posts únicos, junto com seus comentários, e o formulário "post a comment":

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

São muitos códigos para se digerir de uma vez, então vamos aos poucos. As primeiras linhas geram o corpo do post, usando a ajuda ´_post´. Depois temos a lista de comentários, novamente com o corpo de cada comentário gerado por uma ajuda ´_comment´. No meio temos uma mensagem de erro que é gerada se o usuário tentar postar um comentário vazio, e então o formulário "add a comment". Um formulário precisa de muito HTML, então tem bastante código, mas é um HTML bem básico e auto-explicatório (torná-lo bonito é responsabilidade do style sheet).

A ajuda ´_comment´ é bem simples:

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

Por último, precisamos configurar todas essas funções de visualização para gerador programático de HTML:

<pre> 
orbit.htmlify(blog, "layout", "_.+", "render_.+") 
</pre> 

A função ´orbit.htmlify´ pega uma tabela e uma lista de modelos, e configura todas as funções nessa tabela com nomes que casem com um dos modelos para gerar HTML. Aqui configuraremos a função ´layout´, todas as funções ´render_´, e todas as ajudas (as funções começando com ´_´´).

## Distribuição

Para esta parte do tutorial é melhor você utilizar o diretório `samples/blog` da distribuição Orbit (novamente, procure no diretório `rocks` caso você tenha instalado o Orbit via Kepler ou LuaRocks). Uma aplicação Orbit é uma aplicação WSAPI, portanto a distribuição é bastante simples, basta copiar todos os arquivos (`blog.lua`, `blog_config.lua`, `blog.db`, `head.jpg`, e `style.css`) para um diretório em sua raiz web (se você instalou Kepler, este seria o diretório `kepler/htdocs`), e criar um script disparador neste diretório. O script disparador é bem curto (chame-o de `blog.ws`):

<pre> 
#!/usr/bin/env wsapi.cgi 
require "blog" 
return blog 
</pre> 

<pre> 
#!/usr/bin/env wsapi.cgi 
require "blog" 
return blog 
</pre> 

Dependendo de sua configuração, você pode ter que instalar os rocks `luasql-sqlite3` e `markdown` antes de executar sua aplicação. Feito isso basta iniciar o Xavante, apontar o seu browser para blog.ws, e você deve ver a página inicial do blog. Se você criou um arquivo blog.db a partir do zero você não verá nenhum post. A aplicação de blog em `samples/blog' inclui um arquivo blog.db já contendo posts e comentários de exemplo. 