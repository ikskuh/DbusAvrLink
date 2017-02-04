
CFLAGS=-g

all: ti-send ti-receive ti-ls ti-tokenize ti-extract

%: %.c io.c tty.c debug.c tokens.c ti83f.c
	gcc $(CFLAGS) -Wall -o $@ $^

%\:link: %
	rm ~/bin/$< || true
	ln $< ~/bin/$<
	
links: ti-send\:link ti-receive\:link ti-ls\:link
