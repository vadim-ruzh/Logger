#include"Registry.h"
#include <codecvt>
#include "unordered_map"
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

namespace details
{
	Reg::ResultCode SplitPathToReKey(const std::wstring& pathToRegKey, HKEY& dstHandleKey, std::wstring& dstSubKey)
	{
		std::wstring handleKey = pathToRegKey;	//By default, take the whole expression as a handle key
		std::wstring subKey; // By default,no sub key

		const size_t delimPosition = pathToRegKey.find_first_of(L'\\');
		//If an expression contains the delimiter ...
		if (delimPosition != std::string::npos)
		{
			//...then the expression has two parts
				//else use the default values
			handleKey = pathToRegKey.substr(0, delimPosition);
			subKey = pathToRegKey.substr(delimPosition + 1, std::string::npos);
		}

		const std::unordered_map<std::wstring, HKEY> handleRegKeyMap
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
			return Reg::ResultCode::eIncorrectPathToKey;
		}

		dstHandleKey = requiredHandleKey->second;
		dstSubKey = subKey;

		return Reg::ResultCode::sOk;
	}

	Reg::ResultCode OpenRegKey(const HKEY& handleKey, const std::wstring& subKey,REGSAM access, HKEY& dstOpenRegKey)
	{
		HKEY openRegKey;

		const LSTATUS openStatus = RegOpenKeyExW(
			handleKey,
			subKey.c_str(),
			0, //reserved ,must be 0
			access,
			&openRegKey
		);
		if (openStatus != ERROR_SUCCESS)
		{
			return Reg::ResultCode::eKeyOpeningError;
		}

		dstOpenRegKey = openRegKey;

		return Reg::ResultCode::sOk;
	}

	Reg::ResultCode CreateRegKey(const HKEY& handleKey, const std::wstring& subKey, REGSAM access, HKEY& dstOpenRegKey)
	{
		HKEY openRegKey;

		const LSTATUS createStatus = RegCreateKeyExW(
			handleKey,
			subKey.c_str(),
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
			return Reg::ResultCode::eKeyCreationError;
		}

		dstOpenRegKey = openRegKey;

		return Reg::ResultCode::sOk;
	}

	Reg::ResultCode CreateIfKeyCantBeOpen(const HKEY& handleKey, const std::wstring& subKey,REGSAM access, HKEY& dstOpenRegKey)
	{
		HKEY openRegKey;

		const Reg::ResultCode openKeyResultCode = details::OpenRegKey(handleKey, subKey, access, openRegKey);
		//if the key cannot be opened...
		if (openKeyResultCode != Reg::ResultCode::sOk)
		{
			//..try to create it, if you have write access

			if (access != KEY_ALL_ACCESS && access != KEY_WRITE)
			{
				return Reg::ResultCode::ePermissionError;
			}

			const Reg::ResultCode createKeyResultCode = details::CreateRegKey(handleKey, subKey, access, openRegKey);
			if (createKeyResultCode != Reg::ResultCode::sOk)
			{
				return createKeyResultCode;
			}
		}

		dstOpenRegKey = openRegKey;
		return Reg::ResultCode::sOk;
	}

}


Reg::Exception::Exception(const std::string& errorMessage, const ResultCode& errorCode) : runtime_error(errorMessage), mErrorCode_(errorCode)
{
	
}

Reg::ResultCode Reg::Exception::GetErrorCode() const
{
	return mErrorCode_;
}



Reg::Editor::Editor(const std::wstring& pathToRegKey,const bool& readOnly) noexcept(false)
{
	mRegKey_ = std::make_unique<Key>(pathToRegKey, readOnly);
}

Reg::Editor::~Editor()
{
	mRegKey_.reset();
}

/**
 * @remark If an exception is raised during the initialization of a new key, the old key will not be changed
 */
Reg::ResultCode Reg::Editor::ChangeKey(const std::wstring& pathToRegKey, const bool& readOnly) noexcept(false)
{
	try
	{
		mRegKey_ = std::make_unique<Key>(pathToRegKey,readOnly);
	}
	catch (Reg::Exception& err)
	{
		return err.GetErrorCode();
	}

	return ResultCode::sOk;
}


