#ifndef _USB_MSC_H_
#define _USB_MSC_H_

typedef void (*read_callback_t)(uint32_t lba, uint32_t size);
typedef void (*write_callback_t)(uint32_t lba, uint32_t size);
typedef void (*data_callback_t)(uint32_t lba, void *data, uint32_t size);

void usbMscDeinit(void);

void usbMscInit(read_callback_t read, write_callback_t write, data_callback_t data);

#endif

