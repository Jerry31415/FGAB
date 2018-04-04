#include <string>
#include <vector>
#include <array>
#include <functional>
#include <iostream>
#include <map>

typedef unsigned char BYTE;

// максимально возможное количество слов в системе
#define maxWords 250
// максимальная длина одного Форт-слова
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
	void dump();			// вывести стек на экран

	// Встроенные команды, возвращают код ошибки-----------------------------------
	int StApp(BYTE b);		// приращение стека командной b
	int StDepth(BYTE b);	// требуемая глубина стека командны b
	std::string WName(BYTE b);	// имя командны b
	int Tag(BYTE b, int t = -1);// tag[b]
	BYTE FIndex(char *word);// индекс слова с именем word, 255 если такого слова нет

	bool isSecond(BYTE *pr, int i);		// pr[i] - второй байт двухбайтовой команды?
	int  signature(BYTE *pr, char *sgn);// sgn[i] = приращение стека ПОСЛЕ i-й команды. Возвращает максимальную глубину стека
	//-----------------------------------------------------------------------------
	BYTE addWord(char *name, BYTE *pr, int stApp_, int stDepth_, int tag_);	// добавить в словарь слово pr
	BYTE addWord(const char *name, const char *str, int tag_);	// добавить в словарь слово из текстовой строки str

	std::string pr2string(BYTE *pr);				// Форт-программу в текст
	int string2pr(const std::string& str, BYTE *pr);		// из строки str получить программу pr, возваращает длину

	//-----------------------------------------------------------------------------
	int c_exec(BYTE *pr);	// Выполнить программу. Возвращает код ошибки
	
	int emulator(const std::string& str);	// Выполнить программу в текстовом виде. Возвращает код ошибки
	int emulator(std::vector<BYTE>& bytecode);

	void mem_clear();		// очистить стек
	void mem_set(const std::vector<int>&);	// заполнить стек
	bool check_result(const std::vector<int>&);	// сравнить стек с вектором

	//----счётчик команд-----------------------------------------------------------
	void CClear();			// очистить счётчик
	int  CGet();			// текущее значение счётчика
	void CMaxSet(int m);	// установить максимальное значение счётчика
		
	void SetUsingCommand(std::vector<std::string>&);

	int IWords;		// количество встроенных слов
	int NWords;	// текущее количество слов
	BYTE FW[maxWords][maxWLen];	// сами форт-слова
	int SP;		// номер вершины стека [-1..stSize]
	int MFcount, MF_max; // счётчики команд
	std::vector<int> stack; // стек данных
	std::vector<std::function<int()>> words;
	std::array<int, maxWords> stApp, stDepth, tag;
	std::array<std::string, maxWords> names;
};