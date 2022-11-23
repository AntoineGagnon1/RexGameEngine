#pragma once

// Usage : RE_STATIC_CONSTRUCTOR({SomeStuff(); SomeOtherStuff();})
#define RE_STATIC_CONSTRUCTOR(funcDefinition) inline static int StaticInit = [] { funcDefinition return 0; }();