#ifdef _WIN32

#include "Surface.hpp"
#include "../internal/ErrorCheck.hpp"

#include <Windows.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

VkSurfaceKHR VulkanSurface::Create(VkInstance instance, WindowHandle windowHandle)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = ::GetModuleHandleA(0);
	surfaceCreateInfo.hwnd = windowHandle;
	
	VkSurfaceKHR surface;
	VulkanCheck(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));

	return surface;
}

#endif