


all: send receive ls

%: %.c io.c tty.c debug.c tokens.c ti83f.c
	gcc -Wall -o $@ $^

%\:link: %
	rm ~/bin/ti-$< || true
	ln $< ~/bin/ti-$<
	
links: send\:link receive\:link ls\:link
