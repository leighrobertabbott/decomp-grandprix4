#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the 7-byte slot-order table at 0x8273f7 for the first slot whose dword at ebx+0x14+slot*4 is nonzero
// and stores that slot number at [ebx+0xc]. CF = 0 when a slot was found, CF = 1 when all 7 are empty (the
// other flags are those of the final `cmp ecx, 7`). eax and ecx are saved and restored.
// FUNCTION: GP4 0x00530d81
void __cdecl FUN_00530d81(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(r->ebx);
    const uint8_t* order = GP4_ARRAY(const uint8_t, 0x008273f7);

    uint32_t i = 0;
    uint32_t slot = 0;
    do {
        slot = order[i];
        if (GP4_FIELD(uint32_t, rec, 0x14 + slot * 4u) != 0u) {
            break;
        }
        ++i;
    } while (i != 7u);

    const bool found = i < 7u;
    if (found) {
        GP4_FIELD(uint32_t, rec, 0xc) = slot;
    }
    const uint32_t arith = (gp4::flags::sub32(i, 7u) & ~gp4::flags::CF) | (found ? 0u : gp4::flags::CF);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00530d81, FUN_00530d81, "regs(ebx:ptr[0x440]:u32{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_008273f7")
