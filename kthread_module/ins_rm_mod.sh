#!/bin/bash

# Define kernel version and module path
KERNEL_VERSION="5.15.0-125-generic"
MODULES_DIR="/lib/modules/$KERNEL_VERSION"
KO_FILE=$(find . -maxdepth 1 -name "*.ko" | head -n 1)

if [ -z "$KO_FILE" ]; then
    echo "Error: No .ko file found in the current directory."
    exit 1
fi

MODULE_NAME=$(basename "$KO_FILE" .ko)

# Function: Install the module
install_module() {
    echo "Installing module: $MODULE_NAME"
    # Copy the module to the target path
    sudo cp "$KO_FILE" "$MODULES_DIR"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to copy $KO_FILE to $MODULES_DIR"
        exit 1
    fi

    # Update module dependencies
    sudo depmod
    if [ $? -ne 0 ]; then
        echo "Error: depmod failed."
        exit 1
    fi

    # Load the module
    sudo modprobe "$MODULE_NAME"
    if [ $? -ne 0 ]; then
        echo "Error: modprobe failed for module $MODULE_NAME."
        exit 1
    fi

    echo "Module $MODULE_NAME installed successfully."
}

# Function: Remove the module
remove_module() {
    echo "Removing module: $MODULE_NAME"
    # Unload the module
    sudo rmmod "$MODULE_NAME"
    if [ $? -ne 0 ]; then
        echo "Warning: rmmod failed for module $MODULE_NAME. It may not be loaded."
    fi

    # Delete the module file
    sudo rm -rf "$MODULES_DIR/$MODULE_NAME.ko"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to remove $MODULES_DIR/$MODULE_NAME.ko"
        exit 1
    fi

    echo "Module $MODULE_NAME removed successfully."
}

# Display options if no valid argument is provided
display_usage() {
    echo "Usage: $0 [in|rm]"
    echo "Options:"
    echo "  in    Install the module."
    echo "  rm    Remove the module."
    exit 1
}

# Check the input argument and perform the corresponding action
case "$1" in
    "in"|"")
        install_module
        ;;
    "rm")
        remove_module
        ;;
    *)
        display_usage
        ;;
esac
