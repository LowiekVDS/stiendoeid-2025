from dataclasses import dataclass
from enum import IntEnum
from typing import List
import struct

class EffectType(IntEnum):
    NONE = 0
    ALTERNATING = 1
    CHASE = 2
    DISSOLVE = 3
    PULSE = 4
    SET_LEVEL = 5
    STROBE = 6
    TWINKLE = 7

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
            partial_data = struct.pack('<HB', step - activated_at_step, len(active_effects))
            for effect in active_effects:
                partial_data += effect.effect.serialize()
            print(f"Step {activated_at_step: <8} to {step: <8}: ", partial_data.hex(' '))
            serialized_data += partial_data

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

# Example usage
sequences = [
    SequenceItem(0, 75, EffectObject(0, 0, 10, EffectType.SET_LEVEL, b'\xff\x00\x00')),
    SequenceItem(50, 100, EffectObject(0, 10, 20, EffectType.SET_LEVEL, b'\x00\x00\xff')),
    SequenceItem(80, 100, EffectObject(0, 0, 5, EffectType.SET_LEVEL, b'\x00\xff\x00')),
]

serialized_data = serialize_sequence(sequences)
print("final")
print(serialized_data.hex(' '))

# Write this to sequence.bin
with open('/home/lowiek/stiendoeid-2025/receiver/data/sequence.bin', 'wb') as f:
    f.write(serialized_data)