NAME=proj4
all: $(NAME).tex
	latex $(NAME).tex
	bibtex $(NAME).aux
	latex $(NAME).tex
	latex $(NAME).tex
	dvips -t a4 $(NAME).dvi
	ps2pdf	-sPAPERSIZE=a4 $(NAME).ps

clean:
	rm $(NAME).aux $(NAME).dvi $(NAME).log $(NAME).bbl $(NAME).ps $(NAME).blg $(NAME).pdf