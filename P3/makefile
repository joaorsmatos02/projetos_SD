# Grupo 20
# Tomás Barreto nº 56282
# João Matos nº 56292
# Diogo Pereira nº 56302

OBJ_dir = object
BIN_dir = binary
LIB_dir = lib
CLIENT_LIB = client_stub.o network_client.o read_write-private.o data.o entry.o
TREE_CLIENT = tree_client.o sdmessage.pb-c.o
TREE_SERVER = tree_server.o entry.o data.o tree_skel.o network_server.o read_write-private.o sdmessage.pb-c.o
LDFLAGS = /usr/lib/x86_64-linux-gnu/libprotobuf-c.a
PROTO = sdmessage.proto

all: protobuf client-lib tree-client tree-server

protobuf: $(PROTO)
	protoc --c_out=./ $(PROTO)
	mv sdmessage.pb-c.c source
	mv sdmessage.pb-c.h include

client-lib: $(CLIENT_LIB)
	ld -r $(addprefix $(OBJ_dir)/,$(CLIENT_LIB)) -o $(LIB_dir)/client-lib.o

tree-client: $(TREE_CLIENT)
	gcc -Wall -g -o $(BIN_dir)/tree-client $(addprefix $(OBJ_dir)/,$(TREE_CLIENT)) $(LIB_dir)/client-lib.o $(LDFLAGS)

tree-server: $(TREE_SERVER)
	gcc -Wall -g -o $(BIN_dir)/tree-server $(addprefix $(OBJ_dir)/,$(TREE_SERVER)) object/tree.o $(LDFLAGS)

sdmessage.pb-c.o: source/sdmessage.pb-c.c
	gcc -Wall -g -I include -o $(OBJ_dir)/sdmessage.pb-c.o -c source/sdmessage.pb-c.c

%.o: source/%.c $($@)
	gcc -Wall -g -I include -o $(OBJ_dir)/$@ -c $<

clean:
	rm $(OBJ_dir)/client_stub.o
	rm $(OBJ_dir)/data.o
	rm $(OBJ_dir)/entry.o
	rm $(OBJ_dir)/network_client.o
	rm $(OBJ_dir)/network_server.o
	rm $(OBJ_dir)/read_write-private.o
	rm $(OBJ_dir)/sdmessage.pb-c.o
	rm $(OBJ_dir)/tree_client.o
	rm $(OBJ_dir)/tree_server.o
	rm $(OBJ_dir)/tree_skel.o
	rm $(BIN_dir)/*
	rm $(LIB_dir)/*
	rm source/sdmessage.pb-c.c
	rm include/sdmessage.pb-c.h
