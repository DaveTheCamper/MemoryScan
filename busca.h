#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <pthread.h>
#include <vector>
#include "Memory.h"

using namespace std;

typedef struct PALAVRA_T {
	DWORD* g_palavra;
	int size;
} PALAVRA;

// Precisa ser multiplo de 2
#define MAX_THREAD 8

#define NAME_1 "{\"name\":\"positions-state\","
#define NAME_2 "{\"name\":\"heartbeat\",\"msg\":"
#define NAME_3 "{\"name\":\"candle-generated\",\"microserviceName\":"
#define NAME_4 "{\"name\":\"traders-mood-changed\","



double g_ja_foi[200];
DWORD **g_palavra;
int g_current_thread;
int g_partes_encontradas;
int g_size;
long g_percent;
long g_partes;
long g_inicio;
double g_last_dispach;
vector<DWORD> g_resultados;


bool verifyPalavra(DWORD* palavra, long address, int size);
void pesquisarPart(DWORD inicio, DWORD fim);
void *threadProcess(void* args);

void filterVector(DWORD** palavra, int size);


namespace Busca {
	DWORD* translatePalavra(std::string palavra) {
		const int size = palavra.length();
		//\"candle-generated\",\"microserviceName\":\"quotes\",\"msg\":{\"active_id\":

		DWORD* translate = (DWORD*)malloc(sizeof(DWORD) * size);

		for (int i = 0; i < size; i++) {
			translate[i] = (int)palavra.at(i);
			//std::cout << translate[i] << endl;
		}

		return translate;
	}

	std::string readPalavra(long address, int size) {
		std::string palavra = "";
		for (int y = 0; y < size; y++) {
			char c = (char) ProcessMemory::RPM<std::byte>(address + y);
			palavra = palavra + c;
		}
		return palavra;
	}

	void salvaTempFile(DWORD endereco) {
		ofstream outFile;
		std::ostringstream stream;
		outFile.open("cache.txt");

		std::cout << "salvando " << (long) endereco << endl;

		stream << (long) endereco;

		outFile << stream.str();

		outFile.close();
	}
}


void pesquisar(DWORD *translate, int size) {
	g_palavra = &translate;
	g_size = size;
	g_partes_encontradas = 0;
	g_resultados.clear();


	DWORD procid = ProcessMemory::pID;
	unsigned char* addr = 0;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procid);
	MEMORY_BASIC_INFORMATION mbi;

	while (VirtualQueryEx(hProc, addr, &mbi, sizeof(mbi)))
	{
		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && mbi.Protect != PAGE_GUARD)
		{
			std::cout << "base : 0x" << std::hex << mbi.BaseAddress << " end : 0x" << std::hex << (uintptr_t)mbi.BaseAddress + mbi.RegionSize << "\n";
			pesquisarPart((uintptr_t)mbi.BaseAddress, mbi.RegionSize);
		}
		addr += mbi.RegionSize;
	}

	CloseHandle(hProc);

	std::cout << "Encontrados " << g_resultados.size() << endl;

	filterVector(g_palavra, g_size);
	
}

void pesquisarPart(DWORD inicio, DWORD fim) {
	pthread_t thread[MAX_THREAD];

	g_inicio = inicio;
	g_partes = fim / MAX_THREAD;
	g_percent = 0;
	g_current_thread = -1;


	//std::cout << "PROCURANDO" << endl;
	//std::cout << "Inicio: " << inicio << endl;
	//std::cout << "Fim: " << fim << endl;

	for (int i = 0; i < MAX_THREAD; i++) {
		pthread_create(&thread[i], NULL, threadProcess, (void*)NULL);
	}

	for (int i = 0; i < MAX_THREAD; i++) {
		pthread_join(thread[i], NULL);
	}

	std::cout << "Fim da busca " << inicio << " " << fim << endl;
}

void *threadProcess(void* args) {
	g_current_thread++;
	
	int thread_id = g_current_thread;
	for (long i = g_inicio + (thread_id * g_partes); i < g_inicio + ((thread_id+1) * g_partes); i++) {
		g_percent++;

		if (verifyPalavra((*g_palavra), i, g_size)) {
			g_resultados.push_back(i);
		}
	}

	pthread_exit(NULL);

	return (void*)NULL;
}

bool verifyPalavra(DWORD* palavra, long address, int size) {
	for (int y = 0; y < size; y++) {
		DWORD a = ProcessMemory::RPM<DWORD>(address + y);
		if (palavra[y] != a) {
			return false;
		}
	}
	return true;
}

void filterVector(DWORD** palavra, int size) {
	std::string val1 = NAME_1;
	std::string val2 = NAME_2;
	std::string val3 = NAME_3;
	std::string val4 = NAME_4;

	PALAVRA pal1 = { Busca::translatePalavra(val1), val1.length() };
	PALAVRA pal2 = { Busca::translatePalavra(val2), val2.length() };
	PALAVRA pal3 = { Busca::translatePalavra(val3), val3.length() };
	PALAVRA pal4 = { Busca::translatePalavra(val3), val3.length() };

	int timeout = 5;
	vector<std::string> log;

	while (g_resultados.size() > 1 && timeout > 0) {
		std::cout << "Encontrados antes espera " << g_resultados.size() << endl;

		timeout--;
		Sleep(5 * 1000);
		ProcessMemory::updateProcess();
		int removidos = 0;
		for (int y = 0; y < g_resultados.size(); y++) {
			long address = g_resultados.at(y);
			if (!verifyPalavra(pal1.g_palavra, address, pal1.size) && 
					!verifyPalavra(pal2.g_palavra, address, pal2.size) &&
					!verifyPalavra(pal3.g_palavra, address, pal3.size)) {

				g_resultados.erase(g_resultados.begin() + y);
				y--;
				removidos++;
			} else if (log.size() > 0) {
				std::cout << "Log verificando " << endl << log.at(removidos + y) << " " << Busca::readPalavra(address, 200) << " " << address << " " << verifyPalavra(Busca::translatePalavra(log.at(removidos + y)), address, 200) << endl << endl;
				if (verifyPalavra(Busca::translatePalavra(log.at(removidos + y)), address, 200)) {
					g_resultados.erase(g_resultados.begin() + y);
					y--;
					removidos++;
				}
			}
		}

		log.clear();
		for (int y = 0; y < g_resultados.size(); y++) {
			long address = g_resultados.at(y);
			log.push_back(Busca::readPalavra(address, 200));
		}
	}

	std::cout << "Enderecos encontrados:" << endl;
	for (int y = 0; y < g_resultados.size(); y++) {
		std::cout << g_resultados.at(y) << endl;
	}

	Busca::salvaTempFile(g_resultados.at(0));

	free(pal1.g_palavra);
	free(pal2.g_palavra);
	free(pal3.g_palavra);
}