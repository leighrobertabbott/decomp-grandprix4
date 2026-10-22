#include <gp4/gp4.h>

// Object whose vtable slot 0xd4 (index 53) takes a 16-bit value and a second value.
struct Unknown_005ab000_Target {
    virtual void Slot00() = 0;  virtual void Slot01() = 0;  virtual void Slot02() = 0;  virtual void Slot03() = 0;
    virtual void Slot04() = 0;  virtual void Slot05() = 0;  virtual void Slot06() = 0;  virtual void Slot07() = 0;
    virtual void Slot08() = 0;  virtual void Slot09() = 0;  virtual void Slot10() = 0;  virtual void Slot11() = 0;
    virtual void Slot12() = 0;  virtual void Slot13() = 0;  virtual void Slot14() = 0;  virtual void Slot15() = 0;
    virtual void Slot16() = 0;  virtual void Slot17() = 0;  virtual void Slot18() = 0;  virtual void Slot19() = 0;
    virtual void Slot20() = 0;  virtual void Slot21() = 0;  virtual void Slot22() = 0;  virtual void Slot23() = 0;
    virtual void Slot24() = 0;  virtual void Slot25() = 0;  virtual void Slot26() = 0;  virtual void Slot27() = 0;
    virtual void Slot28() = 0;  virtual void Slot29() = 0;  virtual void Slot30() = 0;  virtual void Slot31() = 0;
    virtual void Slot32() = 0;  virtual void Slot33() = 0;  virtual void Slot34() = 0;  virtual void Slot35() = 0;
    virtual void Slot36() = 0;  virtual void Slot37() = 0;  virtual void Slot38() = 0;  virtual void Slot39() = 0;
    virtual void Slot40() = 0;  virtual void Slot41() = 0;  virtual void Slot42() = 0;  virtual void Slot43() = 0;
    virtual void Slot44() = 0;  virtual void Slot45() = 0;  virtual void Slot46() = 0;  virtual void Slot47() = 0;
    virtual void Slot48() = 0;  virtual void Slot49() = 0;  virtual void Slot50() = 0;  virtual void Slot51() = 0;
    virtual void Slot52() = 0;
    virtual int Slot53(int16_t a, int b) = 0;
};

#pragma pack(push, 1)
struct Unknown_005ab000 {
    uint8_t Unknown00[0x1c];
    Unknown_005ab000_Target* Target1c;
    uint8_t Unknown20[0x34];
    int16_t Value54;
    uint8_t Flag56;
    uint8_t Unknown57[0x17];
    int16_t Value6e;
};
#pragma pack(pop)

// Stores `value` (low 16 bits, clamped below at -1000) at +0x6e and, when the byte at +0x56 is 1,
// forwards the 16-bit value at +0x54 and `value` to virtual slot 0xd4 of the object at +0x1c and
// returns its result; otherwise returns 0.
// FUNCTION: GP4 0x005ab000
int __fastcall FUN_005ab000(Unknown_005ab000* self, void* /*edx*/, int value) {
    if ((int16_t)value < -1000) {
        value = -1000;
    }
    self->Value6e = (int16_t)value;
    if (self->Flag56 == 1) {
        return self->Target1c->Slot53(self->Value54, value);
    }
    return 0;
}
GP4_IMPL(0x005ab000, FUN_005ab000, "thiscall(ptr[0x80]:{0x1c:ptr[0x10]:{0:ptr[0xd8]:{0xd4:u32{=0x005a82a0}}}, 0x54:u16, 0x56:u8[0..2]}, i32) -> i32")
