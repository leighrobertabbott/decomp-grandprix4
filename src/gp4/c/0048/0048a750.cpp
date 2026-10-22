#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Unknown_0048a750 {
    uint8_t Unknown00[0x30];
    uint32_t Words30[25];
    uint8_t Unknown94[0x4f];
    uint8_t Bytes0e3[26];
    uint8_t Unknown0fd[3];
    uint32_t Words100[26];
};
#pragma pack(pop)

// Copies the pending staged tables (25 dwords from 0x00a4d460 to +0x30, 25 bytes from 0x00a4d42c to
// +0xe4, 25 dwords from 0x00a4d3c8 to +0x100) into the object when the "pending" byte 0x00a4d694 is
// set, then clears that byte. The original only loads al when nothing is pending (the other bytes of
// eax stay as the caller left them); after a copy eax = 0x19, ecx = the last staged dword,
// edx = object + 0x164. The flags end as the result of a zero compare in both paths.
// FUNCTION: GP4 0x0048a750
void __cdecl FUN_0048a750(gp4::Regs* r) {
    Unknown_0048a750* self = (Unknown_0048a750*)r->ecx;
    uint32_t eax = r->eax;
    if (GP4_GLOBAL(uint8_t, 0x00a4d694) != 0) {
        uint32_t last = 0;
        int i = 0;
        do {
            self->Words30[i] = GP4_ARRAY(uint32_t, 0x00a4d460)[i];
            int n = i + 1;
            self->Bytes0e3[n] = GP4_ARRAY(uint8_t, 0x00a4d42b)[n];
            last = GP4_ARRAY(uint32_t, 0x00a4d3c4)[n];
            self->Words100[i] = last;
            i = n;
        } while (i < 0x19);
        GP4_GLOBAL(uint8_t, 0x00a4d694) = 0;
        r->eax = (uint32_t)i;
        r->ecx = last;
        r->edx = (uint32_t)(uintptr_t)self + 0x164;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32((uint32_t)i, 0x19));
    } else {
        r->eax = eax & 0xffffff00u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0));
    }
}
GP4_IMPL(0x0048a750, FUN_0048a750, "regs(ecx:ptr[0x170]:bytes, eax:u32) -> (eax, ecx, edx); globals=fuzz:i32")
