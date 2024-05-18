#include "..\single_include\TxtPutizer\TxtPutizer.hpp"

void example_checkbox_menu()
{
	CheckboxMenu cbm(L"CHECKBOX_MENU");
	cbm.addOption(L"opt1", L"This option is good");
	cbm.addOption(L"opt2", L"This option is better");
	cbm.addOption(L"opt3", L"This option is best");

	cbm.execute();
	MenuState cbmState = cbm.getState();

	std::wcout << L"\r\nOPTION\t DESCRIPTION\t\t SELECTED" << std::endl;
	for (Option opt : cbmState.options) {
		std::wcout << opt._displayName << L"\t" << opt._description << L"\t" << opt.IsSelected() << std::endl;
	}
}

void example_radio_menu()
{
	RadioMenu rm(L"RADIO_MENU");
	rm.addOption(L"opt1", L"This option is good");
	rm.addOption(L"opt2", L"This option is better");
	rm.addOption(L"opt3", L"This option is best");

	rm.execute();
	MenuState rmState = rm.getState();

	Option sel = rmState.getSelectedOptions()[0];
	std::wcout << "\r\nSELECTED OPTION: " << sel._displayName << std::endl;
}

void example_prompt_menu()
{
	PromptMenu pm(L"Do you know DA way?");
	pm.addOption(L"Yes", L"come");
	pm.addOption(L"No", L"I will show you DA way");
	pm.addOption(L"Maybe?", L"");

	pm.execute();
	MenuState pmState = pm.getState();

	Option sel = pmState.getSelectedOptions()[0];
	std::wcout << "\r\nSELECTED OPTION: " << sel._displayName << std::endl;
}

int main()
{
	//example_checkbox_menu();
	//example_radio_menu();
	example_prompt_menu();

	return 0;
}