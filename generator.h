#include <iostream>
#include <string>
#include <functional>

#define size_bc 64

template<typename T, typename MEM_TYPE>
class Generator {
	T emulator_obj;
	std::vector<BYTE> src;
public:

	Generator(){
		emulator_obj = T();
		src.resize(size_bc, 0);
	}

	void mem_set(const std::vector<MEM_TYPE>& mem){
		emulator_obj.mem_set(mem);
	}

	void mem_clear(){
		emulator_obj.mem_clear();
	}

	int run(const std::string& source){
		return emulator_obj.emulator(source);
	}

	int run(const std::vector<BYTE>& bytecode){
		return emulator_obj.emulator(bytecode);
	}

	bool check_result(const std::vector<MEM_TYPE>& res){
		return emulator_obj.check_result(res);
	}

	void dump(){
		emulator_obj.dump();
	}



};