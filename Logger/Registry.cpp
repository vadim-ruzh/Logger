#pragma once
#include <exception>
#include <string>
#include <windows.h>

enum class Hkeys
{
	HkeyClassesRoot,
	HkeyCurrentUser,
	HkeyLocalMashine,
	HkeyUsers,
	HkeyCurrentConfig
};

enum class AccessRights
{
	AccessRead,
	AccessWrite,
	AccessAll
};

inline REGSAM AccessRightsConverter(const AccessRights& acc)
{
	switch (acc)
	{
	case AccessRights::AccessWrite:
	{
		return KEY_WRITE;
	}
	case AccessRights::AccessRead:
	{
		return KEY_READ;
	}
	case AccessRights::AccessAll:
	{
		return KEY_ALL_ACCESS;
	}
	}

	return 0;
}

inline HKEY HkeysConverter(const Hkeys& key)
{
	switch (key)
	{
	case Hkeys::HkeyClassesRoot:
	{
		return HKEY_CLASSES_ROOT;
	}
	case Hkeys::HkeyCurrentUser:
	{
		return HKEY_CURRENT_USER;
	}
	case Hkeys::HkeyLocalMashine:
	{
		return HKEY_LOCAL_MACHINE;
	}
	case Hkeys::HkeyUsers:
	{
		return HKEY_USERS;
	}
	case Hkeys::HkeyCurrentConfig:
	{
		return HKEY_CURRENT_CONFIG;
	}

	}

	return 0;
}

class RegException final : std::exception
{
public:
	RegException(LSTATUS errorCode, const std::string& message) : m_message(message.c_str()) {}
	RegException(LSTATUS errorCode, const char* message) : m_message(message) {}

	const char* what() const override
	{
		return  m_message;
	}

private:
	const char* m_message;
};

class WinRegKey
{
public:
	WinRegKey(const Hkeys& key, const std::wstring& subKey, const AccessRights& access)
	{
		OpenKey(key, subKey, access);
	}

	WinRegKey() = default;

	~WinRegKey()
	{
		CloseKey();
	}

	HKEY GetKey()
	{
		return mKey;
	}

	void CloseKey()
	{
		RegCloseKey(mKey);
		isOpen = false;
		mKey = nullptr;
	}

	void OpenKey(const Hkeys& key, const std::wstring& subkey, const AccessRights& access, LSTATUS& err)
	{
		const HKEY hkey = HkeysConverter(key);
		const REGSAM acc = AccessRightsConverter(access);

		auto tmpKey = OpenRegKey(hkey, subkey, acc, err);
		if (err != ERROR_SUCCESS)
		{
			tmpKey = CreateRegKey(hkey, subkey, acc, err);
			if (err != ERROR_SUCCESS)
			{
				mKey = nullptr;
				return;
			}
		}

		isOpen = true;
		mKey = tmpKey;
	}

	void OpenKey(const Hkeys& key, const std::wstring& subkey, const AccessRights& access)
	{
		LSTATUS status;
		OpenKey(key, subkey, access, status);
		if (status != ERROR_SUCCESS)
		{
			throw RegException{ status,"Fail open registry key" };
		}

		isOpen = true;
	}

protected:

	HKEY OpenRegKey(const HKEY key, const std::wstring& subKey, REGSAM access, LSTATUS& err)
	{
		HKEY resultHkey = nullptr;

		const LSTATUS status = RegOpenKeyExW(
			key,
			subKey.c_str(),
			0,
			access,
			&resultHkey
		);

		err = status;

		return resultHkey;
	}

	HKEY CreateRegKey(const HKEY key, const std::wstring& subKey, REGSAM access, LSTATUS& err)
	{
		HKEY resultHkey = nullptr;

		const LSTATUS status = RegCreateKeyExW(
			key,
			subKey.c_str(),
			0,
			nullptr,
			REG_OPTION_NON_VOLATILE,
			access,
			nullptr,
			&resultHkey,
			nullptr);

		err = status;

		return resultHkey;
	}

	HKEY mKey{ nullptr };

	bool isOpen{ false };
};

class WinRegManager : public WinRegKey
{
public:
	WinRegManager(const Hkeys& key, const std::wstring& subKey, const AccessRights& access) :WinRegKey(key, subKey, access) {}

