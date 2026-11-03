#ifndef __DATA_KEEPER_H__
#define __DATA_KEEPER_H__

struct buffer_info_t {
	unsigned char *data;
	unsigned int  size;
	unsigned int read_which_chip;
};

typedef enum {
	READ_FROM_RC = 0,
	READ_FROM_EP0 = 1
} READ_FROM_CHIP_TYPE;

int read_system_data(unsigned int data_id, struct buffer_info_t *buffer);

#endif
