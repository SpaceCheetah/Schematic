#include "AppMain.h"
#include "FrameMain.h"
#include "Resources.h"
#include "wx/cmdline.h"
#include <wx/image.h>

wxIMPLEMENT_APP(AppMain);

namespace {
    void initWindows();
    void cleanupWindows();
}

bool AppMain::OnInit() {
    wxInitAllImageHandlers();
    initWindows();
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

int AppMain::OnExit() {
    cleanupWindows();
    return 0;
}

namespace {
#ifdef _WIN32
#include <ShlObj_core.h>
    void initWindows() {
        wchar_t buffer[1024];
        if (GetModuleFileNameW(nullptr, buffer, 1024) == 1024) return;
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
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
        //Register app and file type
        RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Classes\\Schematic.app", nullptr, REG_SZ, reinterpret_cast<const BYTE *>(L"Schematic"), 20);
        RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Classes\\Schematic.app\\shell\\open\\command", nullptr, REG_SZ,reinterpret_cast<const BYTE *>(pathWithArg.c_str()), pathWithArg.size() * 2 + 2);
        RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Classes\\Schematic.app\\DefaultIcon", nullptr, REG_SZ,reinterpret_cast<const BYTE *>(icoPathStr.c_str()), icoPathStr.size() * 2 + 2);
        std::wstring subkey = std::wstring{L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\"} + name.generic_wstring();
        RegSetKeyValueW(HKEY_CURRENT_USER, subkey.c_str(), nullptr, REG_SZ, reinterpret_cast<const BYTE *>(pathStr.c_str()), pathStr.size() * 2 + 2);
        subkey = std::wstring{L"Software\\Classes\\Applications\\"} + name.generic_wstring() + L"\\shell\\open\\command";
        RegSetKeyValueW(HKEY_CURRENT_USER, subkey.c_str(), nullptr, REG_SZ, reinterpret_cast<const BYTE *>(pathWithArg.c_str()), pathWithArg.size() * 2 + 2);
        subkey = std::wstring{L"Software\\Classes\\Applications\\"} + name.generic_wstring() + L"\\SupportedTypes";
        RegSetKeyValueW(HKEY_CURRENT_USER, subkey.c_str(), L".schematic", REG_SZ, L"", 2);
        HKEY key{};
        if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\.schematic", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, nullptr) ==0) {
            DWORD bufferSize = 2048;
            LSTATUS status = RegQueryValueExW(key, nullptr, nullptr, nullptr, reinterpret_cast<BYTE *>(buffer), &bufferSize);
            if (bufferSize == 0 || status != 0) { //No default value set, probably unset file type
                RegSetValueExW(key, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(L"Schematic.App"), 28);
                SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_DWORD, nullptr, nullptr);
            } else {
                std::wstring keyValue{buffer, bufferSize / 2 - 1};
                if (keyValue != L"Schematic.App") { //Another app added itself
                    HKEY openWithKey;
                    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\.schematic\\OpenWithProgids", 0, nullptr, REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS, nullptr, &openWithKey, nullptr) == 0) {
                        if (RegQueryValueExW(openWithKey, L"Schematic.App", nullptr, nullptr, nullptr, nullptr) == ERROR_FILE_NOT_FOUND) {
                            RegSetValueExW(openWithKey, L"Schematic.App", 0, REG_SZ, reinterpret_cast<const BYTE *>(L""), 2);
                            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_DWORD, nullptr, nullptr);
                        }
                        RegCloseKey(openWithKey);
                    }
                }
            }
            RegCloseKey(key);
        }
        //Add to start menu
        wchar_t* programsPathStr;
        if(SHGetKnownFolderPath(FOLDERID_Programs, 0, nullptr, &programsPathStr) != 0) return;
        std::filesystem::path shortcutPath = std::filesystem::path{std::wstring{programsPathStr}} / "Schematic.lnk";
        std::wstring shortcutPathStr = shortcutPath.generic_wstring();
        std::replace(shortcutPathStr.begin(), shortcutPathStr.end(), L'/', L'\\');
        CoTaskMemFree(programsPathStr); //COM uses its own alloc/free
        if(exists(shortcutPath)) return; //If the shortcut already exists, no need to continue
        IShellLinkW* shortcut;
        if(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<LPVOID *>(&shortcut)) != 0) return;
        shortcut->SetDescription(L"Electrical schematic editor");
        shortcut->SetIconLocation(icoPathStr.c_str(), 0);
        shortcut->SetPath(pathStr.c_str());
        IPersistFile* shortcutFile;
        if(shortcut->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID *>(&shortcutFile)) == 0) {
            shortcutFile->Save(shortcutPathStr.c_str(), true);
            shortcutFile->Release();
        }
        shortcut->Release();
    }
    void cleanupWindows() {
        CoUninitialize();
    }
#else
    void initWindows() {}
    void cleanupWindows() {}
#endif
}