#define TRIGGER 0x03
#define MODE 0x04

#include "AP_Camera_Runcam6.h"

#if AP_CAMERA_RUNCAM6_ENABLED

#include <AP_SerialManager/AP_SerialManager.h>
#include <AP_Math/crc.h>

// update - should be called at 50hz
void AP_Camera_Runcam6::update()
{
    if (trigger_counter > 0) {
        trigger_counter--;
    }

    // call parent update
    AP_Camera_Backend::update();
}

void AP_Camera_Runcam6::init()
{
    // configure serial communication

    AP_SerialManager *serial_manager = AP_SerialManager::get_singleton();

    if (serial_manager) {
        uart = serial_manager->find_serial(AP_SerialManager::SerialProtocol_RunCam, 0);
    }

    if (uart == nullptr) 
    {
        return;
    }

    start_uart();
}

// entry point to actually take a picture.  returns true on success
bool AP_Camera_Runcam6::trigger_pic()
{
    // fail if have not completed previous picture
    if (trigger_counter > 0) {
        return false;
    }

    // set camera taking photo
    drain();
    send_command(TRIGGER);

    // set counter to enable next photo after this many iterations of update (assumes 50hz update rate)
    trigger_counter = constrain_float(_params.trigger_duration * 50, 0, UINT16_MAX);

    return true;
}

void AP_Camera_Runcam6::cam_mode_toggle() 
{
    // fail if have not completed previous picture
    if (trigger_counter > 0) {
        return false;
    }

    // set camera taking photo
    drain();
    send_command(MODE);

    // set counter to enable next photo after this many iterations of update (assumes 50hz update rate)
    trigger_counter = constrain_float(_params.trigger_duration * 50, 0, UINT16_MAX);
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

void AP_Camera_Runcam6::send_command(uint8_t cmd)
{
    // is this device open?
    if (!uart) {
        return;
    }

    uint8_t buffer[4] = { 0xCC, 0x01, 0x00, 0x00 };

    buffer[2] = cmd;

    uint8_t crc = 0;
    for (uint8_t i = 0; i < 3; i++) {
        crc = crc8_dvb_s2(crc, buffer[i]);
    }

    buffer[3] = crc;

    // send data if possible
    uart->write(buffer, 4);
    uart->flush();
}

void AP_Camera_Runcam6::drain()
{
    if (!uart) {
        return;
    }

    uart->discard_input();
}

#endif // AP_CAMERA_RUNCAM6_ENABLED
