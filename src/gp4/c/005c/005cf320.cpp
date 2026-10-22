#include <gp4/gp4.h>

// Abstract view of the owned object: only the call through vtable slot 12 (+0x30) matters here.
struct Object_005cf320 {
    virtual void Slot00() = 0;  virtual void Slot01() = 0;  virtual void Slot02() = 0;  virtual void Slot03() = 0;
    virtual void Slot04() = 0;  virtual void Slot05() = 0;  virtual void Slot06() = 0;  virtual void Slot07() = 0;
    virtual void Slot08() = 0;  virtual void Slot09() = 0;  virtual void Slot10() = 0;  virtual void Slot11() = 0;
    virtual void Slot12(Object_005cf320* arg) = 0;
};

#pragma pack(push, 1)
struct Unknown_005cf320 {
    Object_005cf320* Object00;
    uint8_t Unknown04[4];
    int32_t Value08;
    int32_t Value0c;
    uint8_t Unknown10[0xf0];
};
#pragma pack(pop)

// Copies the dword at +8 to +0xc, then, if the object pointer at +0 is non-null, calls its virtual
// slot 12 (vtable +0x30) passing the object itself as the stack argument.
// FUNCTION: GP4 0x005cf320
void __fastcall FUN_005cf320(Unknown_005cf320* self, void* /*edx*/) {
    self->Value0c = self->Value08;
    Object_005cf320* object = self->Object00;
    if (object != nullptr) {
        object->Slot12(object);
    }
}
GP4_IMPL(0x005cf320, FUN_005cf320, "thiscall(ptr[0x100]:{0:nptr[0x100]:{0:u32{=0x5fa62c}}, 8:u32}) -> void")
