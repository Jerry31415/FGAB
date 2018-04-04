#include <string>
#include <vector>
#include <array>
#include <functional>
#include <iostream>
#include <map>

typedef unsigned char BYTE;

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
	int StApp(BYTE b);		// ���������� ����� ��������� b
	int StDepth(BYTE b);	// ��������� ������� ����� �������� b
	std::string WName(BYTE b);	// ��� �������� b
	int Tag(BYTE b, int t = -1);// tag[b]
	BYTE FIndex(char *word);// ������ ����� � ������ word, 255 ���� ������ ����� ���

	bool isSecond(BYTE *pr, int i);		// pr[i] - ������ ���� ������������ �������?
	int  signature(BYTE *pr, char *sgn);// sgn[i] = ���������� ����� ����� i-� �������. ���������� ������������ ������� �����
	//-----------------------------------------------------------------------------
	BYTE addWord(char *name, BYTE *pr, int stApp_, int stDepth_, int tag_);	// �������� � ������� ����� pr
	BYTE addWord(const char *name, const char *str, int tag_);	// �������� � ������� ����� �� ��������� ������ str

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

	int IWords;		// ���������� ���������� ����
	int NWords;	// ������� ���������� ����
	BYTE FW[maxWords][maxWLen];	// ���� ����-�����
	int SP;		// ����� ������� ����� [-1..stSize]
	int MFcount, MF_max; // �������� ������
	std::vector<int> stack; // ���� ������
	std::vector<std::function<int()>> words;
	std::array<int, maxWords> stApp, stDepth, tag;
	std::array<std::string, maxWords> names;
};