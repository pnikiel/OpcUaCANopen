import os
import sys
import argparse

quasar_path = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.pardir))

sys.path.insert(0, os.path.join(quasar_path, 'FrameworkInternals'))

from transformDesign import transformDesign

parser = argparse.ArgumentParser()
parser.add_argument('--output_dir', default=quasar_path)

args = parser.parse_args()

transformDesign(
    transform_path='Warnings/templates/designToWarnings.jinja',
    outputFile=os.path.join(args.output_dir, 'Warnings.cpp'),
    requiresMerge=False,
    astyleRun=True)
