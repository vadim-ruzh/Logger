#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <Windows.h>
#include<boost/noncopyable.hpp>

namespace Reg
{
	enum class ResultCode : uint16_t
	{
		sOk,
		eValueReadingError,
		eValueSetError,
		ePermissionError,
		eKeyOpeningError,
		eKeyCloseError,
		eKeyCreationError,
		eIncorrectPathToKey,
		eDeleteError,
		eRenameValueError
	};

	class Exception : std::runtime_error
	{
	public:
		Exception(const std::string& errorMessage, const Reg::ResultCode& errorCode);
		ResultCode GetErrorCode() const;
	private:
		ResultCode mErrorCode_;
	};

	class Key : boost::noncopyable
	{
	public:
		Key(const std::wstring& pathToRegistryKey,const bool& readOnly = false) noexcept(false);
		~Key();

		ResultCode RenameSubKey(const std::wstring& subKey, const std::wstring& newSubKeyName) const;
		ResultCode CreateSubKey(const std::wstring& subKey) const;
		ResultCode DeleteSubKey(const std::wstring& subKey) const;

		HKEY GetOpenKey() const;
		bool IsReadOnly() const;
	private:
		HKEY mOpenRegistryKey_;
		REGSAM mAccessRight_;
	};

	class Editor : boost::noncopyable
	{
	public:
		Editor(const std::wstring& pathToRegKey,const bool& readOnly = false) noexcept(false);
		~Editor();

		ResultCode ChangeKey(const std::wstring& pathToRegKey,const bool& readOnly = false);

		ResultCode GetDword(const std::wstring& valueName, DWORD& data) const;
		ResultCode GetQword(const std::wstring& valueName, ULONGLONG& data) const;
		ResultCode GetString(const std::wstring& valueName, std::wstring& data) const;
		ResultCode GetExpandString(const std::wstring& valueName, std::wstring& data) const;

		ResultCode SetDword(const std::wstring& valueName, const DWORD& data) const;
		ResultCode SetQword(const std::wstring& valueName, const ULONGLONG& data) const;
		ResultCode SetString(const std::wstring& valueName, const std::wstring& data) const;
		ResultCode SetExpandString(const std::wstring& valueName, const std::wstring& data) const;
	private:
		std::unique_ptr<Key> mRegKey_;
	};

}

std::wstring utf8toUtf16(const std::string& str);