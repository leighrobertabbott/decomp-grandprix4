#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041034e
// Copies the 6 bytes at ebx+2 to body+0x1c6 (body = esi), replacing them by the table bytes
// 0x0062d1ba (0x0062d1ca in mode 1 or 3) when [0x00601f20] != 0xff and the mask byte [0x00601fab]
// hits the dword of the table 0x00601f28 selected by [0x00601fa8] & 3 (& 1 when [0x00601fa9] != 0).
// 0x00410c1b then converts the 7-byte block at body+0x1c6 in place. When byte body+0x1cc is
// nonzero the body is flagged (+0xcf |= 0x40, +0x1cd = 7); otherwise, outside a replay
// ([0x00622424] == 0), with body+0x7c >= 0, body+0x73 != 0 and the table byte
// [0x0062ebe6 + min(((body+0x73) - 1) & 0x3f, 10)] == 7, 0x00410c70 ramps the block and the same
// flag is set. 0x0041043d finishes. pushal/popal keep every register; the flags are those left
// by 0x0041043d.
void __cdecl FUN_0041034e(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_FIELD(uint8_t, body, 0xcf) &= 0xbfu;
    GP4_FIELD(uint8_t, body, 0x1cd) = 6;

    for (uint32_t i = 0; i != 6; ++i) {
        uint8_t value = GP4_FIELD(uint8_t, source, 2 + i);
        if (GP4_GLOBAL(uint8_t, 0x00601f20) != 0xffu) {
            uint32_t index = GP4_GLOBAL(uint8_t, 0x00601fa8) & 3u;
            if (GP4_GLOBAL(uint8_t, 0x00601fa9) != 0u) {
                index &= 1u;
            }
            const uint32_t mask = GP4_ARRAY(uint32_t, 0x00601f28)[index];
            if ((GP4_GLOBAL(uint8_t, 0x00601fab) & static_cast<uint8_t>(mask)) != 0u) {
                if (GP4_GLOBAL(uint8_t, 0x00601fa8) == 1u || GP4_GLOBAL(uint8_t, 0x00601fa8) == 3u) {
                    value = GP4_ARRAY(uint8_t, 0x0062d1ca)[i];
                } else {
                    value = GP4_ARRAY(uint8_t, 0x0062d1ba)[i];
                }
            }
        }
        GP4_FIELD(uint8_t, body, 0x1c6 + i) = value;
    }

    gp4::Regs convert = *r;
    convert.eax = r->esi + 0x1c6u;
    convert.edx = r->esi + 0x1c6u;
    gp4::call_regs(0x00410c1b, convert);

    if (GP4_FIELD(uint8_t, body, 0x1cc) != 0u) {
        GP4_FIELD(uint8_t, body, 0xcf) |= 0x40u;
        GP4_FIELD(uint8_t, body, 0x1cd) = 7;
    } else if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u
               && static_cast<int8_t>(GP4_FIELD(uint8_t, body, 0x7c)) >= 0
               && GP4_FIELD(uint8_t, body, 0x73) != 0u) {
        uint32_t row = static_cast<uint32_t>(static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x73) - 1u)) & 0x3fu;
        if (static_cast<int32_t>(row) > 10) {
            row = 10;
        }
        if (GP4_ARRAY(uint8_t, 0x0062ebe6)[row] == 7u) {
            gp4::Regs ramp = convert;
            ramp.eax = r->esi + 0x1c6u;
            ramp.edx = r->esi + 0x1c6u;
            gp4::call_regs(0x00410c70, ramp);
            GP4_FIELD(uint8_t, body, 0xcf) |= 0x40u;
            GP4_FIELD(uint8_t, body, 0x1cd) = 7;
        }
    }

    gp4::Regs finish = *r;
    gp4::call_regs(0x0041043d, finish);

    r->eflags = (r->eflags & ~0x8d5u) | (finish.eflags & 0x8d5u);
}
GP4_IMPL(0x0041034e, FUN_0041034e, "regs(ebx:ptr[8]:bytes, esi:ptr[0x250]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
