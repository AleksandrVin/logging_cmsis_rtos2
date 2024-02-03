// additional include for USB logging


#if INTERFACE == USE_USB_CDC
#define INTERFACE_printf(...)         \
    while (CDC_IsBusy() == USBD_BUSY && CDC_WAIT_FOR_PC_TO_READ == 1) \
    {                                 \
        osThreadYield();              \
    }                                 \
    CDC_Transmit_FS((uint8_t *)CDC_USB_TX_BUF, snprintf(CDC_USB_TX_BUF, INTERFACE_CDC_BUFFER_SIZE, __VA_ARGS__))