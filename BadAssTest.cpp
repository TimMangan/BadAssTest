// Copyright 2022 TMurgent Technologies, LLP
// Free for any use.
// 
// // BadAssTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "psf_stuff.h"

std::filesystem::path g_packageRoot;
std::filesystem::path g_packageWritableRoot;


std::wstring widen(std::string_view str, UINT codePage = CP_UTF8)
{
    std::wstring result;
    if (str.empty())
    {
        // MultiByteToWideChar fails when given a length of zero
        return result;
    }

    // UTF-16 should occupy at most as many characters as UTF-8
    result.resize(str.length());

    // NOTE: Since we call MultiByteToWideChar with a non-negative input string size, the resulting string is not null
    //       terminated, so we don't need to '+1' the size on input and '-1' the size on resize
    if (auto size = ::MultiByteToWideChar(
        codePage,
        MB_ERR_INVALID_CHARS,
        str.data(), static_cast<int>(str.length()),
        result.data(), static_cast<int>(result.length())))
    {
        assert(static_cast<std::size_t>(size) <= result.length());
        result.resize(size);
    }
    else
    {
        //throw_last_error();
        throw std::system_error(GetLastError(), std::system_category(), "Exception");
    }

    return result;
};

int main()
{
    std::cout << "BadAssTest!\n";
    std::cout << "===========\n";

    // Find path to our process exe
    HANDLE hProcess = GetCurrentProcess();
    std::wstring path;
    DWORD size = MAX_PATH;
    path.resize(size - 1);
    while (true)
    {
        if (::QueryFullProcessImageNameW(hProcess, 0, path.data(), &size))
        {
            path.resize(size);
            break;
        }
        else if (auto err = ::GetLastError(); err == ERROR_INSUFFICIENT_BUFFER)
        {
            size *= 2;
            path.resize(size - 1);
        }
        else
        {
            // Unexpected error
            return FALSE;
        }
    }
    CloseHandle(hProcess);

    g_packageRoot = current_package_path();
    g_packageWritableRoot = known_folder(FOLDERID_LocalAppData)  / std::filesystem::path(L"Packages") / current_package_family_name() / L"LocalCache\\Local\\Microsoft\\WritablePackageRoot";

    // Read in configuration file and process line by line
    std::wstring ConfigFile = path.substr(0, path.find_last_of(L'\\'));
    ConfigFile.append(L"\\BadAssTest.txt");

    std::fstream FileStream;

    FileStream.open(ConfigFile.c_str(), std::ios::in);
    if (FileStream.is_open())
    {
        std::string Line;
        while (std::getline(FileStream, Line))
        {
            std::wstring wLine = widen(Line);
            Tester(wLine);
        }
        FileStream.close();
    }
    else
    {
        ShowHelp();
    }

    std::wcout << L"================= End of all tests =================\n\n";

    Sleep(60 * 1000);
    return 0;
}

