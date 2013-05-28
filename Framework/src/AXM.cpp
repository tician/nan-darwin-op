/*
 *   AXM.cpp
 *
 *  
 *
 */
#include "AXM.h"

using namespace Robot;

const int AXM::MIN_VALUE = 0;

const int AXM::CENTER_VALUE = 512;
const int AXM::MAX_VALUE = 1023;
const double AXM::MIN_ANGLE = -150.0; // degree
const double AXM::MAX_ANGLE = 150.0; // degree
const double AXM::RATIO_VALUE2ANGLE = 0.293; // 300 / 1024
const double AXM::RATIO_ANGLE2VALUE = 3.413; // 1024 / 300

const int AXM::PARAM_BYTES = 5;
