#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads one interpolation record {count, target} from the stream at esi. When the mode byte
// 0x0086857c is not 1 the record is skipped (esi += 4, flags of that add). Otherwise esi advances by
// 4 as well: a zero count latches the target in 0x00870610 and clears the count (0x00870614) and the
// step (0x00870612); a nonzero count stores count/target (0x00870614/0x00870616) and the step
// (target - 0x00870610) / count (signed 16-bit difference, 32-bit idiv) in 0x00870612. ax/dx are the
// partial registers written; the non-zero path also leaves the quotient in eax, the remainder in edx
// and the zero-extended count in ebp. The flags are those of the last flag writer (the add, or the
// 16-bit sub on the non-zero path since idiv leaves them alone).
// FUNCTION: GP4 0x005574d8
void __cdecl FUN_005574d8(gp4::Regs* r) {
    const uint32_t start = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = start + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(start, 4u));
        return;
    }

    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(start));
    const uint16_t count = words[0];
    const uint32_t afterCount = start + 2u;
    uint32_t flags = gp4::flags::logic16(count);

    if (count == 0u) {
        const uint16_t target = words[1];
        GP4_GLOBAL(uint16_t, 0x00870610) = target;
        GP4_GLOBAL(uint16_t, 0x00870614) = 0;
        GP4_GLOBAL(uint16_t, 0x00870612) = 0;
        r->eax = (r->eax & 0xffff0000u) | target;
        r->edx = r->edx & 0xffff0000u;
        r->esi = afterCount + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(afterCount, 2u));
        return;
    }

    GP4_GLOBAL(uint16_t, 0x00870614) = count;
    const uint16_t target = words[1];
    GP4_GLOBAL(uint16_t, 0x00870616) = target;
    const uint16_t base = GP4_GLOBAL(uint16_t, 0x00870610);
    const uint16_t delta = static_cast<uint16_t>(target - base);
    flags = gp4::flags::sub16(target, base);
    const int32_t dividend = static_cast<int16_t>(delta);
    const int32_t divisor = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00870614)));
    const int32_t quotient = dividend / divisor;
    const int32_t remainder = dividend % divisor;
    GP4_GLOBAL(uint16_t, 0x00870612) = static_cast<uint16_t>(quotient);
    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(divisor);
    r->esi = afterCount + 2u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005574d8, FUN_005574d8, "regs(eax:u32, edx:u32, ebp:u32, esi:ptr[0x10]:u16{0,0,1,2,7,0x100,0x8000,0xffff}) -> (eax, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
