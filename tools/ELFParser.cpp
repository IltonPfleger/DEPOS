#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <elf.h>

#include "memory/MemoryMap.hpp"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <ELF> <output file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);

    Elf64_Ehdr ehdr;
    if (fread(&ehdr, 1, sizeof(ehdr), f) != sizeof(ehdr)) {
        fprintf(stderr, "Error reading ELF header.\n");
        fclose(f);
        return 1;
    }

    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "File is not an ELF file.\n");
        fclose(f);
        return 1;
    }

    if (fseek(f, ehdr.e_phoff, SEEK_SET) != 0) {
        perror("fseek (program headers)");
        fclose(f);
        return 1;
    }

    Elf64_Phdr *phdrs = (Elf64_Phdr *)malloc(ehdr.e_phnum * sizeof(Elf64_Phdr));
    if (!phdrs) {
        fprintf(stderr, "Failed to allocate memory for program headers.\n");
        fclose(f);
        return 1;
    }

    if (fread(phdrs, sizeof(Elf64_Phdr), ehdr.e_phnum, f) !=
        (size_t)ehdr.e_phnum) {
        fprintf(stderr, "Error reading program headers.\n");
        free(phdrs);
        fclose(f);
        return 1;
    }

    Elf64_Addr max_addr = 0;
    Elf64_Addr min_addr = 0;
    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            if (phdrs[i].p_vaddr + phdrs[i].p_memsz > max_addr)
                max_addr = phdrs[i].p_vaddr + phdrs[i].p_memsz;
            if (phdrs[i].p_vaddr < min_addr)
                min_addr = phdrs[i].p_vaddr;
        }
    }
    free(phdrs);

    if (fseek(f, ehdr.e_shoff, SEEK_SET) != 0) {
        perror("fseek (section headers)");
        fclose(f);
        return 1;
    }

    Elf64_Shdr *shdrs = (Elf64_Shdr *)malloc(ehdr.e_shnum * sizeof(Elf64_Shdr));
    if (!shdrs) {
        fprintf(stderr, "Failed to allocate memory.\n");
        fclose(f);
        return 1;
    }

    if (fread(shdrs, sizeof(Elf64_Shdr), ehdr.e_shnum, f) !=
        (size_t)ehdr.e_shnum) {
        fprintf(stderr, "Error reading section headers.\n");
        free(shdrs);
        fclose(f);
        return 1;
    }

    Elf64_Shdr strtab_hdr = shdrs[ehdr.e_shstrndx];
    char *shstrtab = (char *)malloc(strtab_hdr.sh_size);
    if (!shstrtab) {
        fprintf(stderr, "Failed to allocate section name table.\n");
        free(shdrs);
        fclose(f);
        return 1;
    }

    fseek(f, strtab_hdr.sh_offset, SEEK_SET);
    fread(shstrtab, 1, strtab_hdr.sh_size, f);

    MemoryMap::App app = {};
    for (int i = 0; i < ehdr.e_shnum; i++) {
        const char *name = shstrtab + shdrs[i].sh_name;

        if (strcmp(name, ".text") == 0) {
            app.text.start = shdrs[i].sh_addr;
            app.text.end = shdrs[i].sh_addr + shdrs[i].sh_size;
        } else if (strcmp(name, ".data") == 0 || strcmp(name, ".sdata") == 0) {
            if (app.data.start == 0 || shdrs[i].sh_addr < app.data.start)
                app.data.start = shdrs[i].sh_addr;
            if (shdrs[i].sh_addr + shdrs[i].sh_size > app.data.end)
                app.data.end = shdrs[i].sh_addr + shdrs[i].sh_size;
        } else if (strcmp(name, ".bss") == 0 || strcmp(name, ".sbss") == 0) {
            if (app.bss.start == 0 || shdrs[i].sh_addr < app.bss.start)
                app.bss.start = shdrs[i].sh_addr;
            if (shdrs[i].sh_addr + shdrs[i].sh_size > app.bss.end)
                app.bss.end = shdrs[i].sh_addr + shdrs[i].sh_size;
        } else if (strcmp(name, ".rodata") == 0) {
            app.rodata.start = shdrs[i].sh_addr;
            app.rodata.end = shdrs[i].sh_addr + shdrs[i].sh_size;
        }
    }

    app.start = app.text.start;
    app.end = max_addr;
    app.main = ehdr.e_entry;

    FILE *file = fopen(argv[2], "wb");
    if (!file) {
        perror("fopen output");
        return 1;
    }

    size_t written = fwrite(&app, sizeof(MemoryMap::App), 1, file);
    if (written != 1) {
        perror("fwrite");
        fclose(file);
        return 1;
    }

    if (fclose(file) != 0) {
        perror("fclose");
        return 1;
    }

    free(shstrtab);
    free(shdrs);
    fclose(f);
    return 0;
}
