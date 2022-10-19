#include <iostream>

#include <RexEngine.h>

int main()
{
	RE_ASSERT(true, "This assert was false");

	RE_LOG_DEBUG("Debug {} {}", 12, "test");
	RE_LOG_INFO("Hello {}", 123);
	RE_LOG_WARN("Warn {}", "warn");
	RE_LOG_ERROR("ERROR {}", 3.0f);

	std::cout << std::endl;

	RE_LOG_CORE_DEBUG("CORE Debug {} {}", 12, "test");
	RE_LOG_CORE_INFO("CORE Hello {}", 123);
	RE_LOG_CORE_WARN("CORE Warn {}", "warn");
	RE_LOG_CORE_ERROR("CORE ERROR {}", 3.0f);
	return 0;
}