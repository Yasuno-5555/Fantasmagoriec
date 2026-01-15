#include "widgets/markdown.hpp"
#include <iostream>
#include <cassert>

int main() {
    using namespace ui;
    
    // Test 1: Bold
    auto spans = ParseMarkdown("Hello **World**!");
    assert(spans.size() == 3);
    assert(spans[0].text == "Hello "); assert(!spans[0].bold);
    assert(spans[1].text == "World"); assert(spans[1].bold);
    assert(spans[2].text == "!"); assert(!spans[2].bold);
    
    // Test 2: Italic
    spans = ParseMarkdown("*Italic*");
    assert(spans.size() == 1);
    assert(spans[0].text == "Italic"); assert(spans[0].italic);
    
    // Test 3: Link
    spans = ParseMarkdown("See [Google](http://google.com)");
    assert(spans.size() == 2);
    assert(spans[0].text == "See ");
    assert(spans[1].text == "Google"); assert(spans[1].is_link); assert(spans[1].link_url == "http://google.com");
    
    // Test 4: Header
    spans = ParseMarkdown("# Title");
    assert(spans.size() == 1);
    assert(spans[0].text == "Title"); assert(spans[0].is_heading); assert(spans[0].heading_level == 1);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}

