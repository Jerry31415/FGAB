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

template<size_t SIZE>
bool isExist(const BYTECODE& bc, const std::array<BYTE, SIZE>& code){
	for (int i = 0; i < bc.size(); ++i){
		for (int j = 0; j < code.size(); ++j)
			if (bc[i] == code[j]) return true;
	}
	return false;
}


unsigned __int64 FindAlgForTrainElems(TrainSubset<int> tss, std::vector<TrainElem<int>> tr, BYTECODE& code, int z = 0, unsigned __int64 beg = 0, unsigned __int64 step = 1){
	std::vector<BYTE> bc;
	FortEmulator FE;
	//FE.addWord("SD", "SWAP DROP");
	//FE.addWord("O3RG", "OVER 3ROLL >");

	FE.CMaxSet(32);
	//FE.SetUsingCommand(usedCommand);
	int N = FE.NWords;
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

	//	bc.clear();
	//	bc = { { 5, 6, 15, 2, 5, 4} };
	//	bc = { { 4, 4 } };

		/*
		bc.insert(bc.begin(), 7);
		bc.push_back(15);
		bc.push_back(3);
		bc.push_back(bc.size()+2);
		bc.push_back(6);
		bc.push_back(5);
		*/
		if (!FE.checkCode(bc)) continue;

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


unsigned __int64 FindAlgForTrainElemsMTH(const TrainSubset<int>& tss, const std::vector<TrainElem<int>>& tr, BYTECODE& code, int z = 0, unsigned __int64 beg = 0, int N = 2){
	ready_flag = false;
	BYTECODE bc;
	std::vector<std::thread> TH;
	for (int i = 0; i < N; ++i){
			TH.push_back(std::thread(&FindAlgForTrainElems, tss, tr, bc, i, beg + i, N));
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
	int CPU_value = 2;// getUseCoresNum(100);
	std::vector<TrainElem<int>> tr;
	GenMaxTrainFile(16, "train.txt");
	//GenMaxTrainFile(128);

	ReadTrainData("train.txt", tr);
	std::vector<TrainSubset<int>> tss2;
	BuildTrainSubset(tr, tss2, 4);
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


	//GOTO SWAP DRAP
	//FE.addWord("SD", "SWAP DROP");
	//FE.addWord("O3RG", "OVER 3ROLL >");

	//	FE.SetUsingCommand(usedCommand);
//	FE.addWord("F0", "GOTO 6 DROP", 0);
	//"NULL", "GOTO", "IF", "CONST", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "2PICK", "3PICK", "4PICK", "3ROLL", "4ROLL", "NEG", "+", "-", "*", "/", "%", "/%", "AND", "OR", "XOR", ">", "<", "=", "=0", ">0", "<0", "++", "--", "NULL" } };
	unsigned __int64 beg = 0;
	for (int i = 0; i < tss2.size(); ++i){

		std::cout << "Find for(" << (i+1) << "/" << tss2.size() << "):\n";
		tss2[i].show();
		std::cout << "\n";
		tmp.clear();
		try{
			beg = FindAlgForTrainElemsMTH(tss2[i], tr, tmp, i, 0, CPU_value);
		}
		catch (std::runtime_error& re){
			std::cout << re.what() << "\n";
			std::cin.get();
			exit(-1);
		}

		
		std::vector<bool> corr;
		int R = CalcRating(tr, tmp, corr);
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

	//Selection(code2, tr, usedCommand);

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
1. Протокол
2. Доступный список команд
3. IFL, IFR, GOTOL, GOTOR пере

R=3/8 A0:5 6 15 4 4 0 SWAP OVER < DROP DROP NULL

R=3/8 A0:5 6 15 2 5 4 0  SWAP OVER < IF 5 DROP NULL
R=6/8 A1:9 5 15 2 5 4 0
R=5/8 A2:6 5 13 2 5 4 0
R=5/8 A3:6 5 13 2 5 4 0

R=3/8 A0:5 6 15 2 5 4 0
R=6/8 A1:9 5 15 2 5 4 0
R=5/8 A2:6 5 13 2 5 4 0
R=5/8 A3:6 5 13 2 5 4 0
*/