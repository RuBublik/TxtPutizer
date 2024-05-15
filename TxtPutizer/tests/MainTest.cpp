#include "..\single_include\TxtPutizer\TxtPutizer.hpp"

void example_checkbox_menu()
{
	CheckboxMenu cbm(L"CHECKBOX_MENU");
	cbm.addOption(L"opt1", L"This option is good");
	cbm.addOption(L"opt2", L"This option is better");
	cbm.addOption(L"opt3", L"This option is best");

	cbm.execute();
	MenuState s = cbm.getState();
}

void example_radio_menu()
{
	RadioMenu rm(L"RADIO_MENU");
	rm.addOption(L"opt1", L"This option is good");
	rm.addOption(L"opt2", L"This option is better");
	rm.addOption(L"opt3", L"This option is best");

	rm.execute();
	MenuState s = rm.getState();
}

void example_prompt_menu()
{
	PromptMenu pm(L"Do you know DA way?");
	pm.addOption(L"Yes", L"come");
	pm.addOption(L"No", L"I will show you DA way");
	pm.addOption(L"Maybe?", L"");

	pm.execute();
	MenuState s = pm.getState();
}

int main()
{
	example_checkbox_menu();
	//example_radio_menu();
	//example_prompt_menu();

	return 0;
}