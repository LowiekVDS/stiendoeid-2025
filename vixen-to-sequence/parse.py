import xml.etree.ElementTree as ET
import json

class DataModelParser:
    def __init__(self, xml_file):
        self.tree = ET.parse(xml_file)
        self.root = self.tree.getroot()
        self.namespace = {
            'd1p1': 'http://schemas.microsoft.com/2003/10/Serialization/Arrays',
            'd2p1': 'http://schemas.datacontract.org/2004/07/VixenModules.Effect',
            'd3p1': 'http://schemas.datacontract.org/2004/07/VixenModules.App.ColorGradients',
            'd5p1': 'http://schemas.datacontract.org/2004/07/ZedGraph',
            'd6p1': 'http://schemas.datacontract.org/2004/07/Common.Controls.ColorManagement.ColorModels',
            'd8p1': 'http://schemas.datacontract.org/2004/07/Common.Controls.ColorManagement.ColorModels'
        }

    def parse_data_models(self):
        data_models = []
        data_models_tag = self.root.find(".//_dataModels", self.namespace)
        
        if data_models_tag is not None:
            for data in data_models_tag:
                model_data = {}
                model_data['type'] = data.attrib.get('{http://www.w3.org/2001/XMLSchema-instance}type', 'Unknown')
                
                for element in data:
                    if 'ColorGradient' in element.tag:
                        model_data['ColorGradient'] = self.parse_color_gradient(element)
                    elif 'Colors' in element.tag:
                        model_data['Colors'] = self.parse_gradient_level_pairs(element)
                    else:
                        model_data[element.tag.split('}')[-1]] = element.text if element.text else ''
                    
                data_models.append(model_data)
        
        return data_models

    def parse_color_gradient(self, color_gradient_element):
        gradient = {'alphas': [], 'color_points': []}
        
        for alpha in color_gradient_element.findall(".//d3p1:AlphaPoint", self.namespace):
            gradient['alphas'].append({
                'focus': alpha.find(".//d3p1:_focus", self.namespace).text,
                'position': alpha.find(".//d3p1:_position", self.namespace).text,
                'alpha': alpha.find(".//d3p1:_alpha", self.namespace).text
            })
        
        for color_point in color_gradient_element.findall(".//d3p1:ColorPoint", self.namespace):
            color_data = {
                'focus': color_point.find(".//d3p1:_focus", self.namespace).text,
                'position': color_point.find(".//d3p1:_position", self.namespace).text
            }
            color = color_point.find(".//d3p1:_color", self.namespace)
            if color is not None:
                color_data['x'] = color.find(".//d6p1:_x", self.namespace).text
                color_data['y'] = color.find(".//d6p1:_y", self.namespace).text
                color_data['z'] = color.find(".//d6p1:_z", self.namespace).text
            gradient['color_points'].append(color_data)
        
        return gradient

    def parse_gradient_level_pairs(self, colors_element):
        gradients = []
        
        for gradient_level_pair in colors_element.findall(".//d3p1:GradientLevelPair", self.namespace):
            gradient = {}
            
            color_gradient = gradient_level_pair.find(".//d3p1:ColorGradient", self.namespace)
            if color_gradient is not None:
                gradient['ColorGradient'] = self.parse_color_gradient(color_gradient)
            
            curve = gradient_level_pair.find(".//d3p1:Curve", self.namespace)
            if curve is not None:
                gradient['Curve'] = self.parse_curve(curve)
            
            gradients.append(gradient)
        
        return gradients

    def parse_curve(self, curve_element):
        curve_points = []
        print(curve_element)
        points_container = curve_element.find(".//{http://schemas.datacontract.org/2004/07/VixenModules.App.Curves}Points")
        print(points_container)
        if points_container is not None:
            for point in points_container.findall(".//{http://schemas.datacontract.org/2004/07/ZedGraph}PointPair", self.namespace):

                curve_points.append({
                    'X': point.find(".//X", self.namespace).text,
                    'Y': point.find(".//Y", self.namespace).text,
                    'Z': point.find(".//Z", self.namespace).text if point.find(".//Z", self.namespace) is not None else None
                })
        
        return curve_points

    def to_json(self, data):
        return json.dumps(data, indent=4)

class EffectNodeSurrogatesParser:
    def __init__(self, xml_file):
        self.tree = ET.parse(xml_file)
        self.root = self.tree.getroot()
        self.namespace = {
            'd1p1': 'http://schemas.microsoft.com/2003/10/Serialization/Arrays',
            'd2p1': 'http://schemas.datacontract.org/2004/07/VixenModules.Effect',
            'd3p1': 'http://schemas.datacontract.org/2004/07/VixenModules.App.ColorGradients',
            'd5p1': 'http://schemas.datacontract.org/2004/07/ZedGraph',
            'd6p1': 'http://schemas.datacontract.org/2004/07/Common.Controls.ColorManagement.ColorModels',
            'd8p1': 'http://schemas.datacontract.org/2004/07/Common.Controls.ColorManagement.ColorModels'
        }

    def parse_effect_node_surrogates(self):
        effect_nodes = []
        events = []
        effect_node_tag = self.root.find(".//_effectNodeSurrogates", self.namespace)
        
        if effect_node_tag is not None:
            for node in effect_node_tag.findall(".//EffectNodeSurrogate", self.namespace):
                instance_id = node.find(".//InstanceId", self.namespace).text
                start_time = self.parse_duration(node.find(".//StartTime", self.namespace).text)
                duration = self.parse_duration(node.find(".//TimeSpan", self.namespace).text)
                end_time = start_time + duration
                target_nodes = [tn.find(".//NodeId", self.namespace).text for tn in node.findall(".//ChannelNodeReferenceSurrogate", self.namespace)]
                
                events.append((start_time, instance_id, target_nodes, True))  # Start event
                events.append((end_time, instance_id, target_nodes, False))   # End event
        
        # Sort events by time
        events.sort()
        active_effects = {}
        
        for time, instance_id, target_nodes, is_start in events:
            for target_node in target_nodes:
                if target_node not in active_effects:
                    active_effects[target_node] = set()
                if is_start:
                    active_effects[target_node].add(instance_id)
                else:
                    active_effects[target_node].discard(instance_id)
            
            effect_nodes.append({
                'end_time': time,
                'active_effects': {tn: list(effects) for tn, effects in active_effects.items()}
            })
        
        return effect_nodes

    def parse_duration(self, duration):
        duration = duration.replace("PT", "")
        seconds = 0
        
        if 'H' in duration:
            hours, duration = duration.split('H')
            seconds += int(hours) * 3600
        if 'M' in duration:
            minutes, duration = duration.split('M')
            seconds += int(minutes) * 60
        if 'S' in duration:
            seconds += float(duration.replace('S', ''))
        
        return seconds
    
    def to_json(self, data):
        return json.dumps(data, indent=4)

if __name__ == "__main__":
    xml_file = "test.tim"  # Replace with the actual file path
    parser = DataModelParser(xml_file)
    parsed_data = parser.parse_data_models()
    print(parser.to_json(parsed_data))

    parser = EffectNodeSurrogatesParser(xml_file)
    parsed_effects = parser.parse_effect_node_surrogates()
    print(parser.to_json(parsed_effects))