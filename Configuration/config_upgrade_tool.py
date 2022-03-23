#!/usr/bin/env python3

# @author: @pnikiel
# originally made for migrating changes for CANopen NG OPC-UA by Piotr Nikiel

import argparse
from lxml import etree

NSMAP={'c' : 'http://cern.ch/quasar/Configuration'}

def fixNodeGuardToMs(tree):
    for bus in tree.xpath('c:Bus', namespaces=NSMAP):
        if 'nodeGuardInterval' in bus.attrib:
            print(f"Found nodeGuardInterval at bus {bus.get('name')}, will transform to nodeGuardIntervalMs")
            bus.attrib['nodeGuardIntervalMs'] = str(int(float(bus.attrib['nodeGuardInterval'])*1000))
            bus.attrib.pop('nodeGuardInterval')


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('--input', required=True)

    args = parser.parse_args()

    xml_parser = etree.XMLParser(resolve_entities=False)

    tree = etree.parse(args.input, xml_parser)

    fixNodeGuardToMs(tree)

    fo = open(args.input + '.new', 'wb')
    fo.write(etree.tostring(tree, pretty_print=True, xml_declaration=True))



if __name__ == "__main__":
    main()
