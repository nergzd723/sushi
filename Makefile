CC = clang
all:
	$(CC) sushi.c -Os -o sushi -lcrypt
	sudo chown root sushi
	sudo chmod u+s sushi
run: all
	./sushi rm a