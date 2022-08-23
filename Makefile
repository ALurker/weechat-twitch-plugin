TARGET = libtwitch.so

WEECHAT_LIB = /usr/local/include/weechat

DEBUG = -ggdb

$(TARGET): twitch.o twitch-plugin.o twitch-stack.o
	gcc -shared -fPIC -std=c99 $(DEBUG) -o $@ $^

twitch.o: twitch.c
	gcc -fPIC -Wall -std=c99 -I$(WEECHAT_LIB) $(DEBUG) -o $@ -c $<

twitch-plugin.o: twitch-plugin.c twitch-plugin.h
	gcc -fPIC -Wall -std=c99 -I$(WEECHAT_LIB) $(DEBUG) -c -o $@ $<

twitch-stack.o: twitch-stack.c twitch-stack.h
	gcc -fPIC -Wall -std=c11 -I$(WEECHAT_LIB) $(DEBUG) -c -o $@ $<

clean:
	rm -rf *.o *.so *.a $(TARGET)
