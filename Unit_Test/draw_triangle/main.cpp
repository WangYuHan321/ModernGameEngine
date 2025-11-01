#include "ApplicationWindow.h"

#if defined(_WIN32)
// Windows entry point
ApplicationWin* app;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (app != NULL)
	{
		app->HandleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int)
{
	app = new ApplicationWin();
	app->InitVulkan();
	app->SetUpWindow(hInstance, WndProc);
	app->Prepare();
	app->RenderLoop();
	delete(app);
	return 0;
}

#elif defined(__ANDROID__)
#include <android_native_app_glue.h>
#include "Render/Vulkan/VulkanAndroid.h"

ApplicationWin* app;
void android_main(android_app* state)
{
    androidApp = state;
	Render::Vulkan::android::GetDeviceConfig();
	app = new ApplicationWin();
	state->userData = app;
    //state->onAppCmd = ???
    state->onInputEvent = ApplicationBase::HandleAppInput;
	app->RenderLoop();
	delete(app);
}

#elif (defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)) && defined(VK_EXAMPLE_XCODE_GENERATED)

ApplicationWin* app;
int main(const int argc, const char *argv[])
{
    @autoreleasepool //只释放 继承自NSObject的对象
    {
        app = new ApplicationWin();
        app->InitVulkan();
        app->SetUpWindow(nullptr);
        app->Prepare();
        app->RenderLoop();
        delete(app);
    }
    return 0;
}

#endif





