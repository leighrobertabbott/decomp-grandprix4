#include <gp4/gp4.h>

// Abstract view of the object: only the call through vtable slot 26 (+0x68) matters here.
struct Object_005ceaa0 {
    virtual void Slot00() = 0;  virtual void Slot01() = 0;  virtual void Slot02() = 0;  virtual void Slot03() = 0;
    virtual void Slot04() = 0;  virtual void Slot05() = 0;  virtual void Slot06() = 0;  virtual void Slot07() = 0;
    virtual void Slot08() = 0;  virtual void Slot09() = 0;  virtual void Slot10() = 0;  virtual void Slot11() = 0;
    virtual void Slot12() = 0;  virtual void Slot13() = 0;  virtual void Slot14() = 0;  virtual void Slot15() = 0;
    virtual void Slot16() = 0;  virtual void Slot17() = 0;  virtual void Slot18() = 0;  virtual void Slot19() = 0;
    virtual void Slot20() = 0;  virtual void Slot21() = 0;  virtual void Slot22() = 0;  virtual void Slot23() = 0;
    virtual void Slot24() = 0;  virtual void Slot25() = 0;
    virtual int Slot26(int mode) = 0;
};

// Calls virtual slot 26 (vtable +0x68) of the object with the argument 2:
// `mov eax, [ecx]; push 2; call [eax + 0x68]; ret` (eax is whatever the slot returns).
// FUNCTION: GP4 0x005ceaa0
int __fastcall FUN_005ceaa0(Object_005ceaa0* self, void* /*edx*/) {
    return self->Slot26(2);
}
GP4_IMPL(0x005ceaa0, FUN_005ceaa0, "thiscall(ptr[0x21498]:u32{=0x5fa62c}) -> i32")
