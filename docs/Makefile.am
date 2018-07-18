# SPDX-License-Identifier: GPL-2.0
# Copyright © 2007-2018 ANSSI. All Rights Reserved.
if HTML_DOC
htmldocdir = $(datadir)/clip-libvserver/html
htmldoc_DATA = html/index.html
html/index.html : doxyfile_html
	doxygen doxyfile_html
HTMLINST=cp -dpR html/* $(DESTDIR)$(htmldocdir)/
else
HTMLINST=
endif !HTML_DOC
install-data-hook:
	$(HTMLINST)

clean-local:
	rm -fr html
