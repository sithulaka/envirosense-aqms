#!/bin/bash

echo "Installing Requirements..."
pip install -r requirements.txt

echo
echo "Running the data conversion..."
python readData.py

echo
echo "Done! Check output.xlsx"
