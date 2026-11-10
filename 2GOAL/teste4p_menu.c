#include <stdio.h>
#include <conio.h>
#include <peekpoke.h>
#include <c64.h>
#include <string.h>
#include <stdlib.h>
#include <6502.h>

// Registradores da CIA 1 (Joysticks 1 e 2)
#define CIA1_PRA    (*(volatile unsigned char*)0xDC00)
#define CIA1_PRB    (*(volatile unsigned char*)0xDC01)

// Registradores da CIA 2 (Joysticks 3 e 4 via User Port)
#define CIA2_PRB    (*(volatile unsigned char*)0xDD01)
#define CIA2_DDRB   (*(volatile unsigned char*)0xDD03)


#define SCREEN_ADDR 0xC000

// Atalho para usar array
#define SCREEN ((unsigned char*)SCREEN_ADDR)

unsigned char joy_bits[4];


static void draw_big_title(void) {
    // Array de cores lido direto da ROM
    static const unsigned char colors[5] = {5, 13, 1, 13, 5}; 
    
    // Arte do "4GOAL" comprimida em bits.
    // 1 = Bloco (X), 0 = Espaço. 4 bytes representam até 32 colunas (usamos 28).
    static const unsigned char title_bits[5][4] = {
        { 0xF3, 0xCF, 0x3C, 0x80 }, // XXXX  XXXX  XXXX  XXXX  X
        { 0x12, 0x09, 0x24, 0x80 }, //    X  X     X  X  X  X  X
        { 0xF2, 0xC9, 0x3C, 0x80 }, // XXXX  X XX  X  X  XXXX  X
        { 0x82, 0x49, 0x24, 0x80 }, // X     X  X  X  X  X  X  X
        { 0xF3, 0xCF, 0x24, 0xF0 }  // XXXX  XXXX  XXXX  X  X  XXXX
    };

    // y_start = 1, x_start = 5 -> Offset inicial é (1 * 40) + 5 = 45
    unsigned char* scr_ptr = (unsigned char*)0x0400 + 45;
    unsigned char* col_ptr = (unsigned char*)0xD800 + 45;
    
    unsigned char row, i, b;
    unsigned char current_color, bits, x;

    for (row = 0; row < 5; ++row) {
        current_color = colors[row];
        x = 0; // Controla a coluna atual (até 28)
        
        for (i = 0; i < 4; ++i) {
            bits = title_bits[row][i];
            
            // Avalia bit a bit (da esquerda para a direita)
            for (b = 0; b < 8; ++b) {
                if (x >= 28) break; // Passou do tamanho do logo, ignora o resto
                
                if (bits & 0x80) { // Testa o bit mais significativo
                    scr_ptr[x] = 94;
                    col_ptr[x] = current_color;
                } else {
                    scr_ptr[x] = 32;
                }
                bits <<= 1; // Desloca para o próximo bit
                x++;
            }
        }
        
        // Avança os ponteiros direto para a próxima linha, sem fazer multiplicação
        scr_ptr += 40;
        col_ptr += 40;
    }
}

void read_protovision(void) {
    unsigned char raw3, raw4;

    CIA2_DDRB = 0x80; // Bit 7 como saída

    // --- PORTAS NATIVAS (1 e 2) ---
    joy_bits[0] = (CIA1_PRB ^ 0xFF) & 0x1F;
    joy_bits[1] = (CIA1_PRA ^ 0xFF) & 0x1F;

    // --- LEITURA BRUTA DOS ESTÁGIOS ---
    
    // Estágio A: Seleciona J3 (Bit 7 = 1)
    CIA2_PRB |= 0x80;
    raw3 = CIA2_PRB;

    // Estágio B: Seleciona J4 (Bit 7 = 0)
    CIA2_PRB &= 0x7F;
    raw4 = CIA2_PRB;

    // --- PROCESSAMENTO J3 ---
    // Direções do J3 vêm do Estágio A (raw3)
    joy_bits[2] = (raw3 ^ 0x0F) & 0x0F; 
    // BOTÃO J3: Está vindo do hardware do J4 (raw4)!
    // De acordo com seu relato, o Fire do J3 está associado ao J4 (Bit 5 do raw4)
    if (!(raw4 & 0x20)) joy_bits[2] |= 0x10;

    // --- PROCESSAMENTO J4 ---
    // Direções do J4 vêm do Estágio B (raw4)
    joy_bits[3] = (raw4 ^ 0x0F) & 0x0F;
    // BOTÃO J4: Está vindo do hardware do J3 (raw3)!
    // De acordo com seu relato, o Fire do J4 está associado ao J3 (Bit 4 do raw3)
    if (!(raw3 & 0x10)) joy_bits[3] |= 0x10;
}


