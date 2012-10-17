CC=gcc
CFLAGS= -Wall -ansi -pedantic
INCDIR=include
LDFLAGS=
OBJDIR=obj
SRCDIR=src
BINDIR=bin

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/srm.h
	$(CC) -c -o $@ $< $(CFLAGS) -I $(INCDIR)



$(BINDIR)/srm: $(OBJDIR)/write_file.o $(OBJDIR)/mult_write_file.o $(OBJDIR)/rename_file.o $(OBJDIR)/remove_df.o $(OBJDIR)/main.o
	gcc -o $@ $^ $(CFLAGS)


clean:
	rm -f  $(OBJDIR)/*.o $(SRCDIR)/*~ $(BINDIR)/*

mrproper: clean
	rm $(EXEC)
