libtwitch.so: twitch.o
	gcc -shared -ggdb -fPIC -std=c99 -o libtwitch.so twitch.o

twitch.o: twitch.c
	gcc -fPIC -ggdb -Wall -std=c99 -I/usr/local/include/weechat -c twitch.c

clean:
	rm -rf *.o *.so
