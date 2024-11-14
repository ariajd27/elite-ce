# ----------------------------
# Makefile Options
# ----------------------------

NAME = ELITE
ICON = icon.png
DESCRIPTION = "Elite for the TI-84+ CE"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz -Wno-pointer-sign
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
