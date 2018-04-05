// mfort.cpp 2018-02-27 ��������

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


std::string FortEmulator::WName(BYTE b) {	// ��� �������� b
	if (b >= NWords) vError("StDepth(%d)", b);
	return names[b];
}

//-----------------------------------------------------------------------------
BYTE FortEmulator::FIndex(char *word)// ������ ����� � ������ word, 255 ���� ������ ����� ���
{
	for (int i = 1; i < NWords; i++)
		if (strcmp(word, names[i].c_str()) == 0) {
			return i;
		}
	return 255;
}
//-----------------------------------------------------------------------------
bool FortEmulator::isSecond(BYTE *pr, int i)	// pr[i] - ������ ���� ������������ �������
{
	if (i == 0) return false;
	return pr[i - 1] <= 3;		// ������������ ������� ����� ������ 1,2,3
}

//-----------------------------------------------------------------------------
BYTE FortEmulator::addWord(char *name, BYTE *pr)	// �������� � ������� ����� pr
{
	if (NWords == maxWords) vError("addWord: iWords=maxWords = %d", IWords);
	int L = (int)strlen((char*)pr);
	memcpy(FW[NWords], pr, L);
	words[NWords] = [](){return 0; };			// ��������������� ���������� �������, � ������ ���!
	char *s = new char[strlen(name) + 1];
	strcpy(s, name);
	names[NWords] = s;
	NWords++;
	return NWords - 1;
}
//-----------------------------------------------------------------------------
BYTE FortEmulator::addWord(const char *name, const char *str)	// �������� � ������� ����� �� ��������� ������ str
{
	if (NWords == maxWords) vError("addWord: iWords=maxWords = %d", IWords);
	int L = string2pr(str, FW[NWords]);
	char sgn[maxWLen];
	char *s = new char[strlen(name) + 1];
	strcpy(s, name);
	names[NWords] = s;
	NWords++;
	return NWords - 1;
}
//-----------------------------------------------------------------------------
std::string FortEmulator::pr2string(BYTE *pr)	// ����-��������� � �����
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
int FortEmulator::string2pr(const std::string& str, BYTE *pr)	// �� ������ str �������� ��������� pr, ����������� �����
{
	char *sep = " \t\n\r";
	char *w1 = strtok((char*)str.c_str(), sep);
	BYTE *p = pr;
	int L = 0;					// ����� ���������
	while (w1 != NULL) {
		if (strcmp(w1, ";") == 0) break;		// ����� ���������
		// ������ ����� �����:
		BYTE iw = FIndex(w1);
		if (iw == 255) vError("addWord: words %s not found", w1);
		*p++ = iw;
		L++;
		if (iw <= 3) {  // ������� ������������
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
	*p = 0;	// ������� ������� �����
	return L;
}

void FortEmulator::CClear() { MFcount = 0; }		// �������� �������
int  FortEmulator::CGet() { return MFcount; }		// ������� �������� ��������
// ���������� ������������ �������� ��������
// ���������� ������������ ����� �������� ��� ���������
void FortEmulator::CMaxSet(int m) { MF_max = m; }	
//-----------------------------------------------------------------------------
void FortEmulator::mem_clear()		// �������� ����
{
	for (auto&e : stack) e = 0;
	SP = -1;
}
//-----------------------------------------------------------------------------
int FortEmulator::c_exec(BYTE *pr)	// ��������� ���������. ���������� ��� ������
{
	BYTE *pr0 = pr;		// ������ ���������
	BYTE b;
	int code;
	while (b = *pr++) {	// b - ������� �������, pr ��������� �� ���������
		if (MFcount++ >= MF_max) return 1;
		if (b >= NWords) return -1;// vError("c_exec: b = %d", b);
		//printf("%-8s", names[b]); dotS();
		if (b == 1) {// 1: ����������� ������� �� ��������� ������. ������� ������� �� 2-� ����:
			//char dist = *pr;
			pr = pr0+*(pr+1);
			//pr = *(pr+1);		
			continue;
		}
		if (b == 2) {// 2: �������� ������� (������� ����� ?0) �� ��������� ������. ������� ������� �� 2-� ����:
			if (SP < stack.size() && SP >= 0){
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
		if (b == 3) {// 3: ������ � ���� �������� ������� (-128..127). ������� ������� �� 2-� ����:
			stack[++SP] = *pr++;
			continue;
		}
		if (b < IWords) {// ���������� �����	
			if (code = words[b]()) return code;
			continue;
		}
		// ������ b - Fort-�����
		if (code = c_exec(FW[b])) return code;		// ����������� �����
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

void FortEmulator::mem_set(const std::vector<int>& a) 	// ��������� ����
{
	for (int i = 0; i < a.size(); i++)
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

	// ���������� ������� ---------------------------------------------------------

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

	// ����������
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

	// ������� ��������
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

	// ���������� �������� (���������)
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
	// ���� ���������� �������, ���������� �����, ��������� ������� ����� �����
	//             0       1       2       3      4       5       6      7      8        9       10       11       12       13       14     15     16     17    18      19     20     21         22     23    24     25    26    27    28     29     30    31    32
	words = { { c_NULL, c_NULL, c_NULL, c_NULL, c_DUP, c_DROP, c_SWAP, c_OVER, c_ROT, c_MROT, c_2PICK, c_3PICK, c_4PICK, c_3ROLL, c_4ROLL, c_NEG, c_ADD, c_SUB, c_MUL, c_DIV, c_MOD, c_DIV_MOD, c_AND, c_OR, c_XOR, c_GH, c_LH, c_EQ, c_0EQ, c_0GT, c_0LT, c_PP, c_MM, c_NULL } };
	names = { { "NULL", "GOTO", "IF", "CONST", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "2PICK", "3PICK", "4PICK", "3ROLL", "4ROLL", "NEG", "+", "-", "*", "/", "%", "/%", "AND", "OR", "XOR", ">", "<", "=", "=0", ">0", "<0", "++", "--", "NULL" } };
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


bool FortEmulator::checkCode(BYTECODE& bc){
	int pnt;
	for (int p = 0; p < bc.size(); ++p){
		if (names[bc[p]] == "CONST") {
			return false;
		}
//"NULL", "GOTO", "IF", "CONST", "DUP", "DROP", "SWAP", "OVER", "ROT", "-ROT", "2PICK", "3PICK", "4PICK", "3ROLL", "4ROLL", "NEG", "+", "-", "*", "/", "%", "/%", "AND", "OR", "XOR", ">", "<", "=", "=0", ">0", "<0", "++", "--", "NULL" 
		if (names[bc[p]] == "NULL") return false;
		if (names[bc[p]] == "GOTO" || names[bc[p]] == "IF"){
			if (p == bc.size() - 1) return false;
			pnt = bc[p + 1];
			if (pnt == p) return false;
			if (pnt >= bc.size()) return false;
			if (pnt > 0){
				std::string t = names[bc[pnt - 1]];
				if (t == "NULL" || t == "GOTO" || t == "IF" || t == "CONST")
					return false;
//				if (bc[pnt - 1] < 4) return false;
			}
			++p;
		}
	}
	return true;
}