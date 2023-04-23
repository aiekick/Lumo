#include <vulkan/vulkan.h>
#include <chrono>
#include <iostream>

// Classe de base pour le profilage
class Profiler {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
};

// Profileur pour le temps CPU
class CPUProfiler : public Profiler {
public:
    void start() override {
        startTime = std::chrono::high_resolution_clock::now();
    }

    void stop() override {
        endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = endTime - startTime;
        std::cout << "Durée CPU: " << duration.count() << " secondes." << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
};

// Profileur pour le temps GPU
class GPUProfiler : public Profiler {
public:
    GPUProfiler(VkDevice device, VkQueue queue)
        : device(device), queue(queue) {
        createQueryPool();
    }

    ~GPUProfiler() {
        vkDestroyQueryPool(device, queryPool, nullptr);
    }

    void start() override {
        vkCmdResetQueryPool(commandBuffer, queryPool, 0, 2);
        vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 0);
    }

    void stop() override {
        vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 1);
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        uint64_t timestamps[2];
        vkGetQueryPoolResults(device, queryPool, 0, 2, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

        double duration = static_cast<double>(timestamps[1] - timestamps[0]) * timestampPeriod * 1e-9;
        std::cout << "Durée GPU: " << duration << " secondes." << std::endl;
    }

private:
    void createQueryPool() {
        VkQueryPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = 2;

        vkCreateQueryPool(device, &createInfo, nullptr, &queryPool);

        // Vous devez récupérer la période de timestamp pour votre GPU
        // Utilisez vkGetPhysicalDeviceProperties pour cela
        // timestampPeriod = physicalDeviceProperties.limits.timestampPeriod;
    }

    VkDevice device;
    VkQueue queue;
    VkCommandBuffer commandBuffer;
    VkSubmitInfo submitInfo;
    VkQueryPool queryPool;
    double timestampPeriod;
};

#define PROFILER_CONCAT_INTERNAL(x, y) x##y
#define PROFILER_CONCAT(x, y) PROFILER_CONCAT_INTERNAL(x, y)

#define PROFILE_SCOPE(profiler, label) \
    profiler.start(); \
    auto PROFILER_CONCAT(scopeGuard, __LINE__) = [&]() { profiler.stop(); }; \
    (void)PROFILER_CONCAT(scopeGuard, __LINE__)

#define PROFILE_FRAME_CPU(label) PROFILE_SCOPE(cpuProfiler, label)
#define PROFILE_FRAME_GPU(label) PROFILE_SCOPE(gpuProfiler, label)

// Utilisation du profileur dans le code

int main() {
    // Initialisation de Vulkan et création des objets nécessaires (device, queue, commandBuffer, etc.)
    // ...

    CPUProfiler cpuProfiler;
    GPUProfiler gpuProfiler(device, queue);

    for (int i = 0; i < 10; ++i) {
        {
            PROFILE_FRAME_CPU("Update");
            // Mettez à jour les données et préparez les commandes pour le rendu
            // ...
        }

        {
            PROFILE_FRAME_GPU("Render");
            // Envoyez les commandes de rendu au GPU
            // ...
        }

        // Présentez l'image au framebuffer
        // ...
    }

    // Nettoyage des ressources Vulkan
    // ...

    return 0;
}
