import re
import xml.etree.ElementTree as ET
from colors import *
from effects import *

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

def remove_xml_prefixes(xml_string):
    xml_string = re.sub(r'</[^:\s>]+:([^>]+)>', r'</\1>', xml_string)  # Fix for end tags
    xml_string = re.sub(r'<[^/][^:\s>]+:([^>]+)>', r'<\1>', xml_string)  # Fix for start tag
    xml_string = re.sub(r':', '', xml_string)
    return xml_string.strip()

if __name__ == "__main__":

    # Example usage
    xml_data = ''.join(open('vanalles.tim', 'r').readlines())
    xml_data = remove_xml_prefixes(xml_data)
    root = ET.fromstring(xml_data)

    data_models = parse_data_models(root.find(".//_dataModels", namespaces={}))
    print(data_models)