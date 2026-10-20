#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the handler of the current menu item [0x00a69878] (handler pointer at +0x1a) in mode
// [0x00a69880]: when the mode is 2 the item's word +0x1f gets 0x80 or-ed in first. The handler is
// called with esi = item, ebx = the item's dword +0xe, edx = mode and al = the flag byte +0x1f
// read before the or; then 0x0053dbbc (a switch on edx over the flag byte +0x1f) runs with whatever
// edx the handler left. pushal/popal (and the push/pop of eax) keep every register; the flags are
// those 0x0053dbbc leaves.

// FUNCTION: GP4 0x0053d2c1
void __cdecl FUN_0053d2c1(gp4::Regs* r) {
    const uint32_t item = GP4_GLOBAL(uint32_t, 0x00a69878);
    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x00a69880);
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(item));

    const uint32_t before = GP4_FIELD(uint8_t, record, 0x1f);
    uint32_t flags = gp4::flags::sub32(mode, 2u);
    if (mode == 2u) {
        const uint32_t word = GP4_FIELD(uint16_t, record, 0x1f) | 0x80u;
        GP4_FIELD(uint16_t, record, 0x1f) = static_cast<uint16_t>(word);
        flags = gp4::flags::logic16(word);
    }

    const uint32_t value = GP4_FIELD(uint32_t, record, 0xe);
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | before;
    c.ebx = value;
    c.edx = mode;
    c.esi = item;
    c.eflags = gp4::flags::merge(r->eflags, flags);
    gp4::call_regs(GP4_FIELD(uint32_t, record, 0x1a), c);

    c.esi = item;
    c.ebx = value;
    gp4::call_regs(0x0053dbbc, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053d2c1, FUN_0053d2c1, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=cur_0053d2c1")
