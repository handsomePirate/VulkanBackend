#pragma once

#ifndef VulkanLogger

#include <SoftwareCore/Singleton.hpp>
#include <SoftwareCore/Logger.hpp>

#define VulkanLogger ::Core::Singleton<::Core::Logger>::GetInstance()

#endif
