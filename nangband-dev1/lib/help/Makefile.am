## Makefile.am -- Process this file with automake to process Makefile.in

SUBDIRS = err coding

angband_files = \
	ahml.xml ahmlt.xsl coding copyright.xml demo.xml general.xml \
	help.xml index.xml list.xml table.xml version.xml

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@help

angband_DATA = \
  $(angband_files)
endif

