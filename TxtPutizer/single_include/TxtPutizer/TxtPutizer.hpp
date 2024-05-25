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

private:
	// restrict instanciation only to friend classes
	// c-tor
	MenuState() : options({}) {}
	MenuState(std::vector<Option>& options) : options(options) {}
};

/// <summary>
/// most basic menu class
/// </summary>
class BasicMenu 
{
public:

	void addOption(const std::wstring& optDisplayName, const std::wstring& optDescription = L"") {
		m_options.push_back({ optDisplayName, optDescription });
	}

	virtual void execute() = 0;

	MenuState getState() {
		return MenuState(m_options);
	}

private:
protected:
	BasicMenu(const std::wstring& menuTitle) : m_title(menuTitle), m_options({}) {}
	~BasicMenu() {};

	std::wstring m_title;
	std::vector<Option> m_options;
	int m_menuCursorPos = 0;
	//int m_consoleCursorPos = 0;

	virtual void renderTitle() = 0;

	void clearLine() {
		std::cout << "\033[2K"; // overwrite current line
		std::cout << "\033[0G"; // Move cursor to beginning of line
	}

	void clearLeft(int chars) {
		std::cout << "\033[" << chars << "D"; // Move cursor n characters to the left
		for (int i = 0; i < chars; ++i) {
			std::cout << ' '; // overwrite with spaces
		}
		std::cout << "\033[" << chars << "D"; // reset console cursor to initial position
	}

	void clearRight(int chars) {
		for (int i = 0; i < chars; ++i) {
			std::cout << ' '; // overwrite with spaces
		}
		std::cout << "\033[" << chars << "D"; // reset console cursor to initial position
	}

	void moveConsoleCursorLeft(int chars) {
		if (chars) {
			std::cout << "\033[" << chars << "D";
		}
	}
	
	void moveConsoleCursorRight(int chars) {
		if (chars) {
			std::cout << "\033[" << chars << "C";
		}
	}

	void moveConsoleCursorUp(int lines) {
		if (lines) {
			std::cout << "\033[" << lines << "A";
		}
	}

	void moveConsoleCursorDown(int lines) {
		if (lines) {
			std::cout << "\033[" << lines << "B";
		}
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
/// base class for vertical menus
/// </summary>
class VerticalMenu : public BasicMenu
{
public:
private:
	const wchar_t _cursorStyle;

protected:

	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle = L'>')
		: BasicMenu(menuTitle), _cursorStyle(cursorStyle) {};
	~VerticalMenu() {};


	void renderTitle()
	{
		std::wcout << m_title << std::endl;
		for (int i = 0; i < m_title.length(); i++) {
			std::wcout << L"-";
		}
		std::wcout << std::endl;
	}

	void renderOption(int optIdx)
	{
		// move console cursor to line of selected option
		moveConsoleCursorDown(optIdx);

		// display option
		clearLine();
		std::wcout << (m_menuCursorPos == optIdx ? _cursorStyle : L' ');
		std::wcout << L" [" << (m_options[optIdx].IsSelected() ? L'*' : L' ') << L"] ";
		std::wcout << m_options[optIdx]._displayName;
		std::wcout << std::endl;

		// reset console cursor to initial position
		moveConsoleCursorUp(optIdx + 1/*account for lines rendered above*/);
	}


	void deleteDescription() {	// DOES NOT RESET CONSOLE CURSOR
		// move console cursor to line of selected option, and delete
		moveConsoleCursorDown(m_options.size() + 1);
		clearLine();
	}

	void renderDescription(int optIdx) {
		deleteDescription();
		std::wcout << m_options[m_menuCursorPos]._description << std::endl;

		// reset console cursor to initial position
		moveConsoleCursorUp(m_options.size() + 2/*account for lines rendered above*/);
	}
};

/// <summary>
/// Allows the user to choose any amount of option from a list of options.
/// </summary>
class CheckboxMenu : public VerticalMenu
{
public:
	CheckboxMenu(const std::wstring& menuTitle, wchar_t cursorStyle = L'>')
		: VerticalMenu(menuTitle, cursorStyle) {};

	void execute()
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		renderTitle();
		
		// display all options
		for (int i = 0; i < m_options.size(); i++) {
			renderOption(i);
		}
		renderDescription(0);

		do
		{
			// MENU CONTROL
			// (get user input & update state)
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_SPACEBAR:
				toggleOption(m_options[m_menuCursorPos]);
				break;
			case KEY_ARROW_UP:
				if (m_menuCursorPos > 0) {
					m_menuCursorPos--;
					renderOption(m_menuCursorPos + 1);
				}
				break;
			case KEY_ARROW_DOWN:
				if (m_menuCursorPos < m_options.size() - 1) {
					m_menuCursorPos++;
					renderOption(m_menuCursorPos - 1);
				}
				break;
			case KEY_ENTER:
				finitoLaComedia = TRUE;
			
			}

			renderOption(m_menuCursorPos);
			renderDescription(m_menuCursorPos);

		} while (!finitoLaComedia);

		// clear description before exit
		deleteDescription();
	}

private:
protected:
};

