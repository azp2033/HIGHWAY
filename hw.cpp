#include <Windows.h>
#include <iostream>
#include "MinHook.h"
#include <vector>
#include <map>
#include "kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "MinHook.lib")
#include "xorstr.h"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

void InitImGui()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\RobotoMono-VariableFont_wght.ttf", 12, NULL, io.Fonts->GetGlyphRangesCyrillic());
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    ImGuiStyle* style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;
    style->FrameBorderSize = 0.f;
    style->ChildBorderSize = 0.f;
    style->WindowBorderSize = 0.f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    //style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    //style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
    //style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
    //style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;

bool ConsolePrint(const char* fmt, ...)
{
    if (!_out)
        return false;

    char buf[1024];
    va_list va;

    va_start(va, fmt);
    _vsnprintf_s(buf, 1024, fmt, va);
    va_end(va);

    return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
}

void AttachConsole()
{
    _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
    _old_err = GetStdHandle(STD_ERROR_HANDLE);
    _old_in = GetStdHandle(STD_INPUT_HANDLE);

    ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

    _out = GetStdHandle(STD_OUTPUT_HANDLE);
    _err = GetStdHandle(STD_ERROR_HANDLE);
    _in = GetStdHandle(STD_INPUT_HANDLE);

    SetConsoleMode(_out, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

    SetConsoleMode(_in, ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
}

void DetachConsole()
{
    if (_out && _err && _in) {
        FreeConsole();

        if (_old_out)
            SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
        if (_old_err)
            SetStdHandle(STD_ERROR_HANDLE, _old_err);
        if (_old_in)
            SetStdHandle(STD_INPUT_HANDLE, _old_in);
    }
}

template<typename T>
class cDetour
{
public:
    explicit cDetour<T>(T target, T detour) :m_target(target), m_detour(detour)
    {
        MH_CreateHook(m_target, m_detour, reinterpret_cast<void**>(&m_trampoline));
    }
    ~cDetour()
    {
        MH_DisableHook(m_target);
    }
    T GetTrampoline() const
    {
        return static_cast<T>(m_trampoline);
    }
    bool IsApplied() const
    {
        return m_isEnabled;
    }
    void Apply()
    {
        if (!m_isEnabled)
        {
            m_isEnabled = MH_EnableHook(m_target) == MH_OK;
            if (m_isEnabled)
                memcpy(m_hookBuffer, m_target, sizeof(m_hookBuffer));
        }
    }
    void Remove()
    {
        m_isEnabled = !(m_isEnabled && MH_DisableHook(m_target) == MH_OK);
    }
    void EnsureApply()
    {
        if (memcmp(m_hookBuffer, m_target, sizeof(m_hookBuffer)) != 0)
        {
            DWORD oldProtect;
            VirtualProtect(m_target, sizeof(m_hookBuffer), PAGE_READWRITE, &oldProtect);
            memcpy(m_target, m_hookBuffer, sizeof(m_hookBuffer));
            VirtualProtect(m_target, sizeof(T), oldProtect, &oldProtect);
        }
    }
private:
    T m_trampoline;
    T m_target;
    T m_detour;
    bool m_isEnabled = false;
    char m_hookBuffer[20];

};

class cContext
{
public:
    static cContext& GetInstance();
    template<typename T> cDetour<T>* CreateDetour(T target, T detour)
    {
        auto pDetour = new cDetour<T>(target, detour);
        return pDetour;
    }
    template<typename T> bool ApplyDetour(T target, T detour, cDetour<T>** ppDetour)
    {
        auto pDetour = CreateDetour(target, detour);
        if (pDetour)
        {
            *ppDetour = pDetour;
            pDetour->Apply();
            return true;
        }
        return false;
    }


    void CloseExit()
    {
        if (!(MH_Uninitialize() == MH_OK))
            TerminateProcess(GetCurrentProcess(), -1);
    }
    cContext() {}
    ~cContext() {}

};

bool bInitialized = false;
cContext& cContext::GetInstance()
{
    if (!bInitialized)
        bInitialized = MH_Initialize() == MH_OK;
    static cContext pCtx;
    return pCtx;
}

typedef FARPROC(__stdcall* _GetProcAddress)(HMODULE, LPCSTR);
static _GetProcAddress XOR_GetProcAddress;

typedef uintptr_t(__stdcall* _func0001)(); // mono_get_root_domain
typedef void(__stdcall* _func0002)(uintptr_t); // mono_thread_attach
typedef uintptr_t(__stdcall* _func0003)(const char*); // mono_image_loaded
typedef uintptr_t(__stdcall* _func0004)(uintptr_t, const char*, const char*); // mono_class_from_name
typedef uintptr_t(__stdcall* _func0005)(uintptr_t, const char*, int); // mono_class_get_method_from_name
typedef uintptr_t(__stdcall* _func0006)(uintptr_t); // mono_compile_method
typedef uintptr_t(__stdcall* _func0007)(uintptr_t, void*, void**, uintptr_t); // mono_runtime_invoke
typedef uintptr_t(__stdcall* _func0008)(uintptr_t, const char*); // mono_class_get_property_from_name 
typedef uintptr_t(__stdcall* _func0009)(uintptr_t, void*, void**, uintptr_t); // mono_property_get_value 
typedef uintptr_t(__stdcall* _func0010)(uintptr_t, const char*); // mono_string_new 
typedef uintptr_t(__stdcall* _func0011)(uintptr_t); //mono_class_get_type 
typedef uintptr_t(__stdcall* _func0012)(uintptr_t, uintptr_t); //mono_class_vtable 
typedef void (__stdcall* _func0013)(uintptr_t, uintptr_t, void*); //mono_field_static_get_value 
typedef uintptr_t (__stdcall* _func0014)(uintptr_t, const char*); //mono_class_get_field_from_name 

static HANDLE hProcess;

template <class T>
static T read(unsigned long long address) {
    T buffer;
    ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}

template <class T>
static void write(unsigned long long address, T value) {
    WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), NULL);
}

