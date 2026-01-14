#include "fanta.h"
#include <iostream>

using namespace fanta;

int main() {
    Init(1280, 720, "Fantasmagorie Crystal Demo - Phase 2 Verification");

    Run([]{
        // Phase 2 Verification: Rect, RoundedRect, Circle, Line
        
        // Main container
        Element("Main Panel")
            .width(1200).height(700)
            .bg(Color::Hex(0x1E1E1EFF))
            .child(
                // Test 1: Basic Rect (should be sharp, no jaggies)
                Element("Rect Test")
                    .width(200).height(100)
                    .bg(Color::Hex(0x5588FFFF))
            )
            .child(
                // Test 2: RoundedRect (corners should not be crushed)
                Element("RoundedRect Test")
                    .width(200).height(100)
                    .bg(Color::Hex(0x44AA44FF))
                    .rounded(20.0f) // 20px corner radius
            )
            .child(
                // Test 3: Small RoundedRect (stress test for corner)
                Element("Small RoundedRect")
                    .width(100).height(50)
                    .bg(Color::Hex(0xAA4444FF))
                    .rounded(25.0f) // Large radius relative to size
            )
            .child(
                // Test 4: Very small RoundedRect (edge case)
                Element("Tiny RoundedRect")
                    .width(60).height(30)
                    .bg(Color::Hex(0xFFAA44FF))
                    .rounded(15.0f) // Should be clamped to min(width,height)/2 = 15
            )
            .child(
                // Test 5: Circle (should not look like polygon)
                Element("Circle Test")
                    .width(100).height(100) // Square for circle
                    .bg(Color::Hex(0xAA44AAFF))
                    .rounded(50.0f) // Perfect circle
            );
    });

    return 0;
}
