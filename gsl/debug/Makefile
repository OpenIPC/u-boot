CSRC  = $(wildcard *.c)
SSRC  = $(wildcard *.S)

# CFLAGS += -nostdinc

#.SILENT:

default:
	echo "  CC    $(CSRC)"
	echo "$(CC) $(CFLAGS) $(CSRC) $(SSRC)"
	$(CC) $(CFLAGS) $(CSRC) $(SSRC)

clean:
	rm *.o *.a -f
