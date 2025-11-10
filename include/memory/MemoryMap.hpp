#pragma once

struct MemoryMap {
    struct {
        void *start;
        void *end;
    } kernel;
    struct {
        void *start;
        void *end;
        void *entry;

        struct {
            void *start;
            void *end;
        } code;

        struct {
            void *start;
            void *end;
        } rodata;

        struct {
            void *start;
            void *end;
        } data;

        struct {
            void *start;
            void *end;
        } bss;

    } app;
};

__attribute__((section(".__MEMORY_MAP__"))) inline MemoryMap __mm;
