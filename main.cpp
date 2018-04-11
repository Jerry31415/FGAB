#pragma warning(disable : 4018)

#include <thread>
#include <mutex>
#include <chrono>
#include "TSG.h"
#include "FortEmulator.h"
#include "utility.h"
//#include "generator.h"

#define M 10000000		// 10 mln
#define MAXC 100
bool ready_flag;
std::mutex v_lock;
BYTECODE bc_tmp;
std::string admit;
std::vector<std::string> w_add;				// список дополнительных слов

void SaveHist2D(std::string name, const std::vector<int>& H, FortEmulator& FE){
	int n = FE.NWords;
	std::ofstream f(name.c_str());
	f << "Str - first, col - second\n";
	for (int i = 0; i < n; ++i){
		f << " " << std::string(FE.names[i]);
	}
	f << "\n";
	for (int i = 0; i < n; ++i){
		f << std::string(FE.names[i]) << " ";
		for (int j = 0; j < n; ++j){
			f << H[i*n + j] << " ";
		}
		f << "\n";
	}
	f.close();
}

// Функция перебора
unsigned __int64 FindAlgForTrainElems(TrainSubset<int> tss, std::vector<TrainElem<int>> tr, 
	BYTECODE& code, int z = 0, unsigned __int64 beg = 0, unsigned __int64 step = 1){
	std::vector<BYTE> bc;
	FortEmulator FE;
	FE.CMaxSet(MAXC);
	// : GCD_1         SWAP OVER % DUP IF -5 DROP ;
	//FE.adm_set(std::string("IF GOTO OVER % DUP DROP =0 SWAP"));
	FE.adm_set(admit);
	FE.adm_add(w_add);
	int N = FE.NWords;
	int corr = 0;
	int cnt_speed = 0;
	int cnt_m = 0;
	int NH2D = (N);
	std::vector<int> H2D(NH2D*NH2D, 0);
	std::vector<int> H(FE.NWords, 0);

	//beg = 1312983;
	int corr_max = 2;		// максимальное количество прошедших тестов
	for (unsigned __int64 i(beg); i < _UI64_MAX; i += step, ++cnt_speed){
		//if( (i&0xfffff) == 0) 
		//	std::cout << "i= " << (i>>10) << "K  \r";

		if (ready_flag){
			std::cerr << "T(" << z << ") - ready\n";
			return 0;
		}
		// преобразует число i в систему счисления по основанию N.
		// рузультат записывает в bc (байткод)
		toSS(i, FE.adm_N(), bc, FE.adm);
		//std::cout << FE.pr2string(bc) << "\n";
		
		if (!FE.checkCode(bc)) continue;
		if (cnt_speed>=M){
			++cnt_m;
			cnt_speed = 0;
			//std::cerr << "           A" << z << "(" << ((cnt_m*M) >> 20) << "MA, " << bc.size() << "):     ";
			std::cerr << "           A" << z << "(" << (i>> 20) << "M, " << bc.size() << "):     ";
			std::cerr << FE.pr2string(bc) << "\n";
		}	
		corr = FE.testing_code(tss, bc);
		if (corr >= corr_max) {			// если результаты на программе bc рекордные

			for (int kk = 0; kk < bc.size()-1;++kk){
				if (kk >= 1){
					if (bc[kk - 1] < 4) continue;
					int y = bc[kk - 1] ;
					int x = bc[kk] ;
			
					H2D[y*NH2D+x]++;
				}
				H[bc[kk]]++;
			}
			SaveHist2D("H2_"+std::to_string(z) + ".txt", H2D, FE);

			corr_max = corr;
			std::cout << "record("<<corr<<"): "<< FE.pr2string(bc) << "\tH=(";
			std::vector<bool> R;
			FE.CalcRating(tr, bc, R);
			for (auto& e : H) std::cout << e << " ";
			std::cout << ")\n";
			// Для отладки, пройдем программу ещё раз:
			//corr = FE.testing_code(tss, bc);
		}
		if (corr == tss.size()){
			v_lock.lock();
			ready_flag = true;
			code.clear();
			for (auto e : bc){
				bc_tmp.push_back(e);
				code.push_back(e);
			}
			v_lock.unlock();
			std::cerr << "T(" << z << ") - ready\n";
			std::cerr << "i=" << i << "\n";
			return i;
		}
	}
	return 0;
}

