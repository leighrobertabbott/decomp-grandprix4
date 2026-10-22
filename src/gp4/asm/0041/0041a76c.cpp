#include <gp4/gp4.h>
#include <gp4/flags.h>
#include <gp4/x87.h>

namespace f = gp4::flags;

static inline uint8_t ReadU8(uint32_t base, uint32_t off) { return GP4_FIELD(uint8_t, base, off); }
static inline uint16_t ReadU16(uint32_t base, uint32_t off) { return GP4_FIELD(uint16_t, base, off); }
static inline uint32_t ReadU32(uint32_t base, uint32_t off) { return GP4_FIELD(uint32_t, base, off); }

// `cmp eax, 0xff; jle; mov eax, 0xff; cmp eax, 0; jge; mov eax, 0`.
static inline uint32_t ClampByte(int32_t value) {
    if (value > 0xff) {
        value = 0xff;
    }
    if (value < 0) {
        value = 0;
    }
    return static_cast<uint32_t>(value);
}

// `imul dword [0x601d50]; shrd eax, edx, shift`, then ClampByte.
static inline uint32_t ScaledByte(uint32_t difference, int shift) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(difference))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601d50));
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
    return ClampByte(static_cast<int32_t>(scaled));
}

// `cmp eax, 0; jge; mov eax, 0; sar eax, 0xf; cmp eax, 0xff; jle; mov eax, 0xff`.
static inline uint32_t LoadByte(int32_t value) {
    if (value < 0) {
        value = 0;
    }
    value >>= 15;
    if (value > 0xff) {
        value = 0xff;
    }
    return static_cast<uint32_t>(value);
}

// `test eax, -1; jns pos; neg eax; cmp eax, limit; jb; mov eax, limit; neg eax; jmp; pos: cmp eax, limit; jb;
// mov eax, limit`: a signed value clamped to +-limit by its magnitude (unsigned compares).
static inline uint32_t ClampMagnitude(uint32_t value, uint32_t limit) {
    if ((value & 0x80000000u) != 0u) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= limit) {
            magnitude = limit;
        }
        return 0u - magnitude;
    }
    if (value >= limit) {
        value = limit;
    }
    return value;
}

// `fild dword [src]; fmul qword [0x6f2b9c]; fmul qword [0x6f2a4c]; fistp dword [0x6f2bf4]`.
static inline uint32_t ScaledAngle(int32_t source) {
    using namespace gp4::x87;
    const Ext product = ext_i32(source) * ext(GP4_GLOBAL(double, 0x006f2b9c)) * ext(GP4_GLOBAL(double, 0x006f2a4c));
    const int32_t rounded = fistp32(product);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;
    return ClampMagnitude(static_cast<uint32_t>(rounded), 0x7fu);
}

// `movsx eax, word [edx+off]; add eax, 0x8000; sar eax, 8`, then ClampByte.
static inline uint32_t PositionByte(uint32_t segment, uint32_t off) {
    const int32_t value = static_cast<int32_t>(static_cast<int16_t>(ReadU16(segment, off))) + 0x8000;
    return ClampByte(value >> 8);
}

