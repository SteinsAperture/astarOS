CPPFLAGS=-D_FILE_OFFSET_BITS=64 -std=c99
CFLAGS=-Wno-unused-label -Wno-unused-function
DEFINE=

PROGRAMS = tagfs

all: clean $(PROGRAMS)

tagfs: tagfs.c dataFile.o dataBase.o
	gcc $(CPPFLAGS) $(CFLAGS) $(DEFINE) -Wall $^ -o $@ -lfuse -lpthread

%.o: %.c
	gcc $(CPPFLAGS) $(CFLAGS) $(DEFINE) -Wall -c -o $@ $^

clean:
	$(RM) $(PROGRAMS) *.log *.o perfsTags perfsFiles

.PHONY:	 mount, unmount, kill, test, restore, log, perfTag, perfFile

mount: $(PROGRAMS)
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
	cd ./tests_de_base/tests/ && ./check_ls.sh && ./check_modifs.sh

perfTag: $(PROGRAMS)
	gcc -o perfsTags perfs_Ntags_1fichier.c
	make mount
	cd ./mnt && ../perfsTags
	sleep 1
	make unmount


perfFile: $(PROGRAMS)
	gcc -o perfsFiles perfs_1tag_Nfichiers.c
	make mount
	cd ./mnt && ../perfsFiles
	sleep 1
	make unmount

log:
	make DEFINE='-DWRITELOG'

