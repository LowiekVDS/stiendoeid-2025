from dataclasses import dataclass
from typing import List
import struct 

@dataclass
class RGBColor:
    r: int
    g: int
    b: int

    def parse_from_xml(xml_element) -> 'RGBColor':
        r = round(float(xml_element.find(".//_x", namespaces={}).text))
        g = round(float(xml_element.find(".//_y", namespaces={}).text))
        b = round(float(xml_element.find(".//_z", namespaces={}).text))
        return RGBColor(r, g, b)

    def serialize(self) -> bytes:
        return struct.pack('>BBB', self.r, self.g, self.b)

@dataclass
class ColorPoint:
    focus: float
    position: float
    color: RGBColor

    def parse_from_xml(xml_element) -> 'ColorPoint':
        focus = float(xml_element.find(".//_focus", namespaces={}).text)
        position = float(xml_element.find(".//_position", namespaces={}).text)
        color = RGBColor.parse_from_xml(xml_element.find(".//_color", namespaces={}))
        return ColorPoint(focus, position, color)

    def serialize(self) -> bytes:
        return struct.pack('<ff', self.focus, self.position) + self.color.serialize()

@dataclass
class ColorGradient:
    color_points: List[ColorPoint]

    def parse_from_xml(xml_element) -> 'ColorGradient':
        color_points = [ColorPoint.parse_from_xml(color_point_elem) for color_point_elem in xml_element.findall(".//ColorPoint", namespaces={})]
        return ColorGradient(color_points)

    def serialize(self) -> bytes:
        serialized_points = b''.join(point.serialize() for point in self.color_points)
        return struct.pack('>B', len(self.color_points)) + serialized_points

@dataclass
class CurvePoint:
    x: float
    y: float

    def parse_from_xml(xml_element) -> 'CurvePoint':
        x = float(xml_element.find(".//X", namespaces={}).text) / 100.0
        y = float(xml_element.find(".//Y", namespaces={}).text) / 100.0
        return CurvePoint(x, y)

    def serialize(self) -> bytes:
        return struct.pack('<ff', self.x, self.y)

@dataclass
class GradientLevelPair:
    colorGradient: ColorGradient
    brightness: List[CurvePoint]

    def parse_from_xml(xml_element) -> 'GradientLevelPair':
        color_gradient_elem = xml_element.find(".//ColorGradient", namespaces={})
        color_gradient = ColorGradient.parse_from_xml(color_gradient_elem)
        brightness_curve_elem = xml_element.find(".//Curve", namespaces={})
        brightness_curve = [CurvePoint.parse_from_xml(point_pair_elem) for point_pair_elem in brightness_curve_elem.findall(".//PointPair", namespaces={})]
        return GradientLevelPair(color_gradient, brightness_curve)

    def serialize(self) -> bytes:
        serialized_brightness = b''.join(point.serialize() for point in self.brightness)
        return self.colorGradient.serialize() + struct.pack('>B', len(self.brightness)) + serialized_brightness