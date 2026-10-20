#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00539ce4
// Menu value "step up" for the mode in edx: 1 clears edx and does nothing else, 2 and 3 load the
// value at [ebx] (width from the esi record, via 0x00539944), add the dword at +0x2a of the esi
// record, keep the result only when it did not overflow and is not above the dword at +0x26 (signed;
// otherwise the +0x26 dword is used), store it back (0x00539a13), and put the record id byte +0xc
// minus 1 in the byte [0x0081e4f8]. eax ends as (stored value with its low byte replaced by the id
// byte) - 1 and the flags are those of that `dec eax`. Other modes leave the flags of `cmp edx, 3`.
void __cdecl FUN_00539ce4(gp4::Regs* r) {
    const uint32_t mode = r->edx;
    if (mode == 1u) {
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(mode, 1u));
        return;
    }
    if (mode != 2u && mode != 3u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(mode, 3u));
        return;
    }

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    gp4::call_regs(0x00539944, c);

    const uint32_t value = c.eax;
    const uint32_t step = GP4_FIELD(uint32_t, record, 0x2a);
    const uint32_t ceiling = GP4_FIELD(uint32_t, record, 0x26);
    const uint32_t sum = gp4::flags::add32(value, step);
    uint32_t result = value + step;
    if ((sum & gp4::flags::OF) != 0u || static_cast<int32_t>(result) > static_cast<int32_t>(ceiling)) {
        result = ceiling;
    }

    c.eax = result;
    gp4::call_regs(0x00539a13, c);

    const uint32_t eax = (c.eax & 0xffffff00u) | GP4_FIELD(uint8_t, record, 0xc);
    const uint32_t decremented = eax - 1u;
    GP4_GLOBAL(uint8_t, 0x0081e4f8) = static_cast<uint8_t>(decremented);

    r->eax = decremented;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(eax, c.eflags));
}
GP4_IMPL(0x00539ce4, FUN_00539ce4, "regs(eax:u32, edx:u32{1,2,3,2,3,0,4}, ebx:ptr[8]:bytes, esi:ptr[0x40]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
