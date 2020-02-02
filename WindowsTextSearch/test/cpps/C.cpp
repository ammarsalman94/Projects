
#include "C.h"

namespace C{
	CE::CE(){
		a.AFunc();
	}
	
	CE::~CE(){
		
	}
	
}

#ifdef TEST_C
int main(){
	using namespace C;
	CE ce;
	Something s = "aaa";
	return 0;
}