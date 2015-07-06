##Apresentação

O Orbit é um framework MVC web para a linguagem Lua. O design é ispirado por framework leves em Ruby como o [Camping](http://code.whytheluckystiff.net/camping/). Abandona completamente o modelo atual de CGILua que usa "scripts" em favor de aplicações, onde cada aplicação do Orbit pode caber em um único arquivo, mas se você quiser, pode dividi-lo em múltiplos arquivos. Todos os aplicativos do Orbit seguem o protocolo [WSAPI](http://wsapi.luaforge.net), por tanto atualmente trabalha com Xavante, CGI e Fastcgi. Isto inclui um disparador que torna mais fácil iniciar uma instância do Xavante para o desenvolvimento. 

##Histórico

* Versão 2.0.1: consertado o bug na função redirect nas Orbit pages (agradecimentos a Ignacio Burgueño por ter encontrado o bug )

* Versão 2.0: Revisão completa do Orbit

* Versão 1.0: Versão inicial, obsoleta

## Hello World

Segue uma aplicação muito simples do Orbit:

<pre>
#!/usr/bin/env wsapi.cgi

require"orbit"

-- As aplicações do Orbit normalmente são módulos,
-- o orbit.new faz as inicializações necessárias

module("hello", package.seeall, orbit.new)

-- Esses são os controladores, cada um recebe um objeto web correspondente 
-- ao request/response, mais qualquer capturas extras dos
-- padrões de dispatch. O controlador envia qualquer cabeçalho extra e/ou
-- o status, caso não seja 200, então retona a resposta. 
-- É recomendado delegar a geração das respostas para um função
-- de visualização.

function index(web)
return render_index()
end

function say(web, name)
return render_say(web, name)
end

-- Constrói a tabela de dispatch das aplicações, você pode 
-- passar multiplos padrões, e todas as capturas são repassadas 
-- ao controlador

hello:dispatch_get(index, "/", "/index")
hello:dispatch_get(say, "/say/(%a+)")

-- Essas são funções de visualização as quais os controladores fazem refência.
-- orbit.htmlify percorre as funções na tabela passada como primeiro parâmetro
-- e tenta combinar seus nomes com os padrões passados (usando um ^ e um $
-- implícitos ao redor do padrão). Cada função que corresponde ao padrão recebe
-- um ambiente onde funções de geração de HTML são geradas por demanda.
-- Estas funções aceitam nil (tags vazias), uma string (texto presente entre
-- a tag de abertura e a de fechamento), ou uma tabela com atributos e uma lista
-- de strings que correspondem ao texto. A indexação das funções adiciona um atributo
-- de classe ao tag. Funções são cacheadas.

-- Função de apoio para as partes comuns de uma página

function render_layout(inner_html)
return html{
head{ title"Hello" },
body{ inner_html }
}
end

function render_hello()
return p.hello"Hello World!"
end
function render_index()
return render_layout(render_hello())
end

function render_say(web, name)
return render_layout(render_hello() .. 
p.hello((web.input.greeting or "Hello ") .. name .. "!"))
end

orbit.htmlify(hello, "render_.+")

return _M
</pre>

O exemplo utiliza a geração de HTML nativa do Orbit, mas sinta-se a vontade para utilizar qualquer método de geração de HTML. 
Por exemplo, um dos exemplos de aplicação do Orbit utiliza a biblioteca de template [Cosmo](http://cosmo.luaforge.net)

## Mapeamento OR

O Orbit inclui um mapeador objeto-relacional básico que no momento funciona somente com  drivers
SQLite3 e MySQL do [LuaSQL's](http://luaforge.net/projects/luasql). O mapeador fornece um método dinâmico de busca 'como o ActiveRecord de Rails (find\_by\_field1\_and\_field2{val1, val2}), bem como templates para condições (find_by("field1 = ? or field1 = ?", { val1, val2 })). A aplicação de exemplo utiliza esse mapeador.

Um bom efeito collateral do modelo de aplicação do Orbit é que nós conseguimos um "console de aplicações" de graça. No caso do blog, por exemplo, podemos adicionar uma nova postagem usando:

<pre>
$ lua -l luarocks.require -i blog.lua
> p = blog.posts:new()
> p.title = "Novo post"
> p.body = "Este é um novo post. Use *Markdown* se preferir."
> p.published_at = os.time()
> p:save()
</pre>

Você também pode atualizar ou deletar qualquer itens do modelo direto do seu console, bastanto obte-los do banco de dados, mudar o que você quer e chamar `save()`(ou `delete()` se você quiser remove-lo).

## Download e instalação

A forma mais fácil de baixar e instalar o Orbit é atraves do [LuaRocks](http://luarocks.org). Você pode instalar o Orbit com o simples comando `luarocks install orbit` Vá para o path onde o LuaRocks coloca o Orbit para ver as aplicações e esta documentação. O Luarocks obtem e instala qualquer qualquer dependência que você ainda não tenha.

Você também pode obter o Orbit no site do [LuaForge](http://luaforge.net/projects/orbit). A instalação em sistemas Unix e similares é "configure && make && make install", mas você pode ter que instalar algumas dependências sozinho (como o WSAPI e o Xavante).

## Créditos

O Orbit foi projetado e desenvolvido por Fabio Mascarenhas and André Carregal e é mantido por Fabio Mascarenhas. Orbit foi patrocinado pela Fábrica Digital, FINEP e SEBRAE

## Fale conosco

Para maiores informações, por favor [fale conosco] (mailto:info-NO-SPAM-THANKS@keplerproject.org). Comentários são bem-vindos.

Você também pode nos contactar e a outros desenvolvedores e usuários na [lista de discussão](http://luaforge.net/mail/?group_id=104) do projeto Kepler.