// Многопоточный перебор
unsigned __int64 FindAlgForTrainElemsMTH(const TrainSubset<int>& tss, const std::vector<TrainElem<int>>& tr, BYTECODE& code, int z = 0, unsigned __int64 beg = 0, int N = 2, int step = 2){
	ready_flag = false;
	BYTECODE bc;
	std::vector<std::thread> TH;
	for (int i = 0; i < N; ++i){
			TH.push_back(std::thread(&FindAlgForTrainElems, tss, tr, bc, i, beg + i, step));
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



int main(int argc, char *argv[]){
	FortEmulator FE;
	int CPU_value=1;
	std::vector<TrainElem<int>> tr;
	std::vector<TrainSubset<int>> tss2;
	//GenDetermTrainFile(64, "train.txt");
	//exit(0);
	//GenMaxTrainFile(16, "train.txt");

	if (argc < 3) vError("argc(%d) != 3", argc);
	if (argc == 4){
		CPU_value = getUseCoresNum(std::stoi(argv[3]));
		std::cerr << "Set CPU load: " << std::stoi(argv[3]) << "% " << CPU_value << " cores\n";
	}
	
	// читаем список допустиых слов из файла argv[1] в глобальную строку admit
	std::ifstream f(argv[1]);
	if (!f.is_open()) {
		std::cerr << "Error : file " << argv[1] << " - not found!\n";
		//std::cin.get();
		exit(-1);
	}
	std::getline(f, admit);
	// теперь прочитаем дополнительные слова из того же файла
	w_add.clear();
	std::string str;
	while (std::getline(f, str)) {
		w_add.push_back(str);
	}
	f.close();
	// список допустиых и дополнительных слов прочитан

	// читаем тесты из файла argv[2]
	//ReadTrainData("train.txt", tr);
	ReadTrainData(argv[2], tr);
	BuildTrainSubset(tr, tss2, tr.size());
	std::cerr << "We have read " << tr.size() << " tests\n";

	std::vector<BYTECODE> code2;
	BYTECODE tmp;

	unsigned __int64 beg = 0;
	for (int i = 0; i < tss2.size(); ++i){
		//std::cout << "Find for(" << (i + 1) << "/" << tss2.size() << "):\n";
		//tss2[i].show();
		//std::cout << "\n";
		std::cerr << "Find for(" << (i + 1) << "/" << tss2.size() << "):\n";
		tss2[i].show(std::cerr);
		std::cerr << "\n";
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
		int R = FE.CalcRating(tr, tmp, corr);
		//WriteProtocol(i, tmp, tss2, corr, tr, R, FE);
		std::cout << "Ready: " << (i + 1) << "/" << tss2.size() << " R="<<R<<"\n";
		//std::cout << "code:";
		//for (auto& e : tmp) std::cout << FE.names[(int)e] << " ";
		std::cout << "code:              " << FE.pr2string(tmp) << "\n";
		code2.push_back(tmp);
		if (R == tr.size()) break;
	}

	std::vector<int> R;
	for (auto& e:code2){
		R.push_back(FE.CalcRating(tr, e));
	}

	for (int i = 0; i < code2.size(); ++i){
		std::cerr << "R["<<i<<"]=" << R[i] << "/" << tr.size() << " A" << i << ":       ";
		//for (auto& e : code2[i]) std::cout << (int)e << " ";
		//std::cout << "\n";
		std::cerr << FE.pr2string(code2[i]) << "\n";
	}

	//std::cin.get();
}

/*
1. Протокол
2. Доступный список команд
3. IFL, IFR, GOTOL, GOTOR пере
*/