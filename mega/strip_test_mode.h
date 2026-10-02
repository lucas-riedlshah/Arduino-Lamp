#ifndef STRIP_TEST_MODE_H
#define STRIP_TEST_MODE_H

#include "ModeInterface.h"

class StripTestMode : public ModeInterface {
public:
    void setup() override;
    void loop() override {}
};

#endif
