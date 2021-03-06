// vulkan_1.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
#include "vulkan/vulkan.h"
#ifdef _WIN32
#include "windows.h"
#include "vulkan\vulkan_win32.h"
#endif // _WIN32

typedef struct _layer_properties {
	VkLayerProperties m_properties;
	std::vector<VkExtensionProperties> m_vec_instance_extensions;
	std::vector<VkExtensionProperties> m_vec_device_extensions;
}s_layer_properties;

typedef struct _swap_chain_buffer {
	VkImage m_image;
	VkImageView m_view;
}s_swap_chain_buffer;

typedef struct _texture_object {
	VkSampler m_sampler;
	VkImage m_image;
	VkImageLayout m_image_layout;
	VkDeviceMemory m_mem;
	VkImageView m_view;
	int32_t m_tex_width;
	int32_t m_tex_height;
}s_texture_object;

typedef struct _sample_info {
#ifdef _WIN32
#define APP_NAME_STR_LEN 80
	HINSTANCE m_connection;				//Windows Instance
	char m_name[APP_NAME_STR_LEN];		//Name to put on the window/icon
	HWND m_window;						//window handle
#endif // _WIN32

	VkSurfaceKHR m_surface;
	bool m_b_prepared;
	bool m_b_use_staging_buffer;
	bool m_b_save_images;

	std::vector<const char*> m_vec_instance_layer_name;
	std::vector<const char*> m_vec_instance_extension_names;
	std::vector<s_layer_properties> m_vec_instance_layer_propperties;
	std::vector<VkExtensionProperties> m_instance_extension_properties;
	VkInstance m_inst;

	std::vector<const char*> m_device_extension_names;
	std::vector<VkExtensionProperties> m_device_extension_properties;
	std::vector<VkPhysicalDevice> m_gpus;
	VkDevice m_device;
	VkQueue m_graphics_queue;
	VkQueue m_present_queue;
	uint32_t u32_graphics_queue_family_index;
	uint32_t u32_present_queue_family_index;
	VkPhysicalDeviceProperties m_gpu_props;
	std::vector<VkQueueFamilyProperties> m_queue_props;
	VkPhysicalDeviceMemoryProperties m_memory_properties;

	VkFramebuffer *m_p_framebuffers;
	int m_n_width;
	int m_n_height;
	VkFormat m_format;

	uint32_t m_swapchain_image_count;
	VkSwapchainKHR m_swap_chain;
	std::vector<s_swap_chain_buffer> m_buffers;
	VkSemaphore m_image_acquired_semaphore;

	VkCommandPool m_cmd_pool;

	struct {
		VkFormat m_format;
		VkImage m_image;
		VkDeviceMemory m_mem;
		VkImageView m_view;
	}m_depth;

	std::vector<s_texture_object> m_textures;

	struct {
		VkBuffer m_buf;
		VkDeviceMemory m_mem;
		VkDescriptorBufferInfo m_buffer_info;
	}m_uniform_data;

	struct {
		VkDescriptorImageInfo m_image_info;
	}m_texture_data;
	VkDeviceMemory m_staging_memory;
	VkImage m_staging_image;

	struct {
		VkBuffer m_buf;
		VkDeviceMemory m_mem;
		VkDescriptorBufferInfo m_buffer_info;
	}m_vertex_buffer;
	VkVertexInputBindingDescription m_vi_binding;
	VkVertexInputAttributeDescription m_vi_attribs;

	//glm::mat4 Projection;
	//glm::mat4 View;
	//glm::mat4 Model;
	//glm::mat4 Clip;
	//glm::mat4 MVP;

	VkCommandBuffer m_cmd;
	VkPipelineLayout m_pipeline_layout;
	std::vector<VkDescriptorSetLayout> m_desc_layout;
	VkPipelineCache m_pipeline_cache;
	VkRenderPass m_render_pass;
	VkPipeline m_pipeline;

	VkPipelineShaderStageCreateInfo m_sahder_stages[2];

	VkDescriptorPool m_desc_pool;
	std::vector<VkDescriptorSet> m_desc_set;

	PFN_vkCreateDebugReportCallbackEXT m_db_createdebugreportcallback;
	PFN_vkDestroyDebugReportCallbackEXT m_db_destorydebugreportcallback;
	PFN_vkDebugReportMessageEXT m_dbbreakcallback;
	std::vector<VkDebugReportCallbackEXT> m_debugreportcallbacks;

	uint32_t m_current_buffers;
	uint32_t m_queue_family_count;

	VkViewport m_viewport;
	VkRect2D m_scissor;

}s_sample_info;