inline static _func0001 mono_get_root_domain;
inline static _func0002 mono_thread_attach;
inline static _func0003 mono_image_loaded;
inline static _func0004 mono_class_from_name;
inline static _func0005 mono_class_get_method_from_name;
inline static _func0006 mono_compile_method;
inline static _func0007 mono_runtime_invoke;
inline static _func0008 mono_class_get_property_from_name;
inline static _func0009 mono_property_get_value;
inline static _func0010 mono_string_new;
inline static _func0011 mono_class_get_type;
inline static _func0012 mono_class_vtable;
inline static _func0013 mono_field_static_get_value;
inline static _func0014 mono_class_get_field_from_name;


uintptr_t GetImage(const char* name)
{
    return mono_image_loaded(name);
}

uintptr_t GetClass(uintptr_t image, const char* _namespace, const char* _class)
{
    return mono_class_from_name(image, _namespace, _class);
}

uintptr_t GetClassMethod(uintptr_t class_, const char* name, int params)
{
    return mono_class_get_method_from_name(class_, name, params);
}

uintptr_t GetClassMethod(uintptr_t image, const char* namespace_, const char* class_, const char* name, int params)
{
    uintptr_t hClass = GetClass(image, namespace_, class_);
    if (hClass == NULL) return NULL;
    return GetClassMethod(hClass, name, params);
}

uintptr_t GetClassMethod(const char* namespace_, const char* class_, const char* name, int params, const char* image_ = "Assembly-CSharp")
{
    uintptr_t hClass = GetClass(GetImage(image_), namespace_, class_);
    if (hClass == NULL) return NULL;
    return GetClassMethod(hClass, name, params);
}

uintptr_t GetProperty(uintptr_t image, const char* namespace_, const char* class_, const char* name)
{
    return mono_class_get_property_from_name(GetClass(image, namespace_, class_), name);
}

uintptr_t GetPropertyValue(uintptr_t property)
{
    return mono_property_get_value(property, 0, 0, 0);
}

uintptr_t GetPropertyValue(uintptr_t property, uintptr_t instance)
{
    return mono_property_get_value(property, (void*)instance, 0, 0);
}

