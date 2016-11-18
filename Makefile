TARGET = libtwitch.so

$(TARGET): twitch.o twitch-plugin.a
	gcc -shared -ggdb -fPIC -std=c99 -o $@ $^

twitch.o: twitch.c
	gcc -fPIC -ggdb -Wall -std=c99 -I/usr/local/include/weechat -o $@ -c $<

twitch-plugin.a: twitch-plugin.o
	ar rcs $@ $^

twitch-plugin.o: twitch-plugin.c twitch-plugin.h
	gcc -fPIC -ggdb -Wall -std=c99 -I/usr/local/include/weechat -c -o $@ $<

clean:
	rm -rf *.o *.so *.a $(TARGET)
