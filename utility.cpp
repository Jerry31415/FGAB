#include "utility.h"
#include <algorithm>
#include <Windows.h>
#include "FortEmulator.h"

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

void WriteProtocol(int index, const BYTECODE& bc, const std::vector<TrainSubset<int>>& tr_subset, const std::vector<bool>& corr, const std::vector<TrainElem<int>>& tr, int Rating, FortEmulator& FE){
	std::ofstream proto("proto" + toString(index + 1) + ".html");
	proto << "<html>\n<head>\n<style = type=\"text/css\">\n";
	proto << "TABLE{ border: 1px solid green; border-collapse: collapse; }\n";
	proto << "TD,TH{ border: 1px solid green; }\n";
	proto << "</style>\n</head>\n<body align=\"center\">\n";

	proto << "<b>Available instructions:</b><p>\n";
	proto << "<table align=\"center\">\n";
	proto << "<tr>";
	for (int t = 0; t < FE.NWords; ++t) proto << "<td>" << t << "</td>";
	proto << "</tr>\n<tr>";
	for (int t = 0; t < FE.NWords; ++t) proto << "<td>" << FE.names[t] << "</td>";
	proto << "</tr>\n</table>\n";

	proto << "\n<p>";
	proto << "<p><b>ByteCode:</b><p>";
	for (auto& e : bc) proto << (int)e << " ";
	proto << "<p><b>Train subset (" << (index + 1) << "/" << tr.size() << "):</b>\n<p>\n";
	proto << "<table align=\"center\">\n";
	proto << "<tr><td>#</td><td>IN</td><td>OUT</td></tr>\n";
	for (int i = 0; i < tr_subset[index].ss.size(); ++i){
		proto << "<tr><td>" << i + 1 << "</td><td>";
		for (auto& e : tr_subset[index].ss[i].in) proto << e << " ";
		proto << "</td><td>";
		for (auto& e : tr_subset[index].ss[i].out) proto << e << " ";
		proto << "</td></tr>\n";
	}
	proto << "</table>\n";
	proto << "<p><b>Correct results (" << Rating << "/" << tr.size() << "):</b>\n";
	proto << "<table align=\"center\">\n";
	proto << "<tr><td>#</td><td>IN</td><td>OUT</td><td>corr</td></tr>\n";
	for (int z = 0; z < corr.size(); ++z){
		proto << "<tr>";
		proto << "<td>" << z << "</td>";
		proto << "<td>";
		for (auto& e : tr[z].in) proto << e << " ";
		proto << "</td>";
		proto << "<td>";
		for (auto& e : tr[z].out) proto << e << " ";
		proto << "</td>";
		proto << "<td>" << ((corr[z]) ? "+" : "-") << "</td>";
		proto << "</tr>\n";
	}
	proto << "</table>";
	proto << "</body>\n</html>\n";
	proto.close();
}

int CalcRating(std::vector<TrainElem<int>> tr, BYTECODE& p){
	FortEmulator FE;
	int res = 0;
	for (int i = 0; i < tr.size(); ++i){
		FE.mem_clear();
		FE.mem_set(tr[i].in);
		if (FE.emulator(p) == 0){
			if (FE.check_result(tr[i].out)){
				res++;
			}
		}
	}
	return res;
}

int CalcRating(std::vector<TrainElem<int>> tr, BYTECODE& p, std::vector<bool>& R){
	FortEmulator FE;
	int res = 0;
	for (int i = 0; i < tr.size(); ++i){
		FE.mem_clear();
		FE.mem_set(tr[i].in);
		if (FE.emulator(p) == 0){
			if (FE.check_result(tr[i].out)){
				R.push_back(1);
				res++;
			}
			else {
				R.push_back(0);
			}
		}
	}
	return res;
}


int testing_code(FortEmulator& FE, const TrainSubset<int>& tss, std::vector<BYTE>& bc){
	int res = 0;
	for (auto& elem : tss.ss){
		FE.mem_set(elem.in);
		FE.CClear();
		if (FE.emulator(bc) == 0){
			if (FE.check_result(elem.out)){
				res++;
			}
		}
	}
	return res;
}
