# Grupo 20
# Tomás Barreto nº 56282
# João Matos nº 56292
# Diogo Pereira nº 56302

OBJ_dir = object
BIN_dir = binary
OBJECTOS = tree.o test_tree.o data.o test_data.o entry.o test_entry.o serialization.o
TREE = entry.o data.o tree.o test_tree.o
DATA = data.o test_data.o
ENTRY = entry.o data.o test_entry.o
SERIALIZATION = serialization.o

all: test_tree test_data test_entry serialization

test_tree: $(TREE)
	gcc -Wall -g -o $(BIN_dir)/test_tree $(addprefix $(OBJ_dir)/,$(TREE))

test_data: $(DATA)
	gcc -Wall -g -o $(BIN_dir)/test_data $(addprefix $(OBJ_dir)/,$(DATA))

test_entry: $(ENTRY)
	gcc -Wall -g -o $(BIN_dir)/test_entry $(addprefix $(OBJ_dir)/,$(ENTRY))

serialization: $(SERIALIZATION)
	echo "serialization criado"

%.o: source/%.c $($@)
	gcc -Wall -g -I include -o $(OBJ_dir)/$@ -c $<

clean:
	rm $(addprefix $(OBJ_dir)/,$(OBJECTOS))
	rm $(BIN_dir)/test_tree
	rm $(BIN_dir)/test_data
	rm $(BIN_dir)/test_entry