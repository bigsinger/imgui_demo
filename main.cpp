#include <atlstr.h>
#include <windows.h>
#include <commctrl.rh>
#include <string>
#include <Dwmapi.h> 
#include <dxgi.h>
#include <d3d11.h>
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx11.h"
using namespace std;

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")


#define ID_HOTKEY_F1	WM_USER+1000
#define ID_HOTKEY_F2	WM_USER+1002
#define ID_HOTKEY_F3	WM_USER+1003
#define ID_HOTKEY_F4	WM_USER+1004
#define ID_HOTKEY_F5	WM_USER+1005
#define ID_HOTKEY_F6	WM_USER+1006
#define ID_HOTKEY_F7	WM_USER+1007
#define ID_HOTKEY_F8	WM_USER+1008  
#define ID_HOTKEY_DOWN	WM_USER+1009 
#define ID_HOTKEY_UP	WM_USER+1010  
#define ID_HOTKEY_LEFT	WM_USER+1011 
#define ID_HOTKEY_RIGHT	WM_USER+1012
#define ID_HOTKEY_HOME  WM_USER+1100
#define ID_HOTKEY_DEL	WM_USER+1200


static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
ID3D11Texture2D					*m_depthStencilBuffer = NULL;	//深度/模板缓冲区
ID3D11RenderTargetView			*m_renderTargetView = NULL;		//渲染对象视图
ID3D11DepthStencilView			*m_depthStencilView = NULL;		//深度/模板视图


HWND g_hWndMain = NULL;
bool g_isShowWindow = true;
bool g_isFirstPageInited = false;
bool g_isSecondPageInited = false;

bool g_bChecked1 = false;
bool g_bChecked2 = false;
bool g_bChecked3 = false;
int g_nIndexRadioOption = false;

std::string strLabelText1, strLabelText2;
static char strEditText1[MAX_PATH] = { 0 };
static char strEditText2[MAX_PATH] = { 0 };


LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
CString GetWindowsPath();

// 初始化字体，默认字体不能正常显示中文
bool initFont();

void CreateRenderTarget();
void CleanupRenderTarget();
HRESULT CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();

void RegHotKey(HWND hWnd);
void UnHotKey(HWND hWnd);

void EnableClickable(HWND hWnd, bool isCanClick);

CString GetWindowsPath() {
	TCHAR str[MAX_PATH] = { 0 };
	int nLen = GetWindowsDirectory(str, MAX_PATH);
	if (nLen) {
		str[nLen] = '\\';
	}
	return str;
}

bool initFont() {
	bool bOK = false;
	CStringA strFontFilePath = GetWindowsPath() + _T("Fonts\\msyh.ttc");
	if (PathFileExistsA(strFontFilePath) == TRUE) {
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF(strFontFilePath, 12.0f, 0, io.Fonts->GetGlyphRangesChinese());
		io.FontAllowUserScaling = false;
		bOK = io.Fonts->Build();
	} else {
		bOK = false;
	}

	return bOK;
}

void CreateRenderTarget() {
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();

	g_pd3dDeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	D3D11_VIEWPORT viewPort;
	viewPort.Width = static_cast<FLOAT>(GetSystemMetrics(SM_CXSCREEN));
	viewPort.Height = static_cast<FLOAT>(GetSystemMetrics(SM_CYSCREEN));
	viewPort.MaxDepth = 1.f;
	viewPort.MinDepth = 0.f;
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	g_pd3dDeviceContext->RSSetViewports(1, &viewPort);
}

