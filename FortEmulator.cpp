// mfort.cpp 2018-02-27 минифорт
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#include "FortEmulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>		// for vError
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "utility.h"

#pragma warning(disable : 4996)

using namespace std;

//-----------------------------------------------------------------------------
void vError(const char *format, ...) // show error function
{
	va_list argp;
	va_start(argp, format);
	fprintf(stderr, "\nvError: ");
	vfprintf(stderr, format, argp);
	va_end(argp);
	fprintf(stderr, "\nPress <Enter>\n");
	getchar();
	_exit(1);
}

//-----------------------------------------------------------------------------
void FortEmulator::dump() {
	for (int i = 0; i <= SP; i++) std::cout << stack[i] <<" ";
	std::cout << "\n";
}


std::string FortEmulator::WName(BYTE b) {	// имя командны b
	if (b >= NWords) vError("StDepth(%d)", b);
	return names[b];
}

//-----------------------------------------------------------------------------
BYTE FortEmulator::FIndex(const char *word)// индекс слова с именем word, 255 если такого слова нет
{
	for (int i = 1; i < NWords; i++)
		if (strcmp(word, names[i].c_str()) == 0) {
			return i;
		}
	return 255;
}
//-----------------------------------------------------------------------------
bool FortEmulator::isSecond(BYTE *pr, int i)	// pr[i] - второй байт двухбайтовой команды
{
	if (i == 0) return false;
	return pr[i - 1] <= 3;		// двухбайтовые команды имеют номера 1,2,3
}

//-----------------------------------------------------------------------------
BYTE FortEmulator::addWord(char *name, BYTE *pr)	// добавить в словарь слово pr
{
	if (NWords == maxWords) vError("addWord: iWords=maxWords = %d", IWords);
	int L = (int)strlen((char*)pr);
	memcpy(FW[NWords], pr, L);
	words[NWords] = [](){return 0; };			// соответствующая встроенная команда, её просто нет!
	char *s = new char[strlen(name) + 1];
	strcpy(s, name);
	names[NWords] = s;
	NWords++;
	return NWords - 1;
}
//-----------------------------------------------------------------------------
BYTE FortEmulator::addWordT(const char *name, const char *str)	// добавить в словарь слово из текстовой строки str
{
	if (NWords == maxWords) vError("addWord: iWords=maxWords = %d", IWords);
	char *name1 = (char *)name;
	if (strlen(name) == 0) {		// если имя пустое, возьмём его из pr
		if( str[0] != ':') vError("addWord: illegal word %s", str);
		const char *nm = str+1;
		while (*nm == ' ') nm++;	
		// теперь nm указывает на первый не пробел после ':'
		int len;
		for (len = 1; len < 99; len++)
			if (nm[len] == ' ')break;
		// len - длина имени
		name1 = new char[len + 1];
		strncpy(name1, nm, len);
		name1[len] = 0;
		str = nm + len + 1;
	}

	int L = string2pr(str, FW[NWords]);
	//char sgn[maxWLen];
	char *s = new char[strlen(name1) + 1];
	strcpy(s, name1);
	names[NWords] = s;
	NWords++;
	return NWords - 1;
}
//-----------------------------------------------------------------------------
std::string FortEmulator::pr2string(BYTE *pr)	// Форт-программу в текст
{
	std::string s("");
	BYTE b;
	char buf[16];
	while (b = *pr) {
		if (b >= NWords){
			return "NULL";
		}// vError("pr2string: byte=%d", b);
		s += (names[b] + " ");
		if (b <= 3) {
			pr++;
			int k = *pr;
			if (k >= 128) k -= 256;
			sprintf(buf, "%d", k);
			s += (std::string(buf) + std::to_string(strlen(buf)) + " ");
		}
		pr++;
	}
	s += ";";
	return s;
}
std::string FortEmulator::pr2string(BYTECODE &pr)	// Форт-программу в текст
{
	BYTE b;
	std::string s("");
	for (int i = 0; i < pr.size(); i++) {
		b = pr[i];
		if (b == 0) break;
		s += names[b] + " ";
		if (b <= 3) {
			int k = pr[i+1];
			s += std::to_string(k) + " ";
			i++;
		}
	}
	s += ";";
	return s;
}
//-----------------------------------------------------------------------------
int FortEmulator::string2pr(const std::string& str, BYTE *pr)	// из строки str получить программу pr, возваращает длину
{
	char *sep = " \t\n\r";
	char *w1 = strtok((char*)str.c_str(), sep);
	BYTE *p = pr;
	int L = 0;					// длина программы
	while (w1 != NULL) {
		if (strcmp(w1, ";") == 0) break;		// конец программы
		// найдем номер слова:
		BYTE iw = FIndex(w1);
		if (iw == 255) vError("addWord: words %s not found", w1);
		*p++ = iw;
		L++;
		if (iw <= 3) {  // команда двухбайтовая
			L++;
			w1 = strtok(NULL, sep);
			int V;
			if (w1[0] == '0' && w1[1] == 0) V = 0;
			else {
				V = atoi(w1);
				if (V == 0) vError("addWord: words %s not found", w1);
			}
			*p++ = V;
		}
		if (L == maxWLen) vError("addWord: program too long");
		w1 = strtok(NULL, sep);
	}// while (w1 != NULL)
	*p = 0;	// добавим признак конца
	return L;
}

