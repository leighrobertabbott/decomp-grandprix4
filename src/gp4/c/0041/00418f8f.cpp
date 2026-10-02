#include <gp4/gp4.h>

struct Unknown_00679d30 { uint8_t Unknown00[0x7d]; uint8_t Unknown7D; };
struct Unknown_0066fef0 { uint8_t Unknown00[0x7a]; uint8_t Flags7A; uint8_t Unknown7B[0x12]; uint8_t Flags8D; };

// FUNCTION: GP4 0x00418f8f
void __cdecl FUN_00418f8f(gp4::Regs* r) {
    const Unknown_00679d30* owner = GP4_GLOBAL(const Unknown_00679d30*, 0x00679d30);
    uint32_t index = owner->Unknown7D;
    for (;;) {
        index += 2;
        const uint16_t offset = GP4_FIELD(uint16_t, GP4_ARRAY(uint8_t, 0x0066fe3c), index);
        if (offset == 0xffff) {
            // stc after "cmp si, -1" found equality: CF, ZF, PF set.
            r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
            return;
        }
        const Unknown_0066fef0* entry =
            reinterpret_cast<const Unknown_0066fef0*>(static_cast<uintptr_t>(0x0066fef0u + offset));
        if ((entry->Flags7A & 0x80) != 0) continue;
        if ((entry->Flags8D & 2) != 0) continue;
        // clc after "test [esi + 0x8d], 2" found zero: ZF, PF set, CF clear.
        r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
}
GP4_IMPL(0x00418f8f, FUN_00418f8f, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00418f8f")
