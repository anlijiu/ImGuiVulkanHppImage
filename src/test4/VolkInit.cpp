// 在 **一个** .cpp 文件中（如 volk_init.cpp）
// 在其他文件中只需包含头文件而不定义实现： 
//   一定只写一行 #include "volk.h" 即可，
//   不能再出现 #define VOLK_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#include "volk.h"
