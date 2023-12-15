#include <vsg/all.h>

#ifdef vsgXchange_FOUND
#    include <vsgXchange/all.h>
#endif

#if defined ( __clang__ ) || defined ( __GNUC__ )
# define TracyFunction __PRETTY_FUNCTION__
#elif defined ( _MSC_VER )
# define TracyFunction __FUNCSIG__
#endif

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#include <vsg/utils/Instrumentation.h>

#include <iostream>

using namespace tracy;

class TracyInstrumentation : public vsg::Inherit<vsg::Instrumentation, TracyInstrumentation>
{
public:

    TracyInstrumentation()
    {
        vsg::info("tracy TracyInstrumentation()");
    }

    mutable std::map<vsg::Device*, VkCtx*> ctxMap;
    mutable VkCtx* ctx = nullptr;

    void markFrame()
    {
        FrameMark;
    }

    void createVkCtx(vsg::ref_ptr<vsg::Device> device, uint32_t queueFamilyIndex)
    {
        auto& context = ctxMap[device];
        if (!context)
        {
            auto queue = device->getQueue(queueFamilyIndex, 0);
            auto commandPool = vsg::CommandPool::create(device, queue->queueFamilyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            auto temporaryCommandBuffer = commandPool->allocate();
            context = TracyVkContext(device->getPhysicalDevice()->vk(), device->vk(), queue->vk(), temporaryCommandBuffer->vk());
        }
    }

    void enterCommandBuffer(vsg::ref_ptr<vsg::CommandBuffer> commandBuffer) override
    {
        ctx = ctxMap[commandBuffer->getDevice()];
        if (ctx)
        {
            TracyVkCollect(ctx, commandBuffer->vk());
        }
    }

    void leaveCommandBuffer() override
    {
        ctx = nullptr;
    }

    void enter(const vsg::SourceLocation* sl, uint64_t& reference) const override
    {
        if (!GetProfiler().IsConnected())
        {
            reference = 0;
            return;
        }

        reference = GetProfiler().ConnectionId();

        TracyQueuePrepare( QueueType::ZoneBegin );
        MemWrite( &item->zoneBegin.time, Profiler::GetTime() );
        MemWrite( &item->zoneBegin.srcloc, (uint64_t)sl );
        TracyQueueCommit( zoneBeginThread );
    }

    void leave(const vsg::SourceLocation*, uint64_t& reference) const override
    {
        if( reference==0 || GetProfiler().ConnectionId() != reference ) return;

        TracyQueuePrepare( QueueType::ZoneEnd );
        MemWrite( &item->zoneEnd.time, Profiler::GetTime() );
        TracyQueueCommit( zoneEndThread );
    }

    void enter(const vsg::SourceLocation* slcloc, uint64_t& reference, vsg::CommandBuffer& cmdbuf) const override
    {
        if (!GetProfiler().IsConnected())
        {
            reference = 0;
            return;
        }

        reference = GetProfiler().ConnectionId();

        const auto queryId = ctx->NextQueryId();
        CONTEXT_VK_FUNCTION_WRAPPER( vkCmdWriteTimestamp( cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, ctx->GetQueryPool(), queryId ) );

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::GpuZoneBeginSerial );
        MemWrite( &item->gpuZoneBegin.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneBegin.srcloc, (uint64_t)slcloc );
        MemWrite( &item->gpuZoneBegin.thread, GetThreadHandle() );
        MemWrite( &item->gpuZoneBegin.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneBegin.context, ctx->GetId() );
        Profiler::QueueSerialFinish();
    }

    void leave(const vsg::SourceLocation* /*slcloc*/, uint64_t& reference, vsg::CommandBuffer& cmdbuf) const override
    {
        if( reference==0 || GetProfiler().ConnectionId() != reference ) return;

        const auto queryId = ctx->NextQueryId();
        CONTEXT_VK_FUNCTION_WRAPPER( vkCmdWriteTimestamp( cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, ctx->GetQueryPool(), queryId ) );

        auto item = Profiler::QueueSerial();
        MemWrite( &item->hdr.type, QueueType::GpuZoneEndSerial );
        MemWrite( &item->gpuZoneEnd.cpuTime, Profiler::GetTime() );
        MemWrite( &item->gpuZoneEnd.thread, GetThreadHandle() );
        MemWrite( &item->gpuZoneEnd.queryId, uint16_t( queryId ) );
        MemWrite( &item->gpuZoneEnd.context, ctx->GetId() );
        Profiler::QueueSerialFinish();
    }

protected:

    ~TracyInstrumentation()
    {
        for(auto itr = ctxMap.begin(); itr != ctxMap.end(); ++itr)
        {
            TracyVkDestroy(itr->second);
        }
    }
};

