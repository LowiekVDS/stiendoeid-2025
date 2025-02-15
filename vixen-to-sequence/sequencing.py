from dataclasses import dataclass
from enum import IntEnum
from typing import List
import struct
import xml.etree.ElementTree as ET
from colors import *
from effects import *
import re
from copy import deepcopy
from pathlib import Path


MS_TO_STEP = 40.0 / 1000.0 # 40 steps per second

class EffectType(IntEnum):
    NONE = 0
    ALTERNATING = 1
    CHASE = 2
    DISSOLVE = 3
    PULSE = 4
    SET_LEVEL = 5
    STROBE = 6
    TWINKLE = 7

def time_to_steps(time: str) -> int:
    time_as_seconds = float(time[2:-1])
    return round(time_as_seconds * MS_TO_STEP * 1000.0)

@dataclass
class EffectNodeSurrogate:
    instance_id: str
    start_time: int # steps
    end_time: int # steps
    target_nodes: List[str]

    def parse_from_xml(xml_element) -> 'EffectNodeSurrogate':
        instance_id = xml_element.find(".//InstanceId", namespaces={}).text
        start_time = time_to_steps(xml_element.find(".//StartTime", namespaces={}).text)
        end_time = time_to_steps(xml_element.find(".//TimeSpan", namespaces={}).text) + start_time
        target_nodes_elem = xml_element.find(".//TargetNodes", namespaces={})
        target_nodes = [node.text for node in target_nodes_elem.findall(".//NodeId", namespaces={})]

        return EffectNodeSurrogate(instance_id, start_time, end_time, target_nodes)

@dataclass
class EffectObject:
    effect_id: int # effect_id: 1 byte
    start_led: int
    end_led: int
    effect_type: EffectType
    effect_config: bytes

    def serialize(self) -> bytes:
        return (struct.pack('<BBHHH', self.effect_id, self.effect_type, self.start_led, 
                self.end_led, len(self.effect_config)) + self.effect_config)
    
@dataclass
class SequenceItem:
    start_time: int # in steps
    end_time: int # in steps
    effect: EffectObject

def serialize_sequence(sequences: List[SequenceItem]) -> bytes:

    # We are going to step through the sequence and serialize each item
    max_step = 0
    for sequence in sequences:
        max_step = max(max_step, sequence.end_time)
    
    # Find the effects that are immediately active and give them an id
    active_effects = [sequence for sequence in sequences if sequence.start_time == 0]
    activated_at_step = 0
    for idx, effect in enumerate(active_effects):
        effect.effect.effect_id = idx
    active_ids = [effect.effect.effect_id for effect in active_effects]

    serialized_data = b''

    for step in range(1, max_step+1):

        # If a change has been made to the active effects, serialize the previous state
        new_active_effects = [sequence for sequence in sequences if sequence.start_time <= step and sequence.end_time > step]
        if new_active_effects != active_effects:

            # Serialize the previous state of active effects TODO
            partial_data_header = struct.pack('<HB', step - activated_at_step, len(active_effects))
            partial_data = b''
            for effect in active_effects:
                partial_data += effect.effect.serialize()
            print(f"Step {activated_at_step: <8} to {step: <8}: [", partial_data_header.hex(), "] ", partial_data.hex(' '))
            serialized_data += partial_data_header + partial_data

            # Determine which effects have ended and remove their ids
            ended_effects = [effect for effect in active_effects if effect not in new_active_effects]
            for effect in ended_effects:
                active_ids.remove(effect.effect.effect_id)
                effect.effect.effect_id = None

            # Determine which effects have started and give them an id
            started_effects = [effect for effect in new_active_effects if effect not in active_effects]
            for effect in started_effects:
                # Id is the first available id in the list, starting from 0
                effect.effect.effect_id = next(i for i in range(len(active_ids) + 1) if i not in active_ids)
                active_ids.append(effect.effect.effect_id)
            
            active_effects = new_active_effects
            activated_at_step = step
    
    return serialized_data

def parse_data_models(xml_element):
    data_models = []
    for data_model_elem in xml_element.findall(".//anyType", namespaces={}):
        data_model_type = data_model_elem.get('itype')
        if data_model_type == 'd2p1AlternatingData':
            data_model = AlternatingConfig.parse_from_xml(data_model_elem)
        elif data_model_type == 'd2p1DissolveData':
            data_model = DissolveConfig.parse_from_xml(data_model_elem)
        elif data_model_type == 'd2p1PulseData':
            data_model = PulseConfig.parse_from_xml(data_model_elem)
        elif data_model_type == 'd2p1SetLevelData':
            data_model = SetLevelConfig.parse_from_xml(data_model_elem)
        elif data_model_type == 'd2p1StrobeData':
            data_model = StrobeConfig.parse_from_xml(data_model_elem)
        elif data_model_type == 'd2p1TwinkleData':
            data_model = TwinkleConfig.parse_from_xml(data_model_elem)
        elif data_model_type == 'd2p1ChaseData':
            data_model = ChaseConfig.parse_from_xml(data_model_elem)
        else:
            print(f"Unknown data model type: {data_model_type}")
            continue

        data_model_instance_id = data_model_elem.find(".//ModuleInstanceId", namespaces={}).text
        data_models.append((data_model_instance_id, data_model))

    return data_models

