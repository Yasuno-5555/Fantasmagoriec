#include "core/application.hpp"

int main() {
    FantasmagorieCore app;
    if(app.init("Fantasmagorie App", 800, 600)) {
        app.run();
    }
    return 0;
}

