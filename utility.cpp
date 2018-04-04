#include "utility.h"
#include <algorithm>
#include <Windows.h>

int NumCPU(){
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

int getUseCoresNum(int max_cpu_usage){
	int cores_number = round(NumCPU()*((double)max_cpu_usage / 100));
	return (cores_number < 1) ? 1 : cores_number;
}

void toSS(const unsigned __int64& v, const int& n, std::vector<BYTE>& res){
	register unsigned __int64 div, r, val(v);
	res.clear();
	//std::vector<int> tmp;
	if (val < n) {
		res.push_back(val);
		return;
	}
	do{
		div = val / n;
		r = div*n;
		res.push_back(val - r);
		val = div;
	} while (val >= n);
	res.push_back(val);
	std::reverse(res.begin(), res.end());
}

std::string getValueByTag(const std::string& str, const std::string& tag){
	size_t pos_a = str.find("<" + tag + ">");
	size_t pos_b = str.find("</" + tag + ">");
	if (pos_a != std::string::npos && pos_b != std::string::npos){
		return str.substr(pos_a + tag.size() + 2, pos_b - (pos_a + tag.size() + 2));
	}
}

void split(const std::string& str, const std::string& split_symbol, std::vector<std::string>& v){
	int p = 0;
	for (int i = 1; i < str.size(); ++i){
		if (str.substr(i, 1) == split_symbol.c_str()){
			v.push_back(str.substr(p, i - p));
			p = i + 1;
		}
	}
	v.push_back(str.substr(p));
}

void ReadTrainData(const std::string& filename, std::vector<TrainElem<int>>& tr){
	std::ifstream f(filename.c_str());
	if (!f.is_open()) {
		std::cerr << "Error : file " << filename << " - not found!\n";
		std::cin.get();
		exit(-1);
	}
	std::string str;
	std::vector<std::string> s;

	std::string in_tmp, out_tmp;

	std::vector<std::string> tmp;
	TrainElem<int> elem;
	while (std::getline(f, str)){
		elem.clear();
		s.push_back(str);
		in_tmp = getValueByTag(str, "in");
		tmp.clear();
		split(in_tmp, " ", tmp);
		for (auto& e : tmp) elem.in.push_back(std::stoi(e));
		out_tmp = getValueByTag(str, "out");
		tmp.clear();
		split(out_tmp, " ", tmp);
		for (auto& e : tmp) elem.out.push_back(std::stoi(e));
		tr.push_back(elem);
	}
	f.close();
}
