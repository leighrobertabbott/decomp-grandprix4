#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the car-code list 0x0066d8f4 (length dword 0x0066d8ec): every entry with bit 7 set is a
// flagged car (counted in ebp -> word 0x007c2efc); of those the ones with bit 6 clear are also
// counted in eax -> word 0x007c2eb8. ebx/ecx are saved and restored. The flags are those of the last
// flag writer: "or ecx,ecx" for an empty list, otherwise the "inc ebx" closing the last iteration
// (loop leaves flags alone; CF comes from the preceding test/add, which is always 0).
// FUNCTION: GP4 0x00522ab8
void __cdecl FUN_00522ab8(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    const uint8_t* list = GP4_ARRAY(uint8_t, 0x0066d8f4);

    uint32_t counted = 0u;   // eax
    uint32_t flagged = 0u;   // ebp
    uint32_t arith = gp4::flags::logic32(count);

    for (uint32_t i = 0u; i < count; ++i) {
        const uint8_t code = list[i];
        arith = gp4::flags::logic8(code);
        if ((code & 0x80u) != 0u) {
            arith = gp4::flags::add32(flagged, 1u);
            flagged += 1u;
            arith = gp4::flags::logic8(code & 0x40u);
            if ((code & 0x40u) == 0u) {
                arith = gp4::flags::add32(counted, 1u);
                counted += 1u;
            }
        }
        arith = gp4::flags::inc32(0x0066d8f4u + i, arith);
    }

    GP4_GLOBAL(uint16_t, 0x007c2eb8) = (uint16_t)counted;
    GP4_GLOBAL(uint16_t, 0x007c2efc) = (uint16_t)flagged;

    r->eax = counted;
    r->ebp = flagged;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522ab8, FUN_00522ab8, "regs() -> (eax, ebp, cf, pf, af, zf, sf, of); globals=layout=list_00443278")
