#!/bin/bash
# init in-build  test environment 
# usage: 
#        source test_env.sh


# Get the directory of this script
if [ -n "${BASH_SOURCE[0]-}" ]; then
    __SCRIPT_SOURCE="${BASH_SOURCE[0]}"
elif [ -n "${ZSH_VERSION-}" ]; then
    __SCRIPT_SOURCE="$(eval 'echo ${(%):-%x}')"
else
    __SCRIPT_SOURCE="$0"
fi

__SCRIPT_DIR="$(cd "$(dirname "$__SCRIPT_SOURCE")" && pwd)"
__PROJECT_DIR="$(dirname "$__SCRIPT_DIR")"
__DEPEND_DIR="${__PROJECT_DIR}/3rd"

echo " == SCRIPT_DIR=$__SCRIPT_DIR"
echo " == DEPEND_DIR=$__DEPEND_DIR"

# Set library path
# Do not add if LD_LIBRARY_PATH already contains SCRIPT_DIR/lib
if [ -n "$LD_LIBRARY_PATH" ] && echo ":$LD_LIBRARY_PATH:" | grep -q ":$__DEPEND_DIR/lib:"; then
    echo "LD_LIBRARY_PATH already contains $__DEPEND_DIR/lib, not modifying"
else
    export LD_LIBRARY_PATH="$__DEPEND_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi

export CYCLONEDDS_URI=file://${__PROJECT_DIR}/config/cyclonedds.xml

echo ""
echo " == LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo " == CYCLONEDDS_URI=$CYCLONEDDS_URI"

# Avoid leaking temporary variables into current shell when sourced.
unset __SCRIPT_SOURCE __SCRIPT_DIR __PROJECT_DIR __DEPEND_DIR