// FUNCTION: GP4 0x0041a76c
// Telemetry recorder for the player's car: with esi == [0x66d820], byte [0x623ada] == 0xff and a datalog
// buffer [0x6975cc], it either closes the log ([0x6f2f64] == 0xff: header +0x10 = 0x10000000, 0x0051f2a1, flags
// reset) or, with the segment edi below the pit start [0x891500] and a nonzero lap byte +0x70, appends one
// 0x1b-byte sample at the cursor [0x623ae0] (bounded by the end [0x6975d0]). A new lap (+0x70 differs from
// [0x623ad9]) first stores the lap, the car's table byte 0x66ec4a[(+0x7c & 0x3f) - 1, at most 0x15] in
// [0x623ad8], closes the previous lap (header +0x10 = lap time +0x28, count decremented when above 1,
// 0x0051f2a1) unless [0x623adb] / [0x6f2f68] mark the first, and starts a 0x20-byte lap header at the buffer
// (0x0041a6f5). The sample holds the track distance, speed, steering, the inputs, scaled wheel / load bytes
// (masked by the +0x40 wheel bits), two angle bytes and the segment's position bytes; the header count is then
// incremented. Registers and flags are those the original leaves on the path taken.
void __cdecl FUN_0041a76c(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    uint32_t flags = r->eflags;

    const uint32_t car = s.esi;
    flags = f::sub32(car, GP4_GLOBAL(uint32_t, 0x0066d820));
    if (car != GP4_GLOBAL(uint32_t, 0x0066d820)) {
        goto done;
    }
    flags = f::sub8(GP4_GLOBAL(uint8_t, 0x00623ada), 0xffu);
    if (GP4_GLOBAL(uint8_t, 0x00623ada) != 0xffu) {
        goto done;
    }
    flags = f::logic32(GP4_GLOBAL(uint32_t, 0x006975cc));
    if (GP4_GLOBAL(uint32_t, 0x006975cc) == 0u) {
        goto done;
    }
    flags = f::sub8(GP4_GLOBAL(uint8_t, 0x006f2f64), 0xffu);
    if (GP4_GLOBAL(uint8_t, 0x006f2f64) == 0xffu) {
        s.ebx = GP4_GLOBAL(uint32_t, 0x006975cc);
        s.eax = 0x10000000u;
        GP4_FIELD(uint32_t, s.ebx, 0x10) = s.eax;
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x0051f2a1, s);
        flags = s.eflags;
        GP4_GLOBAL(uint8_t, 0x00623adb) = 0u;
        GP4_GLOBAL(uint8_t, 0x00623ad9) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x006f2f64) = 0u;
        GP4_GLOBAL(uint8_t, 0x00623ada) = 0u;
        goto done;
    }
    flags = f::sub32(s.edi, GP4_GLOBAL(uint32_t, 0x00891500));
    if (s.edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        goto done;
    }
    {
        const uint8_t lap = ReadU8(car, 0x70);
        s.eax = (s.eax & 0xffffff00u) | lap;
        flags = f::sub8(lap, 0u);
        if (lap == 0u) {
            goto done;
        }
        flags = f::sub8(lap, GP4_GLOBAL(uint8_t, 0x00623ad9));
        if (lap == GP4_GLOBAL(uint8_t, 0x00623ad9)) {
            goto sample;
        }
        GP4_GLOBAL(uint8_t, 0x00623ad9) = lap;
    }
    {
        uint32_t index = (static_cast<uint32_t>(ReadU8(car, 0x7c)) & 0x3fu) - 1u;
        if (static_cast<int32_t>(index) > 0x15) {
            index = 0x15u;
        }
        const uint8_t team = GP4_ARRAY(uint8_t, 0x0066ec4a)[static_cast<int32_t>(index)];
        s.eax = (index & 0xffffff00u) | team;
        GP4_GLOBAL(uint8_t, 0x00623ad8) = team;
    }
    flags = f::sub8(GP4_GLOBAL(uint8_t, 0x00623adb), 0xffu);
    if (GP4_GLOBAL(uint8_t, 0x00623adb) != 0xffu) {
        GP4_GLOBAL(uint8_t, 0x00623adb) = 0xffu;
    } else {
        flags = f::sub8(GP4_GLOBAL(uint8_t, 0x006f2f68), 0xffu);
        if (GP4_GLOBAL(uint8_t, 0x006f2f68) == 0xffu) {
            GP4_GLOBAL(uint8_t, 0x006f2f68) = 0u;
        } else {
            s.ebx = GP4_GLOBAL(uint32_t, 0x006975cc);
            s.eax = ReadU32(car, 0x28);
            GP4_FIELD(uint32_t, s.ebx, 0x10) = s.eax;
            s.eax = ReadU32(s.ebx, 0) - 1u;
            flags = f::sub32(s.eax, 1u);
            if (static_cast<int32_t>(s.eax) >= 1) {
                GP4_FIELD(uint32_t, s.ebx, 0) = s.eax;
            }
            s.eflags = f::merge(s.eflags, flags);
            gp4::call_regs(0x0051f2a1, s);
            flags = s.eflags;
        }
    }
    s.ebx = GP4_GLOBAL(uint32_t, 0x006975cc);
    GP4_GLOBAL(uint32_t, 0x00623ae0) = s.ebx;
    s.ebx += 0x20u;
    flags = f::sub32(s.ebx, GP4_GLOBAL(uint32_t, 0x006975d0));
    if (s.ebx >= GP4_GLOBAL(uint32_t, 0x006975d0)) {
        goto done;
    }
    s.ebx = GP4_GLOBAL(uint32_t, 0x00623ae0);
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0041a6f5, s);
    flags = s.eflags;
    s.ebx += 0x20u;
    GP4_GLOBAL(uint32_t, 0x00623ae0) = s.ebx;

