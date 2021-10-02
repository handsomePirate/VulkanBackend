#pragma once
#include <Singleton/Singleton.hpp>
#include <Logger/Logger.hpp>

#define VulkanLogger ::Core::Singleton<::Core::Logger>::GetInstance()
