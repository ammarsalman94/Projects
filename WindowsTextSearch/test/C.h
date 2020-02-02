#pragma once

#include "A.h"
#include "B.h"

#include <string>

namespace C{
	using Something = std::string;
	class CE{
	public:
		CE();
		~CE();
	private:
		Ammar a;
		Bee b;
	}
}