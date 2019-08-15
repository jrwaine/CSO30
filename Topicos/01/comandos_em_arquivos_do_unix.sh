echo "1)"
man palavra_para_pesquisar
echo ""

echo "2)"
ls ~/
echo ""

echo "3)"
pwd
echo ""

echo "4)"
whoami
echo ""

echo "5)"
# A partir da conta, utilizar o comando
id -g 
echo ""

echo "6)"
pwd
cd /home/espinf
pwd
cd ../
pwd
cd /
pwd
cd /usr
pwd
cd /tmp
pwd
cd /usr/bin
pwd
cd ~
pwd
echo ""

echo "7)"
# Não é detalhado qual tipo de data deve ser escolhida. 
# Então os arquivos são listados por meio do "ctime", data de alteração
echo "    a)"
cd ~
ls -c /home/espinf
ls -c /home
ls -c /
ls -c /usr
ls -c /tmp
ls -c /usr/bin
ls -c ~
echo "    b)"
cd /home/espinf
ls
cd /home
ls
cd /
ls
cd /usr
ls
cd /tmp
ls
cd /usr/bin
ls
cd ~
ls
echo ""

echo "8)"
echo "A opcao -a 'Lista todos os  arquivos  nos  diretórios,  incluindo"
echo "            todos  os arquivos começados com '.'.'" 
echo "segundo o comando 'man ls'"
echo "A opcao -l 'Escreve (no formato de coluna simples)  o  modo  do  arquivo,  o"
echo "            número  de  ligações  para  o arquivo, o nome do proprietário, o"
echo "            nome do grupo, o tamanho do arquivo  (em  bytes),  o  rótulo  de"
echo "            tempo, e o nome do arquivo.'"
echo "segundo o comando 'man ls'"
echo ""

echo "9)"
id wainejr
echo ""

echo "10)"
echo "Diretórios iniciados com '.' são diretórios ocultos."
echo ""

echo "11)"
ln ./infraComp atalho
ls atalho
echo ""

echo "12)"
echo "O comando 'du' mostra o espaço em disco utilizado pelos diretórios e"
echo "arquivos do atual diretório. Já o comando 'df' mostra informações sobre"
echo "a ocupação e a divisão do disco da máquina"
echo ""

echo "13)"
echo "As colunas de 'ls -l' significam: permissão; número de ligações fortes;"
echo "nome do proprietário; nome do grupo; tamanho em bytes; rótulo de tempo;"
echo "nome do arquivo."
ls -l ~
ls -a ~
ls -l -S /var/spool/mail
ls -l /etc
ls -lt /home
ls -R -S /usr
echo ""

echo "14)"
mkdir ~a ~a/b ~a/b/c ~a/b/c/d ~a/b/c/d/h ~a/b/c/e ~a/b/f ~a/b/f/d ~a/b/j
tree ~a
echo ""

echo "15)"
tar aux.tar ~
cp aux.tar auxGzip.tar
cp aux.tar auxBzip2.tar
cp aux.tar auxCompress.tar
compress auxCompress.tar
gzip auxGzip.tar
bzip2 auxBzip2.tar
ls -l -h # para comparar tamanho de arquivos
echo ""
