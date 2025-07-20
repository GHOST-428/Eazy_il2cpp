#include "includes.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
int TabId = 1;
bool menu;

//esp
bool esp_line;
bool esp_box;

//fov
bool isfov;
float fov = 60.0;

//crosshair
ImColor Crosshair = ImColor(255.0f / 255, 255.0f / 255, 255.0f / 255);
float size = 10.0f;      // Äëèíà ëèíèé ïðèöåëà
float thickness = 2.0f;   // Òîëùèíà ëèíèé
bool isdraw;

//objects
std::vector<Unity::CGameObject*> listGame(NULL);
Unity::CGameObject* Selected;
float posX;
float posY;
float posZ;

//fps
Unity::CGameObject* fps;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

bool WorldToScreen(Unity::Vector3 world, Vector2& screen)
{
	const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	Unity::CCamera* CameraMain = Unity::Camera::GetMain(); // Get The Main Camera
	if (!CameraMain) {
		return false;
	}

	Unity::Vector3 buffer = CameraMain->CallMethodSafe<Unity::Vector3>("WorldToScreenPoint", world, 2); // Call the worldtoscren function using monoeye (2)

	if (buffer.x > screenSize.x || buffer.y > screenSize.y || buffer.x < 0 || buffer.y < 0 || buffer.z < 0) // check if point is on screen
	{
		return false;
	}

	if (buffer.z > 0.0f) // Check if point is in view
	{
		screen = Vector2(buffer.x, screenSize.y - buffer.y);
	}

	if (screen.x > 0 || screen.y > 0) // Check if point is in view
	{
		return true;
	}
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui::StyleNex();
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto isFrames = ImGui::GetFrameCount();
	static float isRed = 0.0f, isGreen = 0.01f, isBlue = 0.0f;
	if (isFrames % 1 == 0)
	{
		if (isGreen == 0.01f && isBlue == 0.0f)
		{
			isRed += 0.01f;

		}
		if (isRed > 0.99f && isBlue == 0.0f)
		{
			isRed = 1.0f;

			isGreen += 0.01f;

		}
		if (isGreen > 0.99f && isBlue == 0.0f)
		{
			isGreen = 1.0f;

			isRed -= 0.01f;

		}
		if (isRed < 0.01f && isGreen == 1.0f)
		{
			isRed = 0.0f;

			isBlue += 0.01f;

		}
		if (isBlue > 0.99f && isRed == 0.0f)
		{
			isBlue = 1.0f;

			isGreen -= 0.01f;

		}
		if (isGreen < 0.01f && isBlue == 1.0f)
		{
			isGreen = 0.0f;

			isRed += 0.01f;

		}
		if (isRed > 0.99f && isGreen == 0.0f)
		{
			isRed = 1.0f;

			isBlue -= 0.01f;

		}
		if (isBlue < 0.01f && isGreen == 0.0f)
		{
			isBlue = 0.0f;

			isRed -= 0.01f;

			if (isRed < 0.01f)
				isGreen = 0.01f;

		}
	}
	auto Rainbow = ImVec4(isRed, isGreen, isBlue, 1.0f);

	ImGui::Begin("NeoExploit | Unity Il2cpp Hack", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse);
	if (ImGui::Button("Visual", ImVec2(125, 40))) {
		TabId = 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Object Manager", ImVec2(125, 40))) {
		auto list = Unity::Object::FindObjectsOfType<Unity::CComponent>("UnityEngine.Transform");
		for (int i = 0; i < list->m_uMaxLength; i++) {
			listGame.push_back(list->operator[](i)->GetGameObject());
		}

		TabId = 2;
	}

	ImGui::SameLine();
	if (ImGui::Button("?", ImVec2(125, 40))) {
		TabId = 3;
	}

	if (TabId == 1) {
		ImGui::Text("ESP");
		ImGui::BeginChild("ESP", ImVec2(255, 35), true, ImGuiWindowFlags_NoScrollbar);
		ImGui::Checkbox("Line", &esp_line);
		ImGui::EndChild();

		ImGui::Text("FOV");
		ImGui::BeginChild("FOV", ImVec2(255, 60), true, ImGuiWindowFlags_NoScrollbar);
		ImGui::SliderFloat("", &fov, 30, 180);
		ImGui::Checkbox("Apply", &isfov);
		ImGui::EndChild();

		ImGui::Text("CROSSHAIR");
		ImGui::BeginChild("CROSSHAIR", ImVec2(255, 105), true, ImGuiWindowFlags_NoScrollbar);
		ImGui::ColorEdit3("##CrosshairColor", (float*)&Crosshair);
		ImGui::SliderFloat("Size", &size, 1, 30);
		ImGui::SliderFloat("Thickness", &thickness, 1, 30);
		ImGui::Checkbox("Draw", &isdraw);
		ImGui::EndChild();
	}
	if (TabId == 2) {
		ImGui::Text("OBJECTS");
		ImGui::BeginChild("OBJECTS", ImVec2(255, 105), true);

		for (int i = 0; i < listGame.size(); i++) {
			if (ImGui::Button(listGame[i]->GetName()->ToString().c_str(), ImVec2(245, 30))) {
				Selected = listGame[i];

				auto OwnPos = Selected->GetTransform()->GetPosition();

				posX = OwnPos.x;
				posY = OwnPos.y;
				posZ = OwnPos.z;
			}
		}
		ImGui::EndChild();

		ImGui::InputFloat("X", &posX);
		ImGui::InputFloat("Y", &posY);
		ImGui::InputFloat("Z", &posZ);

		if (ImGui::Button("Apply", ImVec2(250, 40))) {
			Selected->GetTransform()->SetPosition(Unity::Vector3(posX, posY, posZ));
		}
	}

	if (TabId == 3) {
		// Нуу...
	}
	ImGui::End();

	if (esp_line) {
		auto players = Unity::GameObject::FindWithTag("Player");

		for (int i = 0; i < players->m_uMaxLength; i++) {
			Vector2 pos;
			auto playerPos = players->operator[](i)->GetTransform()->GetPosition();
			const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
			const ImVec2 screenCenter(screenSize.x / 2.0f, screenSize.y);

			// Ðèñóåì ëèíèþ
			if (WorldToScreen(playerPos, pos)) {
				ImGui::GetBackgroundDrawList()->AddLine(screenCenter, ImVec2(pos.x, pos.y), ImColor(Rainbow.x, Rainbow.y, Rainbow.z), 1.5f);
			}
		}
	}

	if (isfov) {
		auto LocalCamera = Unity::Camera::GetMain();
		LocalCamera->SetFieldOfView(fov);
	}

	if (isdraw) {
		float x = ImGui::GetIO().DisplaySize.x;
		float y = ImGui::GetIO().DisplaySize.y;
		auto screenPosition = ImVec2(x / 2, y / 2);

		ImGui::GetBackgroundDrawList()->AddLine(
			ImVec2(screenPosition.x - size, screenPosition.y),
			ImVec2(screenPosition.x + size, screenPosition.y),
			Crosshair, thickness
		);

		// Ðèñóåì âåðòèêàëüíóþ ëèíèþ
		ImGui::GetBackgroundDrawList()->AddLine(
			ImVec2(screenPosition.x, screenPosition.y - size),
			ImVec2(screenPosition.x, screenPosition.y + size),
			Crosshair, thickness
		);
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		IL2CPP::Initialize();
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
