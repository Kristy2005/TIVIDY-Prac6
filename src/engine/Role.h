#ifndef ROLE_H
#define ROLE_H

#include <string>


namespace tividy {
	template <typename T>
	class Role {

	private:
		std::string name;

	public:
		void configureRole();
	};

	// #include "Role.cpp"
}

#endif 
