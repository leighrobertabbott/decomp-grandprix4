#include <gp4/gp4.h>
#include <gp4/flags.h>

// The ring tables are indexed by the constant edx = 2; the compiler would fold the base into an
// absolute address and hide the table globals from the shape audit.
static __declspec(noinline) uint32_t* tableSlot(uint32_t* base, uint32_t index) {
    return &base[index];
}

// Copies the replay items at esi into the slot rows [0x88f614]..[0x88f618] of the slot area [0x7a8e18]
// (pitch [0x6975c4]): every row of that range is first tagged 1, then up to [0x7c2e7c] / 3 items are
// walked (row counter ebx from the ring-2 lower bound [0x679ba4] while below its limit [0x679b2c]).
// Items are skipped (0x005135fa, counted in [0x88f600], [0x88f5fe] = 0xff) until the first kind-3 item
// whose limit distance [0x679b2c] - ebx is below [0x7a8e1c] (or at once when byte [0x7c2e82] is set);
// from then on each item is copied into row [0x88f60c] (kind 1: 4 bytes, 2: 8 bytes, 3: a whole row),
// the row advancing and wrapping to [0x88f614] after [0x88f618] (stopping instead when [0x7c2e82] is
// set). The remaining items are skipped, then ring 2 is moved to the new rows: limit -= old lower bound
// + skipped - new first row, lower bound = [0x88f614], wrap end = [0x88f618], position = limit + 1
// (the lower bound past the wrap end). ecx is saved and restored, edx = 2, ebp = 3; the flags are those
// of the final `cmp position, wrap end`.
// FUNCTION: GP4 0x0051361e
void __cdecl FUN_0051361e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t* const limits = GP4_ARRAY(uint32_t, 0x00679b24);
    uint32_t* const positions = GP4_ARRAY(uint32_t, 0x00679b34);
    uint32_t* const lowers = GP4_ARRAY(uint32_t, 0x00679b9c);
    uint32_t* const ends = GP4_ARRAY(uint32_t, 0x00679bac);

    GP4_GLOBAL(uint8_t, 0x0088f5fe) = 0;
    int32_t row = GP4_GLOBAL(int32_t, 0x0088f614);
    uint32_t tag = static_cast<uint32_t>(row) * GP4_GLOBAL(uint32_t, 0x006975c4) + GP4_GLOBAL(uint32_t, 0x007a8e18);
    do {
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(tag)) = 1u;
        tag += GP4_GLOBAL(uint32_t, 0x006975c4);
        ++row;
    } while (row <= GP4_GLOBAL(int32_t, 0x0088f618));

    GP4_GLOBAL(uint8_t, 0x0088f5fd) = 0;
    GP4_GLOBAL(uint32_t, 0x0088f60c) = GP4_GLOBAL(uint32_t, 0x0088f614);
    GP4_GLOBAL(uint32_t, 0x007a8e14) = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2e7c)) / 3u;
    GP4_GLOBAL(uint32_t, 0x0088f610) = GP4_GLOBAL(uint32_t, 0x0088f60c);
    GP4_GLOBAL(uint32_t, 0x0088f600) = 0;
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a8e14);
    uint32_t ebx = *tableSlot(lowers, 2u);

    for (;;) {
        bool copy = GP4_GLOBAL(uint8_t, 0x0088f5fd) == 0xffu;
        if (!copy) {
            const uint32_t kind = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi));
            if (kind == 3u) {
                if (GP4_GLOBAL(uint8_t, 0x007c2e82) != 0) {
                    copy = true;
                } else {
                    const int32_t distance = static_cast<int32_t>(*tableSlot(limits, 2u) - ebx);
                    copy = distance < GP4_GLOBAL(int32_t, 0x007a8e1c);
                }
            }
        }
        if (!copy) {
            gp4::call_regs(0x005135fa, c);
            GP4_GLOBAL(uint32_t, 0x0088f600) += 1u;
            GP4_GLOBAL(uint8_t, 0x0088f5fe) = 0xff;
        } else {
            GP4_GLOBAL(uint8_t, 0x0088f5fd) = 0xff;
            uint32_t dst = GP4_GLOBAL(uint32_t, 0x0088f60c) * GP4_GLOBAL(uint32_t, 0x006975c4)
                         + GP4_GLOBAL(uint32_t, 0x007a8e18);
            uint32_t src = c.esi;
            const uint32_t kind = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(src));
            if (kind == 1u) {
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(dst)) =
                    *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(src));
                src += 4u;
                dst += 4u;
            } else if (kind == 2u) {
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(dst)) =
                    *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(src));
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(dst + 4u)) =
                    *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(src + 4u));
                src += 8u;
                dst += 8u;
            } else if (kind == 3u) {
                const uint32_t n = GP4_GLOBAL(uint32_t, 0x006975c4);
                for (uint32_t i = 0u; i != n; ++i) {
                    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(dst + i)) =
                        *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src + i));
                }
                src += n;
                dst += n;
            }
            c.esi = src;
            c.edi = dst;

            int32_t next = GP4_GLOBAL(int32_t, 0x0088f60c) + 1;
            if (next > GP4_GLOBAL(int32_t, 0x0088f618)) {
                if (GP4_GLOBAL(uint8_t, 0x007c2e82) != 0) {
                    --count;
                    break;
                }
                next = GP4_GLOBAL(int32_t, 0x0088f614);
            }
            GP4_GLOBAL(int32_t, 0x0088f60c) = next;
        }
        ++ebx;
        --count;
        if (count == 0u) {
            break;
        }
        if (static_cast<int32_t>(ebx) >= *reinterpret_cast<int32_t*>(tableSlot(limits, 2u))) {
            break;
        }
    }

    while (count != 0u) {
        gp4::call_regs(0x005135fa, c);
        --count;
    }

    uint32_t* const limit = tableSlot(limits, 2u);
    *limit = *limit - *tableSlot(lowers, 2u) - GP4_GLOBAL(uint32_t, 0x0088f600) + GP4_GLOBAL(uint32_t, 0x0088f614);
    *tableSlot(lowers, 2u) = GP4_GLOBAL(uint32_t, 0x0088f614);
    *tableSlot(ends, 2u) = GP4_GLOBAL(uint32_t, 0x0088f618);
    uint32_t position = *limit + 1u;
    const uint32_t arith = gp4::flags::sub32(position, *tableSlot(ends, 2u));
    if (static_cast<int32_t>(position) > *reinterpret_cast<int32_t*>(tableSlot(ends, 2u))) {
        position = *tableSlot(lowers, 2u);
    }
    *tableSlot(positions, 2u) = position;

    r->eax = position;
    r->ebx = ebx;
    r->edx = 2u;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = 3u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051361e, FUN_0051361e, "regs(ecx:u32, esi:ptr[0x1a010]:u32{1,2,3,0}, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_00512d68+rows_0051361e")
