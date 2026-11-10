#include <c64.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <6502.h>
#include <peekpoke.h>


#define SCREEN_ADDR 0xC000
#define CHARSET_ADDR 0xF800
#define SPRITE_ADDR 0xF000

// Atalho para usar array
#define SCREEN ((unsigned char*)SCREEN_ADDR)


// Prototypes
/* --- Protótipos de Funções para evitar erros de declaração --- */
static void clear_screen_c000_with_color(unsigned char color) ;
static void draw_big_title(void) ;
static void fast_clrscr(unsigned char color) ;
static void __fastcall__  put_text(unsigned char x, unsigned char y, const unsigned char* txt) ;
static void refresh_menu_visuals(void) ;
static void setup_sid(void) ;
static void show_menu(void);
static void wait_vblank(void) ;
static void init_random(void);
static void limpa_vic_total(void);


/* --- Globais --- */
unsigned int px, py, bx, by;
int bdx, bdy, score;
unsigned char i, joy;
unsigned char last_joy = 0;   // Para detectar o clique do botão
unsigned char joy1, joy2;   // Leitura das duas portas de joystick
unsigned char joy3, joy4;
unsigned int dx, dy;                   // Posição do Zagueiro
int d_bdx, d_bdy;               // Auxiliares de movimento
unsigned long menu_timer = 0; // Para contar os 30 segundos
unsigned char whistle_timer = 0;
unsigned char whistle_state = 0; // 0: desligado, 1: curto, 2: longo(final), 3: intervalo, 4: curto(final)
unsigned char crowd_timer = 0;
unsigned char crowd_state = 0; // 0: desligado, 1: curto, 2: longo(final), 3: intervalo, 4: curto(final)
unsigned char field_style = 0; // 0 = Checkers, 1 = Solid
unsigned char ia_frame_toggle = 0; 
 
 
/* --- Assets --- */

/* --- Mensagens do Menu (Screen Codes) --- */
/* --- Assets de Texto --- */
const unsigned char msg_op1[]    = {49, 32, 45, 32, 49, 32, 16, 12, 1, 25, 5, 18, 0};
const unsigned char msg_op2[]    = {50, 32, 45, 32, 50, 32, 16, 12, 1, 25, 5, 18, 19, 32, 24, 32, 3, 54, 52, 0};
const unsigned char msg_op3[]    = {51, 32, 45, 32, 49, 32, 16, 12, 1, 25, 5, 18, 32, 24, 32, 50, 32, 16, 12, 1, 25, 5, 18, 0};
const unsigned char msg_prompt[] = {15, 16, 3, 1, 15, 58, 0};


const unsigned char msg_titulo[] = "2goal v1.1"; // "2GOAL v1.1"
const unsigned char msg_line[]  = "--------------------------------------";
const unsigned char msg_credits[] = "by eduardo maronas monks - 2026";
const unsigned char msg_fire[] = "press fire to start";


const unsigned char field_charset[17 * 8] = {

/* 0 - vazio */
0,0,0,0,0,0,0,0,

/* 1 - linha horizontal */
0,0,0,255,255,0,0,0,

/* 2 - linha vertical */
24,24,24,24,24,24,24,24,

/* 3 - canto TL */
0,0,0,31,31,24,24,24,

/* 4 - canto TR */
0,0,0,248,248,24,24,24,

/* 5 - canto BL */
24,24,24,31,31,0,0,0,

/* 6 - canto BR */
24,24,24,248,248,0,0,0,

/* 7 - cruzamento */
24,24,24,255,255,24,24,24,

/* 8 - círculo TL */
0,0,28,50,64,64,64,0,

/* 9 - círculo TR */
0,0,56,76,2,2,2,0,

/* 10 - círculo BL */
0,64,64,64,50,28,0,0,

/* 11 - círculo BR */
0,2,2,2,76,56,0,0,

/* 12 - marca do pênalti */
0,0,0,24,24,0,0,0,

/* 13 - centro do círculo */
0,24,60,126,126,60,24,0,


/* 13–15 não usados */
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,

/* 16 - rede da goleira */
0,  36,  0,  36,  0,  36,  0,  0

};


