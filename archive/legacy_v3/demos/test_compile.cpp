#define NOMINMAX
#include <glad/gl.h>
#include <stdio.h>

int main() {
    printf("GLAD version: %d\n", GLAD_VERSION_MAJOR(version));
    return 0;
}
