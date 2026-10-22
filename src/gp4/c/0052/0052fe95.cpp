#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) void CallDeviceBit(gp4::Regs& c) {
    gp4::call_regs(0x00531847, c);
}
static __declspec(noinline) void CallItemFlags(gp4::Regs& c) {
    gp4::call_regs(0x0053e4db, c);
}

// `mov esi, [esi]` repeated `hops` times.
static inline uint32_t Follow(uint32_t node, int hops) {
    do {
        node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
    } while (--hops != 0);
    return node;
}

// pushal wrapper. GlobalMemoryStatus fills the MEMORYSTATUS at 0x8270e3; its total physical memory
// (+8, signed compares) picks the word [0x7a8e34]: <= 32 MB 0x3c, <= 48 MB 0x1e0, <= 64 MB 0x384,
// <= 128 MB 0xa8c, else 0x1518. That value goes to dwords +0x26/+0x2e of item 14 of the chain at
// 0x843404 and +0x26 of item 15. [0x7ad869] = 0xff, [0x7ad86c] = [0x827056] - 1. The device record
// 0x827160 + [0x827056] * 0x3c gives the item flags ((byte +4 ^ 0x1f) << 6, | 0x38 when the device
// bit of 0x531847 is clear, | 0x7000 when [0x7aafd4] is set) applied along the chain by 0x53e4db.
// After 0x530774, items 7..11 get +0xe = &[0x7ad870 + 4 * slot], +0x26 = the record's key +0x14 + 4i,
// bit 7 of +0x1f = (key == [0x7ad870 + 4 * slot]) and +0x12 = the table 0x52ffea entry 2i + (key ==
// [0x7ad880 + 4 * slot]). popal restores every register; the flags are those of `cmp ecx, 5`.
// FUNCTION: GP4 0x0052fe95
void __cdecl FUN_0052fe95(gp4::Regs* r) {
    uint8_t* const status = GP4_ARRAY(uint8_t, 0x008270e3);
    gp4::stdcall_<0x005a4900, void>(status);
    const int32_t total = GP4_FIELD(int32_t, status, 8);
    uint16_t limit;
    if (total <= 0x2000000) {
        limit = 0x3cu;
    } else if (total <= 0x3000000) {
        limit = 0x1e0u;
    } else if (total <= 0x4000000) {
        limit = 0x384u;
    } else if (total <= 0x8000000) {
        limit = 0xa8cu;
    } else {
        limit = 0x1518u;
    }
    GP4_GLOBAL(uint16_t, 0x007a8e34) = limit;

    uint32_t node = Follow(0x00843404u, 14);
    const uint32_t value = GP4_GLOBAL(uint16_t, 0x007a8e34);
    GP4_FIELD(uint32_t, node, 0x26) = value;
    GP4_FIELD(uint32_t, node, 0x2e) = value;
    node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
    GP4_FIELD(uint32_t, node, 0x26) = value;

    GP4_GLOBAL(uint8_t, 0x007ad869) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x007ad86c) = GP4_GLOBAL(uint32_t, 0x00827056) - 1u;
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00827056) * 0x3cu + 0x00827160u;

    gp4::Regs c = *r;
    c.ebx = record;
    c.ecx = 0u;
    c.esi = node;
    c.edi = 0x008270e3u;
    c.eax = static_cast<uint32_t>(*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record + 4u)) ^ 0x1fu) << 6;
    CallDeviceBit(c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        c.eax |= 0x38u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
        c.eax |= 0x7000u;
    }
    c.esi = 0x00843404u;
    CallItemFlags(c);

    node = Follow(0x00843404u, 4);
    gp4::cdecl_<0x00530774, uint32_t>();
    node = Follow(node, 3);

    const uint32_t* const current =
        reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>((GP4_GLOBAL(uint32_t, 0x00827056) << 2) + 0x007ad870u));
    const uint32_t* const chosen =
        reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>((GP4_GLOBAL(uint32_t, 0x00827056) << 2) + 0x007ad880u));
    const uint32_t* const labels = GP4_ARRAY(const uint32_t, 0x0052ffea);
    for (uint32_t i = 0u; i != 5u; ++i) {
        GP4_FIELD(uint32_t, node, 0xe) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(current));
        const uint32_t key = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record + i * 4u + 0x14u));
        GP4_FIELD(uint32_t, node, 0x26) = key;
        if (key == *current) {
            GP4_FIELD(uint8_t, node, 0x1f) |= 0x80u;
        } else {
            GP4_FIELD(uint8_t, node, 0x1f) &= 0x7fu;
        }
        const uint32_t match = (key == *chosen) ? 1u : 0u;
        GP4_FIELD(uint32_t, node, 0x12) = labels[i * 2u + match];
        node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(5u, 5u));
}
GP4_IMPL(0x0052fe95, FUN_0052fe95, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=dev_0053106d+cb_00531648+bits_007ad844+menu_0052fe95")