// Musica
// ==========================
// MÚSICA FUTEBOL (2 VOZES)
// ==========================

static unsigned char chord_step = 0;
static unsigned char step = 0;
static unsigned char frame = 0;


// Dm7 (D F A C)
const unsigned int chord_Dm7[] = {0x1125, 0x1525, 0x1825, 0x1C25};

// G7 (G B D F)
const unsigned int chord_G7[]  = {0x1525, 0x1B25, 0x1125, 0x1525};

// Cmaj7 (C E G B)
const unsigned int chord_Cmaj7[] = {0x1025, 0x1425, 0x1825, 0x1B25};

// A7 (A C# E G)
const unsigned int chord_A7[] = {0x1825, 0x1325, 0x1625, 0x1525};

const unsigned int* progression[] = {
    chord_Dm7,
	chord_Cmaj7,
	chord_Cmaj7,
	chord_Dm7
        
};

#define LEN 8

void music_init(void) {

    SID.amp = 0x02;

    // VOZ 1 — cordas (rasgueado)
    SID.v1.ad = 0x12;   // ataque rápido
    SID.v1.sr = 0x88;

    // VOZ 2 — ghost/percussão
    SID.v2.ad = 0x06;
    SID.v2.sr = 0x02;

    // VOZ 3 — baixo (polegar)
    SID.v3.ad = 0x12;
    SID.v3.sr = 0xA8;
}

void music_play(void) {
    const unsigned int* chord = progression[chord_step];

    frame++;

    // subdivisão (swing natural)
    if (frame < (step & 1 ? 16 : 14)) return;
    frame = 0;

    

    switch (step) {

        // 1 — BAIXO (polegar)
        case 0:
            SID.v3.ctrl = 0x00;
            SID.v3.freq = chord[0];
            SID.v3.ctrl = 0x11;
        break;

        // "&" — STRUM (descendo)
        case 1:
            SID.v1.ctrl = 0x00;
            SID.v1.freq = chord[0];
            SID.v1.ctrl = 0x21;
        break;

        // 2 — GHOST (toque leve)
        case 2:
            SID.v2.ctrl = 0x00;
            SID.v2.freq = 0x0200;
            SID.v2.ctrl = 0x81;
        break;

        // "&" — STRUM (subindo)
        case 3:
            SID.v1.ctrl = 0x00;
            SID.v1.freq = chord[2];
            SID.v1.ctrl = 0x21;
        break;

        // 3 — BAIXO (quinta)
        case 4:
            SID.v3.ctrl = 0x00;
            SID.v3.freq = chord[2];
            SID.v3.ctrl = 0x11;
        break;

        // "&" — STRUM leve
        case 5:
            SID.v1.ctrl = 0x00;
            SID.v1.freq = chord[1];
            SID.v1.ctrl = 0x21;
        break;

        // 4 — GHOST
        case 6:
            SID.v2.ctrl = 0x00;
            SID.v2.freq = 0x0300;
            SID.v2.ctrl = 0x81;
        break;

        // "&" — STRUM final
        case 7:
            SID.v1.ctrl = 0x00;
            SID.v1.freq = chord[3];
            SID.v1.ctrl = 0x21;
        break;
    }

    step++;
    if (step >= 8) {
        step = 0;
        chord_step++;
        if (chord_step >= 4) chord_step = 0;
    }
}

 // Função com a sua estrutura exata, agora recebendo o nome do arquivo
