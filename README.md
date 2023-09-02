# vsgTracy

[Tracy](https://github.com/wolfpld/tracy) profiler integration with [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph)

## Checking out vsgTracy

~~~ sh
git clone https://github.com/vsg-dev/vsgTracy.git
~~~

### Building vsgTracy

The first run of cmake will automatically checkout Tracy as submodule when required.

~~~ sh
cd vsgTracy
cmake .
make -j 8
~~~

