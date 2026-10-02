"""Declared fixed-address input fields; never alter the original executable."""
import json
from pathlib import Path

from .spec import INT_TYPES, _type

LAYOUT_DIR = Path(__file__).resolve().parent.parent / "specs" / "layouts"


def _number(value):
    return int(value, 0) if isinstance(value, str) else int(value)


def global_buffers(generator, name, binary):
    layout = json.loads((LAYOUT_DIR / f"{name}.json").read_text())
    fields = layout.get("globals")
    if not isinstance(fields, list) or not fields:
        raise ValueError(f"global layout {name} needs a nonempty globals array")
    out = []
    emitted = {}
    for field in fields:
        kind = field["type"]
        if kind == "ptr":
            text = f"ptr[{_number(field['size'])}]:{field.get('fill', 'bytes')}"
        else:
            text = kind
            if "lo" in field or "hi" in field:
                text += f"[{field['lo']}..{field['hi']}]"
        arg_type = _type(text)
        width = 4 if kind in ("ptr", "f32") else 8 if kind == "f64" else INT_TYPES[kind]
        count = _number(field.get("count", 1))
        stride = _number(field.get("stride", width))
        if count < 1 or count > 65536 or stride < width:
            raise ValueError("global layout count/stride is invalid")
        pattern = field.get("pattern")
        into = field.get("points_into")
        alias = field.get("alias")
        choices = field.get("values")
        bias = _number(field.get("bias", 0))
        nullable = bool(field.get("nullable", False))
        if (bias or nullable) and kind != "ptr":
            raise ValueError("bias and nullable apply to ptr fields only")
        if bias < 0 or (kind == "ptr" and bias >= _number(field["size"])):
            raise ValueError("ptr bias must lie inside the buffer")
        if choices is not None:
            if kind in ("ptr", "f32", "f64") or into is not None or alias is not None or pattern is not None                     or "lo" in field or "hi" in field or not isinstance(choices, list) or not choices:
                raise ValueError("values needs a nonempty list on a plain integer field")
            choices = [_number(v) for v in choices]
            if any(v < 0 or v >= 1 << (8 * width) for v in choices):
                raise ValueError("values entry does not fit the field")
        if into is not None:
            if kind != "u32" or count != 1 or pattern is not None or alias is not None or "lo" in field or "hi" in field:
                raise ValueError("points_into needs a single plain u32 field")
            base, rec, lo, hi = (_number(into[k]) for k in ("base", "stride", "lo", "hi"))
            if rec < 1 or lo < 0 or hi < lo or hi - lo > 0xFFFF or base + hi * rec > 0xFFFFFFFF:
                raise ValueError("points_into range is invalid")
        if alias is not None:
            if count != 1 or pattern is not None or _number(alias) not in emitted:
                raise ValueError("alias needs a single field and an earlier layout field")
            if len(emitted[_number(alias)]) != width:
                raise ValueError("alias width differs from the field it copies")
        if pattern is not None:
            bit = _number(field.get("bit", 0))
            if pattern != "bit_scan" or kind != "u8" or count < 2 or bit < 1 or bit > 128 or bit & (bit - 1):
                raise ValueError("bit_scan needs repeated u8 fields and one bit in 1..128")
            if "lo" in field or "hi" in field:
                raise ValueError("bit_scan fields cannot also restrict their scalar range")
            # Mix unconstrained bytes with correlated cases that reach the full
            # scan and every possible first-hit position. Lower bits still vary.
            scan_mode = generator.rng.randrange(3)
            hit = generator.rng.randrange(count) if scan_mode == 2 else -1
        for index in range(count):
            address = _number(field["addr"]) + index * stride
            section = binary.section_of(address)
            if not section or not section.in_scope or not section.writable or address + width > section.end:
                raise ValueError(f"global field {address:#x} is outside writable in-scope data")
            if alias is not None and generator.rng.randrange(2):
                data = emitted[_number(alias)]    # equal to the other global in about half the cases
            elif choices is not None:
                data = generator.rng.choice(choices).to_bytes(width, "little")
            elif into is not None:
                data = (base + generator.rng.randint(lo, hi) * rec).to_bytes(4, "little")
            elif kind == "ptr":
                if nullable and generator.rng.randrange(4) == 0:
                    value = 0                       # null pointer in about a quarter of the cases
                else:
                    value = generator.alloc(generator.fill(arg_type.size, arg_type.fill)) + bias
                data = value.to_bytes(4, "little")
            elif kind in ("f32", "f64"):
                import struct
                data = struct.pack("<f" if kind == "f32" else "<d", generator.scalar(arg_type))
            else:
                value = generator.scalar(arg_type) & ((1 << (8 * width)) - 1)
                if pattern is not None and scan_mode != 0:
                    value &= ~bit
                    if index == hit:
                        value |= bit
                data = value.to_bytes(width, "little")
            emitted[address] = data
            out.append((address, data))
    return out