void chamar_menu(const char* nome) {
    // 0. CAPTURA IMEDIATA (Salva os caracteres antes de chavear a memoria)
    unsigned char c1 = nome[0];
    unsigned char c2 = nome[1];

    asm("sei");

    // 1. Restaura Memoria e Kernal
    *((unsigned char*)0x01) = 0x37;   
    *((unsigned char*)0xDD00) = 0x03; 

    // --- MODO BLACKOUT: TUDO PRETO (Cor 0) ---
    *((unsigned char*)0xD020) = 0;    // Borda preta
    *((unsigned char*)0xD021) = 0;    // Fundo preto
    *((unsigned char*)0x0286) = 0;    // Cor do texto do BASIC (cursor) preto
	
	

    // 2. Configura Visual (Cores para o DX)
    *((unsigned char*)0xD018) = 0x14; 
    *((unsigned char*)0xD020) = 0x00; 
    *((unsigned char*)0xD021) = 0x00; 
    *((unsigned char*)0xD015) = 0;    

// 2. Limpa a tela (JSRE544) - Isso remove o lixo da imagem que você postou
    asm("jsr $e544");
	
    // 3. Expandir o limite do buffer
    POKE(649, 20); 

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

    // 6. Volta para o BASIC e executa o buffer
    asm("cli");
    asm("jmp $a474"); 
}



static void init_random(void) {
    // Registradores do SID para gerar número aleatório real
    *(unsigned char*)0xD40E = 0xFF; // FREQ LO (Voz 3)
    *(unsigned char*)0xD40F = 0xFF; // FREQ HI (Voz 3)
    *(unsigned char*)0xD412 = 0x80; // Control (Ruído - Noise)
    
    // Dá um tempo para o SID começar a oscilar
    for(i=0; i<255; i++); 
    
    // Usa o valor do oscilador do SID como semente para o rand do C
    srand(*(unsigned char*)0xD41B); 
}




static void fast_clrscr(unsigned char color) {
    // 32 é o código do espaço vazio (sem gramado)
    memset(SCREEN, 32, 1000); 
    // Força a cor branca em tudo para o texto aparecer
    memset((void*)0xD800, color, 1000); 
}



static void setup_sid(void) {
    SID.amp = 0x1F;        // Volume máximo (15) + sem filtros
    SID.v1.ad = 0x09;     // Attack/Decay para o chute
    SID.v1.sr = 0x00;     // Sustain/Release
}




static void __fastcall__  put_text(unsigned char x, unsigned char y, const unsigned char* txt) {
    unsigned int offset = y * 40 + x;
   // Força escrita em 0xC000
//    unsigned char* screen_ptr = (unsigned char*)(SCREEN_ADDR + offset);
     unsigned char* screen_ptr;
    unsigned char* color_ptr  = (unsigned char*)(0xD800 + offset);
    unsigned char ch;
    
    // Se o VIC estiver no Bank 3 (addr < 0x10), escreve em $C000, senão $0400
    if (VIC.addr < 0x10) { 
        screen_ptr = (unsigned char*)(0xC000 + offset);
    } else {
        screen_ptr = (unsigned char*)(0x0400 + offset);
    }

    while (*txt) {
        ch = (unsigned char)*txt; // Cast aqui resolve os warnings

        // Lógica de conversão que resolve os blocos:
        if (ch >= 'a' && ch <= 'z') {
            ch = ch - 'a' + 1;      // Letras minúsculas
        } else if (ch >= 'A' && ch <= 'Z') {
            ch = ch - 'A' + 1;      // Letras maiúsculas
        } else if (ch >= '0' && ch <= '9') {
            ch = ch - '0' + 48;     // Números (mantém o padrão do charset)
        } else if (ch == ':') {
            ch = 58;
        } else if (ch == ' ' || ch == '-') { // Adicionado hífem
            ch = (ch == ' ') ? 32 : 64;
        }

        *screen_ptr = ch;
        *color_ptr = 1; // Branco para contraste no campo verde
        
        screen_ptr++;
        color_ptr++;
        txt++;
    }
}


static void wait_vblank(void) {
    // Espera enquanto o chip de vídeo (VIC) está renderizando a parte visível (raster < 255)
    while (VIC.rasterline < 255);
    
    // Espera um pouco mais para garantir que ele saiu da linha 255
    while (VIC.rasterline == 255);
}


static void silence_sid(void) {
    unsigned char i;
    // Zera o volume mestre e filtros
    *((unsigned char*)0xD418) = 0;

    // Percorre os 24 registradores principais de áudio zerando tudo
    for (i = 0; i < 24; i++) {
        *((unsigned char*)(0xD400 + i)) = 0;
    }
}



