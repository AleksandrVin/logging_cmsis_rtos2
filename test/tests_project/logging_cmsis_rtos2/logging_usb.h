// additional include for USB logging
#include "usbd_cdc_if.h"
#include "usb_device.h"

#define INTERFACE_CDC_BUFFER_SIZE 200

extern char CDC_USB_RX_BUF[INTERFACE_CDC_BUFFER_SIZE];
extern char CDC_USB_TX_BUF[INTERFACE_CDC_BUFFER_SIZE];

#define CDC_WAIT_FOR_PC_TO_READ 1

#define INTERFACE_printf(FATAL_FLAG, ...)                                           \
    while (CDC_IsBusy() == USBD_BUSY && CDC_WAIT_FOR_PC_TO_READ == 1)   \
    {                                                                   \
        osThreadYield();                                                \
    }                                                                   \
    CDC_Transmit_FS((uint8_t *)CDC_USB_TX_BUF,                          \
                    snprintf(CDC_USB_TX_BUF, INTERFACE_CDC_BUFFER_SIZE, __VA_ARGS__));

