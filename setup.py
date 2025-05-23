import os
import sys
import platform
import subprocess
from pathlib import Path

def pkg_config_has_vulkan():
    try:
        subprocess.run(['pkg-config', '--exists', 'vulkan'], check=True)
        return True
    except subprocess.CalledProcessError:
        return False
    except FileNotFoundError:
        print("Error: 'pkg-config' not found in PATH.")
        return False

def detect_vulkan_sdk():
    vulkan_sdk = os.environ.get('VULKAN_SDK')
    if not vulkan_sdk:
        print("VULKAN_SDK environment variable not set. Please install the Vulkan SDK.")
        return None
    return Path(vulkan_sdk)

def generate_pc_file(sdk_path, output_dir):
    system = platform.system().lower()

    include_dir = sdk_path / "Include"
    lib_dir = None
    libname = "-lvulkan"

    if system == "windows":
        lib_dir = sdk_path / "Lib"
        libname = "-lvulkan-1"
    elif system == "darwin":
        lib_dir = sdk_path / "macOS/lib"
        libname = "-lvulkan"
    else:
        # Assume Linux
        lib_dir = sdk_path / "lib"

    pc_content = f"""prefix={sdk_path.as_posix()}
exec_prefix=${{prefix}}
includedir=${{prefix}}/Include
libdir={lib_dir.as_posix()}

Name: Vulkan
Description: Vulkan SDK
Version: custom
Libs: -L${{libdir}} {libname}
Cflags: -I${{includedir}}
"""

    pc_path = Path(output_dir) / "vulkan.pc"
    pc_path.write_text(pc_content)
    print("Add this to your environment:")
    print(f"export PKG_CONFIG_PATH=\"{pc_path.parent.as_posix()}:$PKG_CONFIG_PATH\"")

def main():
    if pkg_config_has_vulkan():
        print("Vulkan already found via pkg-config. No action needed.")
        return

    sdk_path = detect_vulkan_sdk()
    if not sdk_path:
        sys.exit(1)

    output_dir = Path("thirdparty/pkgconfig")
    output_dir.mkdir(parents=True, exist_ok=True)
    generate_pc_file(sdk_path, output_dir)

if __name__ == "__main__":
    main()
