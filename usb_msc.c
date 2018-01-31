#include <string.h>

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device_stack_interface.h"
#include "usb_class_msc.h"
#include "usb_descriptor.h"

#include "board.h"
#include "printd.h"
#include "fat16.h"
#include "usb_msc.h"

#define TOTAL_LOGICAL_ADDRESS_BLOCKS    (4000)
#define DISK_SIZE                       (FATBytesPerSec * TOTAL_LOGICAL_ADDRESS_BLOCKS)
#define LOGICAL_UNIT_SUPPORTED          (1)

#define MSD_SEND_BUFFER_SIZE            (FATBytesPerSec)
#define MSD_RECV_BUFFER_SIZE            (FATBytesPerSec)

extern usb_desc_request_notify_struct_t desc_callback;

static uint8_t              bulk_in_buffer[FATBytesPerSec];
static uint8_t              bulk_out_buffer[FATBytesPerSec];
static msd_handle_t         msd_app_handle;
static read_callback_t      read_callback;
static write_callback_t     write_callback;
static data_callback_t      data_callback;

void USB_App_Device_Callback(uint8_t event, void* val,void* arg)
{
    uint16_t speed;

    printd("usb device event %d\r\n", event);

    switch (event)
    {
        case USB_DEV_EVENT_BUS_RESET:
            if (USB_Class_MSC_Get_Speed(msd_app_handle, &speed) == USB_OK)
                USB_Desc_Set_Speed(msd_app_handle, speed);
            break;

        case USB_DEV_EVENT_ENUM_COMPLETE:
            break;

        case USB_DEV_EVENT_ERROR:
            break;

        case USB_MSC_DEVICE_GET_SEND_BUFF_INFO:
            if (NULL != val)
                *((uint32_t *)val) = MSD_SEND_BUFFER_SIZE;
            break;

        case USB_MSC_DEVICE_GET_RECV_BUFF_INFO:
            if (NULL != val)
                *((uint32_t *)val) = MSD_RECV_BUFFER_SIZE;
            break;
    }
}

uint8_t USB_App_Class_Callback
(
    uint8_t     event,
    uint16_t    value,
    uint8_t**   data,
    uint32_t*   size,
    void*       arg
)
{
    uint8_t                     error = USB_OK;
    uint8_t**                   msc_buffer_ptr_ptr;
    lba_app_struct_t*           lba_data_ptr;
    device_lba_info_struct_t*   device_lba_info_ptr;
    uint32_t                    fat_lba;

    printd("usb class event %d\r\n", event);

    switch(event)
    {
        case USB_DEV_EVENT_SEND_COMPLETE :
            break;

        case USB_DEV_EVENT_DATA_RECEIVED :
            lba_data_ptr = (lba_app_struct_t*) size;
            fat_lba = lba_data_ptr->offset / FATBytesPerSec;

            if (fat_lba >= FATDataSec0)
            {
                data_callback(fat_lba, lba_data_ptr->buff_ptr, lba_data_ptr->size);
            }

            break;

        case USB_MSC_START_STOP_EJECT_MEDIA :
            break;

        case USB_MSC_DEVICE_READ_REQUEST :
            /* copy data from storage device before sending it on USB Bus
            (Called before calling send_data on BULK IN endpoints)*/
            msc_buffer_ptr_ptr = data;
            *msc_buffer_ptr_ptr = bulk_in_buffer;

            lba_data_ptr = (lba_app_struct_t*) size;
            fat_lba = lba_data_ptr->offset / FATBytesPerSec;

            read_callback(fat_lba, lba_data_ptr->size);

            /* read data from mass storage device to driver buffer */
            FATReadLBA(fat_lba,*msc_buffer_ptr_ptr);
            break;

        case USB_MSC_DEVICE_WRITE_REQUEST :
            /* copy data from USB buffer to Storage device
            (Called before after recv_data on BULK OUT endpoints)*/
            msc_buffer_ptr_ptr = data;
            if (size != NULL)
            {
                *msc_buffer_ptr_ptr = bulk_out_buffer;

                lba_data_ptr = (lba_app_struct_t*) size;
                fat_lba = lba_data_ptr->offset / FATBytesPerSec;

                write_callback(fat_lba, lba_data_ptr->size);
            }
            else
            {
                *msc_buffer_ptr_ptr = NULL;
            }
            break;

        case USB_MSC_DEVICE_FORMAT_COMPLETE :
            break;

        case USB_MSC_DEVICE_REMOVAL_REQUEST :
            break;

        case USB_MSC_DEVICE_GET_INFO :
            device_lba_info_ptr = (device_lba_info_struct_t*) size;
            device_lba_info_ptr->total_lba_device_supports = TOTAL_LOGICAL_ADDRESS_BLOCKS;
            device_lba_info_ptr->length_of_each_lab_of_device = FATBytesPerSec;
            device_lba_info_ptr->num_lun_supported = LOGICAL_UNIT_SUPPORTED;
            break;

        default :
            break;
    }

    return error;
}

void usbMscDeinit(void)
{
    USB_Class_MSC_Deinit(msd_app_handle);
}

void usbMscInit(read_callback_t read, write_callback_t write, data_callback_t data)
{
	msc_config_struct_t msd_config;

    memset(&msd_config, 0, sizeof(msc_config_struct_t));

	msd_config.msc_application_callback.callback = USB_App_Device_Callback;
	msd_config.class_specific_callback.callback = USB_App_Class_Callback;
	msd_config.desc_callback_ptr = &desc_callback;

	USB_Class_MSC_Init(0, &msd_config, &msd_app_handle);

    read_callback = read;
    write_callback = write;
    data_callback = data;
}

