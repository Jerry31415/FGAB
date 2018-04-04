// mfort.cpp 2018-02-27 минифорт

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

#pragma warning(disable : 4996)

using namespace std;

template<typename T>
std::string toString(const T& val){
	std::stringstream ss;
	ss << val;
	return ss.str();
}

//-----------------------------------------------------------------------------
void FortEmulator::vError(const char *format, ...) // show error function
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


int FortEmulator::StApp(BYTE b) {		// приращение стека командной b
	if (b >= NWords) return -1;//vError("StApp(%d)", b);
	return stApp[b];
}
int FortEmulator::StDepth(BYTE b) {	// требуемая глубина стека командны b
	if (b >= NWords) return -1; //vError("StDepth(%d)", b);
	return stDepth[b];
}
std::string FortEmulator::WName(BYTE b) {	// имя командны b
	if (b >= NWords) vError("StDepth(%d)", b);
	return names[b];
}
int FortEmulator::Tag(BYTE b, int t) {// tag[b]
	if (b >= NWords) vError("StDepth(%d)", b);
	if (t >= 0) tag[b] = t;
	return tag[b];
}
//-----------------------------------------------------------------------------
BYTE FortEmulator::FIndex(char *word)// индекс слова с именем word, 255 если такого слова нет
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
int FortEmulator::signature(BYTE *pr, char *sgn)// sgn[i] = приращение стека ПОСЛЕ i-й команды. Возвращает максимальную глубину стека
{
	char delta = 0;		// приращение стека ПОСЛЕ i-й команды
	char depth = 0;		// требуемая глубина стека для выполнения i-й команды
	while (*pr) {
		BYTE cmd = *pr;
		char dep1 = delta - StDepth(cmd);
		if (dep1<depth) depth = dep1;		// обновим глубину
		delta += StApp(cmd);
		*sgn = delta;						// запомним приращение 
		if (cmd > 3) {						// команды 1,2,3 - двухбайтовые, остальные однобайтовые.
			sgn++;
			pr++;
		}
		else {
			sgn[1] = delta;
			sgn += 2;
			pr += 2;
		}
	}
	return -depth;
}

//-----------------------------------------------------------------------------
BYTE FortEmulator::addWord(char *name, BYTE *pr, int stApp_, int stDepth_, int tag_)	// добавить в словарь слово pr
{
	if (NWords == maxWords) vError("addWord: iWords=maxWords = %d", IWords);
	int L = (int)strlen((char*)pr);
	memcpy(FW[NWords], pr, L);
	words[NWords] = [](){return 0; };			// соответствующая встроенная команда, её просто нет!
	stApp[NWords] = stApp_;
	stDepth[NWords] = stDepth_;
	char *s = new char[strlen(name) + 1];
	strcpy(s, name);
	names[NWords] = s;
	tag[NWords] = tag_;
	NWords++;
	return NWords - 1;
}
//-----------------------------------------------------------------------------
BYTE FortEmulator::addWord(const char *name, const char *str, int tag_)	// добавить в словарь слово из текстовой строки str
{
	if (NWords == maxWords) vError("addWord: iWords=maxWords = %d", IWords);
	int L = string2pr(str, FW[NWords]);
	char sgn[maxWLen];
	int d = signature(FW[NWords], sgn);
	stApp[NWords] = sgn[L - 1];
	stDepth[NWords] = d;
	char *s = new char[strlen(name) + 1];
	strcpy(s, name);
	names[NWords] = s;
	tag[NWords] = tag_;
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
			s += (std::string(buf) + toString(strlen(buf)) + " ");
		}
		pr++;
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
			if (SP < stack.size() && SP >= 0){
				if (stack[SP] != 0) {
//					char dist = *pr;
//					pr += dist;
					pr = pr0 + *(pr + 1);
					//pr = pr0 + (*pr);
				}
				else pr++;
				SP--;
				continue;
			}
		}
		if (b == 3) {// 3: Кладем в стек числовой литерал (-128..127). Команда состоит из 2-х байт:
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
	for (int i = 0; i < a.size(); i++)
		stack[i] = a[i];
	SP = a.size() - 1;
}

