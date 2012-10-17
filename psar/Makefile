# Macro-définitions

SRC=src_C
SRC_T=src_C/test_C
INC=include
BIN=bin
TEST=test
OBJ=obj
LIB=lib
CC=gcc
CFLAGS=-Wall -ansi -lrt -lpthread -I$(INC)
EXEC= serv_dist serv_loc rps librproc.a tests

all: $(EXEC)

# Mettre ici la liste du jeu de tests
tests: test-rexec test-rwait test-rkill

# --- Compilation --- #

$(OBJ)/librproc.o : $(SRC)/librproc.c 
	$(CC) -o $@ -c $^ -I$(INC) -Wall -ansi -pedantic 

$(OBJ)/fonctions.o:  $(SRC)/fonctions.c
	$(CC) -o $@ -c $^ -I$(INC) -Wall -ansi -pedantic 

librproc.a: $(OBJ)/librproc.o 
	ar crus $(LIB)/$@ $^

serv_dist: $(SRC)/serv_dist.c $(OBJ)/fonctions.o 
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS) 

serv_loc: $(SRC)/serv_loc.c $(OBJ)/fonctions.o
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS) 

rps: $(SRC)/rps.c
	$(CC) -o $(BIN)/$@ $< $(CFLAGS) 



# --- Tests --- #

test-rexec: $(SRC_T)/test-rexec.c $(LIB)/librproc.a
	$(CC) -o $(TEST)/$@ $^ $(CFLAGS)  

test-rwait: $(SRC_T)/test-rwait.c $(LIB)/librproc.a
	$(CC) -o $(TEST)/$@ $^ $(CFLAGS)  

test-rkill: $(SRC_T)/test-rkill.c $(LIB)/librproc.a
	$(CC) -o $(TEST)/$@ $^ $(CFLAGS) 
	
	 

# --- Nettoyage --- #

clean:
	rm -f $(BIN)/* $(BIN)/.nfs* $(OBJ)/*.o $(LIB)/*.a $(SRC)/*~ $(INC)/*~
