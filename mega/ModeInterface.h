#ifndef MODE_INTERFACE_H
#define MODE_INTERFACE_H

class ModeInterface {
public:
    virtual void setup() = 0;   // Called when mode is activated
    virtual void loop() = 0;    // Called every frame while active
    virtual void cleanup() {}   // Optional: called when mode is deactivated
    virtual ~ModeInterface() {}
};

#endif
