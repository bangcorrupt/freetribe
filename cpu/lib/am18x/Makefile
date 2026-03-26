# tary, 0:49 2013/4/15

all : am18x_map_s.h

am18x_map_s.h: am18x_map.h
	sed -n -e "/^#define/{ s/\([^0]0x[0-9a-fA-F]\+\)UL/\1/g;p; }" $< > $@

clean:

.PHONY: all clean
