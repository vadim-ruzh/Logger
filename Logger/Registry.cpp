#include"Registry.h"
#include <codecvt>
#include "unordered_map"
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

namespace details
{
	registry::ResultCode SplitPathToRegistryKey(std::wstring_view pathToRegKey, HKEY& dstHandleKey, std::wstring& dstSubKey)
	{
		std::wstring_view handleKey = pathToRegKey;	//By default, take the whole expression as a handle key
		std::wstring_view subKey; // By default,no sub key

		const size_t delimPosition = pathToRegKey.find_first_of(L'\\');
		//If an expression contains the delimiter ...
		if (delimPosition != std::string::npos)
		{
			//...then the expression has two parts
				//else use the default values
			handleKey = pathToRegKey.substr(0, delimPosition);
			subKey = pathToRegKey.substr(delimPosition + 1, std::string::npos);
		}

		const std::unordered_map<std::wstring_view, HKEY> handleRegKeyMap
		{
			{ L"HKEY_USERS", HKEY_USERS},
			{ L"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT },
			{ L"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
			{ L"HKEY_CURRENT_USER", HKEY_CURRENT_USER },
			{ L"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE}
		};

		const auto requiredHandleKey = handleRegKeyMap.find(handleKey);
		//Checking that the value was found
		if (requiredHandleKey == handleRegKeyMap.end())
		{
			return registry::ResultCode::eIncorrectPathToKey;
		}

		dstHandleKey = requiredHandleKey->second;
		dstSubKey = subKey;

		return registry::ResultCode::sOk;
	}

	registry::ResultCode OpenRegKey(const HKEY& handleKey, std::wstring_view subKey,REGSAM access, HKEY& dstOpenRegKey)
	{
		HKEY openRegKey;

		const LSTATUS openStatus = RegOpenKeyExW(
			handleKey,
			subKey.data(),
			0, //reserved ,must be 0
			access,
			&openRegKey
		);
		if (openStatus != ERROR_SUCCESS)
		{
			return registry::ResultCode::eKeyOpeningError;
		}

		dstOpenRegKey = openRegKey;

		return registry::ResultCode::sOk;
	}

	registry::ResultCode CreateRegKey(HKEY handleKey, std::wstring_view subKey, REGSAM access, HKEY& dstOpenRegKey)
	{
		HKEY openRegKey;

		const LSTATUS createStatus = RegCreateKeyExW(
			handleKey,
			subKey.data(),
			0, //reserved ,must be 0
			nullptr, //This parameter may be ignored(The user-defined class type of this key)
			REG_OPTION_NON_VOLATILE, //use default value
			access,
			nullptr,//if NULL or nullptr uses the default value
			&openRegKey,
			nullptr //This parameter may be ignored(Unnecessary in this case official information)
		);

		if (createStatus != ERROR_SUCCESS)
		{
			return registry::ResultCode::eKeyCreationError;
		}

		dstOpenRegKey = openRegKey;

		return registry::ResultCode::sOk;
	}

	registry::ResultCode CreateIfKeyCantBeOpen(const HKEY& handleKey, std::wstring_view subKey, REGSAM access, HKEY& dstOpenRegKey)
	{
		HKEY openRegKey;

		const registry::ResultCode openKeyResultCode = details::OpenRegKey(handleKey, subKey, access, openRegKey);
		//if the key cannot be opened...
		if (openKeyResultCode != registry::ResultCode::sOk)
		{
			//..try to create it, if you have write access

			if (access != KEY_ALL_ACCESS 
			 && access != KEY_WRITE)
			{
				return registry::ResultCode::ePermissionError;
			}

			const registry::ResultCode createKeyResultCode = details::CreateRegKey(handleKey, subKey, access, openRegKey);
			if (createKeyResultCode != registry::ResultCode::sOk)
			{
				return createKeyResultCode;
			}
		}

		dstOpenRegKey = openRegKey;
		return registry::ResultCode::sOk;
	}

}

registry::Exception::Exception(std::string_view errorMessage, ResultCode errorCode)
	: runtime_error(errorMessage.data())
	, mErrorCode(errorCode)
{
	
}

registry::ResultCode registry::Exception::GetErrorCode() const
{
	return mErrorCode;
}



registry::RegistryEditor::RegistryEditor(std::wstring_view pathToRegKey, const bool& isReadOnly) noexcept(false)
	: mRegKey(std::make_optional<Key>(pathToRegKey, isReadOnly))
{
	
}

registry::RegistryEditor::~RegistryEditor() = default;

registry::ResultCode registry::RegistryEditor::ChangeKey(std::wstring_view pathToRegKey, const bool& isReadOnly) noexcept(false)
{
	try
	{
		mRegKey.emplace(pathToRegKey, isReadOnly);
	}
	catch (registry::Exception& err)
	{
		return err.GetErrorCode();
	}

	return ResultCode::sOk;
}


registry::ResultCode registry::RegistryEditor::GetDword(std::wstring_view name, DWORD& value) const
{
	DWORD buffer = 0;
	DWORD bufferSize = sizeof(buffer);

	const LSTATUS readDataStatus = RegGetValue(
		mRegKey->GetOpenedKey(),
		L"", //no subkey
		name.data(),
		RRF_RT_REG_DWORD, //required value type
		nullptr, //information about the type of data in the value(Not required)
		&buffer,
		&bufferSize
	);

	if (readDataStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	value = buffer;
	return ResultCode::sOk;
}

registry::ResultCode registry::RegistryEditor::GetQword(std::wstring_view name, ULONGLONG& value) const
{
	ULONGLONG buffer = 0;
	DWORD bufferSize = sizeof(buffer);

	const LSTATUS readDataStatus = RegGetValue(
		mRegKey->GetOpenedKey(),
		L"", //no subkey
		name.data(),
		RRF_RT_REG_QWORD, // restricts the type of the registry value
		nullptr, //information about the type of data in the value(Not required)
		&buffer,
		&bufferSize);

	if (readDataStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	value = buffer;
	return ResultCode::sOk;
}

registry::ResultCode registry::RegistryEditor::GetString(std::wstring_view name, std::wstring& value) const
{
	DWORD bufferSize = 0;

	const LSTATUS readDataSizeStatus = RegGetValue(
		mRegKey->GetOpenedKey(),
		L"", //no subkey
		name.data(),
		RRF_RT_REG_SZ, // restricts the type of the registry value
		nullptr, //information about the type of data in the value(Not required)
		nullptr, //no data 
		&bufferSize
	);

	if (readDataSizeStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	DWORD lenghtWcharString = bufferSize / sizeof(wchar_t);
	std::wstring buffer;
	buffer.resize(lenghtWcharString);

	const LSTATUS readDataStatus = RegGetValue(
		mRegKey->GetOpenedKey(),
		L"", //no subkey
		name.data(),
		RRF_RT_REG_SZ, // restricts the type of the registry value
		nullptr, //information about the type of data in the value(Not required)
		&buffer,
		&bufferSize
	);
	if (readDataStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	//Removing a duplicate null character
	buffer.resize(--lenghtWcharString);

	value = buffer;
	return ResultCode::sOk;
}

registry::ResultCode registry::RegistryEditor::GetExpandString(std::wstring_view name, std::wstring& value) const
{
	DWORD bufferSize = 0;

	const LSTATUS readDataSizeStatus = RegGetValue(
		mRegKey->GetOpenedKey(),
		L"", //no subkey
		name.data(),
		RRF_RT_REG_EXPAND_SZ, // restricts the type of the registry value 
		nullptr, //information about the type of data in the value(Not required)
		nullptr, //No data needed
		&bufferSize
	);

	if (readDataSizeStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	DWORD lenghtWcharString = bufferSize / sizeof(wchar_t);
	std::wstring buffer;
	buffer.resize(lenghtWcharString);

	const LSTATUS readDataStatus = RegGetValue(
		mRegKey->GetOpenedKey(),
		L"", //no subkey
		name.data(),
		RRF_RT_REG_EXPAND_SZ, //required value type
		nullptr, //information about the type of data in the value(Not required)
		&buffer,
		&bufferSize
	);
	if (readDataStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	//Removing a duplicate null character
	buffer.resize(--lenghtWcharString);

	value = buffer;
	return ResultCode::sOk;
}

registry::ResultCode registry::RegistryEditor::SetDword(std::wstring_view name, DWORD value) const
{
	if (mRegKey->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	constexpr DWORD dataSize = sizeof(value);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey->GetOpenedKey(),
		name.data(),
		0, //reserved,must be 0
		REG_DWORD, //data type
		reinterpret_cast<const BYTE*>(&value),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;
}

registry::ResultCode registry::RegistryEditor::SetQword(std::wstring_view name, ULONGLONG value) const
{
	if (mRegKey->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	constexpr  DWORD dataSize = sizeof(value);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey->GetOpenedKey(),
		name.data(),
		0, //reserved,must be 0
		REG_QWORD, //data type
		reinterpret_cast<const BYTE*>(&value),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;
}

registry::ResultCode registry::RegistryEditor::SetString(std::wstring_view name, std::wstring_view value) const
{
	if (mRegKey->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	const DWORD dataSize = value.size() * sizeof(wchar_t);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey->GetOpenedKey(),
		name.data(),
		0, //reserved,must be 0
		REG_SZ, //data type
		reinterpret_cast<const BYTE*>(&value),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;

}

registry::ResultCode registry::RegistryEditor::SetExpandString(std::wstring_view name, std::wstring_view value) const
{
	if (mRegKey->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	const DWORD dataSize = value.size() * sizeof(wchar_t);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey->GetOpenedKey(),
		name.data(),
		0, //reserved,must be 0
		REG_EXPAND_SZ, //data type
		reinterpret_cast<const BYTE*>(&value),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;

}

registry::Key::Key(std::wstring_view pathToRegistryKey, bool isReadOnly) noexcept(false)
	: mOpenRegistryKey(nullptr)
	, mAccessRight(KEY_READ)
{
	if(!isReadOnly)
	{
		mAccessRight = KEY_ALL_ACCESS;
	}

	HKEY handleKey;
	std::wstring subKey;

	registry::ResultCode result = details::SplitPathToRegistryKey(pathToRegistryKey, handleKey, subKey);
	if (result != registry::ResultCode::sOk)
	{
		throw registry::Exception("Wrong path to the key", result);
	}

	result = details::CreateIfKeyCantBeOpen(handleKey, subKey, mAccessRight, mOpenRegistryKey);
	if(result != ResultCode::sOk)
	{
		throw registry::Exception("Key initialization error", result);
	}
}

registry::Key::~Key()
{
	RegCloseKey(mOpenRegistryKey);
}

registry::ResultCode registry::Key::RenameSubKey(std::wstring_view subKey, std::wstring_view newSubKeyName) const
{
	if (IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	if (subKey.empty() || newSubKeyName.empty())
	{
		return ResultCode::eIncorrectPathToKey;
	}

	const LSTATUS renameSubKeyStatus = RegRenameKey(mOpenRegistryKey, subKey.data(), newSubKeyName.data());
	if(renameSubKeyStatus != ERROR_SUCCESS)
	{
		return ResultCode::eRenameValueError;
	}

	return ResultCode::sOk;
}

registry::ResultCode registry::Key::CreateSubKey(std::wstring_view subKey) const
{
	if (IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	if(subKey.empty())
	{
		return ResultCode::eIncorrectPathToKey;
	}

	HKEY openSubKey;
	return details::CreateRegKey(mOpenRegistryKey, subKey, mAccessRight, openSubKey);
}

registry::ResultCode registry::Key::DeleteSubKey(std::wstring_view subKey) const
{
	if (IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	if (subKey.empty())
	{
		return ResultCode::eIncorrectPathToKey;
	}

	const LSTATUS deleteSubKeyStatus = RegDeleteKeyExW(mOpenRegistryKey, subKey.data(), mAccessRight, 0);
	if (deleteSubKeyStatus != ERROR_SUCCESS)
	{
		return ResultCode::eDeleteError;
	}

	return ResultCode::sOk;
}

HKEY registry::Key::GetOpenedKey() const
{
	return mOpenRegistryKey;
}

bool registry::Key::IsReadOnly() const
{
	return mAccessRight != KEY_ALL_ACCESS;
}