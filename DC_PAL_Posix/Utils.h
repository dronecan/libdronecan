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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>

#define dronecan_assert(x, message, ...) fprintf(stderr, message "\n", ##__VA_ARGS__); assert(x);
#define DC_MALLOC malloc
#define DC_FREE free
