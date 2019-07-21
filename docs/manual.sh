pandoc manual.md -o manual.html --css manual.css --number-sections --toc --template manual-template --self-contained
pandoc manual.md -o manual.pdf --number-sections --toc \
		--variable=subparagraph \
		--latex-engine=xelatex -V mainfont='Charis SIL' -V mathfont=Asana-Math -V monofont=inconsolata \
		-V fontsize=12pt -V papersize:a4 -V geometry:margin=2.5cm \
                -H manual-pdf.tex
