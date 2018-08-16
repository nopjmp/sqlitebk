src = $(wildcard *.c)
obj = $(src:.c=.o)

CFLAGS = -g -std=c99
LDFLAGS = -lsqlite3

sqlitebk: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) sqlitebk
