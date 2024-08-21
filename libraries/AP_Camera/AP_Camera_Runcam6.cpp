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

#endif // AP_CAMERA_RUNCAM6_ENABLED
