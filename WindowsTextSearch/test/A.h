#pragma once

#include <string>

namespace A{
	namespace B{
	void AFunction(){
		return;
	}
	}
	
	class Ammar{
	public:
		Ammar();
		~Ammar();
		void AFunc();
		
	private:
		int a;
		std::string b;
	}
}

void gFunction(){
	return;
}