# vsgTray

[Tracy](https://github.com/wolfpld/tracy) Profiler integration with [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph)

## Checking out vsgImGui

~~~ sh
git clone https://github.com/vsg-dev/vsgImGui.git
~~~

### Building vsgImGui

The first run of cmake will automatically checkout Tracy as submodule when required.

~~~ sh
cd vsgTracy
cmake .
make -j 8
~~~

