#include "fsl_device_registers.h"
#include "fsl_port_hal.h"
#include "fsl_sim_hal.h"
#include "pin_mux.h"

void pin_mux_configure(PinMuxId id)
{
  switch(id) {
    case kPinMuxProgSw:
        PORT_HAL_SetMuxMode(PORTC,2u,kPortMuxAsGpio);
        PORT_HAL_SetPullMode(PORTC,2u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTC,2u,true);
        break;

    case kPinMuxStatusLed:
        PORT_HAL_SetMuxMode(PORTC,2u,kPortMuxAsGpio);
        PORT_HAL_SetDriveStrengthMode(PORTC,2u,kPortHighDriveStrength);
        break;

    case kPinMuxSdhcCd:
        PORT_HAL_SetMuxMode(PORTE,6u,kPortMuxAsGpio);
        PORT_HAL_SetPullMode(PORTE,6u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,6u,true);
        break;

    case kPinMuxSdhc:
        PORT_HAL_SetMuxMode(PORTE,3u,kPortMuxAlt4);
        PORT_HAL_SetPullMode(PORTE,3u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,3u,true);
        PORT_HAL_SetDriveStrengthMode(PORTE,3u,kPortHighDriveStrength);

        PORT_HAL_SetMuxMode(PORTE,1u,kPortMuxAlt4);
        PORT_HAL_SetPullMode(PORTE,1u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,1u,true);
        PORT_HAL_SetDriveStrengthMode(PORTE,1u,kPortHighDriveStrength);

        PORT_HAL_SetMuxMode(PORTE,0u,kPortMuxAlt4);
        PORT_HAL_SetPullMode(PORTE,0u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,0u,true);
        PORT_HAL_SetDriveStrengthMode(PORTE,0u,kPortHighDriveStrength);

        PORT_HAL_SetMuxMode(PORTE,5u,kPortMuxAlt4);
        PORT_HAL_SetPullMode(PORTE,5u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,5u,true);
        PORT_HAL_SetDriveStrengthMode(PORTE,5u,kPortHighDriveStrength);

        PORT_HAL_SetMuxMode(PORTE,4u,kPortMuxAlt4);
        PORT_HAL_SetPullMode(PORTE,4u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,4u,true);
        PORT_HAL_SetDriveStrengthMode(PORTE,4u,kPortHighDriveStrength);

        PORT_HAL_SetMuxMode(PORTE,2u,kPortMuxAlt4);
        PORT_HAL_SetPullMode(PORTE,2u,kPortPullUp);
        PORT_HAL_SetPullCmd(PORTE,2u,true);
        PORT_HAL_SetDriveStrengthMode(PORTE,2u,kPortHighDriveStrength);
        break;

    case kPinMuxI2c2:
        PORT_HAL_SetMuxMode(PORTA,12u,kPortMuxAlt5);
        PORT_HAL_SetOpenDrainCmd(PORTA,12u,true);
        PORT_HAL_SetMuxMode(PORTA,13u,kPortMuxAlt5);
        PORT_HAL_SetOpenDrainCmd(PORTA,13u,true);
        break;

    case kPinMuxUart4:
        PORT_HAL_SetMuxMode(PORTC,14u,kPortMuxAlt3);
        PORT_HAL_SetMuxMode(PORTC,15u,kPortMuxAlt3);
        break;

    case kPinMuxJtag:
#if RELEASE
        PORT_HAL_SetMuxMode(PORTA,0u,kPortPinDisabled);
        PORT_HAL_SetMuxMode(PORTA,1u,kPortPinDisabled);
        PORT_HAL_SetMuxMode(PORTA,2u,kPortPinDisabled);
        PORT_HAL_SetMuxMode(PORTA,3u,kPortPinDisabled);
#endif        
        PORT_HAL_SetMuxMode(PORTA,4u,kPortPinDisabled);
        break;

    default:
        break;
  }
}

