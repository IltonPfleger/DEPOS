#pragma once

struct MemoryMap {
    struct {
        void* start;
        void* end;
    } kernel;
    struct {
        void* start;
        void* end;
        void* entry;
    } app;
};

__attribute__((section(".__MEMORY_MAP__"))) inline MemoryMap __mm;
