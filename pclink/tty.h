#pragma once

#include <termios.h>
#include <unistd.h>

int ttyOpen(char const * portName, int baud);