TxtPutizer is a lightweight, interactive textual UI library.
It provides various menu options for different types and representations of user interactions, allowing the selection of options and capturing user input in a straightforward and convenient manner.

### Features
The menus are controlled via:
* arrow keys - for navigation between presented options.
* spacebar - for selecting the option pointed by the cursor.
* enter - save current state and exit the menu.

Currently, TxtPutizer supports 3 types of menus:
- **checkbox menu** - Allows the user to select any number of options from a vertical list.

![Checkbox Menu Example](./images/checkbox_menu_demo.gif)

- **radio menu** - Allows the user to choose one option from a vertical list of options. Selecting an option deselects any previously selected option.

![Radio Menu Example](./images/radio_menu_demo.gif)

- **prompt menu** - A single-line prompt for selecting one option from a horizontal list, ideal for short questions such as yes/no.

![Prompt Menu Example](./images/prompt_menu_demo.gif)

**Note: More menu variations may be added in the future.*

Any further requests, ideas, and conributions are very welcome.

## Integration
TxtPutizer is designed with as easiest integration into any project as possible in mind. The entire library is contained within a single header file: [`TxtPutizer.hpp`](https://github.com/RuBublik/TxtPutizer/blob/master/TxtPutizer/single_include/TxtPutizer/TxtPutizer.hpp), requiring no modifications to your project settings.


### Basic usage

To present a menu to a terminal window -
1. Create a Menu object of the desired type, passing just a title for the menu.

```cpp
CheckboxMenu cbm(L"CHECKBOX_MENU_TITLE");
```

2. Add as meny options as you like. Optionally, add description to each option to be displayed below the menu, when the option is hovered.

```cpp
cbm.addOption(L"option 1", L"Description for option 1");
cbm.addOption(L"option 2", L"Description for option 1");
cbm.addOption(L"option 3"); // description is not mandatory
```

3. Execute the menu - running until 'Enter' key is pressed, which saves the current state.

```cpp
cbm.execute();
```

4. Acquire state of the menu upon exit. The returned value is a custom struct, describing the state upon exit.

```cpp
MenuState mState = cbm.getState();
```

5. Interpret and react to user's selections, represented as custom `Option` objects.

```cpp
// VERY BASIC DECLARATION OF `OPTION`
class Option
{
public:
	BOOL IsSelected() const;

	// displayed in menu
	const std::wstring _displayName;
	const std::wstring _description;
private:	
	BOOL _selected;	
protected:
};
```

For interpreting menu state, 2 approaches can be applied -

```cpp
/* 
APPROACH 1: 
acquire only the selected options
*/

//  EITHER -
Option sel = cbmState.getSelectedOptions()[0] // for singular option menus
//  OR -
std::vector<Option> sel = cbmState.getSelectedOptions() // for multiple option menus

// print display name of selected option
std::wcout << "\r\nSELECTED OPTION: " << sel._displayName << std::endl;
```

```cpp
/*
APPROACH 2: 
iterate and check all options
*/

for (Option opt : cbmState.options) {

    // print all properties of each option
	std::wcout << opt._displayName << L"\t " 
    std::wcout << opt._description << L"\t " 
    std::wcout << opt.IsSelected() << std::endl;
}
```

For more complete usage examples, refer to [`here`](https://github.com/RuBublik/TxtPutizer/blob/master/TxtPutizer/tests/MainTest.cpp) file in this repository.

***
Feel free to reach out with any questions or suggestions!