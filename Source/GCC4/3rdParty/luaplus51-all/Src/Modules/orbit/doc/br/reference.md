## Manual de Referência 

Este é um pequeno manual de refência dos métodos do Orbit e sua aplicações. 

## Módulo `orbit` 

**orbit.new([*app*])** - cria uma nova aplicação Orbit, retornando esta (como um módulo).
Se *app* for uma string este é o nome da aplicação e define o campo\_NAME.
Se *app* for uma tabela, esta será usada no lugar de uma tabela vazia.
Isso quer dizer que você pode passar `orbit.new` para a função `module` 

**orbit.htmlify(*func*[, *func*...])** - modifica o ambiente de *func* para incluir funções que gerem HTML 

## Aplicações Orbit 

**app.run(*wsapi\_env*)** - ponto de entrada WSAPI para aplicações, gerado pelo Orbit 

**app.real\_path** - A raiz da aplicação no sistema de arquivos,
por default é o path inferido pelo disparador WSAPI (`wsapi.app\_path`), mas pode ser redefinido 

**app.mapper** - mapeador usado pelo `app:model`, por default é uma instância do `orbit.model`,
mas pode ser invalidado 

**app.not\_found** - tratador padrão, envia uma respota 404 para o cliente, pode ser redefinido.
O handler recebe um objeto `web` 

**app.server\_error** - tratador de erro, envia uma resposta 500 com os detalhes de stack para o cliente,
pode ser redefinido. O tratador recebe um objeto `web`

**app:dispatch\_get(*func*, *patt*[, *patt*...])** - instala a função func* como um tratador de GET
para os padrões *patt*. *func* recebe um objeto `web` e capturas

**app:dispatch\_post(*func*, *patt*[, *patt*...])** - instala *func* como um tratador de POST
para os padrões *patt*. *func* recebe um objeto `web` e capturas 

**app:dispatch\_wsapi(*func*, *patt*[, *patt*...])** - instala *func* como um tratador WSAPI
para os padrões *patt*. *func* recebe um objeto `web` e capturas 

**app:dispatch\_static(*patt*[, *patt*...])** - instala um tratador de arquivos estáticos
para os padrões *patt*. Este tratador assume que PATH\_INFO é um arquivo relativo a
`app.real_path` e o envia para o cliente. O tipo MIME é detectado pela extensão
(sendo o padrão application/octec-stream). 

**app:serve\_static(*web*, *filename*)** - retorna o conteúdo do arquivo *filename*
(que pode estar em qualquer lugar do sistema), e define os cabeçalhos apropriados
de acordo com o tipo MIME do arquivo 

**app:htmlify(*patt*[, *patt*...])** - o mesmo que `orbit.htmlify`,
mas altera todas funções do módulo da aplicação que correspondem aos padrões *patt* 

**app:model(...)** - chama `app.mapper:new(...)`,
de forma que o comportamento depende do mapeador que você estiver usando 

## Métodos `web' 

Os objetos *web* herdam as funções do módulo `wsapi.util` como métodos 

**web.status** - status para ser enviado para o servidor (padrão: "200 Ok") 

**web.headers** - cabeçalhos para serem enviados para o servidor,
uma tabela Lua (que por padrão tem Content-Type como text/html) 

**web.response** - corpo a ser enviado para o cliente (a princípio vazio) 

**web.vars** - ambiente WSAPI original 

**web.prefix** - prefixo da aplicação (se determinado no módulo da aplicação) ou SCRIPT\_NAME 

**web.suffix** - sufixo da aplicação (se determinado no módulo da aplicação) 

**web.real\_path** - localização da aplicação no sistema, obtido a partir de wsapi\_env.APP\_PATH,
ou do real\_path do módulo, ou "." 

**web.doc\_root, web.path\_info, web.script\_name, web.path\_translated, web.method** -
raiz de documentos do servidor, PATH\_INFO, SCRIPT\_NAME, PATH\_TRANSLATED,
e REQUEST\_METHOD (convertido para minúsculas)

**web.GET, web.POST** - variáveis GET and POST 

**web.input** - união de web.GET e web.POST

**web.cookies** - cookies enviados pelo browser

**web:set\_cookie(*name*, *value*)** - define um cookie a ser enviado de volta ao browser 

**web:delete\_cookie(*name*)** - apaga um cookie 

**web:redirect(*url*)** - define o status e cabeçalhos e redireciona o cliente para *url* 

**web:link(*part*, [*params*])** - cria um link interno da aplicação,
utilizando web.prefix e web.suffix, e codificando *params* como uma query string 