	WinRegManager() : WinRegKey()
	{
	}

	//Reg_DWORD
		//noexcept
	DWORD GetDword(const std::wstring& subKey, const std::wstring& valueName, LSTATUS& err)
	{
		if (!isOpen)
		{
			err = ERROR_INVALID_HANDLE;
			return NULL;
		}

		DWORD buffer = 0;
		DWORD bufferSize = sizeof(buffer);

		const LSTATUS status = RegGetValueW(
			mKey,
			subKey.c_str(),
			valueName.c_str(),
			RRF_RT_REG_DWORD,
			nullptr,
			&buffer,
			&bufferSize);


		err = status;

		return buffer;
	}

	DWORD GetDword(const std::wstring& valueName, LSTATUS& err)
	{
		return GetDword(L"", valueName, err);
	}

	void SetDword(const std::wstring& subKey, const DWORD value, LSTATUS& err)
	{
		if (!isOpen)
		{
			err = ERROR_INVALID_HANDLE;
			return;
		}

		const DWORD valueSize = sizeof(value);
		const LSTATUS status = RegSetValueExW(
			mKey,
			subKey.c_str(),
			0,
			REG_DWORD,
			reinterpret_cast<LPCBYTE>(&value),
			valueSize
		);

		err = status;
	}

	//except
	DWORD GetDword(const std::wstring& subKey, const std::wstring& valueName)
	{
		LSTATUS status;

		const auto buffer = GetDword(subKey, valueName, status);
		if (status != ERROR_SUCCESS)
		{
			throw RegException{ status,"Failed read dword value" };
		}

		return buffer;
	}

	DWORD GetDword(const std::wstring& valueName)
	{
		return GetDword(L"", valueName);
	}

	void SetDword(const std::wstring& subKey, const DWORD value)
	{
		LSTATUS status;
		SetDword(subKey, value, status);
		if (status != ERROR_SUCCESS)
		{
			throw RegException{ status,"Failed set dword value" };
		}
	}

	//REG_SZ
		//noexcept
	std::wstring GetSz(const std::wstring& subKey, const std::wstring& valueName, LSTATUS& err)
	{
		if (!isOpen)
		{
			err = ERROR_INVALID_HANDLE;
			return nullptr;
		}

		DWORD dataSize = 0;
		LSTATUS status = RegGetValueW(
			mKey,
			subKey.c_str(),
			valueName.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&dataSize
		);

		if (status != ERROR_SUCCESS)
		{
			err = status;
			return nullptr;
		}

		std::wstring result(dataSize / sizeof(wchar_t), L' ');

		status = RegGetValueW(
			mKey,
			nullptr,
			valueName.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			&result[0],
			&dataSize
		);

		err = status;

		if (status != ERROR_SUCCESS)
		{
			return nullptr;
		}

		return result;
	}

	std::wstring GetSz(const std::wstring& valueName, LSTATUS& err)
	{
		return GetSz(L"", valueName, err);
	}

	void SetSz(const std::wstring& subKey, const std::wstring& value, LSTATUS& err)
	{
		if (!isOpen)
		{
			err = ERROR_INVALID_HANDLE;
			return;
		}

		const DWORD valueSize = value.size() * sizeof(wchar_t);

		const LSTATUS status = RegSetValueExW(
			mKey,
			subKey.c_str(),
			0,
			REG_SZ,
			reinterpret_cast<LPCBYTE>(&value),
			valueSize
		);

		err = status;
	}

	//except
	std::wstring GetSz(const std::wstring& subKey, const std::wstring& valueName)
	{
		LSTATUS status;

		auto buffer = GetSz(subKey, valueName, status);
		if (status != ERROR_SUCCESS)
		{
			throw RegException{ status,"Failed read sz value" };
		}

		return buffer;
	}

	std::wstring GetSz(const std::wstring& valueName)
	{
		return GetSz(L"", valueName);
	}

	void SetSz(const std::wstring& subKey, const std::wstring& value)
	{
		LSTATUS status;
		SetSz(subKey, value, status);
		if (status != ERROR_SUCCESS)
		{
			throw RegException{ status,"Failed set Sz value" };
		}
	}

};

#include <codecvt>
inline std::wstring utf8toUtf16(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.from_bytes(str);
}
