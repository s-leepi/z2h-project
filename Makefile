TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -n -f ./final.db
	./$(TARGET) -f ./final.db -a "Waytoodank,LE1,200"
	./$(TARGET) -f ./final.db -a "Kappa,LE1,200"
	./$(TARGET) -f ./final.db -a "Pepega,LE1,200"
	./$(TARGET) -f ./final.db -a "Poggers,LE1,200"
	./$(TARGET) -f ./final.db -a "Sadge,LE1,200"
	./$(TARGET) -f ./final.db -a "Smile,LE1,200"
	./$(TARGET) -f ./final.db -a "cmonBruh,LE1,200"
	./$(TARGET) -f ./final.db -a "Trihard7,LE1,200"


default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $? -g

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude


