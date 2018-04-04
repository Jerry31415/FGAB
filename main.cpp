#include "TSG.h"
#include "FortEmulator.h"
#include "utility.h"
#include "generator.h"
#include <thread>
#include <mutex>
#include <chrono>

#define M 1000000

bool ready_flag;
std::mutex v_lock;
BYTECODE bc_tmp;

int testing_code (FortEmulator& FE, const TrainSubset<int>& tss, std::vector<BYTE>& bc){
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

int CalcRating(std::vector<TrainElem<int>> tr, std::string& p){
	FortEmulator FE;
	int res = 0;
	for (int i = 0; i < tr.size(); ++i){
		FE.mem_set(tr[i].in);
		if (FE.emulator(p) == 0){
			if (FE.check_result(tr[i].out)){
				res++;
			}
		}
	}
	return res;
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
	proto << "<p><b>Train subset:</b>\n<p>\n";
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
	proto << "<p><b>Correct results:" << Rating << "/" << tr.size() << "</b>\n";
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
		//proto << (corr[z]) ? "+\n" : "-\n";
	}
	proto << "</table>";
	proto << "</body>\n</html>\n";
	proto.close();
}


int CalcRating(std::vector<TrainElem<int>> tr, BYTECODE& p){
	FortEmulator FE;
	int res = 0;
	for (int i = 0; i < tr.size(); ++i){
		FE.mem_set(tr[i].in);
		if (FE.emulator(p) == 0){
			if (FE.check_result(tr[i].out)){
				res++;
			}
		}
	}
	return res;
}

