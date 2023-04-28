CC = gcc
CFlags = -Wall -fsanitize=address,undefined -std=c99

mysh: mysh.c
	$(CC) $(CFlags) -o $@ $^

foo: foo.c
	$(CC) $(CFlags) -o $@ $^

bar: bar.c
	$(CC) $(CFlags) -o $@ $^

baz: baz.c
	$(CC) $(CFlags) -o $@ $^
