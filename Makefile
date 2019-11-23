TARGET = libtwitch.so

$(TARGET): twitch.o twitch-plugin.o twitch-stack.o
	gcc -shared -fPIC -std=c99 -o $@ $^

twitch.o: twitch.c
	gcc -fPIC -Wall -std=c99 -I/usr/local/include/weechat -o $@ -c $<

twitch-plugin.o: twitch-plugin.c twitch-plugin.h
	gcc -fPIC -Wall -std=c99 -I/usr/local/include/weechat -c -o $@ $<

twitch-stack.o: twitch-stack.c twitch-stack.h
	gcc -fPIC -Wall -std=c11 -I/usr/local/include/weechat -c -o $@ $<

clean:
	rm -rf *.o *.so *.a $(TARGET)