void FortEmulator::CClear() { MFcount = 0; }		// очистить счётчик
int  FortEmulator::CGet() { return MFcount; }		// текущее значение счётчика
// установить максимальное значение счётчика
// определяет максимальное число операций для программы
void FortEmulator::CMaxSet(int m) { MF_max = m; }	
//-----------------------------------------------------------------------------
void FortEmulator::mem_clear()		// очистить стек
{
	for (auto&e : stack) e = 0;
	SP = -1;
}
//-----------------------------------------------------------------------------
int FortEmulator::c_exec(BYTE *pr)	// Выполнить программу. Возвращает код ошибки
{
	BYTE *pr0 = pr;		// начало программы
	BYTE b;
	int code;
	while (b = *pr++) {	// b - текущая команда, pr указывает на следующую
		if (MFcount++ >= MF_max) return 1;
		if (b >= NWords) return -1;// vError("c_exec: b = %d", b);
		//printf("%-8s", names[b]); dotS();
		if (b == 1) {// 1: Безусловный переход по заданному адресу. Команда состоит из 2-х байт:
			//char dist = *pr;
			pr = pr0+*(pr+1);
			//pr = *(pr+1);		
			continue;
		}
		if (b == 2) {// 2: Условный переход (вершина стека ?0) по заданному адресу. Команда состоит из 2-х байт:
			if (SP < (int)stack.size() && SP >= 0){
				if (stack[SP] != 0) {
//					char dist = *pr;
//					pr += dist;
					pr = pr0 + *(pr);
					//pr = pr0 + (*pr);
				}
				else pr++;
				SP--;
				continue;
			}
		}
		if (b == 3) {// 3: Кладем в стек числовой литерал (-128..127). Команда состоит из 2-х байт:
			if (SP >= (int)stack.size() - 1) return -1;		// стек исчерпан
			stack[++SP] = *pr++;
			continue;
		}
		if (b < IWords) {// встроенное слово	
			if (code = words[b]()) return code;
			continue;
		}
		// теперь b - Fort-слово
		if (code = c_exec(FW[b])) return code;		// рекурсивный вызов
		continue;
	}
	return 0;
}

int FortEmulator::emulator(const std::string& str){
	BYTE pr[maxWLen];
	string2pr(str.c_str(), pr);
	return c_exec(pr);//s_exec(str.c_str());
}


int FortEmulator::emulator(std::vector<BYTE>& bytecode){
	return c_exec(&*bytecode.begin());
}
//-----------------------------------------------------------------------------

void FortEmulator::mem_set(const std::vector<int>& a) 	// заполнить стек
{
	for (int i = 0; i < (int)a.size(); i++)
		stack[i] = a[i];
	SP = a.size() - 1;
}

