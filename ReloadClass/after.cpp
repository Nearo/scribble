#include <cstdio>
#define IMPLEMENT_MODULE
#include "module.h"

void MODULE_API Hoge::doSomething()
{
    printf("Hoge::doSomething() after: %d\n", m_data);
}
