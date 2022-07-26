#include "pch.h"
#include <iostream>
#include <thread>
#include "Memory.h"
#include "busca.h"
#include "Offsets.hpp"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
#pragma warning(disable : 4996)
#define VERSION 0.1;
bool save = false;
DWORD dw_basemoeda = 0;
int dw_fechado = 0;
int dw_Conexao = 0;
DWORD CLIENT_BASE = 0;

bool Open = true;
void Save() {
	while (true) {
		if (Open == false) {
			ofstream outFile;
			outFile.open("4f.txt");
			string quit = "n";
			std::ostringstream stream;
			//std::ostringstream moedaativA;
			//std::ostringstream conexaow;
			stream << dw_basemoeda;
			//moedaativA << dw_fechado;
			//conexaow << dw_Conexao;
			std::string line = stream.str();
			//std::string line2 = "base_fechado=" + moedaativA.str();
			//std::string line3 = "base_conexao=" + conexaow.str();

			outFile << line << endl;
			//outFile << line2 << endl;
			//outFile << line3 << endl;
			Sleep(100);
			outFile.close();
			save = true;
		}
	}
}
void GlowESP() {
	std::cout << "ready!" << std::endl;
	while (true) {
		if (Open == true) {
			DWORD MoedaAtual = ProcessMemory::RPM<DWORD>(CLIENT_BASE + Offsets::BaseMoedaAtual);
			DWORD MoAtual1 = ProcessMemory::RPM<DWORD>(MoedaAtual + Offsets::MoedaAtual_Offset1);
			DWORD MoAtual2 = ProcessMemory::RPM<DWORD>(MoAtual1 + Offsets::MoedaAtual_Offset2);
			DWORD MoAtual3 = ProcessMemory::RPM<DWORD>(MoAtual2 + Offsets::MoedaAtual_Offset3);
			DWORD MoAtual4 = ProcessMemory::RPM<DWORD>(MoAtual3 + Offsets::MoedaAtual_Offset4);
			//double MoAtual5 = ProcessMemory::RPM<double>(MoAtual4 + Offsets::MoedaAtual_Offset5);
			// 
			//
			DWORD MoedaAtiva = 0x20B88E4;
			DWORD Conexao = 0x20CD74C;
			DWORD MoAtualRetorno = (ProcessMemory::RPM<DWORD>(MoAtual3 + Offsets::MoedaAtual_Offset4) + 0x10);
			
			dw_basemoeda = MoAtualRetorno;
			dw_fechado = MoedaAtiva;
			dw_Conexao = Conexao;
			Sleep(100);
			Open = false;
			break;
			//DWORD glowObject = ProcessMemory::RPM<DWORD>(CLIENT_BASE + signatures::dwGlowObjectManager);
			//int Team = ProcessMemory::RPM<DWORD>(entitybase + netvars::m_iTeamNum);

			//ProcessMemory::WPM<float>(G / 255, (glowObject + ((glowIndex * 0x38) + 0x8)));
			//ProcessMemory::WPM<float>(B / 255, (glowObject + ((glowIndex * 0x38) + 0xC)));
			//ProcessMemory::WPM<float>(A / 255, (glowObject + ((glowIndex * 0x38) + 0x10)));
			//ProcessMemory::WPM<bool>(true, (glowObject + ((glowIndex * 0x38) + 0x24)));
		}
	}
}

bool consultaTempFile() {
	std::string str = "";
	ifstream file;
	file.open("cache.txt");

	std::cout << file.good() << endl;
	if (file.good()) {
		std::getline(file, str);
		DWORD endereco;

		std::istringstream ss(str);
		ss >> endereco;

		std::string palavra = Busca::readPalavra(endereco, 8);
		std::cout << endereco << "<< Endereco Palavra >>" << palavra << " " << palavra.length() << endl << endl;
		if (palavra._Equal("{\"name\":")) {
			file.close();
			return true;
		}
		
		

		std::cout << std::hex << endereco << "\n";
	}

	file.close();
	return false;
}

void buscaEndereco() {
	if (!consultaTempFile()) {
		std::string palavra = "{\"name\":";
		DWORD* translate = Busca::translatePalavra(palavra);

		pesquisar(translate, palavra.length());

		free(translate);
	}
}




int main()
{
	ProcessMemory::Attach(L"IQ Option.exe");
	//GetModuleSize(18228, "IQ Option.exe")
	CLIENT_BASE = ProcessMemory::FindModule(L"IQ Option.exe");

	buscaEndereco();

	//std::thread espthread(GlowESP);
	std::thread Savethread(Save);



	while (true) {
		if (save) {
			ProcessMemory::Deattach();
			Sleep(100);
			exit(0);
		}
		Sleep(10);
	}
	ProcessMemory::Deattach();
}