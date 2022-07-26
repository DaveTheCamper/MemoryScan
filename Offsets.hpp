#pragma once

// 8.12.2018 16:51 Haze Dumper

#include <windows.h>
#include <cstddef>
namespace Offsets {
	constexpr ::std::ptrdiff_t BaseMoedaAtual = 0x1DCAA08;
	constexpr ::std::ptrdiff_t MoedaAtual_Offset1 = 0x108;
	constexpr ::std::ptrdiff_t MoedaAtual_Offset2 = 0x1c;
	constexpr ::std::ptrdiff_t MoedaAtual_Offset3 = 0x520;
	constexpr ::std::ptrdiff_t MoedaAtual_Offset4 = 0x13c;
	constexpr ::std::ptrdiff_t MoedaAtual_Offset5 = 0x98;
	constexpr ::std::ptrdiff_t MoedaAtiva= 0x20B88E4;
} 
namespace OffsetsRetorno {
	constexpr ::std::ptrdiff_t dw_basemoeda = 0x20BDDD4;
}