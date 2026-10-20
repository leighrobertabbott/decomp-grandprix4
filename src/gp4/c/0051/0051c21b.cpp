#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// A 0x20-byte record of the table [0x6975dc]: a dword whose low byte is the sort key, flags at +0x14.
#pragma pack(push, 1)
struct Unknown_Rec20 {
    uint32_t Key00;
    uint8_t Unknown04[0x10];
    uint16_t Flags14;
    uint8_t Unknown16[10];
};
#pragma pack(pop)

// FUNCTION: GP4 0x0051c21b
// Picks, among the first [0x96a368] records of the table [0x6975dc] whose flag word +0x14 has no 0x80 and whose
// slot byte [0x965914 + index] is not 0xff, the one with the smallest ((key & 0xff) - al) & 0xff, marks its
// slot 0xff and returns the distance in eax and the record in ebx. With no candidate the distance stays
// 0x7fffffff and the slot index / record come from the caller's ebp / edi. Carry is set (stc) when the
// distance is above 0xff, else cleared (clc); the other flags are those of `cmp eax, 0xff`. A table of 0x80 or
// more records hits an int3 assertion. esi, edx, edi, ecx and ebp are preserved.
void __cdecl FUN_0051c21b(gp4::Regs* r) {
    const uint32_t target = r->eax;
    const Unknown_Rec20* rec = reinterpret_cast<const Unknown_Rec20*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006975dc)));
    uint32_t best = 0x7fffffffu;
    uint32_t bestIndex = r->ebp;
    uint32_t bestRec = r->edi;
    uint32_t index = 0u;
    do {
        if ((rec->Flags14 & 0x80u) == 0u && GP4_ARRAY(uint8_t, 0x00965914)[index] != 0xffu) {
            const uint32_t distance = ((rec->Key00 & 0xffu) - target) & 0xffu;
            if (distance < best) {
                best = distance;
                bestIndex = index;
                bestRec = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec));
            }
        }
        rec += 1;
        index += 1u;
    } while (index != GP4_GLOBAL(uint32_t, 0x0096a368));
    if (index >= 0x80u) {
        __debugbreak();
    }
    GP4_ARRAY(uint8_t, 0x00965914)[bestIndex] = 0xffu;
    uint32_t arith = gp4::flags::sub32(best, 0xffu);
    if (best <= 0xffu) {
        arith &= ~gp4::flags::CF;
    } else {
        arith |= gp4::flags::CF;
    }
    r->eax = best;
    r->ebx = bestRec;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051c21b, FUN_0051c21b, "regs(eax:u32, edi:u32, ebp:u32[0..7]) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=layout=count_0096a368+recs_0051c21b")
