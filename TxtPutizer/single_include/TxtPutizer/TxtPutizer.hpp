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

// Undefine min/max macro of windows.h
// --> due to conflict with numeric_limits::min() / numeric_limits::max()
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


const int	DEFAULT_OPTIONS_PER_PAGE	= 10;
const char	DEFAULT_CURSOR_STYLE		= L'>';
const int	TRUNC_CHARS					= 5;
const std::wstring NO_INSTRUCTION		= L"";


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
	Option(const std::wstring& optName) : Option(optName, L"") {}
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

	virtual void addOption(const std::wstring& optDisplayName, const std::wstring& optDescription = L"") {
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

	virtual void renderTitle() = 0;
	virtual void renderOption(int optIdx) = 0;
	virtual void renderDescription(int optIdx) = 0;
	virtual void deleteDescription() = 0;
	// scrolls console down enough line so menu is not torn apart
	virtual void scrollConsole() = 0;
	
	std::wstring truncateString(const std::wstring& str, int maxSize)
	{
		if (str.size() <= maxSize) {
			return str;
		}
		int maxSizeWithReplacement = maxSize - TRUNC_CHARS;
		std::wstring truncatedStr = str.substr(0, maxSizeWithReplacement);
		if (truncatedStr.length() < str.length()) {
			truncatedStr += L"~" + str.substr(str.length() - TRUNC_CHARS);
		}
		return truncatedStr;
	}

	void hideConsoleCursor() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(hConsole, &cursorInfo);
		cursorInfo.bVisible = false;
		SetConsoleCursorInfo(hConsole, &cursorInfo);
	}

	void showConsoleCursor() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(hConsole, &cursorInfo);
		cursorInfo.bVisible = true;
		SetConsoleCursorInfo(hConsole, &cursorInfo);
	}

	int getConsoleLineSize() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
			return -1;
		}
		return csbi.dwSize.X;
	}

	// discard any unread input
	void clearInputBuffer() {
		HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(hInput);
	}

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
	virtual void addOption(const std::wstring& optDisplayName, const std::wstring& optDescription = L"") override {
		BasicMenu::addOption(optDisplayName, optDescription);
		m_totalPages = (m_options.size() + m_OPTIONS_PER_PAGE - 1) / m_OPTIONS_PER_PAGE;
		m_B_USE_PAGING = m_totalPages > 1;
	}

private:

	void renderInstruction()
	{
		std::wcout << m_instruction;
	}

	void renderPageInfo(int pageIdx)
	{
		int startIdx = pageIdx * m_OPTIONS_PER_PAGE;
		int endIdx = std::min(startIdx + m_OPTIONS_PER_PAGE, (int)m_options.size());
		std::wcout << L"[ " << startIdx + 1 << L"-" << endIdx << L" / "
			<< m_options.size() << L" ]";
	}

