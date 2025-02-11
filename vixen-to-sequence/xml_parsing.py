import xml.etree.ElementTree as ET
from collections import defaultdict

class TimedSequenceParser:
    def __init__(self, xml_file):
        self.tree = ET.parse(xml_file)
        self.root = self.tree.getroot()
        self.namespace = {'ns': 'http://schemas.datacontract.org/2004/07/VixenModules.Sequence.Timed'}

    def get_basic_info(self):
        return {
            "ModuleInstanceId": self.root.find(".//ModuleInstanceId", self.namespace).text,
            "ModuleTypeId": self.root.find(".//ModuleTypeId", self.namespace).text,
            "Length": self.root.find(".//Length", self.namespace).text,
            "Version": self.root.find(".//Version", self.namespace).text,
        }
    
    def get_effect_nodes(self):
        effects = []
        for effect in self.root.findall(".//EffectNodeSurrogate", self.namespace):
            effects.append({
                "InstanceId": effect.find("InstanceId").text,
                "StartTime": effect.find("StartTime").text,
                "TimeSpan": effect.find("TimeSpan").text,
                "TypeId": effect.find("TypeId").text,
                "TargetNode": effect.find(".//ChannelNodeReferenceSurrogate/Name").text
            })
        return effects
    
    def get_data_models(self):
        models = []
        for data_model in self.root.findall(".//_dataModels//d1p1:anyType", self.namespace):
            model = {
                "ModuleInstanceId": data_model.find("ModuleInstanceId").text,
                "ModuleTypeId": data_model.find("ModuleTypeId").text,
                "EffectType": data_model.get("{http://www.w3.org/2001/XMLSchema-instance}type", "Unknown")
            }
            models.append(model)
        return models
    
    def parse(self):
        return {
            "BasicInfo": self.get_basic_info(),
            "EffectNodes": self.get_effect_nodes(),
            "DataModels": self.get_data_models()
        }

if __name__ == "__main__":
    parser = TimedSequenceParser("test.tim")  # Replace with your XML file path
    data = parser.parse()
    print(data)
