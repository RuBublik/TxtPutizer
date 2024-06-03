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


const int  DEFAULT_OPTIONS_PER_PAGE = 10;
const char DEFAULT_CURSOR_STYLE		= L'>';

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
protected:

	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle, int optsPerPage) 
		: BasicMenu(menuTitle), 
		m_cursorStyle(cursorStyle), m_OPTIONS_PER_PAGE(optsPerPage) {};
	
	VerticalMenu(const std::wstring& menuTitle, int optsPerPage)
		: VerticalMenu(menuTitle, DEFAULT_CURSOR_STYLE, optsPerPage) {};

	VerticalMenu(const std::wstring& menuTitle, wchar_t cursorStyle) 
		: VerticalMenu(menuTitle, cursorStyle, DEFAULT_OPTIONS_PER_PAGE) {};

	VerticalMenu(const std::wstring& menuTitle)
		: VerticalMenu(menuTitle, DEFAULT_CURSOR_STYLE, DEFAULT_OPTIONS_PER_PAGE) {};

	~VerticalMenu() {};

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
		std::wcout << m_options[optIdx]._displayName;
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

	void deletePageInfo() {
		// move console cursor to line of selected option, and delete
		moveConsoleCursorDown(
			m_OPTIONS_PER_PAGE
			+ 1/*for spacing between options and description*/
			+ 1/*for page info*/);
		clearLine();

		// reset console cursor to initial position
		moveConsoleCursorUp(m_OPTIONS_PER_PAGE + 2/*account for lines rendered above*/);
	}

	void renderPageInfo(int pageIdx) {
		deletePageInfo();

		// move console cursor to line of selected option
		moveConsoleCursorDown(
			m_OPTIONS_PER_PAGE
			+ 1/*for spacing between options and description*/
			+ 1/*for page info*/);
		int startIdx = pageIdx * m_OPTIONS_PER_PAGE;
		int endIdx = std::min(startIdx + m_OPTIONS_PER_PAGE, (int)m_options.size());
		std::wcout << L"[ " << startIdx + 1 << L"-" << endIdx << L" / " 
			<< m_options.size()	<< L" ]" << std::endl;

		// reset console cursor to initial position
		moveConsoleCursorUp(m_OPTIONS_PER_PAGE + 3/*account for lines rendered above*/);
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
		std::wcout << m_options[optIdx]._description << std::endl;

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
		int optsPerPage): VerticalMenu(menuTitle, cursorStyle, optsPerPage) {};

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

		scrollConsole();
		renderTitle();

		// Display the first page
		renderPage(0);
		if (m_B_USE_PAGING) {
			renderPageInfo(0);
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
					renderPageInfo(m_currentPageIdx);
					break;
				}
				continue;
			case KEY_ARROW_RIGHT:
				if (m_B_USE_PAGING && m_currentPageIdx < m_totalPages - 1) {
					m_currentPageIdx++;
					renderPage(m_currentPageIdx);
					renderPageInfo(m_currentPageIdx);
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
		hideMenuCursor();
		deleteDescription();
		if (m_B_USE_PAGING) {
			deletePageInfo();
		}
		moveConsoleCursorDown(
			getNumOptionsInPage(m_currentPageIdx) + 2/*JUST AFTER MENU*/);
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
	RadioMenu(const std::wstring& menuTitle, wchar_t cursorStyle, int optsPerPage)
		: VerticalMenu(menuTitle, cursorStyle, optsPerPage) {};

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

		scrollConsole();
		renderTitle();

		// Display the first page
		renderPage(0);
		if (m_B_USE_PAGING) {
			renderPageInfo(0);
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
					renderPageInfo(m_currentPageIdx);
					break;
				}
				continue;
			case KEY_ARROW_RIGHT:
				if (m_currentPageIdx < m_totalPages - 1) {
					m_currentPageIdx++;
					renderPage(m_currentPageIdx);
					renderPageInfo(m_currentPageIdx);
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
		hideMenuCursor();
		deleteDescription();
		if (m_B_USE_PAGING) {
			deletePageInfo();
		}
		moveConsoleCursorDown(getNumOptionsInPage(m_currentPageIdx) + 2/*JUST AFTER MENU*/);
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

	std::wstring truncateString(const std::wstring& str)
	{
		if (str.size() > m_maxOptLength) {
			return str.substr(0, m_maxOptLength - 3) + L"...";
		}
		return str;
	}

	void renderOption(int optIdx) override
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

	void deleteDescription() override {	// DOES NOT RESET CONSOLE CURSOR
		// move console cursor to line of selected option, and delete
		moveConsoleCursorDown(2);
		clearLine();
	}

	void renderDescription(int optIdx) override {
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

	void execute() override
	{
		char keyPress;
		BOOL finitoLaComedia = FALSE;

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
	}

private:
protected:
};

#endif //RUBUBLIK_TXTPUTIZER_HPP_