protected:

	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle, int optsPerPage, 
		const std::wstring& instruction) 
		: BasicMenu(menuTitle), 
		m_cursorStyle(cursorStyle), 
		m_OPTIONS_PER_PAGE(optsPerPage), 
		m_instruction(instruction) {};


	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle, int optsPerPage)
		: VerticalMenu(menuTitle, cursorStyle, optsPerPage, NO_INSTRUCTION) {};

	VerticalMenu(const std::wstring& menuTitle, int optsPerPage,
		const std::wstring& instruction)
		: VerticalMenu(menuTitle, DEFAULT_CURSOR_STYLE, optsPerPage, instruction) {};

	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle,
		const std::wstring& instruction)
		: VerticalMenu(menuTitle, cursorStyle, DEFAULT_OPTIONS_PER_PAGE, instruction) {};


	VerticalMenu(const std::wstring& menuTitle, int optsPerPage)
		: VerticalMenu(menuTitle, DEFAULT_CURSOR_STYLE, optsPerPage, 
			NO_INSTRUCTION) {};

	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle) 
		: VerticalMenu(menuTitle, cursorStyle, DEFAULT_OPTIONS_PER_PAGE, 
			NO_INSTRUCTION) {};

	VerticalMenu(const std::wstring& menuTitle,	const std::wstring& instruction)
		: VerticalMenu(menuTitle, DEFAULT_CURSOR_STYLE, DEFAULT_OPTIONS_PER_PAGE, 
			instruction) {};


	VerticalMenu(const std::wstring& menuTitle)	
		: VerticalMenu(menuTitle, DEFAULT_CURSOR_STYLE, DEFAULT_OPTIONS_PER_PAGE, 
			NO_INSTRUCTION) {};

	~VerticalMenu() {};

	std::wstring m_instruction;
	const wchar_t m_cursorStyle;
	int m_OPTIONS_PER_PAGE;
	BOOL m_B_USE_PAGING = FALSE;
	int m_currentPageIdx = 0;
	int m_totalPages = 0;

	void scrollConsole() override
	{
		int totalLines = 
			2/*title*/ + 
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size())/*options*/ +
			2/*description*/;
		for (int i = 0; i < totalLines; i++) {
			std::cout << "\n";
		}
		moveConsoleCursorUp(totalLines);
	}

	void renderFooter()
	{
		if (!m_B_USE_PAGING && m_instruction == NO_INSTRUCTION)
			return;

		// move console cursor to line of footer, and delete
		moveConsoleCursorDown(
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size())
			+ 1/*for spacing between options and description*/
			+ 1/*for description*/);
		std::cout << "-";
		clearLine();
		moveConsoleCursorDown(1);
		clearLine();
		moveConsoleCursorUp(1);

		for (int i = 0; i < std::max(m_title.length(), m_instruction.length() + 1); i++) {
			std::wcout << L"-";
		}
		std::wcout << std::endl;

		if (m_instruction != NO_INSTRUCTION) {
			renderInstruction();
			std::wcout << L" | ";
		}
		if (m_B_USE_PAGING) {
			renderPageInfo(m_currentPageIdx);
		}
		std::wcout << std::endl;

		// move console cursor to line of footer, and delete
		moveConsoleCursorUp(
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size())
			+ 1/*for spacing between options and description*/
			+ 1/*for description*/
			+ 2/*for footer itself*/);
	}

	void renderTitle() override
	{
		std::wcout << m_title << std::endl;
		for (int i = 0; i < m_title.length(); i++) {
			std::wcout << L"-";
		}
		std::wcout << std::endl;
	}

	void renderOption(int optIdx) override
	{
		// move console cursor to line of selected option
		moveConsoleCursorDown(optIdx % m_OPTIONS_PER_PAGE);

		// display option
		clearLine();
		std::wcout << (m_menuCursorPos == optIdx % m_OPTIONS_PER_PAGE ? m_cursorStyle : L' ');
		std::wcout << L" [" << (m_options[optIdx].IsSelected() ? L'*' : L' ') << L"] ";
		std::wcout << truncateString(m_options[optIdx]._displayName, getConsoleLineSize() - 7);
		std::wcout << std::endl;

		// reset console cursor to initial position
		moveConsoleCursorUp(optIdx % m_OPTIONS_PER_PAGE + 1/*account for lines rendered above*/);
	}

	BOOL isOptionInPage(int optIdx, int pageIdx) {
		int startIdx = pageIdx * m_OPTIONS_PER_PAGE;
		int endIdx = std::min(startIdx + m_OPTIONS_PER_PAGE, (int)m_options.size());

		if (optIdx < endIdx && optIdx >= startIdx) {
			return TRUE;
		}
		return FALSE;
	}

	int getNumOptionsInPage(int pageIdx) {
		return std::min(m_OPTIONS_PER_PAGE, 
			(int)m_options.size() - pageIdx * m_OPTIONS_PER_PAGE);
	}

	void deletePage() {
		int numOptsInPage;
		if (m_B_USE_PAGING) {
			 numOptsInPage = std::max(
				getNumOptionsInPage(m_currentPageIdx),
				m_OPTIONS_PER_PAGE
				);
		}
		else {
			numOptsInPage = getNumOptionsInPage(m_currentPageIdx);
		}
		for (int i = 0; i < numOptsInPage; i++) {
			clearLine();
			moveConsoleCursorDown(1);
		}
		moveConsoleCursorUp(numOptsInPage);
	}

	void renderPage(int pageIdx) {
		deletePage();
		int numOptionsInPage = getNumOptionsInPage(pageIdx);

		// adjust menu cursor position, if needed
		if (m_menuCursorPos > numOptionsInPage - 1) {
			m_menuCursorPos = numOptionsInPage - 1;
		}

		for (int i = pageIdx * m_OPTIONS_PER_PAGE;
			i < pageIdx * m_OPTIONS_PER_PAGE + numOptionsInPage;
			i++
		) {
			renderOption(i);
		}
	}

	void hideMenuCursor() {
		m_menuCursorPos = -1;

		// refresh all options
		renderPage(m_currentPageIdx);
	}

	void deleteDescription() override {
		// move console cursor to line of description, and delete
		moveConsoleCursorDown(
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size())
			+ 1/*for spacing between options and description*/);
		clearLine();
		
		// reset console cursor to initial position
		moveConsoleCursorUp(
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size())
			+ 1/*account for lines rendered above*/
		);
	}

	void renderDescription(int optIdx) override {
		deleteDescription();
		
		// move console cursor to line of description
		moveConsoleCursorDown(
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size())
			+ 1/*for spacing between options and description*/);
		std::wcout << truncateString(m_options[optIdx]._description, getConsoleLineSize()) << std::endl;

		// reset console cursor to initial position
		moveConsoleCursorUp(
			std::min(m_OPTIONS_PER_PAGE, (int)m_options.size()) 
			+ 2/*account for lines rendered above*/
		);
	}
};

