#include <gp4/gp4.h>

// Object at +0x234 of the sound manager (class with vtable 0x005f9e7c, built by 0x005b18c0):
// slot 0 is the scalar deleting destructor, slot 1 an init method, slot 2 the method called here.
struct Unknown_5f9e7c {
    virtual ~Unknown_5f9e7c() = 0;
    virtual int Method04() = 0;
    virtual void Method08() = 0;
    static void operator delete(void*) {}
};

#pragma pack(push, 1)
struct Unknown_4d77a0 {
    uint8_t Unknown00[8];
    uint8_t Flag08;
    uint8_t Unknown09[0x234 - 9];
    Unknown_5f9e7c* Holder234;
};
#pragma pack(pop)

// FUNCTION: GP4 0x004d77a0
void __fastcall FUN_004d77a0(Unknown_4d77a0* self, void* /*edx*/) {
    self->Flag08 = 0;
    if (self->Holder234 != nullptr) {
        self->Holder234->Method08();
        delete self->Holder234;
        self->Holder234 = nullptr;
    }
}
GP4_IMPL(0x004d77a0, FUN_004d77a0, "thiscall(ptr[0x274]:layout=gpaudio_004d77a0) -> void")
