#if defined(_WIN32)
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT
#endif

void EXPORT helperFunc() {}

