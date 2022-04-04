// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
//#include "implot.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <string>

#include "ImGuiApp.h"

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

int main(int, char**);

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void getRandomArray(float* data, size_t array_size, int a, int b);
float* hist(float* data, size_t data_size);
float D(float* data, size_t data_size);
// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Monte-Carlo method"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Monte-Carlo method"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
    //io.ConfigViewportsNoDefaultParent = true;
    //io.ConfigDockingAlwaysTabBar = true;
    //io.ConfigDockingTransparentPayload = true;
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // VARIABLES
    bool showParamWindow = true;
    bool showPlotWindow = false;
    bool showCalcWindow = false;
    bool isArrayGenerated = false;
    float* data;
    float* result_array = new float[10];
    int array_size = 0;
    float disp = 0;
    float rms = 0;
    const char* generateAlg[] = {"Standart C++ function", "Wichman-Hill method"};
    const char* shapes[] = { "Pyramid", "Cone" };
    int currentMethod = 0;
    int currentFirstShape = 0;
    int currentSecondShape = 0;

    float* shapeX;
    float* shapeY;
    float* shapeZ;

    for (size_t i = 0; i < 10; i++)
    {
        result_array[i] = 0;
    }
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        IG::RenderDockingWindow();

        if (showParamWindow)
        {
            static int leftRandBorder = 0;
            static int rightRandBorder = 0;
            static int tempSize = 0;
            ImGui::Begin("Params");
            
            if (ImGui::Button("Show plot window"))
            {
                showPlotWindow = true;
                showCalcWindow = false;
                isArrayGenerated = true;
                data = new float[1];
                data[0] = 0;
            }
            if (ImGui::Button("Show calculation window"))
            {
                showCalcWindow = true;
                showPlotWindow = false;
            }
            ImGui::Text("Random generators");
            ImGui::ListBox("##", &currentMethod, generateAlg, IM_ARRAYSIZE(generateAlg), 2);

            ImGui::Text("Enter parameters");
            if (currentMethod == 0)
            {
                ImGui::InputInt("Min random: ", &leftRandBorder);
                ImGui::InputInt("Max random: ", &rightRandBorder);
            }
            
            ImGui::InputInt("Array size: ", &tempSize);
            ImGui::Text(("Dispersion: " + std::to_string(disp)).c_str());
            ImGui::Text(("Standart deviation: " + std::to_string(rms)).c_str());


            if (ImGui::Button("Generate"))
            {
                if (tempSize > 0 && showPlotWindow)
                {
                    array_size = tempSize;
                    delete[] data;
                    data = new float[array_size];
                    getRandomArray(data, array_size, leftRandBorder, rightRandBorder);
                    delete[] result_array;
                    result_array = hist(data, array_size);
                    disp = D(data, array_size);
                    rms = sqrt(disp);
                }
                
                
            }
            ImGui::End();
        }

        //ImGui::ShowDemoWindow(&show_demo_window);
     

        if (showPlotWindow)
        {
            static float posSize = 1.0f;
            static float negSize = 0.0f;
            ImGui::Begin("Plots");

            ImGui::PlotHistogram("Plot", result_array, 10, 0, 0, negSize, posSize, ImVec2(600.0f, 600.0f), 4);
            
            ImGui::InputFloat("Positive axes Y: ", &posSize);
            ImGui::InputFloat("Negative axes Y: ", &negSize);
            
            ImGui::End();
        }

        if (showCalcWindow)
        {
            static float h1 = 0.0f;
            static float r1 = 0.0f;
            static float h2 = 0.0f;
            static float r2 = 0.0f;
            static float V = 0.0f;
            ImGui::Begin("Calculation window");
            ImGui::Text("Select the first shape");
            ImGui::ListBox("##", &currentFirstShape, shapes, IM_ARRAYSIZE(shapes), 2);
            ImGui::InputFloat("Enter height(h)", &h1);
            ImGui::InputFloat("Enter radius(r)", &r1);
            ImGui::Text("Select the second shape");
            ImGui::InputFloat("Enter height(h)", &h2);
            ImGui::InputFloat("Enter radius(r)", &r2);
            ImGui::ListBox("###", &currentSecondShape, shapes, IM_ARRAYSIZE(shapes), 2);

            if (ImGui::Button("Calculate"))
            {
                //fillSpace(shapeX, shapeY, shapeZ, 1000, 100);
            }

            ImGui::End();
        }



        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    // Dealloc
    if (showPlotWindow)
        delete[] data;

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


#include <ctime>
// FUNCTIONS

struct Range {
    int x1;
    int x2;
};

int getRandom(int a, int b)
{
    return rand() % (b - a - 1) + a;
}

float WichmanHill()
{
    static uint16_t x1 = 3172;
    static uint16_t x2 = 9814;
    static uint16_t x3 = 20125;
    float y1, y2, y3;
    int keeploop = 1;

    
        y1 = (x1 * 171) % 30269;
        y2 = (x2 * 172) % 30307;
        y3 = (x3 * 170) % 30323;

        x1 = y1;
        x2 = y2;
        x3 = y3;
    
    float r = (y1 * 1.0) / 30269 + (y2 * 1.0) / 30307 + (y3 * 1.0) / 30323;
    return abs(r - round(r));
}

void getRandomArray(float* data, size_t array_size, int a, int b)
{
    srand(time(0));

    for (size_t i = 0; i < array_size; i++)
    {
        //data[i] = rand() % (b - a - 1) + a;
        data[i] = round(WichmanHill()*100);
        //data[i] = generate_rand_norm_value(0.5, sqrt(1/(float)12));
        //std::cout << data[i] << std::endl;
    }
}

float* hist(float* data, size_t data_size)
{
    int max = data[0];
    int min = data[0];
    for (size_t i = 0; i < data_size; i++)
    {
        if (data[i] > max)
            max = data[i];
        if (data[i] < min)
            min = data[i];
    }
    int current = min;
    Range H[10];
    float* result_array = new float[10]{ 0 };
    for (size_t i = 0; i < 10; i++)
    {
        H[i].x1 = min;
        H[i].x2 = min + (max / 10);
        min += max / 10;

        for (size_t j = 0; j < data_size; j++)
        {
            if (data[j] >= H[i].x1 && data[j] < H[i].x2)
            {
                result_array[i]++;
            }
        }
        //std::cout << H[i].x1 << " - " << H[i].x2 << "  =  " << result_array[i] << std::endl;
        result_array[i] /= data_size;
    }

    return result_array;
}

float D(float* data, size_t data_size)
{
    float D1 = 0;
    float D2 = 0;

    for (size_t i = 0; i < data_size; i++)
    {
        D1 += data[i] * data[i];
        D2 += data[i];
    }
    D1 /= data_size;
    D2 /= data_size;
    D2 *= D2;

    return D1 - D2;
}


void fillSpace(float*& shapeX, float*& shapeY, float*& shapeZ, int N, int dotRange)
{
    shapeX = new float[N];
    shapeY = new float[N];
    shapeZ = new float[N];

    for (size_t i = 0; i < N; i++)
    {
        shapeX[i] = getRandom(0, dotRange);
        shapeY[i] = getRandom(0, dotRange);
        shapeZ[i] = getRandom(0, dotRange);
    }
    
}
