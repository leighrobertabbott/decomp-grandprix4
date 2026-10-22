#include <gp4/gp4.h>

// Object whose vtable slot 0x4c (index 19) is a three-argument method.
struct Unknown_005a86d0_Target {
    virtual void Slot00() = 0;  virtual void Slot01() = 0;  virtual void Slot02() = 0;  virtual void Slot03() = 0;
    virtual void Slot04() = 0;  virtual void Slot05() = 0;  virtual void Slot06() = 0;  virtual void Slot07() = 0;
    virtual void Slot08() = 0;  virtual void Slot09() = 0;  virtual void Slot10() = 0;  virtual void Slot11() = 0;
    virtual void Slot12() = 0;  virtual void Slot13() = 0;  virtual void Slot14() = 0;  virtual void Slot15() = 0;
    virtual void Slot16() = 0;  virtual void Slot17() = 0;  virtual void Slot18() = 0;
    virtual int Slot19(int a, int b, int c) = 0;
};

#pragma pack(push, 1)
struct Unknown_005a86d0_Link { uint8_t Unknown00[8]; Unknown_005a86d0_Target* Target08; };
struct Unknown_005a86d0 { uint8_t Unknown00[0xc]; Unknown_005a86d0_Link* Link0c; };
#pragma pack(pop)

// Forwards its three stack arguments to virtual slot 0x4c of the object reached through +0xc
// then +8, with the first two swapped (the callee receives second, first, third); returns 1 when
// either link is null.
// FUNCTION: GP4 0x005a86d0
int __fastcall FUN_005a86d0(Unknown_005a86d0* self, void* /*edx*/, int a, int b, int c) {
    if (self->Link0c == 0) {
        return 1;
    }
    Unknown_005a86d0_Target* target = self->Link0c->Target08;
    if (target == 0) {
        return 1;
    }
    return target->Slot19(b, a, c);
}
GP4_IMPL(0x005a86d0, FUN_005a86d0, "thiscall(ptr[0x10]:{0xc:nptr[0x10]:{8:nptr[8]:{0:ptr[0x50]:{0x4c:u32{=0x005a8350}}}}}, i32, i32, i32) -> i32")