VkResult init_global_extension_properties(s_layer_properties &t_layer_pros) {
	VkExtensionProperties *t_p_instance_extensions;
	uint32_t u32_instance_count;
	VkResult t_res;
	char *t_p_layer_name = NULL;

	t_p_layer_name = t_layer_pros.m_properties.layerName;

	do {
		t_res = vkEnumerateInstanceExtensionProperties(t_p_layer_name, &u32_instance_count, NULL);
		if (t_res)
			return t_res;

		if (u32_instance_count == 0)
			return VK_SUCCESS;

		t_layer_pros.m_vec_instance_extensions.resize(u32_instance_count);
		t_p_instance_extensions = t_layer_pros.m_vec_instance_extensions.data();
		t_res = vkEnumerateInstanceExtensionProperties(t_p_layer_name, &u32_instance_count, t_p_instance_extensions);
	} while (t_res==VK_INCOMPLETE);

	return t_res;
}

VkResult init_global_layer_properties(s_sample_info &t_info) {
	uint32_t t_instance_layer_count;
	VkLayerProperties *t_p_vk_props = NULL;
	VkResult t_res;

	do {
		t_res = vkEnumerateInstanceLayerProperties(&t_instance_layer_count, NULL);
		if (t_res)
			return t_res;

		if (t_instance_layer_count == 0)
			return VK_SUCCESS;

		t_p_vk_props = (VkLayerProperties*)realloc(t_p_vk_props, t_instance_layer_count * sizeof(VkLayerProperties));
		t_res = vkEnumerateInstanceLayerProperties(&t_instance_layer_count, t_p_vk_props);
	} while (t_res==VK_INCOMPLETE);

	for (uint32_t i = 0;i < t_instance_layer_count;i++) {
		s_layer_properties t_layer_properties;
		t_layer_properties.m_properties = t_p_vk_props[i];
		t_res = init_global_extension_properties(t_layer_properties);//
		if (t_res)
			return t_res;
		t_info.m_vec_instance_layer_propperties.push_back(t_layer_properties);
	}
	free(t_p_vk_props);

	return t_res;
}

#define APP_SHORT_NAME "vulkan_1"

int main(int argc,char* argv[])
{
	//s_sample_info t_info = {};
	//init_global_layer_properties(t_info);

	VkApplicationInfo t_app_info = {};
	t_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	t_app_info.pNext = NULL;
	t_app_info.pApplicationName = APP_SHORT_NAME;
	t_app_info.apiVersion = 1;
	t_app_info.pEngineName = APP_SHORT_NAME;
	t_app_info.engineVersion = 1;
	t_app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo t_inst_info = {};
	t_inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	t_inst_info.pNext = NULL;
	t_inst_info.flags = 0;
	t_inst_info.pApplicationInfo = &t_app_info;
	t_inst_info.enabledExtensionCount = 0;
	t_inst_info.ppEnabledExtensionNames = NULL;
	t_inst_info.enabledLayerCount = 0;
	t_inst_info.ppEnabledLayerNames = NULL;

	VkInstance t_inst;
	VkResult t_res;

	t_res = vkCreateInstance(&t_inst_info, NULL, &t_inst);
	if (t_res == VK_ERROR_INCOMPATIBLE_DRIVER) {
		printf("cannot find a compatible Vulkan ICD\n");
		exit(-1);
	}
	else if (t_res) {
		printf("unknown error\n");
		exit(-1);
	}
	else {
		printf("create instance successfully\n");
	}

	vkDestroyInstance(t_inst, NULL);
	printf("Press any key to exit...\n");
	getchar();

    return 0;
}

