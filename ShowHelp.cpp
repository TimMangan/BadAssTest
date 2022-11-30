// Copyright 2022 TMurgent Technologies, LLP
// Free for any use.

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "psf_stuff.h"


void ShowHelp()
{
    std::wcout << L"BadAssTest is configured using a file named 'BadAssTest.txt' located in the same folder as the exe.\n";
    std::wcout << L"The file should consist of one or more lines of text.\n";
    std::wcout << L"Each line identifies a path to a file, followed by a comma and then a list of comma separated test numbers that\n";
    std::wcout << L"should be performed.\n";
    std::wcout << L"No quotation marks, escapes, or unnecessary spaces should be used.\n";
    std::wcout << L"The supported test numbers are:\n";

    std::wcout << L"\t1 - std::filesystem::exists(...)\n";
    std::wcout << L"\t2 - GetFileAttributes(...)\n";
    std::wcout << L"\t3 - CreateFile(..., GENERIC_READ, OPEN_EXISTING, ...)\n";
    std::wcout << L"\t4 - CreateFile(..., GENERIC_WRITE, OPENEXISTING, ...)\n";
    std::wcout << L"\t5 - DeleteFile(...)\n\n";
    std::wcout << L"\t6 - FindFirstFile(path to a directory)\n\n";

    std::wcout << L"An example file:\n";
    std::wcout << L"\tC:\\Program Files\\BadAssTest\\Subfolder\\TestFileA.ini,1,2,3\n";
    std::wcout << L"\tC:\\Users\\Tim\\AppData\\Local\\BadAssTest\\Subfolder\\TestFileB.cfg,1,3\n";
    std::wcout << L"\t<Root>\\VFS\\ProgramFilesX64\\BadAssTest\\Subfolder\\TestFileA.cfg,1,2,3,4,5\n";
    std::wcout << L"\t<WritablePackageRoot>\\VFS\\Local AppData\\BadAssTest\\Subfolder\\TestFileB.cfg,1,3,4\n";
    std::wcout << L"\t<WritablePackageRoot>\\VFS\\Local AppData\\BadAssTest\\Subfolder,6\n";
}