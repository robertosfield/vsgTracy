
# Details of Vulkan support from the tracy-4.pdf

## 3.9.2 Vulkan
Similarly, for Vulkan support you should include the public/tracy/TracyVulkan.hpp header file. Tracing
Vulkan devices and queues is a bit more involved, and the Vulkan initialization macro TracyVkContext(physdev,
device, queue, cmdbuf) returns an instance of TracyVkCtx object, which tracks an associated Vulkan
queue. Cleanup is performed using the TracyVkDestroy(ctx) macro. You may create multiple Vulkan
contexts. To set a custom name for the context, use the TracyVkContextName(ctx, name, size) macro.

The physical device, logical device, queue, and command buffer must relate to each other. The queue
must support graphics or compute operations. The command buffer must be in the initial state and be able to
be reset. The profiler will rerecord and submit it to the queue multiple times, and it will be in the executable
state on exit from the initialization function.

To mark a GPU zone use the TracyVkZone(ctx, cmdbuf, name) macro, where name is a string literal
name of the zone. Alternatively you may use TracyVkZoneC(ctx, cmdbuf, name, color) to specify zone
color. The provided command buffer must be in the recording state, and it must be created within the queue
that is associated with ctx context.

You also need to periodically collect the GPU events using the TracyVkCollect(ctx, cmdbuf) macro44.
The provided command buffer must be in the recording state and outside a render pass instance.

Calibrated context In order to maintain synchronization between CPU and GPU time domains, you will
need to enable the [VK_EXT_calibrated_timestamps](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/proposals/VK_EXT_calibrated_timestamps.adoc) device extension and retrieve the following function
pointers: vkGetPhysicalDeviceCalibrateableTimeDomainsEXT and vkGetCalibratedTimestampsEXT.
To enable calibrated context, replace the macro TracyVkContext with TracyVkContextCalibrated and
pass the two functions as additional parameters, in the order specified above.

Using Vulkan 1.2 features Vulkan 1.2 and VK_EXT_host_query_reset provide mechanics to reset the
query pool without the need of a command buffer. By using TracyVkContextHostCalibrated you can make
use of this feature. It only requires a function pointer to vkResetQueryPool in addition to the ones required
for TracyVkContextCalibrated instead of the VkQueue and VkCommandBuffer handles.
However, using this feature requires the physical device to have calibrated device and host time domains. In
addition to VK_TIME_DOMAIN_DEVICE_EXT, vkGetPhysicalDeviceCalibrateableTimeDomainsEXT will have
to additionally return either VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT or VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTE
for Unix and Windows, respectively. If this is not the case, you will need to use TracyVkContextCalibrated
or TracyVkContext macro instead.

Dynamically loading the Vulkan symbols Some applications dynamically link the Vulkan loader, and
manage a local symbol table, to remove the trampoline overhead of calling through the Vulkan loader itself.
When TRACY_VK_USE_SYMBOL_TABLE is defined the signature of TracyVkContext, TracyVkContextCalibrated,
and TracyVkContextHostCalibrated are adjusted to take in the VkInstance, PFN_vkGetInstanceProcAddr,
and PFN_vkGetDeviceProcAddr to enable constructing a local symbol table to be used to call through the
Vulkan API when tracing.