/// <summary>
/// Allows the user to choose one option from a list of options,
/// where selecting one option deselects any previously selected option.
/// </summary>
class RadioMenu : public VerticalMenu
{
public:
	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle = L'>')
		: VerticalMenu(menuTitle, cursorStyle) {};

	void execute()
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		renderTitle();

		// display all options
		for (int i = 0; i < m_options.size(); i++) {
			renderOption(i);
		}
		renderDescription(0);

		do
		{
			// MENU CONTROL
			// (get user input & update state)
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_SPACEBAR:
				// un-select selected option, if one is selected
				if (m_selectedOptIdx != -1) {
					unselectOption(m_options[m_selectedOptIdx]);
					renderOption(m_selectedOptIdx);
				}
				toggleOption(m_options[m_menuCursorPos]);
				m_selectedOptIdx = m_menuCursorPos;
				break;
			case KEY_ARROW_UP:
				if (m_menuCursorPos > 0) {
					m_menuCursorPos--;
					renderOption(m_menuCursorPos + 1);
				}
				break;
			case KEY_ARROW_DOWN:
				if (m_menuCursorPos < m_options.size() - 1) {
					m_menuCursorPos++;
					renderOption(m_menuCursorPos - 1);
				}
				break;
			case KEY_ENTER:
				// prevent exit until an option is selected
				if (m_selectedOptIdx != -1) {
					finitoLaComedia = TRUE;
				}
			}

			renderOption(m_menuCursorPos);
			renderDescription(m_menuCursorPos);

		} while (!finitoLaComedia);

		// clear description before exit
		deleteDescription();
	}

private:
	int m_selectedOptIdx = -1;

protected:
};

/// <summary>
/// base class for horizontal menus
/// </summary>
class HorizontalMenu : public BasicMenu
{
public:
private:
protected:
	size_t m_maxOptLength = 8;

	HorizontalMenu(const std::wstring& menuTitle)
		: BasicMenu(menuTitle) {};
	~HorizontalMenu() {};

	void renderTitle()
	{
		std::wcout << m_title << L"  ";
	}

	std::wstring truncateString(const std::wstring& str)
	{
		if (str.size() > m_maxOptLength) {
			return str.substr(0, m_maxOptLength - 3) + L"...";
		}
		return str;
	}

	void renderOption(int optIdx)
	{
		// move console cursor to line of selected option
		int optStartPos = 0;
		for (int i = 0; i < optIdx; i++) {
			optStartPos += truncateString(
				m_options[i]._displayName).size() > m_maxOptLength ?
				m_maxOptLength : m_options[i]._displayName.size();
			optStartPos += 4/*account for spacing between options*/;
			if (i < m_options.size() - 1) {
				optStartPos += 3;
			}
		}
		moveConsoleCursorRight(optStartPos);

		// display option
		std::wstring truncatedOpt = truncateString(m_options[optIdx]._displayName);
		int optLength = truncatedOpt.size() > m_maxOptLength ?
			m_maxOptLength : m_options[optIdx]._displayName.size() 
			+ 4/*account for spacing between options*/;
		clearRight(optLength);

		if (m_menuCursorPos == optIdx) {
			std::wcout << "[ " << truncatedOpt << " ]";
		}
		else {
			std::wcout << "  " << truncatedOpt << "  ";
		}

		if (optIdx < m_options.size() - 1) {
			std::wcout << L" \| ";
			optLength += 3;
		}

		// reset console cursor to initial position
		moveConsoleCursorLeft(optStartPos + optLength);
	}

	void deleteDescription() {	// DOES NOT RESET CONSOLE CURSOR
		// move console cursor to line of selected option, and delete
		moveConsoleCursorDown(2);
		clearLine();
	}

	void renderDescription(int optIdx) {
		deleteDescription();
		std::wcout << m_options[m_menuCursorPos]._description << std::endl;

		// reset console cursor to initial position
		moveConsoleCursorUp(3/*account for lines rendered above*/);
		moveConsoleCursorRight(m_title.size() + 2/*account for spacing - title to opts*/);
	}
};

/// <summary>
/// Single line prompt, allows the user to choose one option from a list of options.
/// (practically, a horizontal RadioMenu. Best for short questions, like yes/no prompt.)
/// </summary>
class PromptMenu : public HorizontalMenu
{
public:
	PromptMenu(const std::wstring& menuTitle) : HorizontalMenu(menuTitle) {};

	void execute()
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		renderTitle();

		// display all options
		for (int i = 0; i < m_options.size(); i++) {
			renderOption(i);
		}
		renderDescription(0);

		do
		{
			// MENU CONTROL
			// (get user input & update state)
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_ARROW_LEFT:
				if (m_menuCursorPos > 0) {
					m_menuCursorPos--;
					renderOption(m_menuCursorPos + 1);
				}
				break;
			case KEY_ARROW_RIGHT:
				if (m_menuCursorPos < m_options.size() - 1) {
					m_menuCursorPos++;
					renderOption(m_menuCursorPos - 1);
				}
				break;
			// either of space/enter makes a selection
			case KEY_SPACEBAR:
			case KEY_ENTER:
				selectOption(m_options[m_menuCursorPos]);
				finitoLaComedia = TRUE;
				break;
			}

			renderOption(m_menuCursorPos);
			renderDescription(m_menuCursorPos);

		} while (!finitoLaComedia);

		deleteDescription();
	}

private:
protected:
};

#endif //RUBUBLIK_TXTPUTIZER_HPP_