**web:static\_link(*part*)** - se o ponto de entrada de uma aplicação é um script,
ao invés de um path, cria um link para o vpath da aplicação
(por exemplo, se o app.prefix for /foo/app.ws, cria um link para /foo/*part*),
caso contrário equivale a web:link 

**web:empty(*s*)** - retorna true se *s* for nil ou uma string vazia (com zero ou mais espaços) 

**web:empty\_param(*name*)** - retorna true se os parâmetros de entrada forem vazios (como web:empty) 

**web:page(*name*, [*env*])** - carrega e trata uma página Orbit de nome *name*.
Se *name* se inicia com / a página é relativa à raiz de documentos,
caso contrário é relativa ao path da aplicação. Retorna a página tratada.
*env* é um ambiente opcional com variáveis extra 

**web:page_inline(*contents*, [*env*])** - trata uma página Orbit embutida 

## Módulo `orbit.cache`

**orbit.cache.new(*app*, [*base\_path*])** - cria um cache de páginas para a aplicação *app*,
em memória ou no sistema de arquivos como *base\_path* (*não* relativo ao path da aplicação!),
retorna o objeto de cache 

**a\_cache(*handler*)** - torna o tratador *handler* cacheado, retornando um novo tratador;
utiliza o PATH\_INFO como chave do cache 

**a\_cache:get(*key*)** - obtém o valor armazenado na chave *key*;
geralmente não utilizada, use a função anterior 

**a\_cache:set(*key*, *val*)** - armazena um valor no cache;
use a\_cache(*handler*) para encapsular este comportamento 

**a\_cache:invalidate(*key*)** - invalida um valor do cache 

**a\_cache:nuke()** - esvazia o cache 

## Módulo `orbit.model`

**orbit.model.new([*table\_prefix*], [*conn*], [*driver*])** - cria um novo mapeamento ORM (objeto relacional).
*table\_prefix* (padrão "") é uma string adicionada ao início dos nomes de modelos para obter nomes de tabelas;
*conn* é a conexão de banco de dados (pode ser definida posteriormente);
*driver* é o tipo de banco de dados (atualmente "sqlite3", o padrão, ou "mysql").
Retorna uma instância do mapeador e todos os parâmetros podem ser definidos
após a criação desta instância (utilizando a\_mapper.table\_prefix, a\_mapper.conn e a\_mapper.driver)

**a\_mapper:new(*name*, [*tab*])** - cria um novo objeto de modelo;
*name* é usado junto com a\_mapper.table\_prefix para formar o nome da tabela no banco de dados;
campos e tipos são instrospectados a partir da tabela.
*tab* é uma tabela opcional que é usada como base para o objeto modelo, caso exista 

**a\_model.model** - o mapeador para este modelo 

**a\_model.name, a\_model.table\_name** - o nome do modelo e de sua tabela de armazenamento 

**a\_model.driver** - o driver de banco de dados utilizado 

**a\_model.meta** - metainformação sobre o modelo, instrospectada a partir da tabela de dados 

**a\_model:find(*id*)** - busca e retorna a instância do modelo com o *id* passado
(indexado através da coluna `id`, numérica,  da tabela) 

**a\_model:find\_first(*condition*, *args*)** - busca e retorna a primeira instância do modelo
que corresponde à condição *condition*; *args* pode determinar a ordem (args.order) ou injetar
campos de outras tabelas (args.inject) 

Exemplo: `books:find_first("author = ? and year_pub > ?", { "John Doe", 1995, order = "year_pub asc" })` 

**a\_model:find\_all(*condition*, *args*)** - busca e retorna todas as instâncias do modelo que correspondem
à condição *condition*; *args* pode determinar a ordem (args.order) ou injetar campos de outras tabelas
(args.inject) 

**a\_model:new([*tab*])** - cria uma nova instância do modelo, opcionalmente usando *tab*
como os valores iniciais 

**a\_model:find\_by\_xxx(*args*)** - busca e retorna a primeira instância do modelo montando
a condição a partir do nome do método, como no ActiveRecord do Rails 

**a\_model:find\_all\_by\_xxx(*args*)** - busca e retorna todas as instâncias do modelo montando
a condição a partir do nome do método, como no ActiveRecord do Rails 

Exemplo: `books:find_all_by_author_or_author{ "John Doe", "Jane Doe", order = "year_pub asc" }` 

**an\_instance:save([*force\_insert*])** - grava uma instância no banco de dados,
atualizando as mudanças ou criando um novo registro caso a instância seja nova;
se *force\_insert* for true sempre faz uma inserção antes de uma atualização 

Se existir uma coluna chamada `created_at` esta linha é definida como a data de criação do registro;
 se existir uma coluna chamada `updated_at`, esta linha é definida como a data de última atualização do registro. 

**an\_instance:delete()** - remove uma instância do banco de dados 

