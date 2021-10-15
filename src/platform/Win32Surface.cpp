#ifdef _WIN32

#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.hpp>

void VulkanBackend::CreateSurface(VkInstance instance, SurfaceData& surfaceData, void* windowHandle, void* connection)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (::HINSTANCE)connection;
	surfaceCreateInfo.hwnd = (::HWND)windowHandle;

	VulkanCheck(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surfaceData.surface));
}

#endif