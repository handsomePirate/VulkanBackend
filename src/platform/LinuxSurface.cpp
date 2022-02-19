#ifdef __linux__

#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#include <vulkan/vulkan.hpp>

void VulkanBackend::CreateSurface(const BackendData& backendData, SurfaceData& surfaceData, void* windowHandle, void* connection)
{
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.dpy = (::Display*)connection;
	surfaceCreateInfo.window = (::Window)windowHandle;

	VulkanCheck(vkCreateXlibSurfaceKHR(backendData.instance, &surfaceCreateInfo, nullptr, &surfaceData.surface));
}

#endif