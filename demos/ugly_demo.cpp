#include "fanta.h"
#include <string>
#include <vector>
#include <cmath>

using namespace fanta;

int main() {
    Init(800, 600, "Ugly Demo (Stress Test)");

    Run([]{
        static int frame = 0;
        frame++;

        // Root with no size (should autofill)
        Element("Root")
            .bg(Color::Hex(0x220000FF)) // Dark Red background
            
            // Dynamic Header (Changes every 60 frames)
            .child(
                (frame / 60) % 2 == 0 
                ? Element("Mode: A (Green)").height(50).bg(Color::Hex(0x005500FF))
                : Element("Mode: B (Blue)").height(80).bg(Color::Hex(0x000055FF))
            )
            
            // Stress List: 500 items, relying on default height (40)
            // This tests the layout engine's loop performance and scrolling (if it existed, here just overflow)
            .child(Element("StressList").width(400)
                .child(Element("ScrollMe (Not Implemented)").color(Color::White()))
            );

        // Dynamic Elements injection
        // We can't loop easily with fluent child() unless we break chain or use helper.
        // But we can verify "Dynamic Children" by realizing we can't easily do a loop INSIDE the chain 
        // without a helper or temporary variable. This highlights API usage patterns.
        
        // Let's use a temporary vector to attach children? 
        // No, element.child() modifies parent.
        // We can create parent, then loop to attach.
        
        Element list = Element("GeneratedList").width(300).bg(Color::Hex(0x333333FF));
        for(int i=0; i < (frame % 50); ++i) { // Growing list 0->50 items
             list.child(Element("Item").color(Color::White()));
        }
        
        // Attach the list to Root? No, we lost Root handle in the chain above.
        // This reveals "Root" must be returned or held if we want to append later.
        
        // Correct Usage for Dynamic List:
        Element root("RealRoot");
        root.size(800, 600).bg(Color::Hex(0x101010FF));
        
        root.child(Element("Header").height(60).bg(Color::Hex(0x404040FF))
             .child(Element("Ugly Demo").color(Color::White()))
        );
        
        // Massive Blur Stress (Every frame adds a new layer stack? No, just one heavy one)
        if (frame % 200 > 100) {
            root.child(Element("BlurOverlay").size(800,600).backdrop(true).blur(20.0f));
        }

        // 1000 items
        Element container("Container"); 
        for(int i=0; i<200; ++i) { // 200 items to fit on screen ish (overflow)
            container.child(Element("Row").height(5).bg(Color::Hex(0xFFFFFF10)));
        }
        root.child(container);
        
        // The Run() lambda expects us to BUILD the tree.
        // But fanta.h/cpp "store" is global.
        // Element(...) constructor adds to store.
        // .child() links them.
        // But wait, if I create "root" variable, is it added to store? Yes.
        // If I create "container" variable, is it added? Yes.
        // If I do root.child(container), is the hierarchy set? Yes.
        // But do we need to return anything? No. 
        // The store has a linear list of all elements created this frame.
        // EndFrame() scans this list for roots (parent==0).
        // If "container" was added to "root", its parent is set.
        // So only "root" is a root.
        // "list" above (line 39) was created but NOT attached to root. 
        // So it is ALSO a root! It will render on top or below "root".
        
        // This is "Ugly" but correct behavior for this API.
    });

    return 0;
}