int CalcRating(std::vector<TrainElem<int>> tr, BYTECODE& p, std::vector<std::string> usedCommand, std::vector<bool>& R){
	FortEmulator FE;
	FE.SetUsingCommand(usedCommand);
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

/*
bool isExist(const BYTECODE& bc, const BYTE& code){
	for (int i = 0; i < bc.size(); ++i) if (bc[i] == code) return true;
	return false;
}
*/
template<size_t SIZE>
bool isExist(const BYTECODE& bc, const std::array<BYTE, SIZE>& code){
	for (int i = 0; i < bc.size(); ++i){
		for (int j = 0; j < code.size(); ++j)
			if (bc[i] == code[j]) return true;
	}
	return false;
}

bool checkCode(BYTECODE& bc){
	int pnt;
	for (int p = 0; p < bc.size(); ++p){
		if (bc[p] == 3) {
			return false;
		}
		if (bc[p] == 0) return false;
		if (bc[p] == 1 || bc[p] == 2){
			if (p == bc.size() - 1) return false;
			pnt = bc[p + 1];
			if (pnt == p) return false;
			if (pnt >= bc.size()) return false;
			if (pnt > 0){
				if (bc[pnt - 1] < 4) return false;
			}
			++p;
		}
	}
	return true;
}

unsigned __int64 FindAlgForTrainElems(TrainSubset<int> tss, std::vector<TrainElem<int>> tr, std::vector<std::string> usedCommand, BYTECODE& code, int z = 0, unsigned __int64 beg = 0, unsigned __int64 step = 1){
	std::vector<BYTE> bc;
	FortEmulator FE;
	FE.CMaxSet(16);
	FE.SetUsingCommand(usedCommand);
	int N = FE.NWords;
	std::cout << N << "\n";
	int corr = 0;
	//N = 30;
	int cnt_speed = 0;
	int cnt_m = 0;
	for (unsigned __int64 i(beg); i < _UI64_MAX; i += step, ++cnt_speed){
		if (ready_flag){
			std::cout << "T(" << z << ") - ready\n";
			return 0;
		}
		toSS(i, N, bc);
		if (!checkCode(bc)) continue;
		//if (isExist<1>(bc, { {0} }));
		
//		if (isExist<16>(bc, { { 0, 1, 2, 3, 4, 11, 12, 13, 14, 18, 19, 20, 21, 22, 23, 24 } })) continue;
		bc.push_back(0);
		if (cnt_speed>=M){
			++cnt_m;
			cnt_speed = 0;
			std::cout << "A" << z << "(" << cnt_m << "MA, " << bc.size() << ")=";
			for (auto& e : bc){
				std::cout << (int)e << " ";
			}
			std::cout << "\n";
		}	
		corr = testing_code(FE, tss, bc);
		if (corr == tss.size()){
			v_lock.lock();
			ready_flag = true;
			code.clear();
			for (auto e : bc){
				bc_tmp.push_back(e);
				code.push_back(e);
			}
			v_lock.unlock();
			std::cout << "T(" << z << ") - ready\n";			
			return i;
		}
	}
}


unsigned __int64 FindAlgForTrainElemsMTH(const TrainSubset<int>& tss, const std::vector<TrainElem<int>>& tr, std::vector<std::string>& usedCommand, BYTECODE& code, int z = 0, unsigned __int64 beg = 0, int N = 2){
	ready_flag = false;
	BYTECODE bc;
	std::vector<std::thread> TH;
	for (int i = 0; i < N; ++i){
			TH.push_back(std::thread(&FindAlgForTrainElems, tss, tr, usedCommand, bc, i, beg + i, N));
	}
	for (int i = 0; i < N-1; ++i){
		TH[i].detach();
	}
	TH.back().join();
	std::cout << "ready - ok\n";
	if (bc_tmp.empty()){
		throw std::runtime_error("Error: code not found\n");
	}
	else{
		std::swap(code, bc_tmp);
	}
	return 0;
}

void Selection(std::vector<BYTECODE>& bc, std::vector<TrainElem<int>>& tr){
	std::vector<int> R;
	int max_R = -1;
	for (auto& e : bc) R.push_back(CalcRating(tr, e));
	for (auto& e : R) {
		if (e > max_R) max_R = e;
	}
	std::vector<BYTECODE> new_bc;
	for (int i = 0; i < bc.size();++i){
		if (R[i] < max_R) continue;
		new_bc.push_back(bc[i]);
	}
	bc.clear();
	for (auto& e : new_bc) bc.push_back(e);
}



int main(){
	FortEmulator FE;
	int CPU_value = getUseCoresNum(100);
	std::vector<TrainElem<int>> tr;
	GenMax3TrainFile(128, "train.txt");
	//GenMaxTrainFile(128);

	ReadTrainData("train.txt", tr);
	std::vector<TrainSubset<int>> tss2;
	BuildTrainSubset(tr, tss2, 2);
	/*
	for (int i = 0; i < tss2.size(); ++i){
		std::cout << "Group " << i << ":\n";
		tss2[i].show();
		std::cout << "\n";
	}
	*/
	std::cout << tr.size() << " " << tss2.size() << "\n";

	std::vector<BYTECODE> code2;
	BYTECODE tmp;
	std::vector<std::string> usedCommand = {{ "NULL", "GOTO", "IF", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "2PICK", "+", "-", "*", "/", ">", "<", "=", "=0", ">0", "<0"} };
	
	FE.SetUsingCommand(usedCommand);

	//"NULL", "GOTO", "IF", "CONST", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "2PICK", "3PICK", "4PICK", "3ROLL", "4ROLL", "NEG", "+", "-", "*", "/", "%", "/%", "AND", "OR", "XOR", ">", "<", "=", "=0", ">0", "<0", "++", "--", "NULL" } };
	unsigned __int64 beg = 0;
	for (int i = 0; i < tss2.size(); ++i){

		std::cout << "Find for(" << (i+1) << "/" << tss2.size() << "):\n";
		tss2[i].show();
		std::cout << "\n";
		tmp.clear();
		try{
			beg = FindAlgForTrainElemsMTH(tss2[i], tr, usedCommand, tmp, i, 0, CPU_value);
		}
		catch (std::runtime_error& re){
			std::cout << re.what() << "\n";
			std::cin.get();
			exit(-1);
		}

		
		std::vector<bool> corr;
		int R = CalcRating(tr, tmp, usedCommand, corr);
		// Write protocol
		WriteProtocol(i, tmp, tss2, corr, tr, R, FE);
		std::cout << "Ready: " << (i + 1) << "/" << tss2.size() << " R="<<R<<"\n";
		std::cout << "code:";
		for (auto& e : tmp) std::cout << FE.names[(int)e] << " ";
		std::cout << "\n";
		code2.push_back(tmp);
		//std::cin.get();
		if (R == tr.size()) break;
	}

	Selection(code2, tr);

	std::vector<int> R;
	for (auto& e:code2){
		R.push_back(CalcRating(tr, e));
	}

	for (int i = 0; i < code2.size(); ++i){
		std::cout << "R=" << R[i] << "/" << tr.size() << " A" << i << ":";
		for (auto& e : code2[i]) std::cout << (int)e << " ";
		std::cout << "\n";
	}

	std::cin.get();
}

/*
1. ��������
2. ��������� ������ ������
3. IFL, IFR, GOTOL, GOTOR ����

*/