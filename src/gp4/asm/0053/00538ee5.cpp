#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original repeats the test for commands 5 and 6 after it already branched on them; the repeated
// test is hidden behind a noinline helper so the compiler keeps that (unreachable) block and its
// global.
static __declspec(noinline) uint32_t Command(uint32_t command) {
    return command;
}

// FUNCTION: GP4 0x00538ee5
// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell):
//   1     clear bit 7 of the item flags (+0x1f), compare the cell with the dword +0x26 (0x005398c4)
//         and set bit 7 again when equal;
//   5, 6  store the dword +0x26 into the cell (0x00539a13);
// edx comes back 0 for every command. The second test of 5/6 (clearing [0x0082fd88]) can never be
// reached. The flags are those of the last writer on the path.
void __cdecl FUN_00538ee5(gp4::Regs* r) {
    const uint32_t command = r->edx;
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t arith;

    if (command == 1u) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        gp4::Regs c = *r;
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x005398c4, c);
        r->eax = c.eax;
        arith = c.eflags;
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
    } else if (command == 5u || command == 6u) {
        gp4::Regs c = *r;
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x00539a13, c);
        r->eax = c.eax;
        arith = c.eflags;
    } else {
        const uint32_t again = Command(command);
        arith = gp4::flags::sub32(again, 5u);
        if (again != 5u) {
            arith = gp4::flags::sub32(again, 6u);
        }
        if (again == 5u || again == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00538ee5, FUN_00538ee5, "regs(eax:u32, edx:u32{1,5,6,0,2,1,5}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