uintptr_t GetPropertyValue(uintptr_t image, const char* namespace_, const char* class_, const char* name)
{
    return GetPropertyValue(GetProperty(image, namespace_, class_, name));
}

uintptr_t GetPropertyValue(uintptr_t image, const char* namespace_, const char* class_, const char* name, uintptr_t instance)
{
    return GetPropertyValue(GetProperty(image, namespace_, class_, name), instance);
}

uintptr_t InvokeMethod(const char* namespace_, const char* class_, const char* method, int paramsCount, void* instance, const char* image_ = "Assembly-CSharp", void** params = NULL)
{
    return mono_runtime_invoke(GetClassMethod(GetImage(image_), namespace_, class_, method, paramsCount), instance, params, NULL);
}

uintptr_t InvokeMethod(uintptr_t method, void* instance, void** params, uintptr_t exc)
{
    return mono_runtime_invoke(method, instance, params, exc);
}

uintptr_t InvokeMethod(uintptr_t method, void* instance, void** params)
{
    return mono_runtime_invoke(method, instance, params, NULL);
}

uintptr_t InvokeMethod(uintptr_t method, void* instance)
{
    return mono_runtime_invoke(method, instance, NULL, NULL);
}

uintptr_t InvokeMethod(uintptr_t method)
{
    return mono_runtime_invoke(method, NULL, NULL, NULL);
}

template <class T>
T InvokeMethod(uintptr_t method)
{
    return (T)mono_runtime_invoke(method, NULL, NULL, NULL);
}

uintptr_t CompileMethod(uintptr_t method)
{
    return mono_compile_method(method);
}

uintptr_t CompileMethod(const char* namespace_, const char* class_, const char* name, int params, const char* image_ = "Assembly-CSharp")
{
    return CompileMethod(GetClassMethod(GetImage(image_), namespace_, class_, name, params));
}

struct Vector3 {
public:
    float x, y, z;

    Vector3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vector3(float x, float y)
    {
        this->x = x;
        this->y = y;
        this->z = 0.f;
    }

    Vector3()
    {
        this->x = 0.f;
        this->y = 0.f;
        this->z = 0.f;
    }
};

struct Color {
public:
    float r, g, b, a;

    Color(float r, float g, float b, float a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    Color(float r, float g, float b) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = 1.f;
    }

    Color()
    {
        this->r = 1.f;
        this->g = 1.f;
        this->b = 1.f;
        this->a = 1.f;
    }
};

typedef uintptr_t(__stdcall* Camera_GetMainCamera_)();
Camera_GetMainCamera_ Camera_GetMainCamera;
uintptr_t Camera_WorldToScreenPoint;

typedef uintptr_t(__stdcall* GetLocalPlayer_)();
GetLocalPlayer_ GetLocalPlayer;

uintptr_t GetType;
uintptr_t GetComponent;
uintptr_t GetHeldEntity;

uintptr_t GameObject_GetTransform;
uintptr_t Transform_GetPosition;

Vector3 WorldToScreenPosition(float x, float y, float z)
{
    uintptr_t mainCamera = Camera_GetMainCamera();
    if (mainCamera != NULL)
    {
        void* args[1];
        args[0] = new Vector3(x, y, z);
        uintptr_t result = InvokeMethod(Camera_WorldToScreenPoint, (void*)mainCamera, args);
        return Vector3(read<float>(result + 0x10), read<float>(result + 0x14), read<float>(result + 0x18));
    }
    return Vector3();
}

Vector3 GetObjectPosition(uintptr_t object)
{
    if (object == NULL) return Vector3();
    uintptr_t transform = InvokeMethod(GameObject_GetTransform, (void*)object);
    if (transform == NULL) return Vector3();
    uintptr_t position = InvokeMethod(Transform_GetPosition, (void*)transform);
    if (position == NULL) return Vector3();
    return Vector3(read<float>(position + 0x10), read<float>(position + 0x14), read<float>(position + 0x18));
}

typedef uintptr_t(__stdcall* CreateProjectile_)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
cDetour<CreateProjectile_>* t_CreateProjectile;

