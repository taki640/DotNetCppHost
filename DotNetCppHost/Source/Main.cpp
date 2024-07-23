#include <iostream>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <Windows.h>
#include <cassert>

#define STR(s) L ## s
using string_t = std::basic_string<char_t>;

static hostfxr_initialize_for_runtime_config_fn s_InitFunctionPointer;
static hostfxr_get_runtime_delegate_fn s_GetDelegateFunctionPointer;
static hostfxr_close_fn s_CloseFunctionPointer;

// https://github.com/dotnet/samples/blob/main/core/hosting/src/NativeHost/nativehost.cpp#L272C5-L277C6
// Sample on GitHub also has examples for other platforms
static void* LoadLib(const char_t* path)
{
	HMODULE h = ::LoadLibraryW(path);
	assert(h != nullptr);
	return (void*)h;
}

static void* GetExport(void* h, const char* name)
{
	void* f = ::GetProcAddress((HMODULE)h, name);
	assert(f != nullptr);
	return f;
}

static bool LoadHostFxr()
{
	// Find the HostFxr path
	char_t buffer[MAX_PATH];
	size_t bufferSize = sizeof(buffer) / sizeof(char_t);
	// NOTE: If we mannualy search for the path, we don't need to link nethost
	int result = get_hostfxr_path(buffer, &bufferSize, nullptr);
	if (result != 0)
		return false;

	// Load HostFxr and get exports
	void* lib = LoadLib(buffer);
	s_InitFunctionPointer = (hostfxr_initialize_for_runtime_config_fn)GetExport(lib, "hostfxr_initialize_for_runtime_config");
	s_GetDelegateFunctionPointer = (hostfxr_get_runtime_delegate_fn)GetExport(lib, "hostfxr_get_runtime_delegate");
	s_CloseFunctionPointer = (hostfxr_close_fn)GetExport(lib, "hostfxr_close");

	return s_InitFunctionPointer && s_GetDelegateFunctionPointer && s_CloseFunctionPointer;
}

static load_assembly_and_get_function_pointer_fn GetDotNetLoadAssembly(const char_t* configPath)
{
	// Load .NET Core
	hostfxr_handle context = nullptr;
	int result = s_InitFunctionPointer(configPath, nullptr, &context);
	if (result != 0 || context == nullptr)
	{
		std::cerr << "Failed to initialize .NET Core: " << std::hex << std::showbase << result << "\n";
		return nullptr;
	}

	// Get the load assembly function pointer
	void* loadAssemblyAndGetFunctionPointer = nullptr;
	result = s_GetDelegateFunctionPointer(context, hdt_load_assembly_and_get_function_pointer, &loadAssemblyAndGetFunctionPointer);
	if (result != 0 || loadAssemblyAndGetFunctionPointer == nullptr)
	{
		std::cerr << "Get delegate failed: " << std::hex << std::showbase << result << "\n";
	}

	s_CloseFunctionPointer(context);
	return (load_assembly_and_get_function_pointer_fn)loadAssemblyAndGetFunctionPointer;
}

int main()
{
	std::cout << "Loading HostFxr...\n";
	if (!LoadHostFxr())
	{
		std::cerr << "Failed to load HostFxr\n";
		return 1;
	}

	std::cout << "Initializing .NET Core...\n";
	const char_t* configPath = STR("D:\\dev\\learning\\DotNetCppHost\\DotNetLib\\bin\\Release\\net8.0\\DotNetLib.runtimeconfig.json");
	load_assembly_and_get_function_pointer_fn loadAssemblyAndGetFunctionPointer = nullptr;
	loadAssemblyAndGetFunctionPointer = GetDotNetLoadAssembly(configPath);
	if (loadAssemblyAndGetFunctionPointer == nullptr)
	{
		std::cerr << "Failed to initialize .NET Core\n";
		return 1;
	}

	std::cout << "Loading .NET assembly...\n";
	const char_t* dotNetLibPath = STR("D:\\dev\\learning\\DotNetCppHost\\DotNetLib\\bin\\Release\\net8.0\\DotNetLib.dll");
	const char_t* dotNetType = STR("DotNetLib.DotNetLibrary, DotNetLib");
	const char_t* dotNetTypeMethod = STR("EntryPoint");
	component_entry_point_fn entryPointFunction = nullptr;
	int result = loadAssemblyAndGetFunctionPointer(dotNetLibPath, dotNetType, dotNetTypeMethod, nullptr /*default delegate can be nullptr*/, nullptr, (void**)&entryPointFunction);
	if (result != 0 || entryPointFunction == nullptr)
	{
		std::cerr << "Failed to load .NET assembly: " << std::hex << std::showbase << result << "\n";;
		return 1;
	}

	std::cout << "Running managed code!!\n";
	struct LibArgs
	{
		const char_t* Message;
		int Number;
	};

	LibArgs args{};
	args.Message = STR("Hello from C++! The better side!");
	args.Number = 69;
	entryPointFunction(&args, sizeof(args));

	std::cout << "Done!\n.\n.\n.\nDid it work?\n";
	return 0;
}
