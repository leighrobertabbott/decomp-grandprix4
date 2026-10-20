#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053106d
// Unless [0x007a97a3] != 0 and [0x0082741e] == 0 (then nothing happens; flags of that cmp), walks
// the first [0x007a97a4] + 1 device records (0x3c bytes from 0x00827160) in step with the dword
// array 0x007ad880. For a record whose byte +5 is 4 and whose array entry v is nonzero it finds the
// column k of v among the record's dwords +0x14.. (unbounded search), takes the record's dword +0
// and the column value w = dword +0x28 + 4k (w += w >> 2 when dword +0 is 0), and keeps the best
// (dword +0, v, k, w) in 0x00827469/6d/71/75: the first one, then a higher k, or the same k with a
// lower w. Afterwards byte 0x007ad869 = 0xff and 0x007ad86c = best dword +0 - 1 when that is
// nonzero, else 0x007ad869 = 0; eax = 0x00530774(). Register routine: ebx/esi end one past the
// records/entries, ecx = 0, edi = 1 when a record was taken, edx = the last entry read.
void __cdecl FUN_0053106d(gp4::Regs* r) {
    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x007a97a3);
    uint32_t flags = gp4::flags::sub8(enabled, 0u);
    if (enabled != 0u) {
        const uint32_t pending = GP4_GLOBAL(uint8_t, 0x0082741e);
        flags = gp4::flags::sub8(pending, 0u);
        if (pending == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, flags);
            return;
        }
    }
    uint32_t edx = r->edx;
    uint32_t taken = 0u;
    uint32_t entry = 0x007ad880u;
    uint32_t record = 0x00827160u;
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a97a4) + 1u;
    do {
        uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        if (GP4_FIELD(uint8_t, rec, 5) == 4u) {
            edx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry));
            if (edx != 0u) {
                uint32_t column = 0u;
                while (edx != GP4_FIELD(uint32_t, rec, 0x14 + column * 4u)) {
                    ++column;
                }
                const uint32_t owner = GP4_FIELD(uint32_t, rec, 0);
                uint32_t value = GP4_FIELD(uint32_t, rec, 0x28 + column * 4u);
                if (owner == 0u) {
                    value += value >> 2;
                }
                bool store;
                if (taken == 0u) {
                    store = true;
                    taken = 1u;
                } else {
                    const uint32_t bestColumn = GP4_GLOBAL(uint32_t, 0x00827471);
                    if (column > bestColumn) {
                        store = true;
                    } else {
                        store = column == bestColumn && value < GP4_GLOBAL(uint32_t, 0x00827475);
                    }
                }
                if (store) {
                    GP4_GLOBAL(uint32_t, 0x00827469) = owner;
                    GP4_GLOBAL(uint32_t, 0x0082746d) = edx;
                    GP4_GLOBAL(uint32_t, 0x00827471) = column;
                    GP4_GLOBAL(uint32_t, 0x00827475) = value;
                }
            }
        }
        record += 0x3cu;
        entry += 4u;
        --count;
    } while (count != 0u);

    const uint32_t best = GP4_GLOBAL(uint32_t, 0x00827469);
    flags = gp4::flags::logic32(best);
    if (best == 0u) {
        GP4_GLOBAL(uint8_t, 0x007ad869) = 0u;
    } else {
        GP4_GLOBAL(uint8_t, 0x007ad869) = 0xffu;
        flags = gp4::flags::dec32(best, flags);
        GP4_GLOBAL(uint32_t, 0x007ad86c) = best - 1u;
    }
    r->eax = gp4::cdecl_<0x00530774, uint32_t>();
    r->ebx = record;
    r->ecx = 0u;
    r->edx = edx;
    r->esi = entry;
    r->edi = taken;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053106d, FUN_0053106d, "regs(edx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4+sel_00531859+dev_0053106d")
