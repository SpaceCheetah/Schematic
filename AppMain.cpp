#include "AppMain.h"
#include "FrameMain.h"
#include "Resources.h"
#include "wx/cmdline.h"
#include <wx/image.h>

wxIMPLEMENT_APP(AppMain);

namespace {
    void initWindowsStuff();
}

bool AppMain::OnInit() {
    wxInitAllImageHandlers();
    initWindowsStuff();
    wxCmdLineParser parser{argc, argv};
    parser.AddParam("File", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
    parser.Parse();
    FrameMain* frame; //Note: wxWidgets takes care of deleting classes descending from wxWindow and wxSizer.
    if(parser.GetParamCount() == 1) {
        frame = new FrameMain(parser.GetParam(0).ToStdWstring());
    } else {
        frame = new FrameMain();
    }
    frame->Show(true);
    return true;
}

namespace {
#ifdef _WIN32
#include <ShlObj_core.h>
    struct KeyWrapper {
        HKEY key;
        ~KeyWrapper() {
            RegCloseKey(key);
        }
    };
    void initWindowsStuff() {
        //Currently, just registers the .schematic file type and points it to the current executable location. Yes, that requires an absurd amount of code.
        wchar_t buffer[1024];
        if(GetModuleFileNameW(nullptr, buffer, 1024) < 1024) {
            std::filesystem::path path{std::wstring{buffer}};
            std::wstring pathStr = path.generic_wstring();
            //Despite almost everything in windows allowing both / and \, this particular case ONLY allows \. Took me way too long to figure out.
            std::replace(pathStr.begin(), pathStr.end(), L'/', L'\\');
            std::filesystem::current_path(path.parent_path());
            std::filesystem::path name = path.filename();
            std::wstring pathWithArg = std::wstring{L"\""} + pathStr + L"\" \"%1\"";
            std::filesystem::path icoPath{path.parent_path() / "res/resistor-multires.ico"};
            std::wstring icoPathStr = icoPath.generic_wstring();
            std::replace(icoPathStr.begin(), icoPathStr.end(), L'/', L'\\');
            KeyWrapper extensionKey{};
            if(RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\.schematic", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &extensionKey.key, nullptr) == 0) {
                RegSetValueExW(extensionKey.key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(L"Schematic.App"), 28);
            }
            KeyWrapper progIDKey{};
            if(RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\Schematic.App", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &progIDKey.key, nullptr) == 0) {
                RegSetValueExW(progIDKey.key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(L"Schematic"), 20);
                KeyWrapper commandKey{};
                if(RegCreateKeyExW(progIDKey.key, L"shell\\open\\command", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &commandKey.key, nullptr) == 0) {
                    RegSetValueExW(commandKey.key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(pathWithArg.c_str()), pathWithArg.size() * 2 + 2);
                }
                KeyWrapper defaultIconKey{};
                if(RegCreateKeyExW(progIDKey.key, L"DefaultIcon", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &defaultIconKey.key, nullptr) == 0) {
                    RegSetValueExW(defaultIconKey.key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(icoPathStr.c_str()), icoPathStr.size() * 2 + 2);
                }
            }
            KeyWrapper pathKey{};
            std::wstring subkey = std::wstring{L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\"} + name.generic_wstring();
            if(RegCreateKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &pathKey.key, nullptr) == 0) {
                RegSetValueExW(pathKey.key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(pathStr.c_str()), pathStr.size() * 2 + 2);
            }
            subkey = std::wstring{L"Software\\Classes\\Applications\\"} + name.generic_wstring() + L"\\shell\\open\\command";
            KeyWrapper commandKey{};
            if(RegCreateKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &commandKey.key, nullptr) == 0) {
                RegSetValueExW(commandKey.key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(pathWithArg.c_str()), pathWithArg.size() * 2 + 2);
            }
            subkey = std::wstring{L"Software\\Classes\\Applications\\"} + name.generic_wstring() + L"\\SupportedTypes\\.schematic";
            KeyWrapper supportedTypeKey{};
            RegCreateKeyExW(HKEY_CURRENT_USER,subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &supportedTypeKey.key, nullptr);
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_DWORD, nullptr, nullptr);
        }
    }
#else
    void initWindowsStuff() {}
#endif
}