#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cbd90 { uint8_t Unknown00[0x18]; uint8_t Value18; };
#pragma pack(pop)

// Returns the byte at +0x18 of the object in al (mov al, [ecx+0x18]); the upper
// bits of eax are left as the caller had them, so eax is a real input.
// FUNCTION: GP4 0x005cbd90
void __cdecl FUN_005cbd90(gp4::Regs* r)
{
    const Unknown_005cbd90* self = (const Unknown_005cbd90*)r->ecx;
    uint32_t eax = r->eax;
    r->eax = (eax & 0xffffff00u) | self->Value18;
}
GP4_IMPL(0x005cbd90, FUN_005cbd90, "regs(ecx:ptr[0x20]:bytes, eax:u32) -> (eax)")
