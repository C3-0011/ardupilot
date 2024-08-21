#include "AP_Camera_Runcam6.h"

#if AP_CAMERA_RUNCAM6_ENABLED

#include <AP_SerialManager/AP_SerialManager.h>

// update - should be called at 50hz
void AP_Camera_Runcam6::update()
{
    if (trigger_counter > 0) {
        trigger_counter--;
    } else {
        // set camera not taking photo

    }

    // call parent update
    AP_Camera_Backend::update();
}

void AP_Camera_Runcam6::init()
{
    // configure serial communication
}

// entry point to actually take a picture.  returns true on success
bool AP_Camera_Runcam6::trigger_pic()
{
    // fail if have not completed previous picture
    if (trigger_counter > 0) {
        return false;
    }

    // set camera taking photo


    // set counter to enable next photo after this many iterations of update (assumes 50hz update rate)
    trigger_counter = constrain_float(_params.trigger_duration * 50, 0, UINT16_MAX);

    return true;
}

void AP_Camera_Runcam6::start_uart()
{
    // 8N1 communication
    uart->configure_parity(0);
    uart->set_stop_bits(1);
    uart->set_flow_control(AP_HAL::UARTDriver::FLOW_CONTROL_DISABLE);
    uart->set_options(uart->get_options() | AP_HAL::UARTDriver::OPTION_NODMA_TX | AP_HAL::UARTDriver::OPTION_NODMA_RX);
    uart->begin(115200, 10, 10);
    uart->discard_input();
}

void AP_Camera_Runcam6::drain()
{
    if (!uart) {
        return;
    }

    uart->discard_input();
}

#endif // AP_CAMERA_RUNCAM6_ENABLED
