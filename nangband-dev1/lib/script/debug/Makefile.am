## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  test.lua trace.lua

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@script/debug

angband_DATA = \
  $(angband_files)
endif

