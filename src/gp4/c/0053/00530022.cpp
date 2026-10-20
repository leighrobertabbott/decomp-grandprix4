#include <gp4/gp4.h>
#include <gp4/flags.h>

// `pushal ... popal`: binds the items of the static menu 0x00843849 (header +0 = first item, item
// +0 = next item) to the render-state record ebx that 0x00531803 selects: items 5 and 6 get the
// selector bit 1 << [0x007ad87c] in their dword +0x26, the following items get value pointers
// (dword +0xe) into the record (+0x24, +0x25 or the fallback byte 0x008270d7, +0x20 or 0x008270d8,
// +0x00/+0x10, +0x04/+0x14, +0x26, +0x1c, +0x1f, +0x1e, +0x18, +0x1d). A record whose dword +0x10
// has any of the bits 0x1867f83c gets 0x81800700 or-ed into it. The registers are restored; the
// flags are those of the final test / or.
static inline uint8_t* NextItem(uint8_t* item) {
    return *reinterpret_cast<uint8_t**>(item);
}

static inline uint8_t* SetValue(uint8_t* item, uint32_t value) {
    GP4_FIELD(uint32_t, item, 0x0e) = value;
    return NextItem(item);
}

// FUNCTION: GP4 0x00530022
void __cdecl FUN_00530022(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00531803, c);
    const uint32_t record = c.ebx;
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));

    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00843849u));
    for (int i = 0; i < 5; ++i) {
        item = NextItem(item);
    }
    const uint32_t bit = 1u << (GP4_GLOBAL(uint8_t, 0x007ad87c) & 0x1fu);
    GP4_FIELD(uint32_t, item, 0x26) = bit;
    item = NextItem(item);
    GP4_FIELD(uint32_t, item, 0x26) = bit;
    item = NextItem(item);

    item = SetValue(item, record + 0x24u);
    item = SetValue(item, record + 0x24u);

    uint32_t p = (rec[0x24] == 0u) ? 0x008270d7u : record + 0x25u;
    item = SetValue(item, p);
    const uint32_t q = (*reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(p)) == 0u)
                           ? 0x008270d8u
                           : record + 0x20u;
    for (int i = 0; i < 3; ++i) {
        item = SetValue(item, q);
    }
    for (int i = 0; i < 4; ++i) {
        item = SetValue(item, record);
        item = SetValue(item, record + 0x10u);
    }
    for (int i = 0; i < 2; ++i) {
        item = SetValue(item, record + 0x04u);
        item = SetValue(item, record + 0x14u);
    }
    item = NextItem(NextItem(item));
    item = SetValue(item, record + 0x26u);
    for (int i = 0; i < 4; ++i) {
        item = SetValue(item, record + 0x1cu);
    }
    item = SetValue(item, record + 0x1fu);
    item = SetValue(item, record + 0x1eu);
    item = NextItem(item);
    for (int i = 0; i < 3; ++i) {
        item = SetValue(item, record + 0x18u);
    }
    item = SetValue(item, record + 0x1du);
    SetValue(item, record + 0x1du);

    const uint32_t mode = GP4_FIELD(uint32_t, rec, 0x10);
    uint32_t flags = gp4::flags::logic32(mode & 0x1867f83cu);
    if ((mode & 0x1867f83cu) != 0u) {
        const uint32_t updated = mode | 0x81800700u;
        GP4_FIELD(uint32_t, rec, 0x10) = updated;
        flags = gp4::flags::logic32(updated);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00530022, FUN_00530022, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=sel_00531859+rsrec_007ad638+flag_008270d7")
