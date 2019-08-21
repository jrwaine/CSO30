# Carlos Maziero (maziero@ppgia.pucpr.br), junho de 2007
#
# Este é um Makefile "burro" e não otimizado, que não testa as dependências
# temporais entre fontes e objetos. Foi feito às pressas para compor o modelo
# de dissertação, mas pode/deve ser melhorado.

# main file
FILE=main

help:
	@echo "Este Makefile auxilia a compilacao dos fontes LaTeX:"
	@echo ""
	@echo "  make       : esta mensagem"
	@echo "  make latex : compilacao latex simples"
	@echo "  make bib   : compilacao bibtex"
	@echo "  make all   : compilacao latex completa (latex, bibtex, latex, latex)"
	@echo "  make pdf   : gera o PDF usando dvips e ps2pdf"
	@echo "  make clean : remove os arquivos temporarios, backups, etc"
	@echo ""
	@echo "O arquivo principal deve se chamar \"$(FILE)\".tex"
	@echo ""

latex:
	latex $(FILE)

bib:
	bibtex $(FILE)

all:
	latex $(FILE)
	bibtex $(FILE)
	latex $(FILE)
	latex $(FILE)
	dvips -tA4 -Ppdf $(FILE)
	ps2pdf $(FILE).ps

pdf:
	dvips -tA4 -Ppdf $(FILE)
	ps2pdf $(FILE).ps

clean:
	@find . -name \*.aux -exec rm {} \;
	@find . -name \*~ -exec rm {} \;
	@rm -f $(FILE).toc $(FILE).bbl $(FILE).blg $(FILE).log $(FILE).ps
	@rm -f $(FILE).dvi $(FILE).lof $(FILE).lot texput.log
