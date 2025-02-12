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

@dataclass
class ChaseConfig:
    interval: int
    color_handling: int # single byte (0, 1, 2, 3)
    minimum_brightnes: int # single byte
    direction: List[CurvePoint]
    pulse_overlap: int # in steps
    color: GradientLevelPair

    def serialize(self) -> bytes:
        serialized_direction = b''.join(point.serialize() for point in self.direction)
        return (struct.pack('>IBBB', self.interval, self.color_handling, self.minimum_brightnes, len(self.direction))
                + serialized_direction + struct.pack('>I', self.pulse_overlap) + self.color.serialize())

@dataclass
class DissolveConfig:
    interval: int
    is_random: bool
    flip: bool
    both_directions: bool
    random_color_order: bool
    alternate_colors: bool
    density: List[CurvePoint]
    colors: List[GradientLevelPair]

    def serialize(self) -> bytes:
        serialized_density = b''.join(point.serialize() for point in self.density)
        serialized_colors = b''.join(pair.serialize() for pair in self.colors)
        return (struct.pack('>IBBBBBB', self.interval, self.is_random, self.flip, self.both_directions,
                             self.random_color_order, self.alternate_colors, len(self.density))
                + serialized_density + struct.pack('>B', len(self.colors)) + serialized_colors)

@dataclass
class PulseConfig:
    interval: int
    color: GradientLevelPair

    def serialize(self) -> bytes:
        return struct.pack('>I', self.interval) + self.color.serialize()

@dataclass
class SetLevelConfig:
    color: RGBColor
    
    def serialize(self) -> bytes:
        return self.color.serialize()

@dataclass
class StrobeConfig:
    interval: int
    cycle_time: int
    cycle_variation: List[CurvePoint]
    on_time: List[CurvePoint]
    color: GradientLevelPair

    def serialize(self) -> bytes:
        serialized_cycle_variation = b''.join(point.serialize() for point in self.cycle_variation)
        serialized_on_time = b''.join(point.serialize() for point in self.on_time)
        return (struct.pack('>IIB', self.interval, self.cycle_time, len(self.cycle_variation)) 
                + serialized_cycle_variation + struct.pack('>B', len(self.on_time)) 
                + serialized_on_time + self.color.serialize())

@dataclass
class TwinkleConfig:
    interval: int
    avg_pulse_interval: float
    coverage: float
    coverage_variation: float
    min_brightness: int # single byte
    max_brightness: int # single byte
    brightness_variation: float
    color_handling: int # single byte (0, 1, 2, 3)
    color: GradientLevelPair

    def serialize(self) -> bytes:
        return (struct.pack('>I', self.interval) + struct.pack('<fff', self.avg_pulse_interval, self.coverage, self.coverage_variation)
                             + struct.pack('>BB', self.min_brightness, self.max_brightness) + struct.pack('<f', self.brightness_variation) + struct.pack('>B', self.color_handling)
                + self.color.serialize())