bool FortEmulator::check_result(const std::vector<int>& a){
	//if (stack.size() != a.size()) return false;

	if (SP != a.size() - 1) return false;
	for (int i = 0; i < a.size(); i++)
		if (stack[i] != a[i]) return false;
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
		if (SP < 0 || SP >= stack.size()){
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
		if (SP <1 || SP >= stack.size()) return 7; // error("c_OVER: stack");
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
		if (SP < n - 1) return 10; // error("c_PICK: stack");
		stack[SP] = stack[SP - n - 1];
		return 0;
	};

	auto c_ROLL = [&]() {
		int n = stack[SP];
		if (n < 1) return 11; // error("c_ROLL: stack");
		--SP;
		if (SP < n) return 11; // error("c_ROLL: stack");
		int t = stack[SP - n];
		for (int i = n; i > 0; --i) stack[SP - i] = stack[SP - i + 1];
		stack[SP] = t;
		return 0;
	};

	auto c_2PICK = [&]() {
		if (SP < 2 || SP >= stack.size()) return 10;
		stack[SP + 1] = stack[SP - 2];
		SP++;
		return 0;
	};

	auto c_3PICK = [&]() {
		if (SP < 3 || SP >= stack.size()) return 10;
		stack[SP + 1] = stack[SP - 3];
		SP++;
		return 0;
	};

	auto c_4PICK = [&]() {
		if (SP < 4 || SP >= stack.size()) return 10;
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
	//             0       1       2       3      4       5       6      7      8        9       10       11       12       13       14     15     16     17    18      19     20     21         22     23    24     25    26    27    28     29     30    31    32
	words = { { c_NULL, c_NULL, c_NULL, c_NULL, c_DUP, c_DROP, c_SWAP, c_OVER, c_ROT, c_MROT, c_2PICK, c_3PICK, c_4PICK, c_3ROLL, c_4ROLL, c_NEG, c_ADD, c_SUB, c_MUL, c_DIV, c_MOD, c_DIV_MOD, c_AND, c_OR, c_XOR, c_GH, c_LH, c_EQ, c_0EQ, c_0GT, c_0LT, c_PP, c_MM, c_NULL } };
	stApp = { { 0, 0, -1, 1, 1, -1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, -777 } };
	stDepth = { { 0, 0, 1, 0, 1, 1, 2, 2, 3, 3, 3, 4, 5, 4, 5, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, -777 } };
	tag = { { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 } };
	names = { { "NULL", "GOTO", "IF", "CONST", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "2PICK", "3PICK", "4PICK", "3ROLL", "4ROLL", "NEG", "+", "-", "*", "/", "%", "/%", "AND", "OR", "XOR", ">", "<", "=", "=0", ">0", "<0", "++", "--", "NULL" } };
}

void FortEmulator::SetUsingCommand(std::vector<std::string>& c){
	std::vector<std::function<int()>> new_words;
	std::array<int, maxWords> new_stApp, new_stDepth, new_tag;
	std::array<std::string, maxWords> new_names;
	int indx = 0;
	for (auto& e : c){
		std::transform(e.begin(), e.end(), e.begin(), ::toupper);
		for (int i = 0; i < names.size(); ++i){
			if (e == names[i]){
				new_names[indx] = names[i];
				new_words.push_back(words[i]);
				new_stApp[indx] = stApp[i];
				new_stDepth[indx] = stDepth[i];
				new_tag[indx] = tag[i];
				++indx;
			}
		}
	}
	if (!new_words.empty()){
		std::swap(new_words, words);
		std::swap(new_names, names);
		std::swap(new_stDepth, stDepth);
		std::swap(new_stApp, stApp);
		std::swap(new_tag, tag);
		IWords = NWords = words.size();
	}
	else
		throw std::runtime_error("Error: available instruction list is empty!");
}


/*
OVER SUB DUP >0  * +
7 17 4 29 18 16 0

6 7 21 18 16 0

OVER OVER > IF 2 SWAP DROP
7 7 25 2 6 5

176 446 985

7*(30^4+30^5)+25*30^3+2*30^2+30*6+5
30^5=24300000
30^4=810000
30^3=27000
30^2=900

*/