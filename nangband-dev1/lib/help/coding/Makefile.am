## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
	ahml.xml coding_style.xml compileoptions.xml index.xml taglist.xml \
	xmlband.xml

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@help

angband_DATA = \
  $(angband_files)
endif