static void draw_big_title(void) {
    // Gradiente: Cinza Escuro -> Cinza Claro -> Branco -> Cinza Claro -> Cinza Escuro
    unsigned char colors[] = {5, 13, 1, 13, 5}; 
    unsigned char y_start = 1;
    unsigned char x_start = 5; // Ajustado para centralizar melhor
    unsigned char row, col;

    // Matriz desenhando 4 G O A L
    const char* title_art[] = {
        "XXXX  XXXX  XXXX  XXXX  X   ",
        "   X  X     X  X  X  X  X   ",
        "XXXX  X XX  X  X  XXXX  X   ",
        "X     X  X  X  X  X  X  X   ",
        "XXXX  XXXX  XXXX  X  X  XXXX"
    };

    for (row = 0; row < 5; ++row) {
        for (col = 0; col < 28; ++col) {
            unsigned int offset = (y_start + row) * 40 + (x_start + col);
            if (title_art[row][col] == 'X') {
                SCREEN[offset] = 87; // Bloco sólido PETSCII
                ((unsigned char*)0xD800)[offset] = colors[row]; 
            } else {
                SCREEN[offset] = 32;  // Espaço vazio (limpa resíduos)
            }
        }
    }
}


static void refresh_menu_visuals(void) {
    VIC.bordercolor = 0; 
    VIC.bgcolor0 = 0;    

    // 1. Restaura o Bank 3 e o VIC
    *((unsigned char*)0xDD00) = (*((unsigned char*)0xDD00) & 0xFC) | 0x00;
    VIC.addr = 0x0E; 

    // 2. Limpa e desenha o título
    fast_clrscr(1);
    clear_screen_c000_with_color(COLOR_WHITE);
    draw_big_title();

    // 3. Textos Estáticos (Ajustados para caber o Modo 5)
    put_text(14, 7, msg_titulo);
    put_text(4, 8, msg_credits);
    put_text(1, 9, msg_line);

     // Sobrescrevemos os textos de opções para listar os novos modos
	put_text(10, 11, (const unsigned char*)"1. CLUB WORLD CUP"); 
	put_text(10, 12, (const unsigned char*)"2. WORLDCUP"); 
    put_text(10, 13, (const unsigned char*)"3. C64 LEAGUE");
	put_text(10, 14, (const unsigned char*)"4. CHAMPIONS LEAGUE");
    put_text(10, 15, (const unsigned char*)"5. TOURNAMENT & FRIENDLY");
    put_text(10, 16, (const unsigned char*)"6. CHALLENGE & TRAINING");
    put_text(10, 17, (const unsigned char*)"7. 4P JOY TESTER");
	
    
 
    // Instrução movida para a linha 17 para dar respiro
    put_text(12, 20, (const unsigned char*)"SELECT GAME MODE");
	
	put_text(1, 22, msg_line);
	put_text(8, 23, (const unsigned char*)"HTTPS://EMMONKS.ITCH.IO/");
    
    // A linha divisória (msg_line) será desenhada pela show_menu 
    // na linha 18 conforme os submenus aparecem.
}

static void clear_screen_c000_with_color(unsigned char color) {
    memset((void*)0xC000, 32, 1000);
    memset((void*)0xD800, color, 1000);
}



static void limpa_vic_total(void) {
    unsigned char i;

    asm("sei");

    // 1. Desabilita todos os Sprites
    VIC.spr_ena = 0x00;
    
    // 2. Reseta Expansão (X e Y)
    VIC.spr_exp_x = 0x00;
    VIC.spr_exp_y = 0x00;

    // 3. Reseta Multicolor e Prioridade
    // $D01C é o Multicolor, $D01B é a Prioridade (Sprite/Background)
    *((unsigned char*)0xD01C) = 0x00; 
    *((unsigned char*)0xD01B) = 0x00;

    // 4. Zera as posições e cores de todos os 8 sprites
    for (i = 0; i < 8; i++) {
        VIC.spr_pos[i].x = 0;
        VIC.spr_pos[i].y = 0;
        VIC.spr_color[i] = 0;
    }

    // 5. Zera o registro de bit extra para a posição X (MSB)
    // Se o sprite estava na metade direita da tela, esse bit fica ligado
    *((unsigned char*)0xD010) = 0x00;

    // 6. Restaura as cores padrão do sistema (Opcional, mas recomendado)
    VIC.bordercolor = 14; // Azul claro
    VIC.bgcolor0 = 6;    // Azul escuro

    // 7. Limpa interrupções pendentes do VIC
    VIC.imr = 0x00; // Desliga máscara
    VIC.irr = 0x81; // Limpa flags

    asm("cli");
}