def parse_node_surrogates(xml_element):
    surrogates = []
    for node_surrogate_elem in xml_element.findall(".//EffectNodeSurrogate", namespaces={}):
        surrogate = EffectNodeSurrogate.parse_from_xml(node_surrogate_elem)
        surrogates.append(surrogate)
    return surrogates

def remove_xml_prefixes(xml_string):
    xml_string = re.sub(r'</[^:\s>]+:([^>]+)>', r'</\1>', xml_string)  # Fix for end tags
    xml_string = re.sub(r'<[^/][^:\s>]+:([^>]+)>', r'<\1>', xml_string)  # Fix for start tag
    xml_string = re.sub(r':', '', xml_string)
    return xml_string.strip()

def parse_from_tim(xml_filename: str):

    xml_data = ''.join(open(xml_filename, 'r').readlines())
    xml_data = remove_xml_prefixes(xml_data)
    root = ET.fromstring(xml_data)

    data_models = parse_data_models(root.find(".//_dataModels", namespaces={}))
    effect_node_surrogates = parse_node_surrogates(root.find(".//_effectNodeSurrogates", namespaces={}))

    # TODO: parse the start led and end led from a file as well
    node_id_to_led_range = {
        '167a3cf4-1763-4954-b926-3d866beb4706': (0, 276),
        'b73ce993-9866-4bfe-8419-363d80993188': (276, 504),
        '7e8c2286-86da-4c30-8438-b78ba59d989c': (504, 582),
        'd4f73b37-d92e-429a-bf27-8ee82b1c0789': (582, 654),
    }

    sequences = []
    for surrogate in effect_node_surrogates:
        data_model_instance_id = surrogate.instance_id
        data_model = None
        for instance_id, model in data_models:
            if instance_id == data_model_instance_id:
                data_model = model
                break
        if data_model is None:
            print(f"Could not find data model with id {data_model_instance_id}")
            continue
        effect_type = None
        if isinstance(data_model, AlternatingConfig):
            effect_type = EffectType.ALTERNATING
        elif isinstance(data_model, DissolveConfig):
            effect_type = EffectType.DISSOLVE
        elif isinstance(data_model, PulseConfig):
            effect_type = EffectType.PULSE
        elif isinstance(data_model, SetLevelConfig):
            effect_type = EffectType.SET_LEVEL
        elif isinstance(data_model, StrobeConfig):
            effect_type = EffectType.STROBE
        elif isinstance(data_model, TwinkleConfig):
            effect_type = EffectType.TWINKLE
        elif isinstance(data_model, ChaseConfig):
            effect_type = EffectType.CHASE
        else:
            print(f"Unknown effect type: {data_model}")
            continue
        start_led, end_led = node_id_to_led_range[surrogate.target_nodes[0]]
        data_model_copy = deepcopy(data_model)
        if effect_type != EffectType.SET_LEVEL and effect_type != EffectType.ALTERNATING:
            data_model_copy.interval = surrogate.end_time - surrogate.start_time
        
        effect = EffectObject(0, start_led, end_led, effect_type, data_model_copy.serialize())
        
        sequences.append(SequenceItem(surrogate.start_time, surrogate.end_time, effect))

    serialized_data = serialize_sequence(sequences)
    
    path = Path(__file__).parent / "../receiver/data/sequence.bin"
    with path.open('wb') as f:
        f.write(serialized_data)

parse_from_tim('vanalles.tim')


# if __name__ == "__main__":

#     from pathlib import Path

#     from effects import TwinkleConfig
#     from colors import *

#     config = TwinkleConfig(200, 10, 0.5, 0.3, 0, 15, 0.5, 1, 
#                 GradientLevelPair(
#                     ColorGradient([
#                         ColorPoint(focus=0.5, position=0, color=RGBColor(255, 0, 0)),
#                         ColorPoint(focus=0.5, position=0.5, color=RGBColor(0, 255, 0)),
#                         ColorPoint(focus=0.5, position=1, color=RGBColor(0, 0, 255))
#                     ]),
#                     [CurvePoint(0, 0), CurvePoint(0.1, 1), CurvePoint(0.9, 1), CurvePoint(1, 0)]
#                 ))

#     # Example usage
#     sequences = [
#         SequenceItem(0, 200, EffectObject(0, 0, 654, EffectType.TWINKLE, config.serialize())),
#         # SequenceItem(50, 100, EffectObject(0, 10, 20, EffectType.SET_LEVEL, b'\x00\x00\xff')),
#         # SequenceItem(80, 100, EffectObject(0, 0, 5, EffectType.SET_LEVEL, b'\x00\xff\x00')),
#     ]

#     serialized_data = serialize_sequence(sequences)

#     # Write this to sequence.bin
#     path = Path(__file__).parent / "../receiver/data/sequence.bin"
#     with path.open('wb') as f:
#         f.write(serialized_data)