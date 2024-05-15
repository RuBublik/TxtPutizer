#ifndef RUBUBLIK_TXTPUTIZER_HPP_
#define RUBUBLIK_TXTPUTIZER_HPP_

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>
#include <string>

#define KEY_ENTER		13	// '\r'
#define KEY_SPACEBAR	32	// ' '
#define KEY_ARROW_UP	72	// 'H'
#define KEY_ARROW_LEFT	75	// 'K'
#define KEY_ARROW_RIGHT 77	// 'M'
#define KEY_ARROW_DOWN	80	// 'P'


class Option
{
public:
	BOOL IsSelected() const {
		return _selected;
	}
	// displayed in menu
	const std::wstring _displayName;
	
	// optional, can contain additional info that would be displayed when cursor hovers
	// on this option.
	const std::wstring _description;

	// grant 'BasicMenu' and derived classes access to private methods/members
	friend class BasicMenu;

	// d-tor
	~Option() {}

private:	
	// stores current state, initialized to FALSE.
	BOOL _selected;	

	// restrict instanciation only to friend classes
	// c-tor
	Option(const std::wstring& optName, const std::wstring& optDescription)
		: _displayName(optName), _description(optDescription), _selected(FALSE) {};
	Option(const std::wstring& _title) : Option(_title, L"") {}
	Option() : Option(L"") {}

protected:
};


struct MenuState
{
	std::vector<Option> options;
	
	std::vector<Option> getSelectedOptions()
	{
		std::vector<Option> selectedOpts;
		for (const auto& opt : options) {
			if (opt.IsSelected()) {
				selectedOpts.push_back(opt);
			}
		}
		return selectedOpts;
	}

	// d-tor
	~MenuState() {}

	// grant 'BasicMenu' and derived classes access to private methods/members
	friend class BasicMenu;
	friend class RadioMenu;

private:
	// restrict instanciation only to friend classes
	// c-tor
	MenuState() : options({}) {}
	MenuState(std::vector<Option>& options) : options(options) {}
};


class BasicMenu 
{
public:
	BasicMenu(std::wstring menuTitle) : _title(menuTitle), _options({}) {}

	void addOption(const std::wstring& optDisplayName, const std::wstring& optDescription = L"") {
		_options.push_back({ optDisplayName, optDescription });
	}

	virtual void execute() = 0;

	MenuState getState() {
		return MenuState(_options);
	}

private:
protected:
	std::wstring _title;
	std::vector<Option> _options;
	int _cursor = 0;

	virtual void renderTitle() = 0;
	virtual void renderOptions() = 0;

	void clearLine() {
		std::cout << "\033[2K"; // overwrite current line
		std::cout << "\033[0G"; // Move cursor to beginning of line
	}

	void moveCursorUp(int lines) {
		std::cout << "\033[" << lines << "A";
	}

	void moveCursorDown(int lines) {
		std::cout << "\033[" << lines << "B";
	}

	void toggleOption(Option& option) {
		option._selected = !option._selected;
	}

	void selectOption(Option& option) {
		option._selected = TRUE;
	}

	void unselectOption(Option& option) {
		option._selected = FALSE;
	}
};


/// <summary>
/// Allows the user to choose any amount of option from a list of options.
/// </summary>
class CheckboxMenu : public BasicMenu 
{
public:
	CheckboxMenu(const std::wstring& menuTitle, wchar_t cursorStyle = L'>')
		: _cursorStyle(cursorStyle), BasicMenu(menuTitle) {};

	void execute()
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		renderTitle();
		do
		{
			renderOptions();

			// MENU CONTROL
			// (get user input & update state)
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_SPACEBAR:
				toggleOption(_options[_cursor]);
				break;
			case KEY_ARROW_UP:
				if (_cursor > 0) {
					_cursor--;
				}
				break;
			case KEY_ARROW_DOWN:
				if (_cursor < _options.size() - 1) {
					_cursor++;
				}
				break;
			case KEY_ENTER:
				finitoLaComedia = TRUE;
			}

		} while (!finitoLaComedia);

		// clear description before exit
		renderDeleteDescription();
	}

private:
	const wchar_t _cursorStyle;
	BOOL bFirstRender = TRUE;

	void renderTitle()
	{
		std::wcout << _title << std::endl;
		for (int i = 0; i < _title.length(); i++) {
			std::wcout << L"-";
		}
		std::wcout << std::endl;
	}

	void renderOptions()
	{
		// skip moving cursor on first render
		if (bFirstRender) {
			bFirstRender = FALSE;
		}
		else {
			renderDeleteDescription();
			moveCursorUp(_options.size());
		}

		// print options, line by line.
		int idx = 0;
		for (const auto& opt : _options) {
			
			// clear screen
			clearLine();
			
			// display state
			std::wcout << (_cursor == idx ? _cursorStyle : L' ');
			std::wcout << L" [" << (opt.IsSelected() ? L'*' : L' ') << L"] ";
			std::wcout << opt._displayName;
			std::wcout << std::endl;
			
			idx++;
		}

		// display description of hovered option
		std::wcout << L"-\n" << _options[_cursor]._description << std::endl;
	}

	void renderDeleteDescription()
	{
		moveCursorUp(1);
		clearLine();
		moveCursorUp(1);
		clearLine();
	}

