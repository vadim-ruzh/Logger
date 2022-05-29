#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <Windows.h>
#include <filesystem>
#include <string_view>
#include <boost/noncopyable.hpp>

namespace registry
{
	enum ResultCode : uint16_t
	{
		sOk = 0,
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
		Exception(std::string_view errorMessage, registry::ResultCode errorCode);
		[[nodiscard]] ResultCode GetErrorCode() const;

	private:
		ResultCode mErrorCode;
	};

	class Key : boost::noncopyable
	{
	public:
		/**
		 * @remark The exception is called to exclude an undefined state of the key
		 * @remark When an exception is raised, the key cannot be opened, so there is no need to close it
		 * @remark If @param isReadOnly is false and if the key in the given path cannot be opened, there will be an attempt to create it
		 */
		explicit Key(std::wstring_view pathToRegistryKey, bool isReadOnly = false) noexcept(false);
		~Key();

		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::eIncorrectPathToKey if any of the parameters is empty.
		 *  @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode RenameSubKey(std::wstring_view subKey, std::wstring_view newSubKeyName) const;
		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::eIncorrectPathToKey if @param subKey is empty.
		 * @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode CreateSubKey(std::wstring_view subKey) const;
		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::eIncorrectPathToKey if @param subKey is empty.
		 *  @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode DeleteSubKey(std::wstring_view subKey) const;

		[[nodiscard]] HKEY GetOpenedKey() const;
		[[nodiscard]] bool IsReadOnly() const;

	private:
		HKEY mOpenRegistryKey;
		REGSAM mAccessRight;
	};

	class RegistryEditor : boost::noncopyable
	{
	public:
		explicit RegistryEditor(std::wstring_view pathToRegKey, bool isReadOnly = false) noexcept(false);
		~RegistryEditor();

		/**
		 * @remark If an exception is raised during the initialization of a new key, the old key will not be changed
		 */
		[[nodiscard]] ResultCode ChangeKey(std::wstring_view pathToRegKey, const bool& isReadOnly = false);

		[[nodiscard]] ResultCode GetDword(std::wstring_view name, DWORD& value) const;
		[[nodiscard]] ResultCode GetQword(std::wstring_view name, ULONGLONG& value) const;
		[[nodiscard]] ResultCode GetString(std::wstring_view name, std::wstring& value) const;
		[[nodiscard]] ResultCode GetExpandString(std::wstring_view name, std::wstring& value) const;

		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode SetDword(std::wstring_view name, DWORD value) const;
		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode SetQword(std::wstring_view name, ULONGLONG value) const;
		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode SetString(std::wstring_view name, std::wstring_view value) const;
		/**
		 * @remark has no effect and returns @memberof registry::ResultCode::ePermissionError if key @memberof registry::Key::IsReadOnly
		 */
		[[nodiscard]] ResultCode SetExpandString(std::wstring_view name, std::wstring_view value) const;

	private:
		std::optional<Key> mRegKey;
	};

}