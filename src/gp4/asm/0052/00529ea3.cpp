#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu value "step up" for the mode in edx: 1 clears edx and does nothing else, 2 and 3 load the value at [ebx]
// (width from the esi record, via 0x00539944), add the dword at +0x2a of the esi record, keep the result only when
// it did not overflow and is not above the dword at +0x26 (signed; otherwise the +0x26 dword is used) and store it
// back (0x00539a13). When the selection byte [0x0081d1eb] is 0xff the value is also stored through the pointer at
// +0x2e of the record (ebx = that pointer). Other modes leave the flags of `cmp edx, 3`; the flags otherwise are
// those of the last compare or of the last store routine.
// FUNCTION: GP4 0x00529ea3
void __cdecl FUN_00529ea3(gp4::Regs* r) {
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

    const uint8_t selection = GP4_GLOBAL(uint8_t, 0x0081d1eb);
    uint32_t flags = gp4::flags::sub8(selection, 0xffu);
    if (selection == 0xffu) {
        c.ebx = GP4_FIELD(uint32_t, record, 0x2e);
        gp4::call_regs(0x00539a13, c);
        flags = c.eflags;
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00529ea3, FUN_00529ea3, "regs(eax:u32, edx:u32{1,2,3,2,3,0,4}, ebx:ptr[8]:bytes, esi:ptr[0x40]:layout=rec_00529dab) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rec_00529dab")
