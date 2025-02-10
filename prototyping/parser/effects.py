from dataclasses import dataclass
from typing import List
import struct

from colors import *

@dataclass
class AlternatingConfig:
    interval: int
    is_static: bool
    colors: List[GradientLevelPair]

    def serialize(self) -> bytes:
        serialized_colors = b''.join(pair.serialize() for pair in self.colors)
        return struct.pack('>IB', self.interval, self.is_static) + struct.pack('>B', len(self.colors)) + serialized_colors
    

def test_serialization():
    color = RGBColor(255, 0, 128)
    gradient = ColorGradient([
        ColorPoint(0.0, 0.0, color),
        ColorPoint(1.0, 1.0, color)
    ])
    pair = GradientLevelPair(gradient, [CurvePoint(0.0, 0.0), CurvePoint(1.0, 1.0)])
    config = AlternatingConfig(100, True, [pair])
    
    serialized_data = config.serialize()
    print("Serialized Data:", serialized_data.hex())
    print("Length of Serialized Data:", len(serialized_data))

    output = "C++ version: const uint8_t data[] = {"
    for i in range(len(serialized_data)):
        output += f"0x{serialized_data[i]:02X}, "
    output += "};"
    print(output)
    # Length should be: 4 + 1 + (1 + (1 + 2 * (11))) = 29 

# Run test
test_serialization()
