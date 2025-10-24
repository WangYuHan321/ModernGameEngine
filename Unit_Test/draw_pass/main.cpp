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

#endif





