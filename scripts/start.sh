#!/bin/bash
# Optimized startup script

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

EMBD_NAME="uwb_runner"


# Set library path
# Do not add if LD_LIBRARY_PATH already contains SCRIPT_DIR/lib
if [ -n "$LD_LIBRARY_PATH" ] && echo ":$LD_LIBRARY_PATH:" | grep -q ":$SCRIPT_DIR/lib:"; then
    echo "LD_LIBRARY_PATH already contains $SCRIPT_DIR/lib, not modifying"
else
    export LD_LIBRARY_PATH="$SCRIPT_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi
echo "== LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

# Set CycloneDDS configuration
export CYCLONEDDS_URI="file://$SCRIPT_DIR/cyclonedds.xml"
echo "== CYCLONEDDS_URI=$CYCLONEDDS_URI"

# Check executable
EXECUTABLE="$SCRIPT_DIR/bin/$EMBD_NAME"
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: executable not found: $EXECUTABLE"
    exit 1
fi

# Run program
echo "Starting ${EMBD_NAME} ..."
echo "Library path: $LD_LIBRARY_PATH"
echo "Executable: $EXECUTABLE"
echo ""
echo ""

# Pass all command-line arguments
"$EXECUTABLE" 