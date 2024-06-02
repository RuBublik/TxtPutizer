#include "..\single_include\TxtPutizer\TxtPutizer.hpp"

void example_checkbox_menu()
{
	CheckboxMenu cbm(L"CHECKBOX_MENU");
	cbm.addOption(L"opt1", L"desc1", TRUE);
	cbm.addOption(L"opt2", L"desc2", TRUE);
	cbm.addOption(L"opt3", L"desc3", TRUE);
	cbm.addOption(L"opt4", L"desc4");
	cbm.addOption(L"opt5", L"desc5");
	cbm.addOption(L"opt6", L"desc6");
	
//	un-comment to enable paging
//
//	cbm.addOption(L"opt7", L"desc7");
//	cbm.addOption(L"opt8", L"desc8");
//	cbm.addOption(L"opt9", L"desc9");
//	cbm.addOption(L"opt10", L"desc10");
//	cbm.addOption(L"opt11", L"desc11");
//	cbm.addOption(L"opt12", L"desc12");
//	cbm.addOption(L"opt13", L"desc13");
//	cbm.addOption(L"opt14", L"desc14");
//	cbm.addOption(L"opt15", L"desc15");
//	cbm.addOption(L"opt16", L"desc16");
//	cbm.addOption(L"opt17", L"desc17");
//	cbm.addOption(L"opt18", L"desc18");
//	cbm.addOption(L"opt19", L"desc19");
//	cbm.addOption(L"opt20", L"desc20");
//	cbm.addOption(L"opt21", L"desc21");
//	cbm.addOption(L"opt22", L"desc22");
//	cbm.addOption(L"opt23", L"desc23");
//	cbm.addOption(L"opt24", L"desc24");
	cbm.execute();
	MenuState cbmState = cbm.getState();

	std::wcout << L"OPTION\t DESCRIPTION\t\t SELECTED" << std::endl;
	for (Option opt : cbmState.options) {
		std::wcout << opt._displayName << L"\t " << opt._description << L"\t\t " << opt.IsSelected() << std::endl;
	}
}


void example_radio_menu()
{
	RadioMenu rm(L"RADIO_MENU");
	rm.addOption(L"opt1", L"desc1");
	rm.addOption(L"opt2", L"desc2");
	rm.addOption(L"opt3", L"desc3");
	rm.addOption(L"opt4", L"desc4");
	rm.addOption(L"opt5", L"desc5");
	rm.addOption(L"opt6", L"desc6");

//	un-comment to enable paging
//
//	rm.addOption(L"opt7", L"desc7");
//	rm.addOption(L"opt8", L"desc8");
//	rm.addOption(L"opt9", L"desc9");
//	rm.addOption(L"opt10", L"desc10");
//	rm.addOption(L"opt11", L"desc11");
//	rm.addOption(L"opt12", L"desc12");
//	rm.addOption(L"opt13", L"desc13");
//	rm.addOption(L"opt14", L"desc14");
//	rm.addOption(L"opt15", L"desc15");
//	rm.addOption(L"opt16", L"desc16");
//	rm.addOption(L"opt17", L"desc17");
//	rm.addOption(L"opt18", L"desc18");
//	rm.addOption(L"opt19", L"desc19");
//	rm.addOption(L"opt20", L"desc20");
//	rm.addOption(L"opt21", L"desc21");
//	rm.addOption(L"opt22", L"desc22");
//	rm.addOption(L"opt23", L"desc23");
//	rm.addOption(L"opt24", L"desc24");

	rm.execute();
	MenuState rmState = rm.getState();

	Option sel = rmState.getSelectedOptions()[0];
	std::wcout << "SELECTED OPTION: " << sel._displayName << std::endl;
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
	example_radio_menu();
	//example_prompt_menu();

	return 0;
}

