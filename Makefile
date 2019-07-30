CC=clang
COPTS=-Wall -Wextra -Wfatal-errors
FRAMEWORKS=-framework CoreFoundation -framework ImageIO -framework QuickLook

all:
	$(CC) $(COPTS) -o ql-icon ql-icon.c $(FRAMEWORKS)

clean:
	$(RM) ql-icon