/// <summary>
/// Allows the user to choose any amount of option from a list of options.
/// </summary>
class CheckboxMenu : public VerticalMenu
{
public:
	CheckboxMenu(const std::wstring& menuTitle, wchar_t cursorStyle, 
		int optsPerPage, const std::wstring& instruction) 
		: VerticalMenu(menuTitle, cursorStyle, optsPerPage, instruction) {};


	CheckboxMenu(const std::wstring& menuTitle, wchar_t cursorStyle, int optsPerPage)
		: VerticalMenu(menuTitle, cursorStyle, optsPerPage) {};

	CheckboxMenu(const std::wstring& menuTitle, int optsPerPage,
		const std::wstring& instruction)
		: VerticalMenu(menuTitle, optsPerPage, instruction) {};

	CheckboxMenu(const std::wstring& menuTitle, wchar_t cursorStyle, 
		const std::wstring& instruction)
		: VerticalMenu(menuTitle, cursorStyle, instruction) {};


	CheckboxMenu(const std::wstring& menuTitle, const std::wstring& instruction)
		: VerticalMenu(menuTitle, instruction) {};

	CheckboxMenu(const std::wstring& menuTitle, int optsPerPage) 
		: VerticalMenu(menuTitle, optsPerPage) {};

	CheckboxMenu(const std::wstring& menuTitle, wchar_t cursorStyle) 
		: VerticalMenu(menuTitle, cursorStyle) {};


	CheckboxMenu(const std::wstring& menuTitle)
		: VerticalMenu(menuTitle) {};


	void addOption(const std::wstring& optDisplayName, 
		const std::wstring& optDescription = L"", BOOL isSelectedByDefault = FALSE) {
		VerticalMenu::addOption(optDisplayName, optDescription);
		if (isSelectedByDefault) {
			toggleOption(m_options.back());
		}
	}

	void execute() override
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		hideConsoleCursor();
		scrollConsole();
		renderTitle();
		renderPage(0);
		renderDescription(0);
		renderFooter();

		do
		{
			// MENU CONTROL
			// (get user input & update state)
			clearInputBuffer();
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_SPACEBAR:
				toggleOption(m_options[m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos]);
				break;
			case KEY_ARROW_UP:
				if (m_menuCursorPos > 0) {
					m_menuCursorPos--;
					renderOption(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos + 1);
					break;
				}
				continue;
			case KEY_ARROW_DOWN:
				if (m_menuCursorPos < getNumOptionsInPage(m_currentPageIdx) - 1) {
					m_menuCursorPos++;
					renderOption(
						m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos - 1);
					break;
				}
				continue;
			case KEY_ARROW_LEFT:
				if (m_B_USE_PAGING && m_currentPageIdx > 0) {
					m_currentPageIdx--;
					renderPage(m_currentPageIdx);
					renderFooter();
					break;
				}
				continue;
			case KEY_ARROW_RIGHT:
				if (m_B_USE_PAGING && m_currentPageIdx < m_totalPages - 1) {
					m_currentPageIdx++;
					renderPage(m_currentPageIdx);
					renderFooter();
					break;
				}
				continue;
			case KEY_ENTER:
				finitoLaComedia = TRUE;
				break;
			default:
				continue;
			}

