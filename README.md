# Light-FS
Trabalho final desenvolvido para a cadeira de Arquitetura II do curso de ciência da computação da Universidade Federal do Rio Grande do Sul.
## Estrutura de MetaDados
É composta por 8 Bytes que informam, respectivamente as seguintes informações sobre o sistema de arquivo.
- Tamanho do índice.
- Tamanho do cluster.
- Byte onde o índice inicia.
- Byte onde o primeiro cluster inicia.\
![asdas](https://raw.githubusercontent.com/LeoHernandes/FATSimulator/main/Imagens/Metadados.png)
## Tabela de índices
A tabela de índices é composta por 254 bytes, sendo cada um deles a representação de um ponteiro para um determinado cluster na área de dados do sistema.
## Estrutura do Cluster
Um cluster é composto pelas seguintes informações:
- **Nome**

- **Extensao**
- **Pai**
- **Nome**
- **Marcador**\
Um marcador representa o inicio da lista de filhos[^1] de um cluster. A estrutura do tipo Cluster é chamada de NodoCluster e serve para representar tanto diretórios quanto arquivos.

## Funcionamento do sistema
Quando o usuário executa o programa e não possui um arquivo que representa o disco em sua máquina, o programa cria, automaticamente, um arquivo binário com o nome de *ArqDisco.bin* para que seja possível operar sobre o mesmo.

Na inicialização do arquivo, é realizado a criação da área de metadados, primeiros 8 Bytes, da área de tabela de índices, 254 bytes, da área do cluster raiz, root, e os 253 próximos clusters. Sendo assim, torna-se visivel que é possível armazenar 254 clusters, contando com o root, no sistema de arquivos, pois os ultimos dois clusters FF e FF, são utilizados como flags para representar, respectivamente, cluster vazio e cluster disponível.

### Listando o conteúdo de um diretório
Para listar o conteúdo de um diretório, basta digitar o comando **DIR**. A partir disso o sistema informará *<vazio>* caso não tenha nenhum conteúdo ou listará, linha por linha, todo o conteúdo do diretório.

![](https://github.com/LeoHernandes/FATSimulator/blob/main/Imagens/DIR.png)
### Criando um diretório
Para criar um diretório basta executar o comando **MKDIR** *nome do diretório*. A partir disso, o programa irá procurar a primeira posição disponível(vazio/disponível) na tabela de índices para ocupar, marcando-a com FF. Além disso, em seguida, irá procurar o marcador **'*'** na pasta onde foi executado o comando para adicionar na sua Lista de Filhos do diretório o novo diretório criado.

### Entrando em um diretório

### Removendo um diretório
  
### Criando um arquivo
  
### Editando um arquivo
  
### Renomeando um diretório/arquivo

### Movendo um diretório/arquivo

### Removendo um diretório/arquivo


[^1]: A organização de pastas e arquivos dentro do sistema Light-FS pode ser visto como uma árvore de grau n. O nó raiz do sistema é o diretório root. Uma lista de filhos são os filhos do nó pai.