typedef Vector3(__stdcall* GetModifiedAimConeDirection_)(float, Vector3, bool);
cDetour<GetModifiedAimConeDirection_>* t_GetModifiedAimConeDirection;

typedef void (__stdcall* BasePlayerLoad_)(uintptr_t, uintptr_t);
cDetour<BasePlayerLoad_>* t_BasePlayerLoad;

struct BaseRecoil {
public:
    float E28, E2C, E30, E34, E38, E3C, E40;
};

std::map<UINT, BaseRecoil> ItemsRecoil;

bool MagicBullet = false;
float BulletThickness = 0.f;
float NoRecoilPercentage = 0.f;
float NoSpreadPercentage = 0.f;
bool AdminHack = false;

uintptr_t OnCreateProjectile(uintptr_t instance, uintptr_t prefabPath, uintptr_t pos, uintptr_t forward, uintptr_t velocity)
{
    uintptr_t localPlayer = GetLocalPlayer();
    if (localPlayer != 0) {
        UINT itemID = read<UINT>(localPlayer + 0x5B0); // this.clActiveItem (BaesPlayer)
        uintptr_t recoil = read<uintptr_t>(instance + 0x248); // this.recoil (BaseProjectile)
        if (recoil != 0) {
            if (!(ItemsRecoil.count(itemID))) {
                BaseRecoil weaponRecoil;
                weaponRecoil.E28 = read<float>(recoil + 0x28); // recoilYawMin
                weaponRecoil.E2C = read<float>(recoil + 0x2C); // recoilYawMax
                weaponRecoil.E30 = read<float>(recoil + 0x30); // recoilPitchMin
                weaponRecoil.E34 = read<float>(recoil + 0x34); // recoilPitchMax
                weaponRecoil.E38 = read<float>(recoil + 0x38); // timeToTakeMin
                weaponRecoil.E3C = read<float>(recoil + 0x3C); // timeToTakeMax
                weaponRecoil.E40 = read<float>(recoil + 0x40); // ADSScale
                ItemsRecoil.insert({ itemID, weaponRecoil });
            }
            write<float>(recoil + 0x28, (ItemsRecoil[itemID].E28 / 100) * NoRecoilPercentage); // recoilYawMin
            write<float>(recoil + 0x2c, (ItemsRecoil[itemID].E2C / 100) * NoRecoilPercentage); // recoilYawMax
            write<float>(recoil + 0x30, (ItemsRecoil[itemID].E30 / 100) * NoRecoilPercentage); // recoilPitchMin
            write<float>(recoil + 0x34, (ItemsRecoil[itemID].E34 / 100) * NoRecoilPercentage); // recoilPitchMax
            write<float>(recoil + 0x38, (ItemsRecoil[itemID].E38 / 100) * NoRecoilPercentage); // timeToTakeMin
            write<float>(recoil + 0x3c, (ItemsRecoil[itemID].E3C / 100) * NoRecoilPercentage); // timeToTakeMax
            write<float>(recoil + 0x40, (ItemsRecoil[itemID].E40 / 100) * NoRecoilPercentage); // ADSScale
        }
    }

    uintptr_t result = t_CreateProjectile->GetTrampoline()(instance, prefabPath, pos, forward, velocity); // return Projectile
    if (MagicBullet) write<float>(result + 0x9C, BulletThickness); // this.thickness (Projectile)
    return result;
}

Vector3 GetModifiedAimConeDirection(float aimCone, Vector3 inputVec, bool ai)
{
    aimCone = (aimCone / 100) * NoSpreadPercentage;
    return t_GetModifiedAimConeDirection->GetTrampoline()(aimCone, inputVec, ai);
}

enum PlayerFlags
{
    Unused1 = 1,
    Unused2 = 2,
    IsAdmin = 4,
    ReceivingSnapshot = 8,
    Sleeping = 16,
    Spectating = 32,
    Wounded = 64,
    IsDeveloper = 128,
    Connected = 256,
    VoiceMuted = 512,
    ThirdPersonViewmode = 1024,
    EyesViewmode = 2048,
    ChatMute = 4096,
    NoSprint = 8192,
    Aiming = 16384,
    DisplaySash = 32768,
    Relaxed = 65536,
    SafeZone = 131072,
    ServerFall = 262144,
    Workbench1 = 1048576,
    Workbench2 = 2097152,
    Workbench3 = 4194304
};

