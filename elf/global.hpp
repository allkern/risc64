#define OELF_MODE(m) OELF_MODE_##m
#define OELF_MODE_EXPORT __declspec(dllexport)
#define OELF_MODE_IMPORT __declspec(dllimport)

void OELF_MODE(import) 