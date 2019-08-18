echo "1)"
mkdir ~/infraComp ~/infraComp/aula01 ~/infraComp/aula02 ~/infraComp/aula03 ~/infraComp/aula04 ~/infraComp/aula05 ~/infraComp/aula02/exercicios
tree ~/infraComp
echo ""

echo "2)"
cd ~
touch ~/infraComp/DATA
touch ~/infraComp/aula01/DATA
touch ~/infraComp/aula02/DATA
touch ~/infraComp/aula02/exercicios/DATA
touch ~/infraComp/aula03/DATA
touch ~/infraComp/aula04/DATA
touch ~/infraComp/aula05/DATA
tree infraComp
echo ""

echo "3)"
ls -l ~/..
chmod 750 ~
ls -l ~/..
echo ""

echo "4)"
mkdir ~/mydir
chmod 722 ~/mydir
touch ~/mydir/wainejr # nao tenho vizinho, entao vai no meu
echo ""

echo "5)"
echo "Em ambos os casos o arquivo 'teste.bak' é criado. Porém no primeiro, o arquivo 'teste' é mantido, enquanto no segundo caso ele é 'renomeado' para 'teste.bak'"
echo ""

echo "6)"
echo "Ler um diretório consiste em ver o conteúdo dentro dele, entre arquivos e outros diretórios. Já executar um diretório dá a possibilidade de entrar nele, permitindo também acesso aos subdiretórios, se possível."
echo ""

echo "7)"
echo "Ler um arquivo permite ver o seu conteúdo, o que está escrito nele. A execução de um arquivo permite que ele execute comandos, como em arquivos bash"
echo ""

echo "8)"
echo "Apenas com 'mv' não, pois o diretório \$HOME tem o nome do seu usuário e o SO não permite a alteração desse. Uma alternativa é utilizar o comando 'usermod'"
echo ""

echo "9)"
touch ~/teste
chmod 666 ~/teste #rw-rw-rw-
ls -l ~/teste
chmod 664 ~/teste #rw-rw-r--
ls -l ~/teste
chmod 555 ~/teste #r-xr-xr-x
ls -l ~/teste
chmod 644 ~/teste #rw-r--r--
ls -l ~/teste
chmod 400 ~/teste #r--------
ls -l ~/teste
echo ""

echo "10)"
umask 113
touch ~/teste1
umask 222
touch ~/teste2
umask 133
touch ~/teste3
umask 377
touch ~/teste4
ls -l ~/teste1 ~/teste2 ~/teste3 ~/teste4
umask 013
echo "O arquivo nao tem o resultado esperado, pois o comando 'umask' atua apenas como máscara. Ou seja, ele não 'adiciona' permissões, apenas faz uma máscara que não permite que os arquivos obtenham certas permissões."
echo ""

echo "11)"
mkdir ~/d1
mkdir ~/d2
chmod 755 ~/d1 ~/d2
touch ~/d1/teste ~/d2/teste
chmod 444 ~/d1
chmod 555 ~/d2
ls -l ~/d1
ls -l ~/d2
cd ~/d1
cd ~/d2
echo "Não é possível acessar o diretório d1, apenas listar seu conteúdo com 'ls'. É possível entrar e listar o conteúdo detalhado 'ls -l' do diretório"
echo ""
