#pragma once

typedef __builtin_va_list va_list;
#define va_start(...) __builtin_va_start(__VA_ARGS__)
#define va_arg(...) __builtin_va_arg(__VA_ARGS__)
#define va_end(...) __builtin_va_end(__VA_ARGS__)
