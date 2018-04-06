#include "TSG.h"
#include "FortEmulator.h"
#include "utility.h"
#include "generator.h"
#include <thread>
#include <mutex>
#include <chrono>

#define M 1000000
#define MAXC 32
bool ready_flag;
std::mutex v_lock;
BYTECODE bc_tmp;

// Функция перебора
unsigned __int64 FindAlgForTrainElems(TrainSubset<int> tss, std::vector<TrainElem<int>> tr, BYTECODE& code, int z = 0, unsigned __int64 beg = 0, unsigned __int64 step = 1){
	std::vector<BYTE> bc;
	FortEmulator FE;
	FE.CMaxSet(MAXC);
	int N = FE.NWords;
	int corr = 0;
	int cnt_speed = 0;
	int cnt_m = 0;
	for (unsigned __int64 i(beg); i < _UI64_MAX; i += step, ++cnt_speed){
		if (ready_flag){
			std::cout << "T(" << z << ") - ready\n";
			return 0;
		}
		// преобразует число i в систему счисления по основанию N.
		// рузультат записывает в bc (байткод)
		toSS(i, N, bc);
		
		if (!FE.checkCode(bc)) continue;
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

// Многопоточный перебор
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



int main(){
	FortEmulator FE;
	int CPU_value = getUseCoresNum(100);
	std::vector<TrainElem<int>> tr;
	std::vector<TrainSubset<int>> tss2;

	GenMaxTrainFile(16, "train.txt");
	ReadTrainData("train.txt", tr);
	BuildTrainSubset(tr, tss2, 2);

	std::vector<BYTECODE> code2;
	BYTECODE tmp;

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
		WriteProtocol(i, tmp, tss2, corr, tr, R, FE);
		std::cout << "Ready: " << (i + 1) << "/" << tss2.size() << " R="<<R<<"\n";
		std::cout << "code:";
		for (auto& e : tmp) std::cout << FE.names[(int)e] << " ";
		std::cout << "\n";
		code2.push_back(tmp);
		if (R == tr.size()) break;
	}

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
*/