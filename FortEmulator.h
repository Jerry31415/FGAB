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

class FortEmulator {
	void Init();
public:

	FortEmulator(int stack_size = 512) : IWords(33), NWords(33), MFcount(0), MF_max(0x7fffffff){
		stack.resize(stack_size, 0);
		SP = 0;
		for (int i = 0; i < maxWords; ++i) words.push_back([](){return 0;});
		Init();
	}

	void vError(const char *format, ...);		// show error function
	void error(char *msg);
	void dump();			// ������� ���� �� �����

	// ���������� �������, ���������� ��� ������-----------------------------------
	std::string WName(BYTE b);	// ��� �������� b
	BYTE FIndex(char *word);// ������ ����� � ������ word, 255 ���� ������ ����� ���

	bool isSecond(BYTE *pr, int i);		// pr[i] - ������ ���� ������������ �������?
	//-----------------------------------------------------------------------------
	BYTE addWord(char *name, BYTE *pr);	// �������� � ������� ����� pr
	BYTE addWord(const char *name, const char *str);	// �������� � ������� ����� �� ��������� ������ str

	std::string pr2string(BYTE *pr);				// ����-��������� � �����
	int string2pr(const std::string& str, BYTE *pr);		// �� ������ str �������� ��������� pr, ����������� �����

	//-----------------------------------------------------------------------------
	int c_exec(BYTE *pr);	// ��������� ���������. ���������� ��� ������
	
	int emulator(const std::string& str);	// ��������� ��������� � ��������� ����. ���������� ��� ������
	int emulator(std::vector<BYTE>& bytecode);

	void mem_clear();		// �������� ����
	void mem_set(const std::vector<int>&);	// ��������� ����
	bool check_result(const std::vector<int>&);	// �������� ���� � ��������

	//----������� ������-----------------------------------------------------------
	void CClear();			// �������� �������
	int  CGet();			// ������� �������� ��������
	void CMaxSet(int m);	// ���������� ������������ �������� ��������
		
	void SetUsingCommand(std::vector<std::string>&);

	bool checkCode(BYTECODE& bc);

	int IWords;		// ���������� ���������� ����
	int NWords;	// ������� ���������� ����
	BYTE FW[maxWords][maxWLen];	// ���� ����-�����
	int SP;		// ����� ������� ����� [-1..stSize]
	int MFcount, MF_max; // �������� ������
	std::vector<int> stack; // ���� ������
	std::vector<std::function<int()>> words;
	std::array<std::string, maxWords> names;

	std::map<std::string, int> pIC;

};