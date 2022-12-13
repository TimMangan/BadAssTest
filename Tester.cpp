// Copyright 2022 TMurgent Technologies, LLP
// Free for any use.

#define NUMTESTS 9

#include <Windows.h>
#include <iostream>
#include <string>
#include <winreg.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <cassert>

#include "psf_stuff.h"

std::string narrow(std::wstring_view str, UINT codePage)
{
    std::string result;
    if (str.empty())
    {
        // WideCharToMultiByte fails when given a length of zero
        return result;
    }

    // UTF-8 can occupy more characters than an equivalent UTF-16 string. WideCharToMultiByte gives us the required
    // size, so leverage it before we resize the buffer on the first pass of the loop
    for (int size = 0; ; )
    {
        // NOTE: Since we call WideCharToMultiByte with a non-negative input string size, the resulting string is not
        //       null terminated, so we don't need to '+1' the size on input and '-1' the size on resize
        size = ::WideCharToMultiByte(
            codePage,
            (codePage == CP_UTF8) ? WC_ERR_INVALID_CHARS : 0,
            str.data(), static_cast<int>(str.length()),
            result.data(), static_cast<int>(result.length()),
            nullptr, nullptr);

        if (size > 0)
        {
            auto finished = (static_cast<std::size_t>(size) <= result.length());
            assert(finished ^ (result.length() == 0));
            result.resize(size);

            if (finished)
            {
                break;
            }
        }
        else
        {
            //throw_last_error();
            throw std::system_error(GetLastError(), std::system_category(), "Exception");
        }
    }

    return result;
}



