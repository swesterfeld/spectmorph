include $(top_srcdir)/Makefile.decl

EXTRA_DIST += spectmorph.odt spectmorph.pdf

dist_man_MANS = smstrip.1 smenc.1 sminspector.1 smjack.1 smplay.1 smsfimport.1 \
                smtool.1 smvisualize.1 smwavset.1 spectmorphoscgui.1 sminstbuilder.1

DOCS_DIR = ~/spectmorph.org/docs-$(VERSION)

rebuild-manpage:
	for p in $(dist_man_MANS); do \
          curl -sfS https://testbit.eu/wiki/$$p?action=render >$$p.web; \
	  curl -sfS "https://testbit.eu/wiki.mw/index.php?title=$$p&action=raw" > $$p.wiki; \
	  $(WIKIHTML2MAN) $$p.web >$$p; \
	  groff -mandoc -Thtml < $$p >$$p.html; \
	  rm $$p.web; \
	done

publish-docs:
	mkdir -p $(DOCS_DIR)/api
	for i in $(dist_man_MANS); do \
	  cp $$i.html $(DOCS_DIR); \
        done
	cp spectmorph.pdf $(DOCS_DIR)
	cd $(top_srcdir); doxygen Doxyfile; cp -a html/* $(DOCS_DIR)/api
	cd $(DOCS_DIR) && find . -type f -print0 | xargs -0 git add
