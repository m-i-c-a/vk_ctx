# vk_ctx

## Windows development

Install the following prerequisites:

- Visual Studio Build Tools with the Desktop development with C++ workload.
- The Vulkan SDK, with `VULKAN_SDK` available in the environment.
- CMake 3.20 or newer. Install it with `winget install Kitware.CMake` to add `cmake` and `ctest` to `PATH`.

Close and reopen VS Code after installing the tools. Install the CMake Tools extension, then select either the `debug` or `release` configure preset from the CMake status bar.

The presets can also be used from PowerShell:

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Use `release` in place of `debug` for an optimized build.