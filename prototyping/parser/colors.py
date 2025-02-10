from dataclasses import dataclass
from typing import List
import struct 

@dataclass
class RGBColor:
    r: int
    g: int
    b: int

    def serialize(self) -> bytes:
        return struct.pack('>BBB', self.r, self.g, self.b)

@dataclass
class ColorPoint:
    focus: float
    position: float
    color: RGBColor

    def serialize(self) -> bytes:
        return struct.pack('<ff', self.focus, self.position) + self.color.serialize()

@dataclass
class ColorGradient:
    color_points: List[ColorPoint]

    def serialize(self) -> bytes:
        serialized_points = b''.join(point.serialize() for point in self.color_points)
        return struct.pack('>B', len(self.color_points)) + serialized_points

@dataclass
class CurvePoint:
    x: float
    y: float

    def serialize(self) -> bytes:
        return struct.pack('<ff', self.x, self.y)

@dataclass
class GradientLevelPair:
    colorGradient: ColorGradient
    brightness: List[CurvePoint]

    def serialize(self) -> bytes:
        serialized_brightness = b''.join(point.serialize() for point in self.brightness)
        return self.colorGradient.serialize() + struct.pack('>B', len(self.brightness)) + serialized_brightness