protected:
};


/// <summary>
/// Allows the user to choose one option from a list of options,
/// where selecting one option deselects any previously selected option.
/// </summary>
class RadioMenu : public BasicMenu
{
public:
	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle = L'>')
		: _cursorStyle(cursorStyle), BasicMenu(menuTitle) {};

	void execute()
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		renderTitle();
		do
		{
			renderOptions();

			// MENU CONTROL
			// (get user input & update state)
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_SPACEBAR:
				for (auto& opt : _options) {
					unselectOption(opt);
				}
				toggleOption(_options[_cursor]);
				break;
			case KEY_ARROW_UP:
				if (_cursor > 0) {
					_cursor--;
				}
				break;
			case KEY_ARROW_DOWN:
				if (_cursor < _options.size() - 1) {
					_cursor++;
				}
				break;
			case KEY_ENTER:
				finitoLaComedia = TRUE;
			}

		} while (!finitoLaComedia);

		// clear description before exit
		renderDeleteDescription();
	}

private:
	const wchar_t _cursorStyle;
	BOOL bFirstRender = TRUE;

	void renderTitle()
	{
		std::wcout << _title << std::endl;
		for (int i = 0; i < _title.length(); i++) {
			std::wcout << L"-";
		}
		std::wcout << std::endl;
	}

	void renderOptions()
	{
		// skip moving cursor on first render
		if (bFirstRender) {
			bFirstRender = FALSE;
		}
		else {
			renderDeleteDescription();
			moveCursorUp(_options.size());
		}

		// print options, line by line.
		int idx = 0;
		for (const auto& opt : _options) {

			// clear screen
			clearLine();

			// display state
			std::wcout << (_cursor == idx ? _cursorStyle : L' ');
			std::wcout << L" [" << (opt.IsSelected() ? L'*' : L' ') << L"] ";
			std::wcout << opt._displayName;
			std::wcout << std::endl;

			idx++;
		}

		// display description of hovered option
		std::wcout << L"-\n" << _options[_cursor]._description << std::endl;
	}

	void renderDeleteDescription()
	{
		moveCursorUp(1);
		clearLine();
		moveCursorUp(1);
		clearLine();
	}

protected:
};

/// <summary>
/// Single line prompt, allows the user to choose one option from a list of options.
/// (practically, a horizontal RadioMenu. Best for short questions, like yes/no prompt.)
/// </summary>
class PromptMenu : public BasicMenu
{
public:
	PromptMenu(const std::wstring& menuTitle) : BasicMenu(menuTitle) {};

	void execute()
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		do
		{
			singleLineRender();

			// MENU CONTROL
			// (get user input & update state)
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_ARROW_LEFT:
				if (_cursor > 0) {
					_cursor--;
				}
				break;
			case KEY_ARROW_RIGHT:
				if (_cursor < _options.size() - 1) {
					_cursor++;
				}
				break;
			// either of space/enter makes a selection
			case KEY_SPACEBAR:
			case KEY_ENTER:
				selectOption(_options[_cursor]);
				finitoLaComedia = TRUE;
				break;
			}

		} while (!finitoLaComedia);

		renderDeleteDescription();
	}

private:
	BOOL bFirstRender = TRUE;

	void singleLineRender()
	{
		// skip moving cursor on first render
		if (bFirstRender) {
			bFirstRender = FALSE;
		}
		else {
			renderDeleteDescription();
			moveCursorUp(1);
			clearLine();
		}
		renderTitle();
		renderOptions();
	}

	void renderDeleteDescription()
	{
		moveCursorUp(1);
		clearLine();
		moveCursorUp(1);
		clearLine();
	}

	void renderTitle()
	{
		std::wcout << _title << L"  ";
	}

	void renderOptions()
	{
		int idx = 0;
		for (auto& opt : _options) {
	
			// display state
			if (_cursor == idx) {
				std::wcout << "[ " << opt._displayName << " ]";
			}
			else {
				std::wcout << "  " << opt._displayName << "  ";
			}			
			std::wcout << L" \| ";

			idx++;
		}
		std::wcout << L"\b\b" << L' ' << std::endl;	// delete trailing slash after loop

		// display description of hovered option
		std::wcout << L"-\n" << _options[_cursor]._description << std::endl;
	}

protected:
};

#endif //RUBUBLIK_TXTPUTIZER_HPP_