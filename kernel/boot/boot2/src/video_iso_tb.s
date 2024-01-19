|*
|****************************************************************
|*								*
|*			video.iso.tb.s				*
|*								*
|*	Tabela de caracteres ISO para o video do PC (16 * 8)	*
|*								*
|*	Vers�o	3.0.0, de 28.08.95				*
|*		3.0.0, de 29.08.95				*
|*								*
|*	M�dulo: Boot2						*
|*		N�CLEO do TROPIX para PC			*
|*								*
|*	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
|*		Copyright � 1995 NCE/UFRJ - tecle "man licen�a"	*
|*								*
|****************************************************************
|*

|*
|****************************************************************
|*	A Tabela						*
|****************************************************************
|*
	.const
	.global	video_tb
video_tb:
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x00: "."
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7E, 0x81, 0xA5, 0x81, 0x81, 0xBD	|* 0x01: "."
	.byte	0x99, 0x81, 0x81, 0x7E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7E, 0xFF, 0xDB, 0xFF, 0xFF, 0xC3	|* 0x02: "."
	.byte	0xE7, 0xFF, 0xFF, 0x7E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x6C, 0xFE, 0xFE, 0xFE	|* 0x03: "."
	.byte	0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x7C, 0xFE	|* 0x04: "."
	.byte	0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x18, 0x3C, 0x3C, 0xE7, 0xE7	|* 0x05: "."
	.byte	0xE7, 0x99, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF	|* 0x06: "."
	.byte	0x7E, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3C	|* 0x07: "."
	.byte	0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC3	|* 0x08: "."
	.byte	0xC3, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x42	|* 0x09: "."
	.byte	0x42, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0x99, 0xBD	|* 0x0A: "."
	.byte	0xBD, 0x99, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	.byte	0x00, 0x00, 0x1E, 0x0E, 0x1A, 0x32, 0x78, 0xCC	|* 0x0B: "."
	.byte	0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C	|* 0x0C: "."
	.byte	0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3F, 0x33, 0x3F, 0x30, 0x30, 0x30	|* 0x0D: "."
	.byte	0x30, 0x70, 0xF0, 0xE0, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7F, 0x63, 0x7F, 0x63, 0x63, 0x63	|* 0x0E: "."
	.byte	0x63, 0x67, 0xE7, 0xE6, 0xC0, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x18, 0x18, 0xDB, 0x3C, 0xE7	|* 0x0F: "."
	.byte	0x3C, 0xDB, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFE, 0xF8	|* 0x10: "."
	.byte	0xF0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x02, 0x06, 0x0E, 0x1E, 0x3E, 0xFE, 0x3E	|* 0x11: "."
	.byte	0x1E, 0x0E, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18	|* 0x12: "."
	.byte	0x18, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66	|* 0x13: "."
	.byte	0x66, 0x00, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7F, 0xDB, 0xDB, 0xDB, 0x7B, 0x1B	|* 0x14: "."
	.byte	0x1B, 0x1B, 0x1B, 0x1B, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x7C, 0xC6, 0x60, 0x38, 0x6C, 0xC6, 0xC6	|* 0x15: "."
	.byte	0x6C, 0x38, 0x0C, 0xC6, 0x7C, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x16: "."
	.byte	0xFE, 0xFE, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18	|* 0x17: "."
	.byte	0x18, 0x7E, 0x3C, 0x18, 0x7E, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18	|* 0x18: "."
	.byte	0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18	|* 0x19: "."
	.byte	0x18, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0C, 0xFE	|* 0x1A: "."
	.byte	0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 0xFE	|* 0x1B: "."
	.byte	0x60, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0	|* 0x1C: "."
	.byte	0xC0, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x6C, 0xFE	|* 0x1D: "."
	.byte	0x6C, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x38, 0x7C	|* 0x1E: "."
	.byte	0x7C, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x7C, 0x7C	|* 0x1F: "."
	.byte	0x38, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x20: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x3C, 0x3C, 0x3C, 0x18, 0x18	|* 0x21: "!"
	.byte	0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x66, 0x66, 0x66, 0x24, 0x00, 0x00, 0x00	|* 0x22: """
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x6C, 0x6C, 0xFE, 0x6C, 0x6C	|* 0x23: "#"
	.byte	0x6C, 0xFE, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00
	.byte	0x18, 0x18, 0x7C, 0xC6, 0xC2, 0xC0, 0x7C, 0x06	|* 0x24: "$"
	.byte	0x86, 0xC6, 0x7C, 0x18, 0x18, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0xC2, 0xC6, 0x0C, 0x18	|* 0x25: "%"
	.byte	0x30, 0x60, 0xC6, 0x86, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x76, 0xDC	|* 0x26: "&"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x06, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x27: "'"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30	|* 0x28: "("
	.byte	0x30, 0x30, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x0C	|* 0x29: ")"
	.byte	0x0C, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x3C, 0xFF	|* 0x2A: "*"
	.byte	0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E	|* 0x2B: "+"
	.byte	0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x2C: ","
	.byte	0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE	|* 0x2D: "-"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x2E: "."
	.byte	0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x0C, 0x18	|* 0x2F: "/"
	.byte	0x30, 0x60, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xCE, 0xD6, 0xD6	|* 0x30: "0"
	.byte	0xE6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x38, 0x78, 0x18, 0x18, 0x18	|* 0x31: "1"
	.byte	0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0x06, 0x0C, 0x18, 0x30	|* 0x32: "2"
	.byte	0x60, 0xC0, 0xC6, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0x06, 0x06, 0x3C, 0x06	|* 0x33: "3"
	.byte	0x06, 0x06, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x0C, 0x1C, 0x3C, 0x6C, 0xCC, 0xFE	|* 0x34: "4"
	.byte	0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFE, 0xC0, 0xC0, 0xC0, 0xFC, 0x0E	|* 0x35: "5"
	.byte	0x06, 0x06, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x38, 0x60, 0xC0, 0xC0, 0xFC, 0xC6	|* 0x36: "6"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFE, 0xC6, 0x06, 0x06, 0x0C, 0x18	|* 0x37: "7"
	.byte	0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0xC6	|* 0x38: "8"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7E, 0x06	|* 0x39: "9"
	.byte	0x06, 0x06, 0x0C, 0x78, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00	|* 0x3A: ":"
	.byte	0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00	|* 0x3B: ";"
	.byte	0x00, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60	|* 0x3C: "<"
	.byte	0x30, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00	|* 0x3D: "="
	.byte	0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06	|* 0x3E: ">"
	.byte	0x0C, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0x0C, 0x18, 0x18	|* 0x3F: "?"
	.byte	0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xDE, 0xDE	|* 0x40: "@"
	.byte	0xDE, 0xDC, 0xC0, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE	|* 0x41: "A"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFC, 0x66, 0x66, 0x66, 0x7C, 0x66	|* 0x42: "B"
	.byte	0x66, 0x66, 0x66, 0xFC, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x66, 0xC2, 0xC0, 0xC0, 0xC0	|* 0x43: "C"
	.byte	0xC0, 0xC2, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x66	|* 0x44: "D"
	.byte	0x66, 0x66, 0x6C, 0xF8, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78, 0x68	|* 0x45: "E"
	.byte	0x60, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78, 0x68	|* 0x46: "F"
	.byte	0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x66, 0xC2, 0xC0, 0xC0, 0xDE	|* 0x47: "G"
	.byte	0xC6, 0xC6, 0x66, 0x3A, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xFE, 0xC6	|* 0x48: "H"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18	|* 0x49: "I"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C	|* 0x4A: "J"
	.byte	0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xE6, 0x66, 0x6C, 0x6C, 0x78, 0x78	|* 0x4B: "K"
	.byte	0x6C, 0x66, 0x66, 0xE6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xF0, 0x60, 0x60, 0x60, 0x60, 0x60	|* 0x4C: "L"
	.byte	0x60, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC3, 0xE7, 0xFF, 0xFF, 0xDB, 0xDB	|* 0x4D: "M"
	.byte	0xC3, 0xC3, 0xC3, 0xC3, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xE6, 0xF6, 0xFE, 0xDE, 0xCE	|* 0x4E: "N"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0xC6	|* 0x4F: "O"
	.byte	0xC6, 0xC6, 0x6C, 0x38, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFC, 0x66, 0x66, 0x66, 0x7C, 0x60	|* 0x50: "P"
	.byte	0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0x51: "Q"
	.byte	0xC6, 0xD6, 0xDE, 0x7C, 0x0C, 0x0E, 0x00, 0x00
	.byte	0x00, 0x00, 0xFC, 0x66, 0x66, 0x66, 0x7C, 0x6C	|* 0x52: "R"
	.byte	0x66, 0x66, 0x66, 0xE6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0x60, 0x38, 0x0C	|* 0x53: "S"
	.byte	0x06, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFF, 0xDB, 0x99, 0x18, 0x18, 0x18	|* 0x54: "T"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0x55: "U"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0x56: "V"
	.byte	0xC6, 0x6C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xDB	|* 0x57: "W"
	.byte	0xDB, 0xFF, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0x6C, 0x6C, 0x38, 0x38	|* 0x58: "X"
	.byte	0x6C, 0x6C, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18	|* 0x59: "Y"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xFE, 0xC6, 0x86, 0x0C, 0x18, 0x30	|* 0x5A: "Z"
	.byte	0x60, 0xC2, 0xC6, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30	|* 0x5B: "["
	.byte	0x30, 0x30, 0x30, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x60, 0x30	|* 0x5C: "\"
	.byte	0x18, 0x0C, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C	|* 0x5D: "]"
	.byte	0x0C, 0x0C, 0x0C, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00	|* 0x5E: "^"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x5F: "_"
	.byte	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00
	.byte	0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x60: "`"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0C, 0x7C	|* 0x61: "a"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xE0, 0x60, 0x60, 0x78, 0x6C, 0x66	|* 0x62: "b"
	.byte	0x66, 0x66, 0x66, 0xDC, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC0	|* 0x63: "c"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x1C, 0x0C, 0x0C, 0x3C, 0x6C, 0xCC	|* 0x64: "d"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xFE	|* 0x65: "e"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x38, 0x6C, 0x64, 0x60, 0xF0, 0x60	|* 0x66: "f"
	.byte	0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xCC, 0xCC	|* 0x67: "g"
	.byte	0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xCC, 0x78, 0x00
	.byte	0x00, 0x00, 0xE0, 0x60, 0x60, 0x6C, 0x76, 0x66	|* 0x68: "h"
	.byte	0x66, 0x66, 0x66, 0xE6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x18, 0x00, 0x38, 0x18, 0x18	|* 0x69: "i"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x06, 0x06, 0x00, 0x0E, 0x06, 0x06	|* 0x6A: "j"
	.byte	0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3C, 0x00
	.byte	0x00, 0x00, 0xE0, 0x60, 0x60, 0x66, 0x6C, 0x78	|* 0x6B: "k"
	.byte	0x78, 0x6C, 0x66, 0xE6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18	|* 0x6C: "l"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xE6, 0xFF, 0xDB	|* 0x6D: "m"
	.byte	0xDB, 0xDB, 0xDB, 0xDB, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x66, 0x66	|* 0x6E: "n"
	.byte	0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC6	|* 0x6F: "o"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x66, 0x66	|* 0x70: "p"
	.byte	0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xCC, 0xCC	|* 0x71: "q"
	.byte	0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0x0C, 0x1E, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x76, 0x62	|* 0x72: "r"
	.byte	0x60, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0x60	|* 0x73: "s"
	.byte	0x38, 0x0C, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x10, 0x30, 0x30, 0xFC, 0x30, 0x30	|* 0x74: "t"
	.byte	0x30, 0x30, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC	|* 0x75: "u"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66	|* 0x76: "v"
	.byte	0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xC3, 0xC3	|* 0x77: "w"
	.byte	0xDB, 0xDB, 0xFF, 0x66, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0x6C, 0x38	|* 0x78: "x"
	.byte	0x38, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0xC6, 0xC6	|* 0x79: "y"
	.byte	0xC6, 0xC6, 0xC6, 0x7E, 0x06, 0x0C, 0xF8, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xCC, 0x18	|* 0x7A: "z"
	.byte	0x30, 0x60, 0xC6, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x0E, 0x18, 0x18, 0x18, 0x70, 0x18	|* 0x7B: "{"
	.byte	0x18, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18	|* 0x7C: "|"
	.byte	0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x70, 0x18, 0x18, 0x18, 0x0E, 0x18	|* 0x7D: "}"
	.byte	0x18, 0x18, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00	|* 0x7E: "~"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6C, 0xC6	|* 0x7F: <del>
	.byte	0xC6, 0xC6, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x80: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x81: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x82: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x83: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x84: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x85: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x86: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x87: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x88: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x89: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x8A: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x8B: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x8C: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x8D: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x8E: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x8F: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x90: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x91: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x92: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x93: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x94: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x95: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x96: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x97: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x98: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x99: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x9A: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x9B: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0x9C: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x10, 0x30, 0x7F, 0xFF	|* 0x9D: "�"
	.byte	0x7F, 0x30, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x08, 0x0C, 0xFE, 0xFF	|* 0x9E: "�"
	.byte	0xFE, 0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF	|* 0x9F: "�"
	.byte	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0xA0: " "
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18	|* 0xA1: "�"
	.byte	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
	.byte	0x00, 0x18, 0x18, 0x7C, 0xC6, 0xC0, 0xC0, 0xC0	|* 0xA2: "�"
	.byte	0xC6, 0x7C, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x38, 0x6C, 0x64, 0x60, 0xF0, 0x60, 0x60	|* 0xA3: "�"
	.byte	0x60, 0x60, 0xE6, 0xFC, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0xC6, 0x7C, 0xC6, 0xC6	|* 0xA4: "�"
	.byte	0xC6, 0xC6, 0x7C, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x7E	|* 0xA5: "�"
	.byte	0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00	|* 0xA6: "�"
	.byte	0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00
	.byte	0x00, 0x7C, 0xC6, 0x60, 0x38, 0x6C, 0xC6, 0xC6	|* 0xA7: "�"
	.byte	0x6C, 0x38, 0x0C, 0xC6, 0x7C, 0x00, 0x00, 0x00
	.byte	0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0xA8: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x7C, 0x82, 0x9A, 0xA2	|* 0xA9: "�"
	.byte	0xA2, 0x9A, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x6C, 0x6C, 0x3E, 0x00, 0x7E	|* 0xAA: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x6C, 0xD8	|* 0xAB: "�"
	.byte	0x6C, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x06	|* 0xAC: "�"
	.byte	0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE	|* 0xAD: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7C, 0x82, 0xB2, 0xAA, 0xB2, 0xAA	|* 0xAE: "�"
	.byte	0xAA, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00	|* 0xAF: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00	|* 0xB0: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18	|* 0xB1: "�"
	.byte	0x18, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x3C, 0x66, 0x0C, 0x18, 0x32, 0x7E, 0x00	|* 0xB2: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x7C, 0x06, 0x3C, 0x06, 0x06, 0x7C, 0x00	|* 0xB3: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00	|* 0xB4: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66	|* 0xB5: "�"
	.byte	0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xC0, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x6C, 0x6C	|* 0xB6: "�"
	.byte	0x6C, 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3C	|* 0xB7: "�"
	.byte	0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	|* 0xB8: "�"
	.byte	0x00, 0x00, 0x18, 0x18, 0x0C, 0x2C, 0x18, 0x00
	.byte	0x00, 0x18, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00	|* 0xB9: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x7C	|* 0xBA: "�"
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x6C, 0x36	|* 0xBB: "�"
	.byte	0x6C, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x60, 0xE0, 0x62, 0x66, 0x6C, 0x18, 0x30	|* 0xBC: "�"
	.byte	0x66, 0xCE, 0x9A, 0x3F, 0x06, 0x06, 0x00, 0x00
	.byte	0x00, 0x60, 0xE0, 0x62, 0x66, 0x6C, 0x18, 0x30	|* 0xBD: "�"
	.byte	0x60, 0xDC, 0x86, 0x0C, 0x18, 0x3E, 0x00, 0x00
	.byte	0x00, 0xE0, 0x30, 0x62, 0x36, 0xEC, 0x18, 0x30	|* 0xBE: "�"
	.byte	0x66, 0xCE, 0x9A, 0x3F, 0x06, 0x06, 0x00, 0x00
	.byte	0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x60	|* 0xBF: "�"
	.byte	0xC0, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0xC0, 0x60, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE	|* 0xC0: "�"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x06, 0x0C, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE	|* 0xC1: "�"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x38, 0x6C, 0xC6, 0x38, 0x6C, 0xC6, 0xC6, 0xFE	|* 0xC2: "�"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x76, 0xDC, 0x00, 0x38, 0x6C, 0xC6, 0xC6, 0xFE	|* 0xC3: "�"
	.byte	0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0xC6, 0xC6, 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6	|* 0xC4: "�"
	.byte	0xFE, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x38, 0x6C, 0x38, 0x10, 0x38, 0x6C, 0xC6, 0xC6	|* 0xC5: "�"
	.byte	0xFE, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3F, 0x73, 0xD9, 0xD8, 0xDA, 0xFE	|* 0xC6: "�"
	.byte	0xDA, 0xD9, 0xDB, 0xDF, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x3C, 0x66, 0xC2, 0xC0, 0xC0, 0xC0	|* 0xC7: "�"
	.byte	0xC0, 0xC2, 0x66, 0x3C, 0x18, 0x70, 0x00, 0x00
	.byte	0x30, 0x18, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78	|* 0xC8: "�"
	.byte	0x68, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x0C, 0x18, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78	|* 0xC9: "�"
	.byte	0x68, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x38, 0x6C, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78	|* 0xCA: "�"
	.byte	0x68, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0xC6, 0xC6, 0x00, 0xFE, 0x66, 0x62, 0x68, 0x78	|* 0xCB: "�"
	.byte	0x68, 0x62, 0x66, 0xFE, 0x00, 0x00, 0x00, 0x00
	.byte	0x30, 0x18, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18	|* 0xCC: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x0C, 0x18, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18	|* 0xCD: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x3C, 0x66, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18	|* 0xCE: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x66, 0x66, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18	|* 0xCF: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xF8, 0x6C, 0x66, 0x66, 0xF6, 0xF6	|* 0xD0: "�"
	.byte	0x66, 0x66, 0x6C, 0xF8, 0x00, 0x00, 0x00, 0x00
	.byte	0x76, 0xDC, 0x00, 0xC6, 0xE6, 0xF6, 0xFE, 0xDE	|* 0xD1: "�"
	.byte	0xCE, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
	.byte	0x30, 0x18, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xD2: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x18, 0x30, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xD3: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x38, 0x6C, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xD4: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x76, 0xDC, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xD5: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0xC6, 0xC6, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xD6: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x7F, 0xFB, 0xD9, 0xDA, 0xDE, 0xDA	|* 0xD7: "�"
	.byte	0xD8, 0xD9, 0xFB, 0x7F, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x04, 0x7C, 0xCE, 0xCE, 0xD6, 0xD6, 0xD6	|* 0xD8: "�"
	.byte	0xD6, 0xE6, 0xE6, 0x7C, 0x40, 0x00, 0x00, 0x00
	.byte	0x30, 0x18, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xD9: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x18, 0x30, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xDA: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x38, 0x6C, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xDB: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0xC6, 0xC6, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6	|* 0xDC: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x0C, 0x18, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C	|* 0xDD: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xF0, 0x60, 0x7C, 0x66, 0x66, 0x66	|* 0xDE: "�"
	.byte	0x66, 0x7C, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0xD8, 0xCC	|* 0xDF: "�"
	.byte	0xC6, 0xC6, 0xC6, 0xCC, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x60, 0x30, 0x18, 0x00, 0x78, 0x0C, 0x7C	|* 0xE0: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x18, 0x30, 0x60, 0x00, 0x78, 0x0C, 0x7C	|* 0xE1: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x10, 0x38, 0x6C, 0x00, 0x78, 0x0C, 0x7C	|* 0xE2: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x76, 0xDC, 0x00, 0x78, 0x0C, 0x7C	|* 0xE3: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xCC, 0xCC, 0x00, 0x78, 0x0C, 0x7C	|* 0xE4: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x38, 0x6C, 0x38, 0x00, 0x78, 0x0C, 0x7C	|* 0xE5: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x36, 0x36	|* 0xE6: "�"
	.byte	0x7E, 0xD8, 0xD8, 0x6E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xC6, 0xC0	|* 0xE7: "�"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x18, 0x70, 0x00, 0x00
	.byte	0x00, 0x60, 0x30, 0x18, 0x00, 0x7C, 0xC6, 0xFE	|* 0xE8: "�"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x0C, 0x18, 0x30, 0x00, 0x7C, 0xC6, 0xFE	|* 0xE9: "�"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x10, 0x38, 0x6C, 0x00, 0x7C, 0xC6, 0xFE	|* 0xEA: "�"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0x00, 0x7C, 0xC6, 0xFE	|* 0xEB: "�"
	.byte	0xC0, 0xC0, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x60, 0x30, 0x18, 0x00, 0x38, 0x18, 0x18	|* 0xEC: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x0C, 0x18, 0x30, 0x00, 0x38, 0x18, 0x18	|* 0xED: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x18, 0x3C, 0x66, 0x00, 0x38, 0x18, 0x18	|* 0xEE: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x66, 0x66, 0x00, 0x38, 0x18, 0x18	|* 0xEF: "�"
	.byte	0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x34, 0x18, 0x2C, 0x06, 0x3E, 0x66	|* 0xF0: "�"
	.byte	0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x76, 0xDC, 0x00, 0xDC, 0x66, 0x66	|* 0xF1: "�"
	.byte	0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x60, 0x30, 0x18, 0x00, 0x7C, 0xC6, 0xC6	|* 0xF2: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x18, 0x30, 0x60, 0x00, 0x7C, 0xC6, 0xC6	|* 0xF3: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x10, 0x38, 0x6C, 0x00, 0x7C, 0xC6, 0xC6	|* 0xF4: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x76, 0xDC, 0x00, 0x7C, 0xC6, 0xC6	|* 0xF5: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0x00, 0x7C, 0xC6, 0xC6	|* 0xF6: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x66, 0xDE, 0xD9, 0xDF	|* 0xF7: "�"
	.byte	0xDE, 0xD8, 0xD9, 0x6E, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xCE, 0xDE	|* 0xF8: "�"
	.byte	0xF6, 0xE6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x60, 0x30, 0x18, 0x00, 0xCC, 0xCC, 0xCC	|* 0xF9: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x18, 0x30, 0x60, 0x00, 0xCC, 0xCC, 0xCC	|* 0xFA: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x30, 0x78, 0xCC, 0x00, 0xCC, 0xCC, 0xCC	|* 0xFB: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x00, 0xCC, 0xCC, 0x00, 0xCC, 0xCC, 0xCC	|* 0xFC: "�"
	.byte	0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x00
	.byte	0x00, 0x0C, 0x18, 0x30, 0x00, 0xC6, 0xC6, 0xC6	|* 0xFD: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7E, 0x06, 0x0C, 0xF8, 0x00
	.byte	0x00, 0x00, 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66	|* 0xFE: "�"
	.byte	0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00
	.byte	0x00, 0x00, 0xC6, 0xC6, 0x00, 0xC6, 0xC6, 0xC6	|* 0xFF: "�"
	.byte	0xC6, 0xC6, 0xC6, 0x7E, 0x06, 0x0C, 0x78, 0x00

|*
|****************************************************************
|*	Tabela dos c�digos para visualiza��o			*
|****************************************************************
|*
|*	0x20: " "	0x21: "!"	0x22: """	0x23: "#"
|*	0x24: "$"	0x25: "%"	0x26: "&"	0x27: "'"
|*	0x28: "("	0x29: ")"	0x2A: "*"	0x2B: "+"
|*	0x2C: ","	0x2D: "-"	0x2E: "."	0x2F: "/"
|*	0x30: "0"	0x31: "1"	0x32: "2"	0x33: "3"
|*	0x34: "4"	0x35: "5"	0x36: "6"	0x37: "7"
|*	0x38: "8"	0x39: "9"	0x3A: ":"	0x3B: ";"
|*	0x3C: "<"	0x3D: "="	0x3E: ">"	0x3F: "?"
|*	0x40: "@"	0x41: "A"	0x42: "B"	0x43: "C"
|*	0x44: "D"	0x45: "E"	0x46: "F"	0x47: "G"
|*	0x48: "H"	0x49: "I"	0x4A: "J"	0x4B: "K"
|*	0x4C: "L"	0x4D: "M"	0x4E: "N"	0x4F: "O"
|*	0x50: "P"	0x51: "Q"	0x52: "R"	0x53: "S"
|*	0x54: "T"	0x55: "U"	0x56: "V"	0x57: "W"
|*	0x58: "X"	0x59: "Y"	0x5A: "Z"	0x5B: "["
|*	0x5C: "\"	0x5D: "]"	0x5E: "^"	0x5F: "_"
|*	0x60: "`"	0x61: "a"	0x62: "b"	0x63: "c"
|*	0x64: "d"	0x65: "e"	0x66: "f"	0x67: "g"
|*	0x68: "h"	0x69: "i"	0x6A: "j"	0x6B: "k"
|*	0x6C: "l"	0x6D: "m"	0x6E: "n"	0x6F: "o"
|*	0x70: "p"	0x71: "q"	0x72: "r"	0x73: "s"
|*	0x74: "t"	0x75: "u"	0x76: "v"	0x77: "w"
|*	0x78: "x"	0x79: "y"	0x7A: "z"	0x7B: "{"
|*	0x7C: "|"	0x7D: "}"	0x7E: "~"	0x7F: ""
|*	0x80: "�"	0x81: "�"	0x82: "�"	0x83: "�"
|*	0x84: "�"	0x85: "�"	0x86: "�"	0x87: "�"
|*	0x88: "�"	0x89: "�"	0x8A: "�"	0x8B: "�"
|*	0x8C: "�"	0x8D: "�"	0x8E: "�"	0x8F: "�"
|*	0x90: "�"	0x91: "�"	0x92: "�"	0x93: "�"
|*	0x94: "�"	0x95: "�"	0x96: "�"	0x97: "�"
|*	0x98: "�"	0x99: "�"	0x9A: "�"	0x9B: "�"
|*	0x9C: "�"	0x9D: "�"	0x9E: "�"	0x9F: "�"
|*	0xA0: "�"	0xA1: "�"	0xA2: "�"	0xA3: "�"
|*	0xA4: "�"	0xA5: "�"	0xA6: "�"	0xA7: "�"
|*	0xA8: "�"	0xA9: "�"	0xAA: "�"	0xAB: "�"
|*	0xAC: "�"	0xAD: "�"	0xAE: "�"	0xAF: "�"
|*	0xB0: "�"	0xB1: "�"	0xB2: "�"	0xB3: "�"
|*	0xB4: "�"	0xB5: "�"	0xB6: "�"	0xB7: "�"
|*	0xB8: "�"	0xB9: "�"	0xBA: "�"	0xBB: "�"
|*	0xBC: "�"	0xBD: "�"	0xBE: "�"	0xBF: "�"
|*	0xC0: "�"	0xC1: "�"	0xC2: "�"	0xC3: "�"
|*	0xC4: "�"	0xC5: "�"	0xC6: "�"	0xC7: "�"
|*	0xC8: "�"	0xC9: "�"	0xCA: "�"	0xCB: "�"
|*	0xCC: "�"	0xCD: "�"	0xCE: "�"	0xCF: "�"
|*	0xD0: "�"	0xD1: "�"	0xD2: "�"	0xD3: "�"
|*	0xD4: "�"	0xD5: "�"	0xD6: "�"	0xD7: "�"
|*	0xD8: "�"	0xD9: "�"	0xDA: "�"	0xDB: "�"
|*	0xDC: "�"	0xDD: "�"	0xDE: "�"	0xDF: "�"
|*	0xE0: "�"	0xE1: "�"	0xE2: "�"	0xE3: "�"
|*	0xE4: "�"	0xE5: "�"	0xE6: "�"	0xE7: "�"
|*	0xE8: "�"	0xE9: "�"	0xEA: "�"	0xEB: "�"
|*	0xEC: "�"	0xED: "�"	0xEE: "�"	0xEF: "�"
|*	0xF0: "�"	0xF1: "�"	0xF2: "�"	0xF3: "�"
|*	0xF4: "�"	0xF5: "�"	0xF6: "�"	0xF7: "�"
|*	0xF8: "�"	0xF9: "�"	0xFA: "�"	0xFB: "�"
|*	0xFC: "�"	0xFD: "�"	0xFE: "�"	0xFF: "�"
