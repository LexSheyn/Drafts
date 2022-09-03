#include <iostream>
#include <string>

#define TO_STRING(Expression) #Expression

template<typename T>
static inline const char* GetLiteral()
{
	return "Literal";
}


#define LITERAL(X) \
template<> static inline const char* GetLiteral<X>()\
{\
	return #X;\
}

using CategoryId_T = const char*;

template<typename T>
struct TSettingsCategory
{
	virtual void Load (const void* Pointer) = 0;
	virtual void Save (const void* Pointer) = 0;
};

struct FEnvironmentSettingsCategory : public TSettingsCategory<FEnvironmentSettingsCategory>
{
	static constexpr CategoryId_T Type = "Environment";

	void Load(const void* Pointer) override
	{
		std::cout << FEnvironmentSettingsCategory::Type << " loaded." << std::endl;
	}

	void Save(const void* Pointer) override
	{
		std::cout << FEnvironmentSettingsCategory::Type << " saved." << std::endl;
	}
};

LITERAL(FEnvironmentSettingsCategory);

struct FTestSettingsCategory : public TSettingsCategory<FTestSettingsCategory>
{
	static constexpr CategoryId_T Type = "Test";

	void Load(const void* Pointer) override
	{
		std::cout << FTestSettingsCategory::Type << " loaded." << std::endl;
	}

	void Save(const void* Pointer) override
	{
		std::cout << FTestSettingsCategory::Type << " saved." << std::endl;
	}
};

int32_t main(int32_t ArgC, const char* ArgV[])
{


	FEnvironmentSettingsCategory EnvironmentSettingsCategory;
	FTestSettingsCategory        TestSettingsCategory;

	EnvironmentSettingsCategory.Load(nullptr);
	EnvironmentSettingsCategory.Save(nullptr);

	std::cout << GetLiteral<FEnvironmentSettingsCategory>() << std::endl;

	TestSettingsCategory.Load(nullptr);
	TestSettingsCategory.Save(nullptr);

	return 0;
}