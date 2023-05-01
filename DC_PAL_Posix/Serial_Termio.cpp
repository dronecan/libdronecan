// /*
// Copyright 2023 Siddharth Bharat Purohit, Cubepilot Pty Ltd.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// */
// #include "Serial_Termio.h"
// #include <fcntl.h>
// #include <unistd.h>
// #include <termio.h>
// #include <stdio.h>
// #include <DC_PAL/DC_PAL.h>

// using namespace dronecan::posix;

// bool Serial_Termio::init()
// {
//     fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
//     if (fd < 0) {
//         return false;
//     }

//     struct termios tty;
//     memset(&tty, 0, sizeof(tty));
//     if (tcgetattr(fd, &tty) != 0) {
//         return false;
//     }

//     cfsetospeed(&tty, baudrate);
//     cfsetispeed(&tty, baudrate);

//     // 8N1
//     tty.c_cflag &= ~PARENB; // no parity
//     tty.c_cflag &= ~CSTOPB; // 1 stop bit
//     tty.c_cflag &= ~CSIZE;  // clear size
//     tty.c_cflag |= CS8;     // 8 bits
//     tty.c_cflag &= ~CRTSCTS; // no hardware flow control

//     // no software flow control
//     tty.c_iflag &= ~(IXON | IXOFF | IXANY);

//     // raw input
//     tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

//     // raw output
//     tty.c_oflag &= ~OPOST;

//     if (tcsetattr(fd, TCSANOW, &tty) != 0) {
//         return false;
//     }

//     // create a thread to read from the serial port
//     auto rx_thread = allocate_obj_callback(this, &Serial_Termio::rx_thread);
//     if (!rx_thread) {
//         return false;
//     }
//     rx_thread = dronecan::pal().threads.start(rx_thread, "serial_rx", DRONECAN_IO_STACK_SIZE, DRONECAN_IO_PRIO);

//     return true;
// }

// bool Serial_Termio::send(uint8_t* payload, size_t length)
// {
//     if (fd < 0) {
//         return false;
//     }

//     size_t bytes_written = 0;
//     while (bytes_written < length) {
//         ssize_t result = write(fd, payload + bytes_written, length - bytes_written);
//         if (result < 0) {
//             return false;
//         }
//         bytes_written += result;
//     }

//     return true;
// }
