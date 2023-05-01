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

#pragma once

#if defined(DRONECAN_USER_CONFIG)
#include <DC_UserConfig.h>
#endif

#ifndef DRONECAN_BASE_PRIO
#define DRONECAN_BASE_PRIO 10
#endif

#define DRONECAN_PRIO(x) (DRONECAN_BASE_PRIO + x)

#ifndef DRONECAN_IO_PRIO
#define DRONECAN_IO_PRIO 1
#endif

#ifndef DRONECAN_IO_STACK_SIZE
#define DRONECAN_IO_STACK_SIZE 1024
#endif
