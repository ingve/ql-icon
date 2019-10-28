CC=/usr/bin/clang
SDKROOT=$(shell xcrun --show-sdk-path)
COPTS=-Wall -Wextra -Wfatal-errors -isysroot $(SDKROOT)
OBJCOPTS=-Wfatal-errors -fobjc-arc -isysroot $(SDKROOT)
FRAMEWORKS=-framework CoreFoundation -framework ImageIO -framework QuickLook

all:
	$(CC) $(COPTS) -o ql-icon ql-icon.c image.c io.c ui.c $(FRAMEWORKS)
	$(CC) $(OBJCOPTS) -o icon icon.m image.c io.c ui.c -framework AppKit -framework QuickLook

clean:
	$(RM) ql-icon
	$(RM) icon
