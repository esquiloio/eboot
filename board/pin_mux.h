#ifndef pin_mux_H_
#define pin_mux_H_

typedef enum {
    kPinMuxProgSw,
    kPinMuxStatusLed,
    kPinMuxSdhc,
    kPinMuxSdhcCd,
    kPinMuxI2c2,
    kPinMuxUart4,
    kPinMuxJtag,
} PinMuxId;

void pin_mux_configure(PinMuxId id);

#endif
