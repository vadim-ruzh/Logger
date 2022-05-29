#define BOOST_TEST_MODULE RegistryTest

#include <boost/test/included/unit_test.hpp>
#include "registry.h"

namespace
{
	std::wstring_view testRegistryPath = L"HKEY_CURRENT_USER\\SOFTWARE\\RegTest";
}

BOOST_AUTO_TEST_CASE(openregistrykey_happypath)
{
	BOOST_CHECK_NO_THROW(registry::RegistryEditor testEditor(testRegistryPath));
}

BOOST_AUTO_TEST_CASE(ReadWriteDwordValue_HappyPath)
{
	std::wstring testValueName = L"testDword";
	DWORD controlData = 1000;

	registry::RegistryEditor editor(testRegistryPath);
	
	BOOST_CHECK(editor.SetDword(testValueName, controlData) == registry::ResultCode::sOk);

	DWORD requiredData;
	BOOST_CHECK(editor.GetDword(testValueName, requiredData) == registry::ResultCode::sOk);

	BOOST_TEST(requiredData == controlData);
}

BOOST_AUTO_TEST_CASE(ReadWriteQwordValue_HappyPath)
{
	std::wstring_view testValueName = L"testQword";
	ULONGLONG controlData = 1000;

	registry::RegistryEditor editor(testRegistryPath);
	
	BOOST_CHECK(editor.SetQword(testValueName, controlData) == registry::ResultCode::sOk);

	ULONGLONG requiredData;
	BOOST_CHECK(editor.GetQword(testValueName, requiredData) == registry::ResultCode::sOk);

	BOOST_TEST(requiredData == controlData);
}

BOOST_AUTO_TEST_CASE(ReadWriteStringValue_HappyPath)
{
	std::wstring_view testValueName = L"testString";
	std::wstring_view controlData = L"HappyString";

	registry::RegistryEditor editor(testRegistryPath);
	
	BOOST_CHECK(editor.SetString(testValueName, controlData) == registry::ResultCode::sOk);

	std::wstring requiredData;
	BOOST_CHECK(editor.GetString(testValueName, requiredData) == registry::ResultCode::sOk);

	BOOST_TEST((requiredData == controlData));
}

BOOST_AUTO_TEST_CASE(ReadWriteExpandStringValue_HappyPath)
{
	std::wstring_view testValueName = L"testExpandString";
	std::wstring_view controlData = L"HappyExpandString";

	registry::RegistryEditor editor(testRegistryPath);
	
	BOOST_CHECK(editor.SetExpandString(testValueName, controlData) == registry::ResultCode::sOk);

	std::wstring requiredData;
	BOOST_CHECK(editor.GetExpandString(testValueName, requiredData) == registry::ResultCode::sOk);

	BOOST_TEST((requiredData == controlData));
}
