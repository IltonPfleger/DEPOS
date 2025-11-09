#include <stdio.h>
#include <symbols.h>

#include <memory/MemoryMap.hpp>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    MemoryMap mm{};
    mm.kernel.start      = reinterpret_cast<void *>(___LD_KERNEL_START__);
    mm.kernel.end        = reinterpret_cast<void *>(___LD_KERNEL_END__);
    mm.app.start = reinterpret_cast<void *>(___LD_APPLICATION_START__);
    mm.app.end   = reinterpret_cast<void *>(___LD_APPLICATION_END__);
    mm.app.entry = reinterpret_cast<void *>(_main);

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
