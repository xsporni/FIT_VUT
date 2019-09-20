proj2.pdf: proj2.ps
	ps2pdf proj2.ps
proj2.ps: proj2.dvi
	dvips -t a4 proj2.dvi
proj2.dvi: proj2.tex
	latex proj2.tex
	latex proj2.tex
clean:
	rm -fr *.aux *.log *.tex# *.ps *.dvi *.pdf