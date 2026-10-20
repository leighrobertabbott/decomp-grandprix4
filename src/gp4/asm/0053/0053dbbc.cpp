#include <gp4/gp4.h>
#include <gp4/flags.h>

// Switch on the selector edx (10 or more clamps to 9) over the flag byte at +0x1f of the esi record,
// through the jump table at 0x0053dbd4:
//   0, 1       nothing
//   2, 3, 8    bit 7 set
//   4, 7       bit 7 cleared
//   5, 6       dword 0x0082fd88 cleared, then bit 7 cleared
//   9 (other)  empty routine 0x0054e232 (inside a pushal/popal), then bit 7 set
// edx comes back as the jump-table offset: (clamped selector * 4) + 0x0053dbd4, and the flags are those
// of that `add`, or of the final `or`/`and` on the flag byte when a case touches it.
// FUNCTION: GP4 0x0053dbbc
void __cdecl FUN_0053dbbc(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t selector = r->edx;
    if (selector >= 0xau) {
        selector = 9u;
    }
    const uint32_t offset = selector << 2;
    uint32_t arith = gp4::flags::add32(offset, 0x0053dbd4u);

    switch (selector) {
    case 0:
    case 1:
        break;
    case 2:
    case 3:
    case 8: {
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x1f) | 0x80u);
        GP4_FIELD(uint8_t, record, 0x1f) = value;
        arith = gp4::flags::logic8(value);
        break;
    }
    case 5:
    case 6:
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        // fall through
    case 4:
    case 7: {
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x1f) & 0x7fu);
        GP4_FIELD(uint8_t, record, 0x1f) = value;
        arith = gp4::flags::logic8(value);
        break;
    }
    default: {
        gp4::cdecl_<0x0054e232, void>();
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x1f) | 0x80u);
        GP4_FIELD(uint8_t, record, 0x1f) = value;
        arith = gp4::flags::logic8(value);
        break;
    }
    }

    r->edx = offset + 0x0053dbd4u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053dbbc, FUN_0053dbbc, "regs(edx:u32[0..12], esi:ptr[0x40]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
