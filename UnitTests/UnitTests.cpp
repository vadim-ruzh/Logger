#define BOOST_TEST_MODULE RegistryTest

#include <boost/test/included/unit_test.hpp>
#include "../Logger/Registry.h"
#include "../Logger/Registry.cpp"


std::wstring testRegistryPath = L"HKEY_CURRENT_USER\\SOFTWARE\\RegTest";


BOOST_AUTO_TEST_CASE(openregistrykey_happypath)
{
	BOOST_CHECK_NO_THROW(Reg::Editor testEditor(testRegistryPath));
}


BOOST_AUTO_TEST_CASE(ReadWriteDwordValue_HappyPath)
{
	std::wstring testValueName = L"testDword";
	DWORD controlData = 1000;
	DWORD requiredData;

	Reg::Editor testEditor(testRegistryPath);

	auto setResult = testEditor.SetDword(testValueName, controlData);
	BOOST_CHECK(setResult == Reg::ResultCode::sOk);


	
	auto readResult = testEditor.GetDword(testValueName, requiredData);
	BOOST_CHECK(readResult == Reg::ResultCode::sOk);

	BOOST_TEST(requiredData == controlData);
}

BOOST_AUTO_TEST_CASE(ReadWriteQwordValue_HappyPath)
{
	std::wstring testValueName = L"testQword";
	ULONGLONG controlData = 1000;
	ULONGLONG requiredData;

	Reg::Editor testEditor(testRegistryPath);

	const auto setResult = testEditor.SetQword(testValueName, controlData);
	BOOST_CHECK(setResult == Reg::ResultCode::sOk);

	const auto readResult = testEditor.GetQword(testValueName, requiredData);
	BOOST_CHECK(readResult == Reg::ResultCode::sOk);

	BOOST_TEST(requiredData == controlData);
}

BOOST_AUTO_TEST_CASE(ReadWriteStringValue_HappyPath)
{
	std::wstring testValueName = L"testString";
	std::wstring controlData = L"HappyString";
	std::wstring requiredData;

	Reg::Editor testEditor(testRegistryPath);

	const auto setResult = testEditor.SetString(testValueName, controlData);
	BOOST_CHECK(setResult == Reg::ResultCode::sOk);


	const auto readResult = testEditor.GetString(testValueName, requiredData);
	BOOST_CHECK(readResult == Reg::ResultCode::sOk);

	BOOST_TEST(requiredData._Equal(controlData));
}

BOOST_AUTO_TEST_CASE(ReadWriteExpandStringValue_HappyPath)
{
	std::wstring testValueName = L"testExpandString";

	std::wstring controlData = L"HappyExpandString";
	std::wstring requiredData;

	Reg::Editor testEditor(testRegistryPath);

	const auto setResult = testEditor.SetExpandString(testValueName, controlData);
	BOOST_CHECK(setResult == Reg::ResultCode::sOk);

	const auto readResult = testEditor.GetExpandString(testValueName, requiredData);
	BOOST_CHECK(readResult == Reg::ResultCode::sOk);

	BOOST_TEST(requiredData._Equal(controlData));
}

BOOST_AUTO_TEST_CASE(ReadWriteExpandStringValue_HappyPath)
{
	std::wstring testValueName = L"testExpandString";

	std::wstring controlData = L"HappyExpandString";
	std::wstring requiredData;

	Reg::Editor testEditor(testRegistryPath);

	const auto setResult = testEditor.SetExpandString(testValueName, controlData);
	BOOST_CHECK(setResult == Reg::ResultCode::sOk);

	const auto readResult = testEditor.GetExpandString(testValueName, requiredData);
	BOOST_CHECK(readResult == Reg::ResultCode::sOk);

	BOOST_TEST(requiredData._Equal(controlData));
}