int main(int argc, char** argv)
{
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();

#ifdef vsgXchange_all
    // add vsgXchange's support for reading and writing 3rd party file formats
    options->add(vsgXchange::all::create());
#endif

    arguments.read(options);

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "vsgtracyinstrumentation";
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
    windowTraits->synchronizationLayer = arguments.read("--sync");
    bool reportAverageFrameRate = arguments.read("--fps");
    if (arguments.read("--double-buffer")) windowTraits->swapchainPreferences.imageCount = 2;
    if (arguments.read("--triple-buffer")) windowTraits->swapchainPreferences.imageCount = 3; // default
    if (arguments.read("--IMMEDIATE")) { windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; }
    if (arguments.read("--FIFO")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (arguments.read("--FIFO_RELAXED")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    if (arguments.read("--MAILBOX")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    if (arguments.read({"-t", "--test"}))
    {
        windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        windowTraits->fullscreen = true;
        reportAverageFrameRate = true;
    }
    if (arguments.read({"--st", "--small-test"}))
    {
        windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        windowTraits->width = 192, windowTraits->height = 108;
        windowTraits->decoration = false;
        reportAverageFrameRate = true;
    }

    if (arguments.read({"--fullscreen", "--fs"})) windowTraits->fullscreen = true;
    if (arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height)) { windowTraits->fullscreen = false; }
    auto maxPagedLOD = arguments.value(0, "--maxPagedLOD");
    auto numFrames = arguments.value(-1, "-f");
    auto pathFilename = arguments.value(vsg::Path(), "-p");
    auto horizonMountainHeight = arguments.value(0.0, "--hmh");
    auto nearFarRatio = arguments.value<double>(0.001, "--nfr");

    auto group = vsg::Group::create();

    vsg::Path path;

    // read any vsg files
    for (int i = 1; i < argc; ++i)
    {
        vsg::Path filename = arguments[i];
        path = vsg::filePath(filename);

        auto object = vsg::read(filename, options);
        if (auto node = object.cast<vsg::Node>())
        {
            group->addChild(node);
        }
        else if (object)
        {
            std::cout << "Unable to view object of type " << object->className() << std::endl;
        }
        else
        {
            std::cout << "Unable to load file " << filename << std::endl;
        }
    }

    if (group->children.empty())
    {
        return 1;
    }

    vsg::ref_ptr<vsg::Node> vsg_scene;
    if (group->children.size() == 1)
        vsg_scene = group->children[0];
    else
        vsg_scene = group;

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    auto window = vsg::Window::create(windowTraits);
    if (!window)
    {
        std::cout << "Could not create window." << std::endl;
        return 1;
    }

    viewer->addWindow(window);

    // compute the bounds of the scene graph to help position camera
    vsg::ComputeBounds computeBounds;
    vsg_scene->accept(computeBounds);
    vsg::dvec3 centre = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;

    // set up the camera
    auto lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));

    vsg::ref_ptr<vsg::ProjectionMatrix> perspective;
    auto ellipsoidModel = vsg_scene->getRefObject<vsg::EllipsoidModel>("EllipsoidModel");
    if (ellipsoidModel)
    {
        perspective = vsg::EllipsoidPerspective::create(lookAt, ellipsoidModel, 30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio, horizonMountainHeight);
    }
    else
    {
        perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 4.5);
    }

    auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));

    // add close handler to respond to the close window button and pressing escape
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));

    if (!pathFilename)
    {
        viewer->addEventHandler(vsg::Trackball::create(camera, ellipsoidModel));
    }
    else
    {
        auto animationPath = vsg::read_cast<vsg::AnimationPath>(pathFilename, options);
        if (!animationPath)
        {
            std::cout<<"Warning: unable to read animation path : "<<pathFilename<<std::endl;
            return 1;
        }

        auto animationPathHandler = vsg::AnimationPathHandler::create(camera, animationPath, viewer->start_point());
        animationPathHandler->printFrameStatsToConsole = true;
        viewer->addEventHandler(animationPathHandler);
    }

    auto commandGraph = vsg::createCommandGraphForView(window, camera, vsg_scene);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    auto instrumentation = TracyInstrumentation::create();
    viewer->instrumentation = instrumentation;

    // set up the Tracy VkCtx
    instrumentation->createVkCtx(window->getOrCreateDevice(), commandGraph->queueFamily);

    // assign instrumentation after settings up recordAndSubmitTasks, but before compile() to allow compile to initialize the Instrumentation with the approach queue etc.
    for(auto& task : viewer->recordAndSubmitTasks)
    {
        task->instrumentation = viewer->instrumentation;
        if (task->earlyTransferTask) task->earlyTransferTask->instrumentation = task->instrumentation;
        if (task->lateTransferTask) task->lateTransferTask->instrumentation = task->instrumentation;

        for(auto cg : task->commandGraphs)
        {
            commandGraph->instrumentation = task->instrumentation;
            commandGraph->getOrCreateRecordTraversal()->instrumentation = task->instrumentation;
        }
    }

    viewer->compile();

    if (maxPagedLOD > 0)
    {
        // set targetMaxNumPagedLODWithHighResSubgraphs after Viewer::compile() as it will assign any DatabasePager if required.
        for(auto& task : viewer->recordAndSubmitTasks)
        {
            if (task->databasePager) task->databasePager->targetMaxNumPagedLODWithHighResSubgraphs = maxPagedLOD;
        }
    }

    viewer->start_point() = vsg::clock::now();

    FrameMark;

    // rendering main loop
    while (viewer->advanceToNextFrame() && (numFrames < 0 || (numFrames--) > 0))
    {
        // vsg::info("viewer->getFrameStamp()->frameCount = ", viewer->getFrameStamp()->frameCount);

        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();

        // vsg::info("end of frame ", viewer->getFrameStamp()->frameCount, "\n");

        instrumentation->markFrame();
    }

    if (reportAverageFrameRate)
    {
        auto fs = viewer->getFrameStamp();
        double fps = static_cast<double>(fs->frameCount) / std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - viewer->start_point()).count();
        std::cout<<"Average frame rate = "<<fps<<" fps"<<std::endl;
    }


    return 0;
}
