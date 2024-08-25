#define TRIGGER 0x01
#define MODE 0x02

#include "AP_Camera_Runcam6.h"

#if AP_CAMERA_RUNCAM6_ENABLED

#include <AP_SerialManager/AP_SerialManager.h>
#include <AP_Math/crc.h>
#include <AP_Math/AP_Math.h>
#include "GCS_MAVLink/GCS.h"

// update - should be called at 50hz
void AP_Camera_Runcam6::update()
{
    // this may not be necessary but I've kept it so the uart doesn't accidentally end up sending
    // commands constantly
    if (trigger_counter > 0) {
        trigger_counter--;
    } else {
        // I don't know how init is called normally so this is in here in case init fails and needs
        // to be recalled until uart is not a nullpointer
        if (!initialized) init();
    }

    // call parent update
    AP_Camera_Backend::update();
}

void AP_Camera_Runcam6::init()
{
    // configure serial communication

    AP_SerialManager *serial_manager = AP_SerialManager::get_singleton();

    // new protocol created in the enum at value 50, the existing one at 26 may work as long as
    // the user doesn't enable both cam1 type 8 and the other runcam library at the same time,
    // but seperating them might make it more clear so I will leave it like this.
    if (serial_manager) {
        uart = serial_manager->find_serial(AP_SerialManager::SerialProtocol_Runcam6, 0);
    }

    if (uart == nullptr) 
    {
        // if init fails pause attempts to reinit or send messages for 6 seconds.
        trigger_counter += 300;
        return;
    }

    initialized = true;

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
        return;
    }

    // set camera change mode
    drain();
    send_command(MODE);

    // set counter to enable next photo after this many iterations of update (assumes 50hz update rate)
    trigger_counter = constrain_float(_params.trigger_duration * 50, 0, UINT16_MAX);
}

// this is copied directly from the other runcam library
void AP_Camera_Runcam6::start_uart()
{
    // 8N1 communication
    uart->configure_parity(0);
    uart->set_stop_bits(1);
    uart->set_flow_control(AP_HAL::UARTDriver::FLOW_CONTROL_DISABLE);
    uart->set_options(uart->get_options() | AP_HAL::UARTDriver::OPTION_NODMA_TX | AP_HAL::UARTDriver::OPTION_NODMA_RX | AP_HAL::UARTDriver::OPTION_PULLUP_RX | AP_HAL::UARTDriver::OPTION_PULLUP_TX);
    uart->begin(115200);
    uart->discard_input();
}

void AP_Camera_Runcam6::send_command(uint8_t cmd)
{
    
    // is this device open?
    if (!uart) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "RUNCAM6 COMMAND SEND *****FAIL*****");
        return;
    }

    GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "RUNCAM6 COMMAND SENT");

    // both camera control messages I'm using start with 0xCC 0x01 and are only 4 bytes long,
    // this could be changed to accomodate other messages
    uint8_t buffer[4] = { 0xCC, 0x01, 0x00, 0x00 };

    buffer[2] = cmd;

    // could totally just hardcode the crc byte because I'm only sending 2 unique messages,
    // but I'll leave it here in case the other runcam protocol functions are ever implemented.
    uint8_t crc = 0;
    for (uint8_t i = 0; i < 3; i++) {
        crc = crc8_dvb_s2(crc, buffer[i]);
    }

    buffer[3] = crc;

    // send data if possible
    uart->write(buffer, 4);
    uart->flush();
}

// not sure that this is necessary but the other runcam library uses it so I included it.
void AP_Camera_Runcam6::drain()
{
    if (!uart) {
        return;
    }

    uart->discard_input();
}

#endif // AP_CAMERA_RUNCAM6_ENABLED
