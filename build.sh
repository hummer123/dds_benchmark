#!/usr/bin/env bash

# Battery Voltage Guard build script
# Usage: ./build.sh [x86_64|aarch64] [Debug|Release]

set -euo pipefail

# Path calculation (assumes the script is in the repository root or under it)
SCRIPT_DIR=$(cd $(dirname "$0") && pwd)
BUILD_DIR="${SCRIPT_DIR}/build"

echo "== SCRIPT_DIR: $SCRIPT_DIR"
echo "== BUILD_DIR: $BUILD_DIR"

# Color output for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error()   { echo -e "${RED}[ERROR]${NC} $1"; }

# Detect host architecture
detect_arch() {
    local machine_arch=$(uname -m)
    case "$machine_arch" in
        x86_64|amd64) echo "x86_64" ;;
        aarch64|arm64) echo "aarch64" ;;
        *) log_warning "Unknown architecture '$machine_arch', defaulting to x86_64"; echo "x86_64" ;;
    esac
}

# Default parameters
ARCH=$(detect_arch)
BUILD_TYPE="Release"

# 显示使用帮助
show_help() {
    cat <<EOF
用法: $0 [架构] [构建类型]
    架构: x86_64 或 aarch64 （可选，默认为 x86_64）
    构建类型: Debug 或 Release （可选，默认为 Release）

示例:
    $0                  # 构建默认版本 (x86_64 Release)
    $0 x86_64           # 构建 x86_64 版本 (Release)
    $0 aarch64         # 为 aarch64 交叉编译版本 (Release)
    $0 x86_64 Debug     # 构建 x86_64 版本 (Debug)
    $0 --help              # 显示帮助信息
EOF
}

# 解析并验证参数
if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
    show_help
    exit 0
fi

# Parse architecture if provided
if [ $# -ge 1 ]; then
    case "$1" in
        x86_64|aarch64)
            ARCH="$1"
            ;;
        *)
            log_error "不支持的架构 '$1'。支持的架构: x86_64, aarch64"
            show_help
            exit 1
            ;;
    esac
fi

# Optional build type parameter
if [ $# -ge 2 ]; then
    case "$2" in
        Debug|Release)
            BUILD_TYPE="$2"
            ;;
        *)
            log_error "不支持的构建类型 '$2'。支持的类型: Debug, Release"
            show_help
            exit 1
            ;;
    esac
fi

# Show build information
echo "=========================================="
echo "Battery Voltage Guard 构建脚本"
echo "=========================================="
echo "架构:       $ARCH"
echo "构建类型:   $BUILD_TYPE"
echo "构建目录:   $BUILD_DIR"
echo "=========================================="

# Check dependencies
check_dependencies() {
    # Check for cross-compilation tools if needed
    if [ "$ARCH" = "aarch64" ]; then
        if ! command -v aarch64-linux-gnu-gcc &> /dev/null; then
            log_error "aarch64-linux-gnu-gcc 未安装。请安装交叉编译工具链。"
            exit 1
        fi
    fi
    
    # Check for required source files
    if [ ! -f "${SCRIPT_DIR}/CMakeLists.txt" ]; then
        log_error "CMakeLists.txt 文件不存在。"
        exit 1
    fi
}

# Change to project root directory
cd "$SCRIPT_DIR"

# Check dependencies first
check_dependencies

# Clean and create the build directory (safety checks)
if [ -z "${BUILD_DIR:-}" ] || [ "$BUILD_DIR" = "/" ]; then
    log_error "BUILD_DIR 未设置或不安全 ('$BUILD_DIR')，中止"
    exit 1
fi

if [ -d "$BUILD_DIR" ]; then
    log_info "检测到已存在的构建目录 '$BUILD_DIR'，正在删除..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Cross-compilation settings for aarch64
CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
if [ "$ARCH" = "aarch64" ]; then
    log_info "为 aarch64 架构进行交叉编译..."
    source "${SCRIPT_DIR}/scripts/env_aarch64_cross_compile.sh"
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=../cmake/aarch64-toolchain.cmake"
fi

# Run CMake configuration
log_info "正在配置CMake..."
if ! cmake $CMAKE_OPTIONS ..; then
    log_error "CMake 配置失败"
    exit 1
fi

# Build
log_info "正在编译..."
if ! make -j"$(nproc)"; then
    log_error "编译失败"
    exit 1
fi

# Install
log_info "正在安装到 deploy 目录..."
if ! make install; then
    log_error "安装失败"
    exit 1
fi


echo "=========================================="
log_success "构建完成！"
echo "可执行文件位于: $BUILD_DIR/deploy/bin/"
echo "配置文件位于:   $BUILD_DIR/deploy/config/"
echo "库文件位于:     $BUILD_DIR/deploy/lib/"
echo "启动脚本位于:   $BUILD_DIR/deploy/"
echo "构建类型:       $BUILD_TYPE"
echo "架构:           $ARCH"
echo "=========================================="
