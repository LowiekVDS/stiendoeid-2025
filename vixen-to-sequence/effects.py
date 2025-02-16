from dataclasses import dataclass
from typing import List
import struct

from colors import *

MS_TO_STEP = 40.0 / 1000.0 # 40 steps per second

@dataclass
class AlternatingConfig:
    interval: int
    is_static: bool
    colors: List[GradientLevelPair]

    def parse_from_xml(xml_element) -> 'AlternatingConfig':
        interval = round(int(xml_element.find(".//Interval", namespaces={}).text) * MS_TO_STEP)
        is_static = bool(xml_element.find(".//EnableStatic", namespaces={}).text == "true")
        colors = [GradientLevelPair.parse_from_xml(gradient_level_pair_elem) for gradient_level_pair_elem in xml_element.findall(".//GradientLevelPair", namespaces={})]
        return AlternatingConfig(interval, is_static, colors)

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

    def parse_from_xml(xml_element) -> 'AlternatingConfig':
        direction = [CurvePoint.parse_from_xml(point_pair_elem) for point_pair_elem in xml_element.find(".//ChaseMovement", namespaces={}).findall(".//PointPair", namespaces={})]
        direction.sort(key=lambda x: x.x)
        color_gradient = ColorGradient.parse_from_xml(xml_element.find(".//ColorGradient", namespaces={}))
        brightness = [CurvePoint(0, 1), CurvePoint(1, 1)]
        color = GradientLevelPair(color_gradient, brightness)

        pulse_overlap = round(float(xml_element.find(".//PulseOverlap", namespaces={}).text) * MS_TO_STEP)

        # TODO: color_handling
        # TODO: minimum_brightnes
        # TODO: direction (check if correct)

        return ChaseConfig(0, 0, 0, direction, pulse_overlap, color)

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

    def parse_from_xml(xml_element) -> 'DissolveConfig':
        both_directions = bool(xml_element.find(".//BothDirections", namespaces={}).text)
        flip = bool(xml_element.find(".//DissolveFlip", namespaces={}).text)
        is_random = bool(xml_element.find(".//RandomDissolve", namespaces={}).text)
        random_color_order = bool(xml_element.find(".//RandomColor", namespaces={}).text)
        alternate_colors = not random_color_order
        colors = [GradientLevelPair.parse_from_xml(gradient_level_pair_elem) for gradient_level_pair_elem in xml_element.findall(".//GradientLevelPair", namespaces={})]
        dissolve_curve_elem = xml_element.find(".//DissolveCurve", namespaces={})
        density = [CurvePoint.parse_from_xml(density_elem) for density_elem in dissolve_curve_elem.findall(".//PointPair", namespaces={})]
        density.sort(key=lambda x: x.x)

        return DissolveConfig(0, is_random, flip, both_directions, random_color_order, alternate_colors, density, colors)

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

    def parse_from_xml(xml_element) -> 'PulseConfig':
        color_gradient = ColorGradient.parse_from_xml(xml_element.find(".//ColorGradient", namespaces={}))
        level_curve_elem = xml_element.find(".//LevelCurve", namespaces={})
        brightness_curve = [CurvePoint.parse_from_xml(point_pair_elem) for point_pair_elem in level_curve_elem.findall(".//PointPair", namespaces={})]
        brightness_curve.sort(key=lambda x: x.x)

        color = GradientLevelPair(color_gradient, brightness_curve)
    
        return PulseConfig(0, color)

    def serialize(self) -> bytes:
        print(self.interval)
        print(self.color.serialize())
        print("Done")
        return struct.pack('>I', self.interval) + self.color.serialize()

@dataclass
class SetLevelConfig:
    color: RGBColor

    def parse_from_xml(xml_element) -> 'SetLevelConfig':
        color = RGBColor(
            int(xml_element.find(".//_r", namespaces={}).text) * 255,
            int(xml_element.find(".//_g", namespaces={}).text) * 255,
            int(xml_element.find(".//_b", namespaces={}).text) * 255
        )
        return SetLevelConfig(color)
    
    def serialize(self) -> bytes:
        return self.color.serialize()

@dataclass
class StrobeConfig:
    interval: int
    cycle_time: int
    cycle_variation: List[CurvePoint]
    on_time: List[CurvePoint]
    color: GradientLevelPair

    def parse_from_xml(xml_element) -> 'StrobeConfig':
        on_time = [CurvePoint.parse_from_xml(point_pair_elem) for point_pair_elem in xml_element.find(".//OnTimeCurve", namespaces={}).findall(".//PointPair", namespaces={})]
        on_time.sort(key=lambda x: x.x)
        cycle_variation = [CurvePoint.parse_from_xml(point_pair_elem) for point_pair_elem in xml_element.find(".//CycleVariationCurve", namespaces={}).findall(".//PointPair", namespaces={})]
        cycle_time = round(int(xml_element.find(".//CycleTime", namespaces={}).text) * MS_TO_STEP)

        intensity = [CurvePoint.parse_from_xml(point_pair_elem) for point_pair_elem in xml_element.find(".//IntensityCurve", namespaces={}).findall(".//PointPair", namespaces={})]
        intensity.sort(key=lambda x: x.x)
        color_gradient = ColorGradient.parse_from_xml(xml_element.find(".//Colors", namespaces={}))
        color = GradientLevelPair(color_gradient, intensity)

        return StrobeConfig(0, cycle_time, cycle_variation, on_time, color)

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
    min_brightness: int
    max_brightness: int
    brightness_variation: float
    color_handling: int 
    color: GradientLevelPair

    def parse_from_xml(xml_element) -> 'TwinkleConfig':
        min_brightness = round(float(xml_element.find(".//MinimumLevel", namespaces={}).text) * 255)
        max_brightness = round(float(xml_element.find(".//MaximumLevel", namespaces={}).text) * 255) 
        brightness_variation = float(xml_element.find(".//LevelVariation", namespaces={}).text) / 100.0

        avg_pulse_interval = float(xml_element.find(".//AveragePulseTime", namespaces={}).text) * MS_TO_STEP
        coverage = float(xml_element.find(".//AverageCoverage", namespaces={}).text) / 100.0
        coverage_variation = float(xml_element.find(".//PulseTimeVariation", namespaces={}).text) / 100.0

        color_gradient = ColorGradient.parse_from_xml(xml_element.find(".//ColorGradient", namespaces={}))
        brightness_curve = [CurvePoint(0, 1), CurvePoint(1, 1)]
        brightness_curve.sort(key=lambda x: x.x)
        color = GradientLevelPair(color_gradient, brightness_curve)

        # TODO: color_handling

        return TwinkleConfig(0, avg_pulse_interval, coverage, coverage_variation, min_brightness, max_brightness, brightness_variation, 0, color)   

    def serialize(self) -> bytes:
        return (struct.pack('>I', self.interval) + struct.pack('<fff', self.avg_pulse_interval, self.coverage, self.coverage_variation)
                             + struct.pack('>BB', self.min_brightness, self.max_brightness) + struct.pack('<f', self.brightness_variation) + struct.pack('>B', self.color_handling)
                + self.color.serialize())
