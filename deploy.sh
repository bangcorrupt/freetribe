#!/bin/bash

# Check if module parameter is provided
if [ $# -eq 0 ]; then
    echo "Error: You must provide the module name as a parameter"
    echo "Usage: $0 <module_name>"
    echo "Example: $0 wave2"
    exit 1
fi

# Assign the parameter to a variable
MODULE_NAME="$1"

# Execute the command with the specified module
sudo docker-compose exec freetribe make APP="$MODULE_NAME" MODULE="$MODULE_NAME" && python ../hacktribe/scripts/execute_freetribe.py cpu/build/cpu.bin