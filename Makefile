CPPFLAGS=-D_FILE_OFFSET_BITS=64 -std=c99
CFLAGS=-Wno-unused-label -Wno-unused-function

PROGRAMS = tagfs

all: $(PROGRAMS)

tagfs: tagfs.c dataFile.o dataBase.o
	gcc $(CPPFLAGS) $(CFLAGS) -Wall $^ -o $@ -lfuse

%: %.o
	gcc $(CPPFLAGS) $(CFLAGS) -Wall -o $@ $^

clean:
	$(RM) $(PROGRAMS) *.log *.o

.PHONY:	 mount, unmount, kill, test, restore

mount:
	cp -f ./tests_de_base/tests/images/.tags ./tests_de_base/tests/images/.oldtags
	mkdir -p mnt && ./$(PROGRAMS) ./tests_de_base/tests/images ./mnt

unmount:
	fusermount -u ./mnt

restore:
	rm ./tests_de_base/tests/images/.tags
	mv ./tests_de_base/tests/images/.oldtags ./tests_de_base/tests/images/.tags

kill:
	kill -9 tagfs

test: $(PROGRAMS)
	cp ./$(PROGRAMS) ./tests_de_base/tests/$(PROGRAMS)
	cd ./tests_de_base/tests/ && ./check_ls.sh