unsigned char has_4player_adapter(void) {
    unsigned char check1, check2;

    // 1. Configura o Bit 7 como Saída, o restante como Entrada
    CIA2_DDRB = 0x80;

    // 2. Coloca o Strobe em 1 e lê a porta
    CIA2_PRB |= 0x80;
    check1 = CIA2_PRB & 0x1F; // Pegamos apenas os bits dos joysticks

    // 3. Coloca o Strobe em 0 e lê a porta
    CIA2_PRB &= 0x7F;
    check2 = CIA2_PRB & 0x1F;

    // Se não houver adaptador, os pinos da User Port geralmente ficam 
    // em 1 (high) por causa dos pull-ups, e mudar o Bit 7 não altera os outros.
    // No adaptador Protovision real, a mudança do Strobe causa variações 
    // elétricas detectáveis nos buffers do adaptador.
    
    // Uma forma comum de detecção no C64 é verificar se há resposta ao Strobe
    // ou se os bits não estão todos "presos" em 1.
    if (check1 == 0x1F && check2 == 0x1F) {
        return 0; // Provavelmente vazio (tudo em 1)
    }

    return 1; // Adaptador detectado
}


void chama_menu(const char* nome) {
    // 0. CAPTURA IMEDIATA (Salva os caracteres antes de chavear a memoria)
    unsigned char c1 = nome[0];
    unsigned char c2 = nome[1];

    asm("sei");
    

    // 1. Restaura Memoria e Kernal
    *((unsigned char*)0x01) = 0x37;   
    *((unsigned char*)0xDD00) = 0x03; 

    // 6. O "Pulo do Gato": Inicializa o hardware de E/S
    asm("jsr $ff81"); // IOINIT
   

   *((unsigned char*)0xD011) = 0x1B; // Tela ligada, modo texto
    *((unsigned char*)0xD018) = 0x14; // Memória de vídeo em $0400
    *((unsigned char*)0xD015) = 0x00; // Desliga sprites
    
    // 2. Limpa a tela (JSRE544) - Isso remove o lixo da imagem que você postou
    asm("jsr $e544");
    
    // 3. Força os vetores de interrupção do Kernal (Crucial!)
    *((unsigned char*)0x0314) = 0x31; 
    *((unsigned char*)0x0315) = 0xEA;

   // 4. Reset de ponteiros do BASIC para evitar "Out of Memory"
    // Aponta o início das variáveis para logo após o fim do programa BASIC ($0801)
    POKE(45, 0x01); POKE(46, 0x08); 
    POKE(47, 0x01); POKE(48, 0x08);
    POKE(49, 0x01); POKE(50, 0x08);

    // 3. Expandir o limite do buffer
    POKE(649, 20); 
    
     // --- MODO BLACKOUT: TUDO PRETO (Cor 0) ---
    *((unsigned char*)0xD020) = 0;    // Borda preta
    *((unsigned char*)0xD021) = 0;    // Fundo preto
    *((unsigned char*)0x0286) = 0;    // Cor do texto do BASIC (cursor) preto

    // 4. Injeta os comandos no buffer ($0277 / 631)
    POKE(198, 0); 
    
    POKE(631, 'l'); POKE(632, 'o'); POKE(633, 'a'); POKE(634, 'd');
    POKE(635, 34);  // "
    POKE(636, c1);  // Ex: 'g'
    POKE(637, c2);  // Ex: '1'
    POKE(638, 34);  // "
    POKE(639, 44);  // ,
    POKE(640, '8'); 
    POKE(641, 13);  // RETURN (ASCII 13)
    
    POKE(642, 'r'); POKE(643, 'u'); POKE(644, 'n');
    POKE(645, 13);  // RETURN

    // 5. Define a quantidade exata de caracteres (15)
    POKE(198, 15); 
    


    asm("cli");

    // 7. Salta para o loop principal do BASIC (depois do cabeçalho de boas-vindas)
    asm("jmp $a480");
}


void main(void) {
    unsigned char i;

	
     bgcolor(COLOR_BLACK);      // Fundo preto
    bordercolor(COLOR_BLACK);  // Borda preta
    textcolor(COLOR_WHITE);    // Letras brancas
	//fast_clrscr(1);
	clrscr();

	 draw_big_title();
    gotoxy(0, 8);
    printf("2GOAL - JOYRIDE ENGINE TEST\n");
    printf("---------------------------\n");
	textcolor(COLOR_YELLOW);    // Letras brancas
	printf("       M KEY TO MENU        \n");
	textcolor(COLOR_WHITE);    // Letras brancas

    while(1) {
        if (kbhit() && cgetc() == 77) break;
      //  draw_big_title();
        read_protovision();
		
        for (i = 0; i < 4; ++i) {
            gotoxy(0, 13 + i);
            printf("PLAYER %d: %02x ", i + 1, joy_bits[i]);
            
            // Desenha as letras apenas se o bit estiver setado (1)
            printf("%c %c %c %c %c    ",
                (joy_bits[i] & 0x01) ? 'U' : '.',
                (joy_bits[i] & 0x02) ? 'D' : '.',
                (joy_bits[i] & 0x04) ? 'L' : '.',
                (joy_bits[i] & 0x08) ? 'R' : '.',
                (joy_bits[i] & 0x10) ? 'F' : '.');
        }
        
        // Pequeno delay para o VICE não "engasgar" na User Port
        for(i=0; i<100; ++i);
     }
	   bgcolor(COLOR_BLACK);      // Fundo preto
    bordercolor(COLOR_BLACK);  // Borda preta
    textcolor(COLOR_BLACK);    // Letras brancas
	clrscr();
chama_menu("m1");

}