void CleanupRenderTarget() {
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

HRESULT CreateDeviceD3D(HWND hWnd) {
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	//sd.BufferDesc.Width = 800;
	//sd.BufferDesc.Height = 900;
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
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, sizeof(featureLevelArray) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return E_FAIL;
	CreateRenderTarget();
	return S_OK;
}

void CleanupDeviceD3D() {
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void RegHotKey(HWND hWnd) {
	if (!RegisterHotKey(hWnd, ID_HOTKEY_HOME, NULL, VK_HOME)) {
		::MessageBoxA(NULL, "热键注册失败", "ERROR", IDOK);
	}
	if (!RegisterHotKey(hWnd, ID_HOTKEY_F1, NULL, VK_F1)) {
		::MessageBoxA(NULL, "热键注册失败", "ERROR", IDOK);
	}
	if (!RegisterHotKey(hWnd, ID_HOTKEY_DEL, NULL, VK_DELETE)) {
		::MessageBoxA(NULL, "热键注册失败", "ERROR", IDOK);
	}

}

void UnHotKey(HWND hWnd) {
	UnregisterHotKey(hWnd, ID_HOTKEY_HOME);
	UnregisterHotKey(hWnd, ID_HOTKEY_F1);
	UnregisterHotKey(hWnd, ID_HOTKEY_DEL);
}

void EnableClickable(HWND hWnd, bool isCanClick) {
	LONG lStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	if (isCanClick) {
		lStyle &= ~WS_EX_LAYERED;
		SetWindowLong(hWnd, GWL_EXSTYLE, lStyle);
		//SetForegroundWindow(hWnd);
	} else {
		lStyle |= WS_EX_LAYERED;
		SetWindowLong(hWnd, GWL_EXSTYLE, lStyle);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
	switch (message) {
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX11_InvalidateDeviceObjects();
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
			ImGui_ImplDX11_CreateDeviceObjects();
		}
		return 0;
	case WM_HOTKEY:
	{
		if (wParam == ID_HOTKEY_HOME) {
			long style = GetWindowLong(hWnd, GWL_EXSTYLE);
			if (g_isShowWindow) {
				SetWindowLong(hWnd, GWL_EXSTYLE, style |= WS_EX_LAYERED);  // 增加WS_EX_LAYERED
				g_isShowWindow = false;
			} else {
				SetWindowLong(hWnd, GWL_EXSTYLE, style &= ~WS_EX_LAYERED); //去掉 WS_EX_LAYERED
				g_isShowWindow = true;
			}
		}
		if (wParam == ID_HOTKEY_DEL) {
			PostQuitMessage(WM_QUIT);
			::PostMessage(hWnd, WM_QUIT, 0, 0);
			::SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		if (g_isShowWindow) {
			if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
				long dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
				if ((dwExStyle & WS_EX_LAYERED) != WS_EX_LAYERED)
					SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle |= WS_EX_LAYERED);  // 增加WS_EX_LAYERED
				g_isShowWindow = false;
			}
		}
		break;
	}
	case WM_RBUTTONDOWN:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void ShowPageFirst() {
	if (g_isFirstPageInited == false) {
		g_isFirstPageInited = true;
		ImGui::SetNextTreeNodeOpen(true);
		ImGui::SetNextWindowPos(ImVec2(0, 0)); //设置位置
	}
	ImGui::Begin(u8"页面1", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowSize(ImVec2(210, 155));

	if (ImGui::TreeNode(u8"设置1")) {
		ImGui::Text(u8"选项配置:");
		ImGui::SameLine();
		ImGui::InputText("##edittext1", strEditText1, IM_ARRAYSIZE(strEditText1), ImGuiInputTextFlags_az_09);
		ImGui::Text(u8"选项配置:");
		ImGui::SameLine();
		ImGui::InputText("##edittext2", strEditText2, IM_ARRAYSIZE(strEditText1), ImGuiInputTextFlags_az_09);
		if (!strLabelText1.empty())
			ImGui::TextColored(ImColor(11, 34, 244), strLabelText1.c_str());
		ImGui::Dummy(ImVec2(185, 20));
		ImGui::SameLine();
		if (ImGui::Button(u8"按钮1", ImVec2(60, 20))) {
			strLabelText1 = u8"看这里";
		}
		ImGui::TreePop();
	}

	ImGui::Separator();
	if (ImGui::TreeNode(u8"设置2")) {
		char strText1[MAX_PATH] = { 0 };
		char strText2[MAX_PATH] = { 0 };

		ImGui::Text(u8"选项配置:");
		ImGui::SameLine();
		ImGui::InputText("##edittext3", strText1, IM_ARRAYSIZE(strText1), ImGuiInputTextFlags_az_09);
		ImGui::Text(u8"选项配置:");
		ImGui::SameLine();
		ImGui::InputText("##edittext4", strText2, sizeof(strText2) / sizeof(*strText2), ImGuiInputTextFlags_az_09);
		ImGui::TextColored(ImColor(255, 0, 0), u8"友情提示信息...");
		if (!strLabelText2.empty())
			ImGui::TextColored(ImColor(0, 128, 0), strLabelText2.c_str());
		ImGui::Dummy(ImVec2(185, 20));
		ImGui::SameLine();
		if (ImGui::Button(u8"按钮2", ImVec2(60, 20))) {
			strLabelText2 = u8"看这里";
		}
		ImGui::TreePop();
	}

	ImGui::Separator();
	if (ImGui::TreeNode(u8"设置3")) {
		char strText1[MAX_PATH] = { 0 };
		char strText2[MAX_PATH] = { 0 };
		ImGui::Text(u8"选项配置:");
		ImGui::SameLine();
		ImGui::InputText("##rcg_strUser", strText1, IM_ARRAYSIZE(strText2), ImGuiInputTextFlags_az_09);
		ImGui::Text(u8"选项配置:");
		ImGui::SameLine();
		ImGui::InputText("##rcg_strCard", strText2, IM_ARRAYSIZE(strText2), ImGuiInputTextFlags_az_09);

		ImGui::Dummy(ImVec2(185, 20));
		ImGui::SameLine();
		if (ImGui::Button(u8"退出", ImVec2(60, 20))) {
			PostQuitMessage(WM_QUIT);
			::PostMessage(g_hWndMain, WM_QUIT, 0, 0);
			::SendMessage(g_hWndMain, WM_CLOSE, 0, 0);
		}
		ImGui::TreePop();
	}

	ImGui::End();
}

void ShowPageSecond() {
	if (g_isSecondPageInited) {
		ImGui::SetNextTreeNodeOpen(true);
		g_isSecondPageInited = false;
	}
	ImGui::Begin(u8"Home隐藏/显示 Del退出", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 255, 255));
	if (ImGui::TreeNode(u8"设置1")) {
		ImGui::Checkbox(u8"复选1", &g_bChecked1);
		ImGui::SameLine();
		ImGui::Checkbox(u8"复选2", &g_bChecked2);
		ImGui::SameLine();
		ImGui::Checkbox(u8"复选3", &g_bChecked3);
		ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
	if (ImGui::TreeNode(u8"设置2")) {
		ImGui::RadioButton(u8"单选1", &g_nIndexRadioOption, 0); ImGui::SameLine();
		ImGui::RadioButton(u8"单选2", &g_nIndexRadioOption, 1);
		ImGui::TreePop();
	}
	ImGui::PopStyleColor();

	ImGui::TextColored(ImColor(128, 0, 0), u8"颜色文本");
	ImGui::TextColored(ImColor(0, 128, 0), u8"颜色文本");
	ImGui::TextColored(ImColor(0, 0, 128), u8"颜色文本");

	ImGui::End();
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int       nCmdShow) {
	CString strWindowName = "imgui_demo";

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), ACS_TRANSPARENT, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL,strWindowName, NULL };
	RegisterClassEx(&wc);
	HWND hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW /*|WS_EX_NOACTIVATE*/, strWindowName, strWindowName, WS_POPUP, 0, 0, 400, 300, NULL, NULL, wc.hInstance, NULL);
	g_hWndMain = hWnd;


	// 设置透明
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	if (CreateDeviceD3D(hWnd) < 0) {
		CleanupDeviceD3D();
		UnregisterClassA(strWindowName, hInstance);
		return 0;
	}

	RegHotKey(hWnd);	//注册按键

	UpdateWindow(hWnd);
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	ImGui::CreateContext();
	ImGui_ImplDX11_Init(hWnd, g_pd3dDevice, g_pd3dDeviceContext);
	//EnableClickable(hWnd, false); //是否可点击

	if (initFont() == false) {
		MessageBox(NULL, "字体初始化失败!", NULL, IDOK);
	}

	ImGui::StyleColorsLight();
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.FrameRounding = 12;
	style.FrameBorderSize = 1;
	style.FramePadding = ImVec2(4, 2);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		ImGui_ImplDX11_NewFrame();
		if (g_isShowWindow) {
			ShowPageFirst();
			ShowPageSecond();
		}

		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		ImVec4 clear_color = ImVec4{ 0.0f,0.0f,0.0f,0.0f };
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, reinterpret_cast<const float*>(&clear_color));

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // 开启垂直同步 
		//g_pSwapChain->Present(0, 0); // 关闭垂直同步
	}

	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
	CleanupDeviceD3D();

	UnHotKey(hWnd);
	UnregisterClassA(strWindowName, hInstance);

	return 0;
}