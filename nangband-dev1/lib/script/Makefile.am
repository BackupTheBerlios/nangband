## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  init.lua object.lua p_exampl.lua p_manage.lua s_manage.lua s_magic.lua \
  s_pray.lua

EXTRA_DIST = \
  $(angband_files)

SUBDIRS = debug

if SET_GID
angbanddir = @DEFAULT_PATH@script

angband_DATA = \
  $(angband_files)
endif