			renderOption(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos);
			renderDescription(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos);

		} while (!finitoLaComedia);

		// cleanup before exit
		//hideMenuCursor();
		moveConsoleCursorDown(
			getNumOptionsInPage(m_currentPageIdx) + 5/*JUST AFTER MENU*/);
		showConsoleCursor();
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
	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle,
		int optsPerPage, const std::wstring& instruction)
		: VerticalMenu(menuTitle, cursorStyle, optsPerPage, instruction) {};


	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle, int optsPerPage)
		: VerticalMenu(menuTitle, cursorStyle, optsPerPage) {};

	RadioMenu(const std::wstring& menuTitle, int optsPerPage,
		const std::wstring& instruction)
		: VerticalMenu(menuTitle, optsPerPage, instruction) {};

	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle,
		const std::wstring& instruction)
		: VerticalMenu(menuTitle, cursorStyle, instruction) {};


	RadioMenu(const std::wstring& menuTitle, const std::wstring& instruction)
		: VerticalMenu(menuTitle, instruction) {};

	RadioMenu(const std::wstring& menuTitle, int optsPerPage)
		: VerticalMenu(menuTitle, optsPerPage) {};

	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle)
		: VerticalMenu(menuTitle, cursorStyle) {};


	RadioMenu(const std::wstring& menuTitle)
		: VerticalMenu(menuTitle) {};

	void execute() override
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		hideConsoleCursor();
		scrollConsole();
		renderTitle();
		renderPage(0);
		renderDescription(0);
		renderFooter();

		do
		{
			// MENU CONTROL
			// (get user input & update state)
			clearInputBuffer();
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_SPACEBAR:
				// un-select selected option, if one is selected
 				if (m_selectedOptIdx != -1) {
					unselectOption(m_options[m_selectedOptIdx]);
					if (isOptionInPage(
						m_selectedOptIdx,
						m_currentPageIdx)
					) {
						renderOption(m_selectedOptIdx);
					}
				}
				toggleOption(m_options[m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos]);
				m_selectedOptIdx = m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos;
				break;
			case KEY_ARROW_UP:
				if (m_menuCursorPos > 0) {
					m_menuCursorPos--;
					renderOption(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos + 1);
					break;
				}
				continue;
			case KEY_ARROW_DOWN:
				if (m_menuCursorPos < getNumOptionsInPage(m_currentPageIdx) - 1) {
					m_menuCursorPos++;
					renderOption(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos - 1);
					break;
				}
				continue;
			case KEY_ARROW_LEFT:
				if (m_currentPageIdx > 0) {
					m_currentPageIdx--;
					renderPage(m_currentPageIdx);
					renderFooter();
					break;
				}
				continue;
			case KEY_ARROW_RIGHT:
				if (m_currentPageIdx < m_totalPages - 1) {
					m_currentPageIdx++;
					renderPage(m_currentPageIdx);
					renderFooter();
					break;
				}
				continue;
			case KEY_ENTER:
				// prevent exit until an option is selected
				if (m_selectedOptIdx != -1) {
					finitoLaComedia = TRUE;
				}
				break;
			default:
				continue;
			}

			renderOption(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos);
			renderDescription(m_currentPageIdx * m_OPTIONS_PER_PAGE + m_menuCursorPos);

		} while (!finitoLaComedia);

		// cleanup before exit
		//hideMenuCursor();
		moveConsoleCursorDown(
			getNumOptionsInPage(m_currentPageIdx) + 5/*JUST AFTER MENU*/);
		showConsoleCursor();
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

	void scrollConsole() override
	{
		int totalLines = 1/*title*/ + 1/*options*/ + 2/*description*/;
		for (int i = 0; i < totalLines; i++) {
			std::cout << "\n";
		}
		moveConsoleCursorUp(totalLines);
	}

	void renderTitle() override
	{
		std::wcout << m_title << L"  ";
	}

	void renderOption(int optIdx) override
	{
		// move console cursor to line of selected option
		int optStartPos = 0;
		for (int i = 0; i < optIdx; i++) {
			optStartPos += 
				truncateString(m_options[i]._displayName, m_maxOptLength).size();
			optStartPos += 4/*account for spacing between options*/;
			if (i < m_options.size() - 1) {
				optStartPos += 3;
			}
		}
		moveConsoleCursorRight(optStartPos);

		// display option
		std::wstring truncatedOpt = 
			truncateString(m_options[optIdx]._displayName, m_maxOptLength);
		int optLength = truncatedOpt.size()	+ 4/*account for spacing between options*/;
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

	void deleteDescription() override {	// DOES NOT RESET CONSOLE CURSOR
		// move console cursor to line of selected option, and delete
		moveConsoleCursorDown(2);
		clearLine();
	}

	void renderDescription(int optIdx) override {
		deleteDescription();
		std::wcout << truncateString(m_options[m_menuCursorPos]._description, getConsoleLineSize()) << std::endl;

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

	void execute() override
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

		hideConsoleCursor();
		scrollConsole();
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
			clearInputBuffer();
			keyPress = _getch();
			switch (keyPress)
			{
			case KEY_ARROW_LEFT:
				if (m_menuCursorPos > 0) {
					m_menuCursorPos--;
					renderOption(m_menuCursorPos + 1);
					break;
				}
				continue;
			case KEY_ARROW_RIGHT:
				if (m_menuCursorPos < m_options.size() - 1) {
					m_menuCursorPos++;
					renderOption(m_menuCursorPos - 1);
					break;
				}
				continue;
			// either of space/enter makes a selection
			case KEY_SPACEBAR:
			case KEY_ENTER:
				selectOption(m_options[m_menuCursorPos]);
				finitoLaComedia = TRUE;
				break;
			default:
				continue;
			}

			renderOption(m_menuCursorPos);
			renderDescription(m_menuCursorPos);

		} while (!finitoLaComedia);

		deleteDescription();
		showConsoleCursor();
	}

private:
protected:
};

#endif //RUBUBLIK_TXTPUTIZER_HPP_