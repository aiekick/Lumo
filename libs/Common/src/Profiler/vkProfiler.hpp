/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <stdarg.h>  // For va_start, etc.
#include <vulkan/vulkan.h>
#include <Common/Globals.h>

namespace vkprof
{
    typedef uint64_t vkTimeStamp;
    
    class COMMON_API vkProfiler
    {
    public:
        bool isActive = false;

    private:
        uint32_t sCurrentDepth = 0U; // Current Depth catched Profiler
        uint32_t sMaxDepth = 0U; // max puDepth encountered ever

    private:
        class QueryZone
        {
        public:
            std::string name;
            std::string section;
            uint32_t start_id = 0U;
            uint32_t end_id = 0U;

        public:
            QueryZone(const std::string& vName, const std::string& vSection, uint32_t vStartId, uint32_t vEndId)
                : name(vName), section(vSection), start_id(vStartId), end_id(vEndId) {}
        };

    public:
        class ScopedZone
        {
        private:
            //char buffer[255 + 1] = "";
            //VkQueryPool m_QueryPool;
            //VkCommandBuffer m_Command;
            //vkProfiler::QueryZone* m_QueryZonePtr = nullptr;
            //uint32_t* currentDepthPtr = nullptr;

        public:
            ScopedZone(const VkCommandBuffer& vCmd, const std::string& vSection, 
                const char* fmt, ...)
            {
                /*m_QueryZonePtr = nullptr;

                auto inst = vkProfiler::Instance();
                if (!inst->isActive)
                    return;
                
                currentDepthPtr = &inst->sCurrentDepth;

                va_list args;
                va_start(args, fmt);
                const int w = vsnprintf(buffer, 255, fmt, args);
                va_end(args);
                if (w)
                {
                    m_QueryZonePtr = inst->GetQueryZone(std::string(buffer, (size_t)w), vSection);
                    if (m_QueryZonePtr)
                    {
                        m_Command = vCmd;
                        m_QueryPool = inst->GetQueryPool();
                        vkCmdWriteTimestamp(m_Command, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_QueryPool, m_QueryZonePtr->start_id);
                        ++(*currentDepthPtr);
                    }
                }*/
            }

            ~ScopedZone()
            {
                /*if (m_QueryZonePtr)
                {
                    vkCmdWriteTimestamp(m_Command, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_QueryPool, m_QueryZonePtr->end_id);
                    --(*currentDepthPtr);
                }*/
            }
        };

    private:
        VkPhysicalDevice m_PhysicalDevice = {};
        VkDevice m_LogicalDevice = {};
        float m_TimeStampPeriod = 0.0f;
        VkQueryPool m_QueryPool = {};
        size_t m_QueryTail = 0U;
        size_t m_QueryHead = 0U;
        size_t m_QueryCount = 0U;
        bool m_IsLoaded = false;
        std::vector<vkTimeStamp> m_TimeStampMeasures;
        std::map<uint32_t, std::vector<QueryZone>> m_Zones;
        std::map<uint32_t, uint32_t> m_LastIndex;

    public:
        bool Init(VkPhysicalDevice vPhysicalDevice, VkDevice vLogicalDevice)
        {
            m_PhysicalDevice = vPhysicalDevice;
            m_LogicalDevice = vLogicalDevice;

            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
            m_TimeStampPeriod = props.limits.timestampPeriod;

            m_QueryCount = 64U * 1024U; // base count

            VkQueryPoolCreateInfo poolInfos = {};
            poolInfos.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            poolInfos.queryCount = (uint32_t)m_QueryCount;
            poolInfos.queryType = VK_QUERY_TYPE_TIMESTAMP;
            while(vkCreateQueryPool(m_LogicalDevice, &poolInfos, nullptr, &m_QueryPool) != VK_SUCCESS)
            {
                m_QueryCount /= 2U;
                poolInfos.queryCount = (uint32_t)m_QueryCount;
            }

            m_TimeStampMeasures.resize(m_QueryCount);

            m_QueryTail = 0U;
            m_QueryHead = 0U;

            m_IsLoaded = true;

            return m_IsLoaded;
        }

        void Unit()
        {
            if (m_QueryPool)
                vkDestroyQueryPool(m_LogicalDevice, m_QueryPool, nullptr);
        }

        void Collect(VkCommandBuffer vCmd)
        {
            if (!m_IsLoaded || !isActive) return;
            if (vkGetQueryPoolResults(
                m_LogicalDevice, 
                m_QueryPool,
                (uint32_t)m_QueryTail,
                (uint32_t)m_QueryHead,
                sizeof(vkTimeStamp) * m_QueryCount, 
                m_TimeStampMeasures.data(),
                sizeof(vkTimeStamp), 
                VK_QUERY_RESULT_64_BIT/* | VK_QUERY_RESULT_WAIT_BIT*/) == VK_NOT_READY)
            {
                return;
            }

            vkCmdResetQueryPool(vCmd, m_QueryPool, 0U, (uint32_t)m_QueryCount);
        }

        int32_t NextQueryId()
        {
            const auto id = m_QueryHead;
            m_QueryHead = (m_QueryHead + 1) % m_QueryCount;
            assert(m_QueryHead != m_QueryTail);
            return (uint32_t)id;
        }

        QueryZone* GetQueryZone(const std::string& vName, const std::string& vSection)
        {
            if (sCurrentDepth > sMaxDepth)
                sMaxDepth = sCurrentDepth;

            auto last_index = m_LastIndex[sCurrentDepth];
            auto count = m_Zones[sCurrentDepth].size();
            if (last_index == count)
            {
                m_Zones[sCurrentDepth].push_back(
                    QueryZone(vName, vSection, NextQueryId(), NextQueryId()));
            }
            ++m_LastIndex[sCurrentDepth];
            return &m_Zones[sCurrentDepth].at(last_index);
        }

        VkQueryPool GetQueryPool()
        {
            return m_QueryPool;
        }

        void DrawFlameGraph()
        {
            isActive = true;

            // on doit pouvoir tout enregister et afficher depuis ici
            // m_Zones a une lsite d'enfants par depth
            // on peut afficher les zones bien placée en fonction des globals timestamps
        }

    public:
        static vkProfiler* Instance()
        {
            static vkProfiler _instance;
            return &_instance;
        }

    public:
        vkProfiler() = default;
        ~vkProfiler() = default;
    };
}

#define VKFPScoped(cmd, section, fmt, ...) \
    auto __VKFP__ScopedZone = vkprof::vkProfiler::ScopedZone(cmd, section, fmt, ## __VA_ARGS__); \
    (void)__VKFP__ScopedZone
