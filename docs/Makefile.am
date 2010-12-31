include $(top_srcdir)/Makefile.decl

EXTRA_DIST += spectmorph.odt spectmorph.pdf

dist_man_MANS = smenc.1 smplay.1 smvisualize.1 smstrip.1 smwavset.1 smsfimport.1 sminspector.1
EXTRA_DIST += smenc.1.doxi smplay.1.doxi smvisualize.1.doxi smstrip.1.doxi smwavset.1.doxi smsfimport.1.doxi sminspector.1.doxi
EXTRA_DIST += smdefs.doxi

DOCS_DIR = ~/public_html/spectmorph/docs-$(VERSION)
DOXER = ~/src/beast/doxer/doxer.py -D SM_VERSION ${VERSION}

rebuild-manpage:
	for i in $(dist_man_MANS); do $(DOXER) doxi2man $$i.doxi; done

publish-docs:
	mkdir -p $(DOCS_DIR)/api
	for i in $(dist_man_MANS); do \
	  $(DOXER) doxi2html $$i.doxi; \
	  cp $$i.html $(DOCS_DIR); \
        done
	cp spectmorph.pdf $(DOCS_DIR)
	cd $(top_srcdir); doxygen Doxyfile; cp -a html/* $(DOCS_DIR)/api
	cd $(DOCS_DIR) && find . -type f -print0 | xargs -0 git add
