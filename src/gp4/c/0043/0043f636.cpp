#include <gp4/gp4.h>

static inline const uint8_t* segmentAt(uint32_t record) {
    return reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record));
}

// Runs the callee on a copy of the registers with edi = record (the original is a pushal/popal
// wrapper, so nothing the callee leaves in the registers or flags survives).
static void callOnRecord(uint32_t address, const gp4::Regs* r, uint32_t record) {
    gp4::Regs call = *r;
    call.edi = record;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(address, call);
}

// Walks the 0x160-byte segment records from 0x008943a0 up to [0x00866ecc]. Every record whose byte
// +0x73 has bit 0 is run through 0x0043f285; if that leaves bit 2 set, the next record with bit 0
// (searching forward, stopping at the end) is the one handed to 0x0043f40d, otherwise the same
// record is. All registers are restored; the loop only ends on `cmp edi, [0x00866ecc]` equal, so the
// flags left are those of a zero result (ZF = PF = 1).
// FUNCTION: GP4 0x0043f636
void __cdecl FUN_0043f636(gp4::Regs* r) {
    uint32_t record = 0x008943a0u;
    for (;;) {
        if ((segmentAt(record)[0x73] & 1u) != 0u) {
            callOnRecord(0x0043f285, r, record);
            if ((segmentAt(record)[0x73] & 4u) != 0u) {
                bool found = false;
                for (;;) {
                    record += 0x160u;
                    if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                        break;
                    }
                    if ((segmentAt(record)[0x73] & 1u) != 0u) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    break;
                }
            }
            callOnRecord(0x0043f40d, r, record);
        }
        record += 0x160u;
        if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            break;
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x044u;
}
GP4_IMPL(0x0043f636, FUN_0043f636, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043f285+ring2_0043f40d")
