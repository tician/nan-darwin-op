/*
 *   AXM.cpp
 *
 *  
 *
 */
#include "AXM.h"

using namespace Robot;

const int MX28::MIN_VALUE = 0;

const int MX28::CENTER_VALUE = 512;
const int MX28::MAX_VALUE = 1023;
const double MX28::MIN_ANGLE = -150.0; // degree
const double MX28::MAX_ANGLE = 150.0; // degree
const double MX28::RATIO_VALUE2ANGLE = 0.293; // 300 / 1024
const double MX28::RATIO_ANGLE2VALUE = 3.413; // 1024 / 300

const int MX28::PARAM_BYTES = 5;
