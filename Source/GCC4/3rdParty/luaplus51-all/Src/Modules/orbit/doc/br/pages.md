## Orbit Pages 

Orbit Pages é um ambiente de programação similar ao PHP, construido sobre o Orbit.
As Orbit Pages são páginas HTML (mas que utilizam a extensão .op em uma instalação do Orbit típica)
que são convertidas dinamicamente em uma aplicação Orbit.
Elas são inicializadas pelos disparadores op.cgi, op.fcg e ophandler.lua,
para FastCGI e Xavante respectivamente. Uma instalação Kepler padrão,
inclui suporte para Orbit Pages por default. 

Uma Orbit Page é também um template que utiliza a linguagem de template
[Cosmo](http://cosmo.luaforge.net). O ambiente deste template é uma *sandbox*
que envolve o ambiente global e é recriado cada vez que é solicitado.
A principal variável deste ambiente é a variável `web`, que é um objeto request/response do Orbit. 

Outras variáveis importantes:

**mapper** - uma instância default do Orbit ORM 

**model(*name*, *dao*)** - é o mesmo que mapper:new(*name*, *dao*), salvo quando *name* 
é uma tabela, neste caso, chama mapper:new(*name[1]*, *name[2]*),
logo você pode utilizar isso no template como `$model{ name, dao }` 

**app** - é o ambiente global da aplicação, que deve ser usado como um cache de sessão
(para conexões de dados, por exemplo) para disparadores persistentes 

**finish(*res*)** - suspende a execução da página atual, e envia *res* como resposta
**no lugar** do conteúdo da página 

**redirect(*target*)** - o mesmo que web:redirect(*target*) seguido por finish(). Se *target* 
for uma tabela, equivale a web:redirect(*target[1]*), então, você pode usar isto no template
como `$redirect{ target }` 

**include(*page*, [*env*])** - avalia o Orbit Page no arquivo *page*
(relativo ao path da página atual), com a opção de utilizar as variaveis em *env*
no ambiente do template. Também pode ser utilizado no template como `$include{ page, env }` 

**forward(*page*, [*env*])** - interrompe a execução da pagina atual,
avalia e envia a página no arquivo *page* no lugar; de outra forma o mesmo que **include** 

Também existem outras poucas variaveis que deveriam ser usadas somente no template:

**$lua{ *code* }** - roda *code* no mesmo ambiente que a página,
logo *code* pode mudar as variáveis do template e até mesmo definir novas 

**$if{ *condition* }[[ *then-part* ]],[[ *else-part* ]]** - se *condition* for verdade,
será subistituida pelo resultado da avaliação do template do *then-part*,
de outra forma *else-part*. *else-part* é opcional, sendo seu valor padrão definido como vazio 


**$fill{ ... }[[ *template* ]]** - substituída pela avaliação do *template* utilizando
o ambiente passado para preencher (*template* **não** herda as variáveis da página) 

Abaixo vemos uma Orbit Page extremamente simples que mostra a maioria dos conceitos acima,
(incluindo o Cosmo, veja a documentação do Cosmo para maiores detalhes): 

<pre>
#!/usr/bin/env op.cgi 
<html> 
<body> 
<p>Hello Orbit!</p> 
<p>I am in $web|real_path, and the script is 
$web|script_name.</p> 
$lua{[[ 
if not web.input.msg then 
web.input.msg = "nothing" 
end 
]]} 
<p>You passed: $web|input|msg.</p> 
$include{ "bar.op" } 
</body> 
</html> 
</pre>

A página `bar.op` que ela inclui é essa: 

<pre>
#!/usr/bin/env op.cgi 
<p>This is bar, and you passed $web|input|msg!</p> 
</pre>

A distribuição do [Kepler](http://www.keplerproject.org) tem exemplos mais completos que possuem
acesso ao banco de dados, POST, e até mesmo de alguns AJAX simples.