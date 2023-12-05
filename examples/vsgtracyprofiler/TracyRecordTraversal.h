#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/app/RecordTraversal.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Switch.h>
#include <vsg/nodes/Light.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/View.h>
#include <vsg/commands/Commands.h>

# include <tracy/Tracy.hpp>
# include <common/TracySystem.hpp>

#if VSG_virtual_RecordTraversal_apply

namespace vsgTracy
{

class TracyRecordTraversal : public vsg::RecordTraversal
{
public:

    explicit TracyRecordTraversal(uint32_t in_maxSlot = 2, std::set<vsg::Bin*> in_bins = {}) :
        RecordTraversal(in_maxSlot, in_bins) {}

    template<typename... Args>
    static vsg::ref_ptr<TracyRecordTraversal> create(Args&&... args)
    {
        return vsg::ref_ptr<TracyRecordTraversal>(new TracyRecordTraversal(args...));
    }

    vsg::vec4 debugColor = {.8f, .8f, .8f, 1.0f};

    template<class T>
    void instrument(const T& object, const char* name)
    {
        ZoneScoped;
        ZoneText(name, std::strlen(name));

        RecordTraversal::apply(object);
    }

    void apply(const vsg::Object& object) override { instrument(object, object.className()); }

    // scene graph nodes
    void apply(const vsg::Group& group) override { instrument(group, group.className()); }
    void apply(const vsg::QuadGroup& quadGroup) override { instrument(quadGroup, quadGroup.className()); }
    void apply(const vsg::LOD& lod) override { instrument(lod, lod.className()); }
    void apply(const vsg::PagedLOD& pagedLOD) override { instrument(pagedLOD, pagedLOD.className()); }
    void apply(const vsg::CullGroup& cullGroup) override { instrument(cullGroup, cullGroup.className()); }
    void apply(const vsg::CullNode& cullNode) override { instrument(cullNode, cullNode.className()); }
    void apply(const vsg::DepthSorted& depthSorted) override { instrument(depthSorted, depthSorted.className()); }
    void apply(const vsg::Switch& sw) override { instrument(sw, sw.className()); }

    // positional state
    void apply(const vsg::Light& light) override { instrument(light, light.className()); }
    void apply(const vsg::AmbientLight& light) override { instrument(light, light.className()); }
    void apply(const vsg::DirectionalLight& light) override { instrument(light, light.className()); }
    void apply(const vsg::PointLight& light) override { instrument(light, light.className()); }
    void apply(const vsg::SpotLight& light) override { instrument(light, light.className()); }

    // Vulkan nodes
    void apply(const vsg::Transform& transform) override { instrument(transform, transform.className()); }
    void apply(const vsg::MatrixTransform& mt) override { instrument(mt, mt.className()); }
    void apply(const vsg::StateGroup& sg) override { instrument(sg, sg.className()); }
    void apply(const vsg::Commands& commands) override { instrument(commands, commands.className()); }
    void apply(const vsg::Command& command) override { instrument(command, command.className()); }

    // Viewer level nodes
    void apply(const vsg::View& view) override { instrument(view, view.className()); }
    void apply(const vsg::CommandGraph& commandGraph) override { instrument(commandGraph, commandGraph.className()); }
};

} // namespace vsgTracy

EVSG_type_name(vsgTracy::TracyRecordTraversal);

#endif