void OnBasePlayerLoad(uintptr_t instance, uintptr_t info)
{
    t_BasePlayerLoad->GetTrampoline()(instance, info); // Call (BasePlayer.??.Load(info))
    if (AdminHack) {
        UINT64 flags = read<UINT64>(instance + 0x530); // this.playerFlags as UINT64
        PlayerFlags playerFlags = static_cast<PlayerFlags>(flags);
        if ((playerFlags & PlayerFlags::IsAdmin) != PlayerFlags::IsAdmin) {
            flags += PlayerFlags::IsAdmin;
            write<UINT64>(instance + 0x530, flags); // this.playerFlags to new value
        }
    }
}

bool init = false;
bool openmenu = true;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, HMODULE hMod)
{
    if (!init)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
        {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            InitImGui();
            init = true;
        }

        else
            return oPresent(pSwapChain, SyncInterval, Flags);
    }

    auto flags = ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize;

    if (GetAsyncKeyState(VK_HOME) & 1) {
        openmenu = !openmenu;
    }

    //if (GetAsyncKeyState(VK_END)) {

    //    kiero::shutdown();
    //}

    if (openmenu) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(420, 600));
        ImGui::Begin(xorstr(u8"HWSOLUTION"), nullptr, flags);

        ImGui::Checkbox(xorstr("Magic Bullet"), &MagicBullet);
        ImGui::SliderFloat(xorstr("Bullet Thickness"), &BulletThickness, 0.f, 10000.f);
        ImGui::SliderFloat(xorstr("Recoil"), &NoRecoilPercentage, 0.f, 100.f);
        ImGui::SliderFloat(xorstr("Spread"), &NoSpreadPercentage, 0.f, 100.f);
        ImGui::Checkbox(xorstr("Admin Hack"), &AdminHack);
        ImGui::End();
        ImGui::Render();

        pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
    return oPresent(pSwapChain, SyncInterval, Flags);
}

void ForeachBaseplayer()
{
    uintptr_t baseplayer_getvpl = GetClassMethod(GetImage("Assembly-CSharp"), xorstr(""), xorstr("BasePlayer"), xorstr("get_VisiblePlayerList"), 0);
    while (true) {
        uintptr_t localplayer = GetLocalPlayer();
        uintptr_t vpl = InvokeMethod(baseplayer_getvpl);
        if (localplayer != NULL && vpl != NULL) {
            uintptr_t target = NULL;
            uintptr_t buffer = read<uintptr_t>(vpl + 0x10);
            uintptr_t count = read<int>(vpl + 0x18);

            for (int i = 0; i <= count; i++) {
                uintptr_t baseplayer_ptr = read<uintptr_t>(buffer + 0x10 * (i * 0x8));
                if (baseplayer_ptr == NULL || baseplayer_ptr == localplayer) continue;
                if (read<float>(baseplayer_ptr + 0x1D0) < 1.f) continue; // this.health < 1 (isDead check)

                Vector3 position = GetObjectPosition(baseplayer_ptr);

                ConsolePrint(xorstr("%.2f / %.2f / %.2f\n"), position.x, position.y, position.z);
            }
        }

        Sleep(5000);
    }
}

// HACK POWERED BY https://vk.com/imthebossman / https://github.com/azp2033
// Source created on 217 devblog "Cookie Rust"

