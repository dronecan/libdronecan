/*
Copyright 2023 Siddharth Bharat Purohit, Cubepilot Pty Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// Router App using Ardupilot DataIface
#if defined(ARDUPILOT_BUILD)
#include <AP_HAL/AP_HAL.h>
#include <dronecan/DataIface_ArduPilot.h>

const AP_HAL::HAL &hal = AP_HAL::get_HAL();

void setup(void);
void loop(void);

void setup(void)
{
}

void loop(void)
{
}

AP_HAL_MAIN();

#endif // ARDUPILOT_BUILD
