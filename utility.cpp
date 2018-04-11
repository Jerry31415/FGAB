#pragma warning(disable : 4018)
#pragma warning(disable : 4244)


#include <algorithm>
#include <Windows.h>
#include "utility.h"
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

// res должен заканчиваться нулём
void toSS(const unsigned __int64& v, const int& n, std::vector<BYTE>& res, std::vector<BYTE>& adm){
	//register unsigned __int64 div, r, val(v);
	//if (val < n) {
	//	res.push_back(val);
	//	res.push_back(0);
	//	return;
	//}

	__int64 val = v;
	res.clear();
	bool tb = false;				// признак того, что предыдущая команда двухбайтовая
	do{
		BYTE a = val%n;				// номер слова в списке adm
		if (tb) {					// предыдущая команда двухбайтовая
			res.push_back(a);
			tb = false;
		}
		else {
			BYTE b = adm[a];			// номер Форт-слова
			tb = b < 4 ? true : false;
			res.push_back(b);
		}
		val /= n;
	} while (val > 0);
	res.push_back(0);
}

std::string getValueByTag(const std::string& str, const std::string& tag){
	size_t pos_a = str.find("<" + tag + ">");
	size_t pos_b = str.find("</" + tag + ">");
	if (pos_a != std::string::npos && pos_b != std::string::npos){
		return str.substr(pos_a + tag.size() + 2, pos_b - (pos_a + tag.size() + 2));
	}
	return "";
}

void split(const std::string& str, const std::string& split_symbol, std::vector<std::string>& v){
	//int p = 0;
	//for (int i = 1; i < str.size(); ++i){
	//	if (str.substr(i, 1) == split_symbol.c_str()){
	//		if( i != p)
	//			v.push_back(str.substr(p, i - p));
	//		p = i + 1;
	//	}
	//}
	//if( str.substr(p).length() >0)
	//v.push_back(str.substr(p));
	v.clear();
	std::string token;

	for_each(str.begin(), str.end(), [&](char c) {
		if (!isspace(c))
			token += c;
		else
		{
			if (token.length()) v.push_back(token);
			token.clear();
		}
	});
	if (token.length()) v.push_back(token);
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
		split(in_tmp, " ", tmp);
		for (auto& e : tmp) elem.in.push_back(std::stoi(e));
		out_tmp = getValueByTag(str, "out");
		split(out_tmp, " ", tmp);
		for (auto& e : tmp) elem.out.push_back(std::stoi(e));
		if (elem.in.empty() && elem.out.empty()) continue;
		tr.push_back(elem);
	}
	f.close();
}

void WriteProtocol(int index, const BYTECODE& bc, const std::vector<TrainSubset<int>>& tr_subset, const std::vector<bool>& corr, 
	const std::vector<TrainElem<int>>& tr, int Rating, FortEmulator& FE){
	std::ofstream proto("proto" + std::to_string(index + 1) + ".html");
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

/*
void Selection(std::vector<BYTECODE>& bc, std::vector<TrainElem<int>>& tr){
	std::vector<int> R;
	int max_R = -1;
	for (auto& e : bc) R.push_back(CalcRating(tr, e));
	for (auto& e : R) {
		if (e > max_R) max_R = e;
	}
	std::vector<BYTECODE> new_bc;
	for (int i = 0; i < bc.size(); ++i){
		if (R[i] < max_R) continue;
		new_bc.push_back(bc[i]);
	}
	bc.clear();
	for (auto& e : new_bc) bc.push_back(e);
}
*/
