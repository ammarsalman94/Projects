#pragma once



namespace A{
	using Something = int;

	namespace B{
		void AFunction(int i){
			// do nothing
		}
	}
	
	class Bee{
	public:
		Bee();
		~Bee();
		
	private:
		int b;
		
	}
}