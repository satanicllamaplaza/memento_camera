#include <iostream>
#include <string>
#include <vector>

struct MenuItem {
    String name;                 // easy printing
    int currentIndex;            // which option is selected
    std::vector<String> options; // available options
};

int main() {
    std::vector<MenuItem> menu = {
        {"LED Brightness", 0, {"Low", "Medium", "High", "Max"}},
        {"LED Color", 0, {"Red", "Green", "Blue", "White"}},
        {"Resolution", 0, {"640x480", "1280x720", "1920x1080", "4K"}}
    };

    int selectedMenu = 0; // which menu item you're on

    // Example: simulate user pressing right on brightness
    MenuItem &item = menu[selectedMenu];
    item.currentIndex = (item.currentIndex + 1) % item.options.size();

    std::cout << item.name << " -> " << item.options[item.currentIndex] << "\n";

    // Move down to next menu item (like pressing "down")
    selectedMenu = (selectedMenu + 1) % menu.size();
    std::cout << "Now editing: " << menu[selectedMenu].name << "\n";

    return 0;
}

