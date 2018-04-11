#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <array>

typedef unsigned char BYTE;
typedef std::vector<BYTE> BYTECODE;

template<size_t SIZE>
bool isExist(const BYTECODE& bc, const std::array<BYTE, SIZE>& code){
	for (int i = 0; i < bc.size(); ++i){
		for (int j = 0; j < code.size(); ++j)
			if (bc[i] == code[j]) return true;
	}
	return false;
}

template<typename T>
struct TrainElem{
	std::vector<T> in;
	std::vector<T> out;
	void show(std::ostream& os = std::cout, bool p=true) const{
		os << "IN: ";
		for (auto& e : in) os << e << " ";
		os << "OUT: ";
		for (auto& e : out) os << e << " ";
		if(p) os << "\n";
	}

	void clear(){
		in.clear();
		out.clear();
	}
};

template<typename T>
struct TrainSubset{
	std::vector<TrainElem<T>> ss;

	void show(std::ostream& os = std::cout) const{
		for (int i = 0; i < ss.size();++i) ss[i].show(os);
	}

	void clear(){
		ss.clear();
	}

	int size() const{
		return ss.size();
	}
};

template<typename T>
void BuildTrainSubset(const std::vector<TrainElem<T>>& src, std::vector<TrainSubset<T>>& dst, int k){
	if (src.size() % k != 0) {
		std::cerr << "Error: src.size() % k != 0\n";
		std::cin.get();
		exit(-1);
	}
	TrainSubset<T> tmp_elem;
	dst.clear();
	int z(k),c(1);
	for (int i = 0; i < src.size(); ++i){
		tmp_elem.ss.push_back(src[i]);
		if ((i+1) == z){
			dst.push_back(tmp_elem);
			tmp_elem.clear();
			++c;
			z = k*c;
			if (z > src.size()) z = src.size() ;
		}
	}
}

int NumCPU();
int getUseCoresNum(int max_cpu_usage = 50);

void toSS(const unsigned __int64& v, const int& n, std::vector<BYTE>& res, std::vector<BYTE>& adm);// res:= преобразует число i в систему счисления по основанию N.
std::string getValueByTag(const std::string& str, const std::string& tag);
void split(const std::string& str, const std::string& split_symbol, std::vector<std::string>& v);
void ReadTrainData(const std::string& filename, std::vector<TrainElem<int>>& tr);

template<typename T>
void WriteProtocol(int index, const BYTECODE& bc, const std::vector<TrainSubset<int>>& tr_subset, const std::vector<bool>& corr, const std::vector<TrainElem<int>>& tr, int Rating, T& FE);

//void Selection(std::vector<BYTECODE>& bc, std::vector<TrainElem<int>>& tr);