sample:
    s.ebx = GP4_GLOBAL(uint32_t, 0x00623ae0) + 0x1bu;
    flags = f::sub32(s.ebx, GP4_GLOBAL(uint32_t, 0x006975d0));
    if (s.ebx >= GP4_GLOBAL(uint32_t, 0x006975d0)) {
        goto done;
    }
    {
        const uint32_t out = GP4_GLOBAL(uint32_t, 0x00623ae0);
        s.ebx = out;

        const int32_t lateral = static_cast<int32_t>(static_cast<int16_t>(ReadU16(car, 0x66))) >> 10;
        const uint32_t distance = ((static_cast<uint32_t>(ReadU16(s.edi, 0x1a)) & 0x7ffu) << 4) + static_cast<uint32_t>(lateral);
        GP4_FIELD(uint16_t, out, 0) = static_cast<uint16_t>(distance);

        uint16_t speed = ReadU16(car, 0x9a);
        if ((speed & 0x8000u) != 0u) {
            speed = static_cast<uint16_t>(0u - speed);
        }
        GP4_FIELD(uint16_t, out, 2) = speed;

        const uint32_t negated = 0u - static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(ReadU16(car, 0xb4))));
        GP4_FIELD(uint16_t, out, 4) = static_cast<uint16_t>(ClampMagnitude(negated, 0x71cu));
        GP4_FIELD(uint16_t, out, 6) = ReadU16(car, 0x62);
        GP4_FIELD(uint8_t, out, 8) = static_cast<uint8_t>(ReadU16(car, 0xee) >> 8);
        GP4_FIELD(uint8_t, out, 9) = static_cast<uint8_t>(static_cast<uint16_t>(ReadU16(car, 0x24a)) >> 8);
        GP4_FIELD(uint8_t, out, 0xa) = ReadU8(car, 0x72);

        const uint32_t wheels = ReadU32(car, 0x40);
        uint32_t value;

        value = ScaledByte(ReadU32(car, 0x2fc) - ReadU32(car, 0x37c), 0x1c);
        GP4_FIELD(uint8_t, out, 0xb) = static_cast<uint8_t>(value);
        value = ScaledByte(GP4_GLOBAL(uint32_t, 0x00601e40) - ReadU32(car, 0x32c) - ReadU32(car, 0x35c), 0x1b);
        if ((wheels & 0x10000u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0xf) = static_cast<uint8_t>(value);
        value = LoadByte(static_cast<int32_t>(ReadU32(car, 0x39c)));
        if ((wheels & 0x10000u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x13) = static_cast<uint8_t>(value);

        value = ScaledByte(ReadU32(car, 0x300) - ReadU32(car, 0x380), 0x1c);
        GP4_FIELD(uint8_t, out, 0xc) = static_cast<uint8_t>(value);
        value = ScaledByte(GP4_GLOBAL(uint32_t, 0x00601e44) - ReadU32(car, 0x330) - ReadU32(car, 0x360), 0x1b);
        if ((wheels & 0x4000u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x10) = static_cast<uint8_t>(value);
        value = LoadByte(static_cast<int32_t>(ReadU32(car, 0x3a0)));
        if ((wheels & 0x4000u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x14) = static_cast<uint8_t>(value);

        value = ScaledByte(ReadU32(car, 0x304) - ReadU32(car, 0x384), 0x1c);
        GP4_FIELD(uint8_t, out, 0xd) = static_cast<uint8_t>(value);
        value = ScaledByte(GP4_GLOBAL(uint32_t, 0x00601e48) - ReadU32(car, 0x334) - ReadU32(car, 0x364), 0x1b);
        if ((wheels & 0x200u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x11) = static_cast<uint8_t>(value);
        value = LoadByte(static_cast<int32_t>(ReadU32(car, 0x3a4)));
        if ((wheels & 0x200u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x15) = static_cast<uint8_t>(value);

        value = ScaledByte(ReadU32(car, 0x308) - ReadU32(car, 0x388), 0x1c);
        GP4_FIELD(uint8_t, out, 0xe) = static_cast<uint8_t>(value);
        value = ScaledByte(GP4_GLOBAL(uint32_t, 0x00601e4c) - ReadU32(car, 0x338) - ReadU32(car, 0x368), 0x1b);
        if ((wheels & 0x80u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x12) = static_cast<uint8_t>(value);
        value = LoadByte(static_cast<int32_t>(ReadU32(car, 0x3a8)));
        if ((wheels & 0x80u) != 0u) {
            value = 0u;
        }
        GP4_FIELD(uint8_t, out, 0x16) = static_cast<uint8_t>(value);

        value = ScaledAngle(GP4_GLOBAL(int32_t, 0x00623ea0));
        GP4_FIELD(uint8_t, out, 0x17) = static_cast<uint8_t>(value);
        value = ScaledAngle(GP4_GLOBAL(int32_t, 0x00623ea8));
        GP4_FIELD(uint8_t, out, 0x18) = static_cast<uint8_t>(value);

        s.edx = ReadU32(car, 0x1c);
        value = PositionByte(s.edx, 4);
        GP4_FIELD(uint8_t, out, 0x19) = static_cast<uint8_t>(value);
        value = PositionByte(s.edx, 8);
        GP4_FIELD(uint8_t, out, 0x1a) = static_cast<uint8_t>(value);
        s.eax = value;

        s.ebx = out + 0x1bu;
        const uint32_t carry = f::add32(out, 0x1bu);
        GP4_GLOBAL(uint32_t, 0x00623ae0) = s.ebx;
        s.ebx = GP4_GLOBAL(uint32_t, 0x006975cc);
        const uint32_t count = ReadU32(s.ebx, 0);
        GP4_FIELD(uint32_t, s.ebx, 0) = count + 1u;
        flags = f::inc32(count, carry);
    }

done:
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = f::merge(r->eflags, flags);
}
// Spec: state_0043260e makes [0x66d820] equal esi in half the cases; fill_00541fad bounds the 0x0051f2a1 clear
// count. The datalog buffer [0x6975cc] (nullable) and the cursor [0x623ae0] are separate buffers; the end
// [0x6975d0] is 0 (every bound check exits) or above both (samples are written). The segment edi is a ring
// record below or above the pit start [0x891500] (record 9 / 10); the lap byte +0x70 matches [0x623ad9] in some
// draws; the car segment +0x1c is a buffer for its two position words.
GP4_IMPL(0x0041a76c, FUN_0041a76c, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x3b0]:{*bytes, 0x1c:ptr[0x10]:bytes, 0x70:u8{=0,1,2}}, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); cases=800; globals=layout=state_0043260e+fill_00541fad; gfields={0x00623ada:u8{=0xff,0xff,0}, 0x006975cc:nptr[0x1000]:bytes, 0x006f2f64:u8{=0xff,0,0,1}, 0x00891500:u32{=0x8950c0,0x895220}, 0x00623ad9:u8{=0,1,2,0xff}, 0x00623adb:u8{=0xff,0}, 0x006f2f68:u8{=0xff,0}, 0x006975d0:u32{=0,0xfffffff0,0xfffffff0}, 0x00623ae0:ptr[0x40]:bytes, 0x00601d50:i32, 0x00601e40*4/4:i32, 0x00623ea0:i32, 0x00623ea8:i32}")