static void show_menu(void) {
    unsigned char menu_sel = 0;
    unsigned char old_sel = 255; 
    unsigned char joy_now, last_joy_state;
	
	limpa_vic_total();
    
    refresh_menu_visuals();
    
   

    last_joy_state = ~CIA1.pra;

    while(1) {
     //    music_play();
        wait_vblank();
        joy_now = ~CIA1.pra;

       

        // Atualização visual da seta de seleção
        if (menu_sel != old_sel) {
			
            if (old_sel != 255) put_text(8, 11 + old_sel, (const unsigned char*)" "); 
            put_text(8, 11 + menu_sel, (const unsigned char*)">");
            old_sel = menu_sel;
        }

        // Navegação (Cima/Baixo) - Limite de 0 a 5 (6 opções)
        if ((joy_now & 1) && !(last_joy_state & 1) && menu_sel > 0) menu_sel--;
        if ((joy_now & 2) && !(last_joy_state & 2) && menu_sel < 6) menu_sel++;
        
        // Seleção com o botão FIRE (Botão 1 / Port 2)
        if ((joy_now & 0x10) && !(last_joy_state & 0x10)) {
            
             silence_sid();
            // Lógica de chamada baseada na posição do menu
            switch(menu_sel) {
				case 0: chamar_menu("g6");  break; // TEAM WORLDCUP
                case 1: chamar_menu("g5");  break; // WORLDCUP
                case 2: chamar_menu("g4");  break; // C64 LEAGUE
                case 3: chamar_menu("g3");  break; // Champions
                case 4: chamar_menu("g2");  break; // Tournament/Friendly
                case 5: chamar_menu("g1");  break; // Training/Challenge
                
				case 6: chamar_menu("t1");  break; // JOY TESTER
            }
            break; // O código nunca chegará aqui pois chamar_menu dá um JMP
        }

        // Atalhos por teclado (Opcional, mas útil)
        if (kbhit()) {
            unsigned char k = cgetc();
			 silence_sid();
            if (k >= '1' && k <= '7') {
                if (k == '1') {
					 
					chamar_menu("g6");
				}
					
                if (k == '2') {
					chamar_menu("g5");
					 
				}
				
					
                if (k == '3') {
					chamar_menu("g4");
					 
				}
                if (k == '4') {
					chamar_menu("g3");
					 
				}
                if (k == '5') {
					chamar_menu("g2");
					 
				}
                if (k == '6') {
					chamar_menu("g1");
					 
				}
				if (k == '7') {
					chamar_menu("t1");
					 
				}

	//			if (k == '8') {
	//				chamar_menu("t1");
					 
	//			}
            }
        }
        
        last_joy_state = joy_now;
    }
}

static void setup_hardware(void) {
    asm("sei");
    *((unsigned char*)0x01) &= 0xFB; 
    memcpy((void*)CHARSET_ADDR, (void*)0xD000, 2048); 
    *((unsigned char*)0x01) |= 0x04; 
    asm("cli");

    memcpy((void*)(CHARSET_ADDR + 1024), field_charset, sizeof(field_charset));
    
    setup_sid();
}


void main(void) {
    
    unsigned char post_match_sel = 0; 
    
   
    silence_sid();
	
    setup_hardware();   // Prepara os assets

   // music_init();
    
    init_random();
    
    while(1) {
        
        show_menu(); // Configura modo, campo, etc.
        
        
    }
    
}