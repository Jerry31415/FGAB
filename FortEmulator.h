#include <string>
#include <vector>
#include <array>
#include <functional>
#include <iostream>
#include <map>

typedef unsigned char BYTE;
typedef std::vector<BYTE> BYTECODE;

// максимально возможное количество слов в системе
#define maxWords 250
// максимальная длина одного Форт-слова
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
	void dump();			// вывести стек на экран

	// Встроенные команды, возвращают код ошибки-----------------------------------
	std::string WName(BYTE b);			// имя командны b
	BYTE FIndex(const char *word);		// индекс слова с именем word, 255 если такого слова нет

	bool isSecond(BYTE *pr, int i);		// pr[i] - второй байт двухбайтовой команды?
	//-----------------------------------------------------------------------------
	BYTE addWord(char *name, BYTE *pr);	// добавить в словарь слово pr
	BYTE addWordT(const char *name, const char *str);	// добавить в словарь слово из текстовой строки str

	std::string pr2string(BYTE *pr);					// Форт-программу в текст
	std::string pr2string(BYTECODE &pr);				// Форт-программу в текст
	int string2pr(const std::string& str, BYTE *pr);	// из строки str получить программу pr, возваращает длину

	//-------------------------------------------------------------------------
	int c_exec(BYTE *pr);	// Выполнить программу. Возвращает код ошибки
	
	int emulator(const std::string& str);	// Выполнить программу в текстовом виде. Возвращает код ошибки
	int emulator(std::vector<BYTE>& bytecode);

	void mem_clear();		// очистить стек
	void mem_set(const std::vector<int>&);	// заполнить стек
	bool check_result(const std::vector<int>&);	// сравнить стек с вектором

	//----счётчик команд-------------------------------------------------------
	void CClear();			// очистить счётчик
	int  CGet();			// текущее значение счётчика
	void CMaxSet(int m);	// установить максимальное значение счётчика
		
	void SetUsingCommand(std::vector<std::string>&);

	bool checkCode(BYTECODE& bc);

	template<typename T>
	int CalcRating(T tr, BYTECODE& p, std::vector<bool>& R);

	template<typename T>
	int CalcRating(T tr, BYTECODE& p);

	template<typename T>
	int testing_code(const T& tss, std::vector<BYTE>& bc);

	int IWords;		// количество встроенных слов
	int NWords;	// текущее количество слов
	BYTE FW[maxWords][maxWLen];	// сами форт-слова
	int SP;		// номер вершины стека [-1..stSize]
	int MFcount, MF_max; // счётчики команд
	std::vector<int> stack; // стек данных
	std::vector<std::function<int()>> words;
	std::array<std::string, maxWords> names;

	//---- список допустимых слов ---------------------------------------------
	BYTECODE adm;									// список допустимых слов
	int adm_N() { return (int)adm.size(); }			// количество допустимых слов
	void adm_all();									// установить полный допустимых список
	void adm_set(std::string &s);					// установить список слов из строки
	void adm_add(BYTE b);							// добавить одно слово
	void adm_add(std::vector<std::string> ww);		// добавить новые слова из текста

};