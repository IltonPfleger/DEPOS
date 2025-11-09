#pragma once

struct MemoryMap {
    struct {
        void* start;
        void* end;
    } kernel;
    struct {
        void* start;
        void* end;
    } application;
};

__attribute__((section(".MemoryMap"))) MemoryMap __memory_map = {};