Reg::ResultCode Reg::Editor::GetDword(const std::wstring& valueName, DWORD& data) const
{
	DWORD buffer = 0;
	DWORD bufferSize = sizeof(buffer);

	const LSTATUS readDataStatus = RegGetValue(
		mRegKey_->GetOpenKey(),
		L"", //no subkey
		valueName.c_str(),
		RRF_RT_REG_DWORD, //required value type
		nullptr, //information about the type of data in the value(Not required)
		&buffer,
		&bufferSize
	);

	if (readDataStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	data = buffer;
	return ResultCode::sOk;
}

Reg::ResultCode Reg::Editor::GetQword(const std::wstring& valueName, ULONGLONG& data) const
{
	ULONGLONG buffer = 0;
	DWORD bufferSize = sizeof(buffer);

	const LSTATUS readDataStatus = RegGetValue(
		mRegKey_->GetOpenKey(),
		L"", //no subkey
		valueName.c_str(),
		RRF_RT_REG_QWORD, // restricts the type of the registry value
		nullptr, //information about the type of data in the value(Not required)
		&buffer,
		&bufferSize);

	if (readDataStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueReadingError;
	}

	data = buffer;
	return ResultCode::sOk;
}

Reg::ResultCode Reg::Editor::GetString(const std::wstring& valueName, std::wstring& data) const
{
	DWORD bufferSize = 0;

	const LSTATUS readDataSizeStatus = RegGetValue(
		mRegKey_->GetOpenKey(),
		L"", //no subkey
		valueName.c_str(),
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
		mRegKey_->GetOpenKey(),
		L"", //no subkey
		valueName.c_str(),
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

	data = buffer;
	return ResultCode::sOk;
}

Reg::ResultCode Reg::Editor::GetExpandString(const std::wstring& valueName, std::wstring& data) const
{
	DWORD bufferSize = 0;

	const LSTATUS readDataSizeStatus = RegGetValue(
		mRegKey_->GetOpenKey(),
		L"", //no subkey
		valueName.c_str(),
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
		mRegKey_->GetOpenKey(),
		L"", //no subkey
		valueName.c_str(),
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

	data = buffer;
	return ResultCode::sOk;
}

/**
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Editor::SetDword(const std::wstring& valueName, const DWORD& data) const
{
	if (mRegKey_->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	constexpr DWORD dataSize = sizeof(data);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey_->GetOpenKey(),
		valueName.c_str(),
		0, //reserved,must be 0
		REG_DWORD, //data type
		reinterpret_cast<const BYTE*>(&data),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;
}

/**
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Editor::SetQword(const std::wstring& valueName, const ULONGLONG& data) const
{
	if (mRegKey_->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	constexpr  DWORD dataSize = sizeof(data);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey_->GetOpenKey(),
		valueName.c_str(),
		0, //reserved,must be 0
		REG_QWORD, //data type
		reinterpret_cast<const BYTE*>(&data),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;
}

/**
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Editor::SetString(const std::wstring& valueName, const std::wstring& data) const
{
	if (mRegKey_->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	const DWORD dataSize = data.size() * sizeof(wchar_t);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey_->GetOpenKey(),
		valueName.c_str(),
		0, //reserved,must be 0
		REG_SZ, //data type
		reinterpret_cast<const BYTE*>(&data),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;

}

/**
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Editor::SetExpandString(const std::wstring& valueName, const std::wstring& data) const
{
	if (mRegKey_->IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	const DWORD dataSize = data.size() * sizeof(wchar_t);

	const LSTATUS setValueStatus = RegSetValueExW(
		mRegKey_->GetOpenKey(),
		valueName.c_str(),
		0, //reserved,must be 0
		REG_EXPAND_SZ, //data type
		reinterpret_cast<const BYTE*>(&data),
		dataSize
	);

	if (setValueStatus != ERROR_SUCCESS)
	{
		return ResultCode::eValueSetError;
	}

	return ResultCode::sOk;

}


/**
 * @remark The exception is called to exclude an undefined state of the key
 * @remark When an exception is raised, the key cannot be opened, so there is no need to close it
 * @remark If ReadOnly = false,if the key in the given path cannot be opened,there will be an attempt to create it
 */
Reg::Key::Key(const std::wstring& pathToRegKey,const bool& readOnly) noexcept(false) : mOpenRegistryKey_(nullptr) , mAccessRight_(KEY_READ)
{
	if(!readOnly)
	{
		mAccessRight_ = KEY_ALL_ACCESS;
	}

	HKEY handleKey;
	std::wstring subKey;

	const Reg::ResultCode convertKeyResultCode = details::SplitPathToReKey(pathToRegKey, handleKey, subKey);
	if (convertKeyResultCode != Reg::ResultCode::sOk)
	{
		throw Reg::Exception("Wrong path to the key",convertKeyResultCode);
	}

	const Reg::ResultCode InitRegKeyResultCode = details::CreateIfKeyCantBeOpen(handleKey, subKey,mAccessRight_,mOpenRegistryKey_);
	if(InitRegKeyResultCode != ResultCode::sOk)
	{
		throw  Reg::Exception("Key initialization error", InitRegKeyResultCode);
	}
}

Reg::Key::~Key()
{
	RegCloseKey(mOpenRegistryKey_);
}

/**
 * @remark The function is not useful, if any of the parameters is empty.
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Key::RenameSubKey(const std::wstring& subKey, const std::wstring& newSubKeyName) const 
{
	if (IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	if (subKey.empty() || newSubKeyName.empty())
	{
		return ResultCode::eIncorrectPathToKey;
	}

	const LSTATUS renameSubKeyStatus = RegRenameKey(mOpenRegistryKey_, subKey.c_str(), newSubKeyName.c_str());
	if(renameSubKeyStatus != ERROR_SUCCESS)
	{
		return ResultCode::eRenameValueError;
	}

	return ResultCode::sOk;
}

/**
 * @remark The function is not useful, if subkey is empty
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Key::CreateSubKey(const std::wstring& subKey) const
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
	return details::CreateRegKey(mOpenRegistryKey_, subKey,mAccessRight_, openSubKey);
}

/**
 * @remark The function is not useful, if subkey is empty
 * @remark The function is not useful, if at Key was initialized with ReadOnly status
 */
Reg::ResultCode Reg::Key::DeleteSubKey(const std::wstring& subKey) const
{
	if (IsReadOnly())
	{
		return ResultCode::ePermissionError;
	}

	if (subKey.empty())
	{
		return ResultCode::eIncorrectPathToKey;
	}

	const LSTATUS deleteSubKeyStatus = RegDeleteKeyExW(mOpenRegistryKey_, subKey.c_str(), mAccessRight_, 0);
	if (deleteSubKeyStatus != ERROR_SUCCESS)
	{
		return ResultCode::eDeleteError;
	}

	return ResultCode::sOk;
}

HKEY Reg::Key::GetOpenKey() const
{
	return mOpenRegistryKey_;
}

bool Reg::Key::IsReadOnly() const
{
	return mAccessRight_ != KEY_ALL_ACCESS ? true : false;
}

std::wstring utf8toUtf16(const std::string& str)
{
	boost::locale::conv::utf_to_utf<wchar_t>(str);
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.from_bytes(str);
}