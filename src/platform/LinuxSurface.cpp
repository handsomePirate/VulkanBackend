#ifdef __linux__

#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#include <vulkan/vulkan.hpp>

void VulkanBackend::CreateSurface(VkInstance instance, SurfaceData& surfaceData, void* windowHandle, void* connection)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.dpy = (::Display*)connection;
	surfaceCreateInfo.window = (::Window)windowHandle;

	VulkanCheck(vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surfaceData.surface));
}

#endif