void Entrypoint()
{
    // Uncomment if you want debug
    //AttachConsole();

    HMODULE hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
    while (hMono == NULL) {
        hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
        Sleep(500);
    }

    hProcess = GetCurrentProcess();

    XOR_GetProcAddress = (_GetProcAddress)GetProcAddress(GetModuleHandleA(xorstr("kernel32.dll")), xorstr("GetProcAddress"));

    mono_get_root_domain = (_func0001)XOR_GetProcAddress(hMono, xorstr("mono_get_root_domain"));
    mono_thread_attach = (_func0002)XOR_GetProcAddress(hMono, xorstr("mono_thread_attach"));
    mono_image_loaded = (_func0003)XOR_GetProcAddress(hMono, xorstr("mono_image_loaded"));
    mono_class_from_name = (_func0004)XOR_GetProcAddress(hMono, xorstr("mono_class_from_name"));
    mono_class_get_method_from_name = (_func0005)XOR_GetProcAddress(hMono, xorstr("mono_class_get_method_from_name"));
    mono_compile_method = (_func0006)XOR_GetProcAddress(hMono, xorstr("mono_compile_method"));
    mono_runtime_invoke = (_func0007)XOR_GetProcAddress(hMono, xorstr("mono_runtime_invoke"));
    mono_class_get_property_from_name = (_func0008)XOR_GetProcAddress(hMono, xorstr("mono_class_get_property_from_name"));
    mono_property_get_value = (_func0009)XOR_GetProcAddress(hMono, xorstr("mono_property_get_value"));
    mono_string_new = (_func0010)XOR_GetProcAddress(hMono, xorstr("mono_string_new"));
    mono_class_get_type = (_func0011)XOR_GetProcAddress(hMono, xorstr("mono_class_get_type"));
    mono_class_vtable = (_func0012)XOR_GetProcAddress(hMono, xorstr("mono_class_vtable"));
    mono_field_static_get_value = (_func0013)XOR_GetProcAddress(hMono, xorstr("mono_field_static_get_value"));
    mono_class_get_field_from_name = (_func0014)XOR_GetProcAddress(hMono, xorstr("mono_class_get_field_from_name"));

    uintptr_t root_domain = mono_get_root_domain();
    mono_thread_attach(root_domain);

    Camera_GetMainCamera = reinterpret_cast<Camera_GetMainCamera_>(CompileMethod(xorstr("UnityEngine"), xorstr("Camera"), xorstr("get_main"), 0, xorstr("UnityEngine.CoreModule")));
    Camera_WorldToScreenPoint = GetClassMethod(xorstr("UnityEngine"), xorstr("Camera"), xorstr("WorldToScreenPoint"), 1, xorstr("UnityEngine.CoreModule"));

    GameObject_GetTransform = GetClassMethod(xorstr("UnityEngine"), xorstr("Component"), xorstr("get_transform"), 0, xorstr("UnityEngine.CoreModule"));
    Transform_GetPosition = GetClassMethod(xorstr("UnityEngine"), xorstr("Transform"), xorstr("get_position"), 0, xorstr("UnityEngine.CoreModule"));

    GetLocalPlayer = reinterpret_cast<GetLocalPlayer_>(CompileMethod(xorstr(""), xorstr("LocalPlayer"), xorstr("get_Entity"), 0));

    auto ctx = cContext::GetInstance();
    ctx.ApplyDetour(
        reinterpret_cast<GetModifiedAimConeDirection_>(
            CompileMethod(xorstr(""), xorstr("AimConeUtil"), xorstr("GetModifiedAimConeDirection"), 3)
            ),
        reinterpret_cast<GetModifiedAimConeDirection_>(GetModifiedAimConeDirection),
        &t_GetModifiedAimConeDirection
    );
    ctx.ApplyDetour(
        reinterpret_cast<CreateProjectile_>(
            CompileMethod(xorstr(""), xorstr("BaseProjectile"), xorstr("CreateProjectile"), 4)
            ),
        reinterpret_cast<CreateProjectile_>(OnCreateProjectile),
        &t_CreateProjectile
    );
    ctx.ApplyDetour(
        reinterpret_cast<BasePlayerLoad_>(
            CompileMethod(xorstr(""), xorstr("BasePlayer"), xorstr("Load"), 1)
            ),
        reinterpret_cast<BasePlayerLoad_>(OnBasePlayerLoad),
        &t_BasePlayerLoad
    );
    bool init_hook = false;
    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            kiero::bind(8, (void**)&oPresent, hkPresent);
            init_hook = true;
        }
    } while (!init_hook);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if(ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Entrypoint, hModule, NULL, NULL);
    }
    return TRUE;
}
