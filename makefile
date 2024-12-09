main: ./src/main.c ./src/matrix.c
	gcc $^ -o ./bin/$@.exe;
	./bin/$@.exe;
test: ./src/test.c ./src/matrix.c
	gcc $^ -o ./bin/$@.exe;
	./bin/$@.exe;
