#pragma once
// Copyright 2022 TMurgent Technologies, LLP
// Free for any use.

#include <Windows.h>
#include <cwctype>
//#include <iostream>
#include <string>
//#include <winreg.h>
//#include <stdio.h>
//#include <appmodel.h>
#include <stdlib.h>
#include <filesystem>
#include <cassert>

#include <combaseapi.h>
#include <Shlobj.h>
#include <KnownFolders.h>

using GetCurrentPackagePath2 = LONG(__stdcall*)(unsigned int, UINT32*, PWSTR);
using GetCurrentPackageFamilyName = LONG(__stdcall*)( UINT32*, PWSTR);

extern std::string narrow(std::wstring_view str, UINT codePage = CP_UTF8);
extern bool Tester(std::wstring testLine);
extern void ShowHelp();

extern std::filesystem::path g_packageRoot;
extern std::filesystem::path g_packageWritableRoot;

static const unsigned int PACKAGE_PATH_TYPE_EFFECTIVE = 2;

inline std::wstring appmodel_string(LONG(__stdcall* AppModelFunc)(UINT32, UINT32*, wchar_t*))
{
	UINT32 length = MAX_PATH + 1;
	std::wstring result(length - 1, '\0');

	const auto err = AppModelFunc(PACKAGE_PATH_TYPE_EFFECTIVE, &length, result.data());
	if ((err != ERROR_SUCCESS) && (err != ERROR_INSUFFICIENT_BUFFER))
	{
		//throw_win32(err, "could not retrieve AppModel string");
		//throw std::system_error(err, std::system_category(), "could not retrieve AppModel string");
		result = L"";
		result.resize(0);
	}
	else
	{
		if (length > 0)
		{
			result.resize(length - 1);
		}
	}
	return result;
}

inline std::wstring appmodel_string(LONG(__stdcall* AppModelFunc)(UINT32*, wchar_t*))
{
	// NOTE: `length` includes the null character both as input and output, hence the +1/-1 everywhere
	UINT32 length = MAX_PATH + 1;
	std::wstring result(length - 1, '\0');

	const auto err = AppModelFunc(&length, result.data());
	if ((err != ERROR_SUCCESS) && (err != ERROR_INSUFFICIENT_BUFFER))
	{
		//throw_win32(err, "could not retrieve AppModel string");
		//throw std::system_error(err, std::system_category(), "could not retrieve AppModel string");
		result = L"";
		result.resize(0);
	}
	else
	{
		if (length > 0)
		{
			result.resize(length - 1);
		}
	}
	return result;
}

inline std::filesystem::path current_package_path()
{
	// Use GetCurrentPackagePath2 if avalible
	std::wstring kernelDll = L"kernel.appcore.dll";
	HMODULE appModelDll = LoadLibraryEx(kernelDll.c_str(), nullptr, 0);

	if (appModelDll == nullptr)
	{
		auto message = narrow(kernelDll.c_str());
		//throw_last_error(message.c_str());
		throw std::system_error(GetLastError(), std::system_category(), message);
	}

	auto getCurrentPackagePath2 = reinterpret_cast<GetCurrentPackagePath2>(GetProcAddress(appModelDll, "GetCurrentPackagePath2"));


	std::wstring result;
	if (getCurrentPackagePath2)
	{
		// If GetCurrentPackagePath 2 does exists
		result = appmodel_string(getCurrentPackagePath2);
		//result = L""
	}
	else
	{
		//result = appmodel_string(&::GetCurrentPackagePath);
		result = L""; 
	}
	result[0] = std::towupper(result[0]);

	return result;
}


inline std::wstring current_package_family_name()
{
	std::wstring kernelDll = L"kernel.appcore.dll";
	HMODULE appModelDll = LoadLibraryEx(kernelDll.c_str(), nullptr, 0);

	if (appModelDll == nullptr)
	{
		auto message = narrow(kernelDll.c_str());
		//throw_last_error(message.c_str());
		throw std::system_error(GetLastError(), std::system_category(), message);
	}

	auto getCurrentPackageFamilyName = reinterpret_cast<GetCurrentPackageFamilyName>(GetProcAddress(appModelDll, "GetCurrentPackageFamilyName"));

	std::wstring result;
	if (getCurrentPackageFamilyName)
	{
		// If GetCurrentPackagePath 2 does exists
		result = appmodel_string(getCurrentPackageFamilyName);
		//result = L"";
	}
	else
	{
		//result = appmodel_string(&::GetCurrentPackageFamilyName);
		result = L"";
	}
	result[0] = std::towupper(result[0]);

	return result;
}
struct cotaskmemfree_deleter
{
	void operator()(wchar_t* ptr)
	{
		if (ptr)
		{
			::CoTaskMemFree(ptr);
		}
	}
};

using unique_cotaskmem_string = std::unique_ptr<wchar_t, cotaskmemfree_deleter>;

inline std::filesystem::path known_folder(const GUID& id, DWORD flags = KF_FLAG_DEFAULT)
{
	PWSTR path;
	if (FAILED(::SHGetKnownFolderPath(id, flags, nullptr, &path)))
	{
		throw std::runtime_error("Failed to get known folder path");
	}
	unique_cotaskmem_string uniquePath(path);

	path[0] = std::towupper(path[0]);

	return path;
}
