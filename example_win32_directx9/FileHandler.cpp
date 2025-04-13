#include "FileHandler.h"


std::string ShowFilePicker(HWND ownerWindow = nullptr) {
    
    if (ownerWindow == nullptr) {
        std::cout << "hwnd is a Null ptr " << std::endl;
    }

    wchar_t filename[MAX_PATH] = L"";  // Wide char buffer
    filename[0] = L'\0';  // Ensure the buffer is cleared

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = ownerWindow;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.TXT\0\0";  // Double null-terminated
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        // Convert wide string to std::string
        char result[MAX_PATH];
        wcstombs(result, filename, MAX_PATH);
        return std::string(result);
    }
    else {
        DWORD err = CommDlgExtendedError();  // Get extended error code
        std::cout << "File dialog failed. Error code: " << err << std::endl;
        return "";  // Return empty string if file dialog failed
    }
}