bool FortEmulator::check_result(const std::vector<int>& a){
	//if (stack.size() != a.size()) return false;
	if (SP != a.size() - 1) return false;
	for (int i = 0; i < a.size(); i++){
		if (stack[i] != a[i]) return false;
	}
	return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//

void FortEmulator::Init(){

	// Встроенные команды ---------------------------------------------------------

	auto c_NULL = []()->int { return 0;	};

	auto c_PP = [&]()->int {	
		if (SP < 0) return 32;
		++stack[SP];
		return 0;
	};

	auto c_MM = [&]()->int {
		if (SP <0) return 33;
		--stack[SP];
		return 0;
	};

	auto c_DUP = [&]() {
		if (SP < 0 || SP >= stack.size()-1){
			return 4; // error("c_DUP: stack");
		}
		stack[SP + 1] = stack[SP];
		SP++;
		return 0;
	};

	auto c_DROP = [&]() {
		if (SP < 0) return 5; // error("c_DROP: stack");
		--SP;
		return 0;
	};

	auto c_SWAP = [&]() {
		if (SP < 1) return 6; // error("c_SWAP: stack");
		int t = stack[SP];
		stack[SP] = stack[SP - 1];
		stack[SP - 1] = t;
		return 0;
	};

	auto c_OVER= [&]() {
		if (SP <1 || SP >= stack.size()-1) return 7; // error("c_OVER: stack");
		stack[SP + 1] = stack[SP - 1];
		SP++;
		return 0;
	};

	auto c_ROT =[&]() {
		if (SP < 2) return 8; // error("c_ROT: stack");
		int t = stack[SP - 2];
		stack[SP - 2] = stack[SP - 1];
		stack[SP - 1] = stack[SP];
		stack[SP] = t;
		return 0;
	};

	auto c_MROT = [&]() {
		if (SP <2) return 9; // error("c_MROT: stack");
		int t = stack[SP - 2];
		stack[SP - 2] = stack[SP];
		stack[SP] = stack[SP - 1];
		stack[SP - 1] = t;
		return 0;
	};

	auto c_PICK = [&]() {
		if (SP <1) return 10; // error("c_PICK: stack");
		int n = stack[SP];
		if (n <2) return 10; // error("c_PICK: stack");
		if (SP <= n) return 10; // error("c_PICK: stack");
		if (SP - n >stack.size()) return 10; // error("c_PICK: stack");
		stack[SP] = stack[SP - n - 1];
		return 0;
	};

	auto c_ROLL = [&]() {
		if (SP <2) return 10; // error("c_PICK: stack");
		int n = stack[SP];
		if (n < 2) return 11; // error("c_ROLL: stack");
		--SP;
		if (SP < n) return 11; // error("c_ROLL: stack");
		int t = stack[SP - n];
		for (int i = n; i > 0; --i) stack[SP - i] = stack[SP - i + 1];
		stack[SP] = t;
		return 0;
	};

	auto c_2PICK = [&]() {
		if (SP < 2 || SP >= stack.size()-1) return 10;
		stack[SP + 1] = stack[SP - 2];
		SP++;
		return 0;
	};

	auto c_3PICK = [&]() {
		if (SP < 3 || SP >= stack.size()-1) return 10;
		stack[SP + 1] = stack[SP - 3];
		SP++;
		return 0;
	};

	auto c_4PICK = [&]() {
		if (SP < 4 || SP >= stack.size()-1) return 10;
		stack[SP + 1] = stack[SP - 4];
		SP++;
		return 0;
	};

	auto c_3ROLL = [&]() {
		if (SP < 3) return 11; // error("c_ROLL: stack");
		int t = stack[SP - 3];
		for (int i = 3; i > 0; --i) stack[SP - i] = stack[SP - i + 1];
		stack[SP] = t;
		return 0;
	};

	auto c_4ROLL = [&]() {
		if (SP < 4) return 11; // error("c_ROLL: stack");
		int t = stack[SP - 4];
		for (int i = 4; i > 0; --i) stack[SP - i] = stack[SP - i + 1];
		stack[SP] = t;
		return 0;
	};

	// Арифметика
	auto c_NEG = [&]() {
		if (SP < 0) return 12;
		stack[SP] = -stack[SP];
		return 0;
	};

	auto c_ADD = [&]() {
		if (SP < 1) return 13;
		stack[SP - 1] += stack[SP];
		--SP;
		return 0;
	};

	auto c_SUB = [&]() {
		if (SP <1) return 14;
		stack[SP - 1] -= stack[SP];
		SP--;
		return 0;
	};

	auto c_MUL = [&]() {
		if (SP <1) return 15;
		stack[SP - 1] *= stack[SP];
		SP--;
		return 0;
	};

	auto c_DIV = [&]() {
		if (SP <1) return 19;
		int q = stack[SP];
		if (q <= 0) return 19;
		stack[SP - 1] /= q;
		SP--;
		return 0;
	};

	auto c_MOD = [&]() {
		if (SP <1) return 20;
		int q = stack[SP];
		if (q <= 0) return 20;
		stack[SP - 1] %= q;
		SP--;
		return 0;
	};

	auto c_DIV_MOD = [&]() {
		if (SP <1) return 21;
		int a = stack[SP - 1];
		int b = stack[SP];
		if (b <= 0) return 21;
		stack[SP - 1] = a / b;
		stack[SP] = a%b;
		return 0;
	};

	// Битовые операции
	auto c_NOT = [&]() {
		if (SP <0) return 19;
		stack[SP] = ~stack[SP];
		return 0;
	};

	auto c_AND = [&]() {
		if (SP < 1) return 20;
		stack[SP - 1] &= stack[SP];
		SP--;
		return 0;
	};

	auto c_OR = [&]() {
		if (SP <1) return 21;
		stack[SP - 1] ^= stack[SP];
		SP--;
		return 0;
	};

	auto c_XOR = [&]() {
		if (SP <1) return 21;
		stack[SP - 1] |= stack[SP];
		SP--;
		return 0;
	};

	// Логические операции (сравнения)
	auto c_GH = [&]() {
		if (SP <1) return 22;
		stack[SP - 1] = (stack[SP - 1]>stack[SP] ? 1 : 0);
		SP--;
		return 0;
	};

	auto c_LH = [&]() {
		if (SP <1) return 23;
		stack[SP - 1] = (stack[SP - 1]<stack[SP] ? 1 : 0);
		SP--;
		return 0;
	};

	auto c_EQ = [&]() {
		if (SP <1) return 24;
		stack[SP - 1] = (stack[SP - 1] == stack[SP] ? 1 : 0);
		SP--;
		return 0;
	};

	auto c_0EQ = [&]() {
		if (SP <0) return 25;
		stack[SP] = (stack[SP] == 0 ? 1 : 0);
		return 0;
	};

	auto c_0GT = [&]() {
		if (SP <0) return 25;
		stack[SP] = (stack[SP]>0 ? 1 : 0);
		return 0;
	};

	auto c_0LT = [&]() {
		if (SP <0) return 25;
		stack[SP] = (stack[SP]<0 ? 1 : 0);
		return 0;
	};
	// сами встроенные функции, приращение стека, требуемая глубина стека стека
	//             0       1       2       3      4       5       6      7      8        9       10       11       12       13       14    15      16      17      18      19     20     21     22     23         24     25    26    27    28     29     30    31    32     33     34
	words = { { c_NULL, c_NULL, c_NULL, c_NULL, c_DUP, c_DROP, c_SWAP, c_OVER, c_ROT, c_MROT, c_PICK, c_2PICK, c_3PICK, c_4PICK, c_ROLL, c_3ROLL, c_4ROLL, c_NEG, c_ADD, c_SUB, c_MUL, c_DIV, c_MOD, c_DIV_MOD, c_AND, c_OR, c_XOR, c_GH, c_LH, c_EQ, c_0EQ, c_0GT, c_0LT, c_PP, c_MM, c_NULL } };
	names = { { "NULL", "GOTO", "IF",  "CONST", "DUP", "DROP",  "SWAP", "OVER", "ROT", "-ROT", "PICK", "2PICK", "3PICK", "4PICK", "ROLL" ,"3ROLL", "4ROLL", "NEG", "+",  "-",   "*",   "/",   "%",   "/%",      "AND",  "OR", "XOR", ">", "<",   "=",  "=0", ">0",  "<0",  "++",  "--", "NULL" } };
	adm_all();			// установить полный допустимых список
}

void FortEmulator::SetUsingCommand(std::vector<std::string>& c){
	std::vector<std::function<int()>> new_words;
	std::array<std::string, maxWords> new_names;
	int indx = 0;
	for (auto& e : c){
		std::transform(e.begin(), e.end(), e.begin(), ::toupper);
		for (int i = 0; i < names.size(); ++i){
			if (e == names[i]){
				bool f = false;
				for (int z = 0; z < indx; ++z){
					if (new_names[z] == e) {
						f = true;
						break;
					}
				}
				if (f) continue;
				new_names[indx] = names[i];
				new_words.push_back(words[i]);
				++indx;
			}
		}
	}
	if (!new_words.empty()){
		std::swap(new_words, words);
		std::swap(new_names, names);
		IWords = NWords = words.size();
	}
	else
		throw std::runtime_error("Error: available instruction list is empty!");

/*	pIC.clear();
	for (int i = 0; i < words.size(); ++i){
		pIC[names[i]] = i;
	}*/
}

// Если bc.size()==5, то
// bc[4] обязательно равно 0,
// bc[3] - последняя команда, sz = 3!
bool FortEmulator::checkCode(BYTECODE& bc){
	unsigned sz = bc.size() - 2;					// индекс последней команды
	if (bc[sz+1] != 0) return false;				// последний байт должен быть 0!
	for (unsigned p = 0; p <= sz; ++p){
		BYTE bcp = bc[p];							// код команды
		if (bcp == 0) return false;					// нулей внутри функции быть не должно
		if (bcp == 3) {								// "CONST", константа при переборе
			++p;
			continue;
		}					
		if (bcp < 4) {								// так как это не 0 и не 3, то 1 или 2, то есть GOTO или IF
			if (p == sz) return false;				// последняя команда не может быть переходом
			BYTE b1 = bc[p + 1];					// куда переходим
			if (b1 == p) return false;				// нельзя переходить на самого себя
			if (b1 > sz+1) return false;			// нельзя переходить за пределы функции. На последний 0 переходить можно.
			if (b1 > 0 && bc[b1 - 1] < 4) return false;// нельзя переходить на второй байт двухбайтовой команды
			p++;									// пропускаем второй байт двухбайтовой команды
		}
		if (p>0){
			// Запрещаем пары операций не влияющих на результат работы программы
			if (names[bcp] == "++" && names[bc[p - 1]] == "--") return false; // ++ --
			if (names[bcp] == "--" && names[bc[p - 1]] == "++") return false; // -- ++
			if (names[bcp] == "DROP" && names[bc[p - 1]] == "DUP") return false; // DUP DROP
			if (bcp == bc[p - 1] && names[bcp] == "SWAP") return false; // SWAP SWAP
			if (names[bcp] == "ROT" && names[bc[p - 1]] == "-ROT") return false; // -ROT ROT
			if (names[bcp] == "-ROT" && names[bc[p - 1]] == "ROT") return false; // ROT -ROT
			if (bcp == bc[p - 1] && names[bcp] == "NEG") return false; // NEG NEG
		}
	}
	return true;
}

template<>
int FortEmulator::CalcRating(std::vector<TrainElem<int>> tr, BYTECODE& p){
	int res = 0;
	for (int i = 0; i < tr.size(); ++i){
		mem_clear();
		mem_set(tr[i].in);
		if (emulator(p) == 0){
			if (check_result(tr[i].out)){
				res++;
			}
		}
	}
	return res;
}

template<>
int FortEmulator::CalcRating(std::vector<TrainElem<int>> tr, BYTECODE& p, std::vector<bool>& R){
	int res = 0;
	for (int i = 0; i < tr.size(); ++i){
		CClear();
		mem_clear();
		mem_set(tr[i].in);
		if (emulator(p) == 0){
			if (check_result(tr[i].out)){
				R.push_back(true);
				res++;
			}
			else {
				R.push_back(false);
			}
		}
	}
	return res;
}

template<>
int FortEmulator::testing_code(const TrainSubset<int>& tss, std::vector<BYTE>& bc){
	int res = 0;
	for (auto& elem : tss.ss){
		mem_set(elem.in);
		CClear();
		if (emulator(bc) != 0) {	// ошибка выполнения. Другие тесты не будем проверять!
			return 0;
		}
		if (check_result(elem.out))
			res++;
	}
	return res;
}


//---- список допустимых слов ---------------------------------------------
//BYTECODE adm;									// список допустимых слов
void FortEmulator::adm_all()					// установить полный список
{
	adm.clear();
	for (BYTE i = 1; i < NWords; i++)
		adm.push_back(i);
}
void FortEmulator::adm_add(BYTE b)				// добавить одно слово
{
	adm.push_back(b);
}
void FortEmulator::adm_set(std::string &s)		// установить список слов из строки
{
	adm.clear();
	std::vector<std::string> ww;				// список слов
	split(s, " ", ww);							// ww := список слов
	for (unsigned i = 0; i < ww.size(); i++) {
		BYTE b = FIndex(ww[i].c_str());
		if (b == 255) vError("FortEmulator::adm_set, Illegal word %s", ww[i].c_str());
		//std::cout << i << " " << (0+b) << "\n";
		adm.push_back(b);
	}
	std::cout << "admit: " << pr2string(adm) << "\n";

}
//-----------------------------------------------------------------------------
void FortEmulator::adm_add(std::vector<std::string> ww)		// добавить новые слова из текста
{
	std::vector<std::string> v;
	for (unsigned i = 0; i < ww.size(); i++) {
		if (ww[i][0] != ':') vError("FortEmulator::adm_add, illegal definition <%s>", ww[i].c_str());
		BYTE b = addWordT("", ww[i].c_str());
		adm_add(b);
	}

}