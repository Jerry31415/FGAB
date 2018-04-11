#include <string>
#include <vector>
#include <array>
#include <functional>
#include <iostream>
#include <map>

typedef unsigned char BYTE;
typedef std::vector<BYTE> BYTECODE;

// ����������� ��������� ���������� ���� � �������
#define maxWords 250
// ������������ ����� ������ ����-�����
#define maxWLen 64		

void vError(const char *format, ...);		// show error function

class FortEmulator {
	void Init();
public:

	FortEmulator(int stack_size = 8) : IWords(35), NWords(35), MFcount(0), MF_max(0x7fffffff){
		stack.resize(stack_size, 0);
		SP = 0;
		for (int i = 0; i < maxWords; ++i) words.push_back([](){return 0;});
		Init();
	}

	void error(char *msg);
	void dump();			// ������� ���� �� �����

	// ���������� �������, ���������� ��� ������-----------------------------------
	std::string WName(BYTE b);			// ��� �������� b
	BYTE FIndex(const char *word);		// ������ ����� � ������ word, 255 ���� ������ ����� ���

	bool isSecond(BYTE *pr, int i);		// pr[i] - ������ ���� ������������ �������?
	//-----------------------------------------------------------------------------
	BYTE addWord(char *name, BYTE *pr);	// �������� � ������� ����� pr
	BYTE addWordT(const char *name, const char *str);	// �������� � ������� ����� �� ��������� ������ str

	std::string pr2string(BYTE *pr);					// ����-��������� � �����
	std::string pr2string(BYTECODE &pr);				// ����-��������� � �����
	int string2pr(const std::string& str, BYTE *pr);	// �� ������ str �������� ��������� pr, ����������� �����

	//-------------------------------------------------------------------------
	int c_exec(BYTE *pr);	// ��������� ���������. ���������� ��� ������
	
	int emulator(const std::string& str);	// ��������� ��������� � ��������� ����. ���������� ��� ������
	int emulator(std::vector<BYTE>& bytecode);

	void mem_clear();		// �������� ����
	void mem_set(const std::vector<int>&);	// ��������� ����
	bool check_result(const std::vector<int>&);	// �������� ���� � ��������

	//----������� ������-------------------------------------------------------
	void CClear();			// �������� �������
	int  CGet();			// ������� �������� ��������
	void CMaxSet(int m);	// ���������� ������������ �������� ��������
		
	void SetUsingCommand(std::vector<std::string>&);

	bool checkCode(BYTECODE& bc);

	template<typename T>
	int CalcRating(T tr, BYTECODE& p, std::vector<bool>& R);

	template<typename T>
	int CalcRating(T tr, BYTECODE& p);

	template<typename T>
	int testing_code(const T& tss, std::vector<BYTE>& bc);

	int IWords;		// ���������� ���������� ����
	int NWords;	// ������� ���������� ����
	BYTE FW[maxWords][maxWLen];	// ���� ����-�����
	int SP;		// ����� ������� ����� [-1..stSize]
	int MFcount, MF_max; // �������� ������
	std::vector<int> stack; // ���� ������
	std::vector<std::function<int()>> words;
	std::array<std::string, maxWords> names;

	//---- ������ ���������� ���� ---------------------------------------------
	BYTECODE adm;									// ������ ���������� ����
	int adm_N() { return (int)adm.size(); }			// ���������� ���������� ����
	void adm_all();									// ���������� ������ ���������� ������
	void adm_set(std::string &s);					// ���������� ������ ���� �� ������
	void adm_add(BYTE b);							// �������� ���� �����
	void adm_add(std::vector<std::string> ww);		// �������� ����� ����� �� ������

};