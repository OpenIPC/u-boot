CSRC  = $(wildcard *.c)

CFLAGS += -nostdinc
ifeq ($(CFG_SM_SUPPORT),1)
CFLAGS += -I../drivers/secure_driver/include/common_include/
endif
#.SILENT:

default:
	echo "  CC    $(CSRC)"
	echo "$(CC) $(CFLAGS) $(CSRC)"
	$(CC) $(CFLAGS) $(CSRC)

clean:
	rm *.o *.a -f