bool Tester(std::wstring testLine)
{
    bool Result = false;
    if (testLine.size() > 0)
    {
        std::wstring FileString;
        bool RunTests[NUMTESTS];
        for(int counter=0; counter<NUMTESTS; counter++)
        {
            RunTests[counter] = false;
        }

        size_t Split = testLine.find(',', 0);
        if (Split == std::wstring::npos)
        {
            FileString = testLine;
            Split = testLine.size() + 1;
        }
        else
        {
            FileString = testLine.substr(0, Split);
        }

        if (FileString.find(L"<Root>") != std::wstring::npos)
        {
            std::filesystem::path substitution = g_packageRoot;
            if (!substitution.empty())
            {
                FileString = (substitution.wstring()).append(FileString.substr(6).c_str());
            }
        }
        if (FileString.find(L"<WritablePackageRoot>") != std::wstring::npos)
        {
            std::filesystem::path substitution = g_packageWritableRoot;
            if (!substitution.empty())
            {
                FileString = (substitution.wstring()).append(FileString.substr(21).c_str());
            }
        }



        size_t LastSplit = Split+1;
        while (testLine.size() >= LastSplit)
        {
            size_t NextSplit = testLine.find(',', LastSplit);
            std::wstring NextTestString;
            if (NextSplit == std::wstring::npos)
            {
                NextTestString = testLine.substr(LastSplit);
                NextSplit = testLine.size() + 1;                
            }
            else
            {
                NextTestString = testLine.substr(LastSplit, NextSplit - LastSplit);
            }
            try
            {
                int NextTestNum = stoi(NextTestString);
                if (NextTestNum > 0 && NextTestNum <= NUMTESTS)
                {
                    RunTests[NextTestNum - 1] = true;
                }
            }
            catch (...)
            {
                ;
            }
            LastSplit = NextSplit + 1;
        }
        HKEY hKey;
        std::wstring SubKeyName = L"SOFTWARE\\BadAssTest";
        int res = RegOpenKeyExW(HKEY_CURRENT_USER, SubKeyName.c_str(), 0, KEY_WRITE, &hKey);
        if (ERROR_SUCCESS != res)
        {
            res = RegCreateKeyW(HKEY_CURRENT_USER, SubKeyName.c_str(), &hKey);
            if (res != ERROR_SUCCESS)
            {
                return false;
            }
        }
        RegSetValueExW(hKey, L"TestLine", 0, REG_SZ, (LPBYTE)testLine.c_str(), (DWORD)(testLine.size() + 1) * 2);
        RegSetValueExW(hKey, L"ParsedPath", 0, REG_SZ, (LPBYTE)FileString.c_str(), (DWORD)(FileString.size() + 1) * 2);
        std::wcout << L"Starting Test ";
        std::wcout << testLine.c_str();
        std::wcout << L"\n";
        std::wcout << L"Parsed Path ";
        std::wcout << FileString.c_str();
        std::wcout << L"\n";




        std::filesystem::path FilePath = FileString;
        bool Btresult;
        DWORD Dtresult;
        HANDLE Htresult;
        HMODULE HMtresult;
        std::wstring search;
        std::string contents = "New File Stuff.";

        for (int counter = 0; counter < NUMTESTS; counter++)
        {
            if (RunTests[counter])
            {
                wchar_t TestString[32];
                _itow_s((counter + 1), TestString, 32, 10);

                RegSetValueExW(hKey, L"TestCase", 0, REG_SZ, (LPBYTE)TestString, (DWORD)(wcslen(TestString) + 1) * 2);
                std::wcout << L"Starting Test ";
                std::wcout << TestString;
                std::wcout << L"\n\t";
                Sleep(1000);

                // Perform the test
                switch (counter + 1)
                {
                case 1:
                    Btresult = std::filesystem::exists(FilePath);
                    Sleep(200);
                    if (Btresult)
                    {
                        std::wcout << L"exists: SUCCESS\n";
                    }
                    else
                    {
                        std::wcout << L"exists: FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }
                    Dtresult = (DWORD)Btresult;
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*) &Dtresult, sizeof(DWORD));
                    break;
                case 2:
                    Dtresult = GetFileAttributesW(FileString.c_str());
                    Sleep(200);
                    if (Dtresult != INVALID_FILE_ATTRIBUTES)
                    {
                        std::wcout << L"GetFileAttributes: SUCCESS=0x";
                        _itow_s(Dtresult, TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";

                    }
                    else
                    {
                        std::wcout << L"GetFileAttributes: FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*) &Dtresult, sizeof(Dtresult));
                    break;
                case 3:
                    Htresult = CreateFileW(FileString.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                    Sleep(200);
                    if (Htresult != INVALID_HANDLE_VALUE)
                    {
                        std::wcout << L"CreateFile(Read): SUCCESS\n";
                        CloseHandle(Htresult);
                    }
                    else
                    {
                        std::wcout << L"CreateFile(Read): FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&Htresult, sizeof(Htresult));
                    break;
                case 4:
                    Htresult = CreateFileW(FileString.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                    Sleep(200);
                    if (Htresult != INVALID_HANDLE_VALUE)
                    {
                        std::wcout << L"CreateFile(Write): SUCCESS\n";
                        CloseHandle(Htresult);
                    }
                    else
                    {
                        std::wcout << L"CreateFile(Write): FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&Htresult, sizeof(Htresult));
                    break;
                case 5:
                    Btresult = DeleteFileW(FileString.c_str());
                    Sleep(200);
                    if (Btresult)
                    {
                        std::wcout << L"DeleteFile: SUCCESS\n";
                    }
                    else
                    {
                        std::wcout << L"DeleteFile: FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }

                    Dtresult = (DWORD)Btresult;
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&Dtresult, sizeof(DWORD));
                    break;
                case 6:
                    Dtresult = 0;
                    WIN32_FIND_DATAW data;
                    search = FileString;
                    search.append(L"\\*");
                    Htresult = FindFirstFileW(search.c_str(), &data);
                    if (Htresult != INVALID_HANDLE_VALUE)
                    {
                        bool more = true;
                        while (more)
                        {
                            std::wcout << "\tFound: ";
                            std::wcout << data.cFileName;
                            std::wcout << "\n";
                            more = FindNextFileW(Htresult, &data);
                        }
                        FindClose(Htresult);
                    }
                    else
                    {
                        Dtresult = GetLastError();
                        std::wcout << "\tFind Error\n";
                    }
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&Dtresult, sizeof(DWORD));
                    break;
                case 7:
                    Htresult = CreateFileW(FileString.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_NEW, 0, NULL);
                    Sleep(50);
                    if (Htresult != INVALID_HANDLE_VALUE)
                    {
                        std::wcout << L"CreateFile(Write): SUCCESS\n";
                        DWORD  rLen;
                        Btresult = WriteFile(Htresult, contents.c_str(), (DWORD)contents.size(), &rLen, NULL);
                        Sleep(50);
                        if (Btresult)
                        {
                            std::wcout << L"WriteFile(): SUCCESS\n";
                            CloseHandle(Htresult);
                        }
                        else
                        {
                            std::wcout << L"WriteFile(): FAIL=0x\n";
                            _itow_s(GetLastError(), TestString, 32, 16);
                            std::wcout << TestString;
                            std::wcout << L"\n";
                            CloseHandle(Htresult);
                            Htresult = 0; // indicate write error in TestResult
                        }
                    }
                    else
                    {
                        std::wcout << L"CreateFile(Write): FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&Htresult, sizeof(Htresult));
                    break;
                case 8:
                    Htresult = CreateFileW(FileString.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                    Sleep(200);
                    if (Htresult != INVALID_HANDLE_VALUE)
                    {
                        std::wcout << L"CreateFile(Read): SUCCESS\n";
                        char buff[1024];
                        DWORD dlen;
                        Btresult = ReadFile(Htresult, (LPVOID)buff, 1024, &dlen, NULL);
                        if (Btresult)
                        {
                            std::cout << "READ: ";
                            std::cout << buff;
                            std::cout << "\n";
                        }
                        else
                        {
                            std::wcout << L"ReadFile: Failure\n";
                        }
                        CloseHandle(Htresult);
                    }
                    else
                    {
                        std::wcout << L"CreateFile(Read): FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << TestString;
                        std::wcout << L"\n";
                    }
                    RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&Htresult, sizeof(Htresult));
                    break;
                case 9:
                    HMtresult = LoadLibraryExW(FileString.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
                    if (HMtresult != NULL)
                    {
                        std::wcout << L"LoadLibraryEx: SUCCESS\n";
                        RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&HMtresult, sizeof(HMtresult));
                        RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&HMtresult, sizeof(HMtresult));
                        FreeLibrary(HMtresult);
                    }
                    else
                    {
                        std::wcout << L"LoadLibraryEx: FAIL=0x";
                        _itow_s(GetLastError(), TestString, 32, 16);
                        std::wcout << L"\n";
                        RegSetValueExW(hKey, L"TestResult", 0, REG_DWORD, (BYTE*)&HMtresult, sizeof(HMtresult));
                    }
                    break;
                default:
                    break;
                }


                std::wcout << L"\n";
                Sleep(500);

            }
        }


        std::wstring DoneString = L"--------------------------Test is done.---------------------------";
        RegSetValueExW(hKey, L"TestCase", 0, REG_SZ, (LPBYTE)DoneString.c_str(), (DWORD)(DoneString.size() + 1) * 2);
        RegCloseKey(hKey);
        std::wcout << DoneString.c_str();
        std::wcout << L"\n\n";
        Sleep(1000);

    }
    return Result;
}
