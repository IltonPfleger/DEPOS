#include <stdio.h>

#include <memory/MemoryMap.hpp>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    MemoryMap mm{};
    mm.kernel.end = reinterpret_cast<void *>(__KERNEL_END__);

    FILE *file = fopen(argv[1], "wb");
    if (!file) {
        perror("fopen");
        return 1;
    }

    size_t written = fwrite(&mm, sizeof(MemoryMap), 1, file);
    if (written != 1) {
        perror("fwrite");
        fclose(file);
        return 1;
    }

    if (fclose(file) != 0) {
        perror("fclose");
        return 1;
    }

    return 0;
}
