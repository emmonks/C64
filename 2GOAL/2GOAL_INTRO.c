#include <cbm.h>
#include <string.h>
#include <peekpoke.h>
#include <conio.h>
#include "arena_font.h" 
#include "big_font.h" 

#include <c64.h>
#include <stdlib.h>
#include <6502.h>
#include <stdio.h>

#define FONT_RAM_ADDR 0x2000 
#define SCREEN_RAM    0x0400 // Endereço padrão da tela no Bank 0

#define CHARSET_ADDR 0xE000


#define TCOLOR_RAM 0xD800


#define SCREEN_ADDR 0xC000
#define SCREEN ((unsigned char*)SCREEN_ADDR)

#define SPR_ZAG ((unsigned char*)0xC3F8)


// Velocidades: quanto maior o número, mais lento o scroll
// (Ex: 1 = todo frame, 2 = a cada 2 frames)
#define SPEED_SCROLL_1 3 
#define SPEED_SCROLL_2 4

#define P1_FOOT_X 125  // Onde a bola deve parar no P1 (Oeste)
#define P2_FOOT_X 210  // Onde a bola deve parar no P2 (Leste)



// Endereços típicos (ajuste conforme o seu arquivo SID/PRG)
#define MUSIC_INIT 0x1000 
#define MUSIC_PLAY 0x1003
#define MUSIC_LOAD_ADDR 0x1000

// Prototypes


static unsigned char get_arena_offset(unsigned char c);
//static void logo(void);
static void put_text_b3(unsigned char x, unsigned char y, const char* text, unsigned char color);
//static void draw_big_char_b3(unsigned char x, unsigned char y, unsigned char c_index, unsigned char base_tile);
//static void show_title_b3(void);
static void draw_big_char_b4(unsigned char x, unsigned char y, unsigned char c_index, unsigned char base_tile);
static void __fastcall__  put_text(unsigned char x, unsigned char y, const unsigned char* txt,unsigned char color) ;
//void stop_music(void);

static void setup_sid(void);
static void silence_sid(void);
static void music_init(void);

// Variaveis e structs




unsigned char bx, i, by,pass_state,timer,ball_speed,y_base,y_bola_pes;

int p1_x, p2_x;

unsigned char crowd_timer = 0;
unsigned char crowd_state = 0; // 0: desligado, 1: curto, 2: longo(final), 3: intervalo, 4: curto(final)

unsigned char whistle_timer = 0;
unsigned char whistle_state = 0; // 0: desligado, 1: curto, 2: longo(final), 3: intervalo, 4: curto(final)

static unsigned char frame_count1 = 0;
static unsigned char frame_count2 = 0;


// Referências externas para os dados no assets.s
extern const unsigned char charset_data[];
extern const unsigned char map_data[];
extern const unsigned char map_colour_data[];
//extern unsigned char music_data[];

// Referências externas para os dados no assets2.s
extern const unsigned char charset2_data[];
extern const unsigned char map2_data[];
extern const unsigned char map2_colour_data[];


extern void start_game(void);

// Declare como funções para facilitar a vida do compilador
//extern void music_init(void);
//extern void music_play(void);

// Remova as antigas e use estas:
//extern void install_music_irq(void);
//extern void stop_music(void);


#define DEF_INACTIVE 0
#define DEF_ENTERING 1  // Subindo
#define DEF_KICKING  2  // Chutou e está descendo

unsigned char pass_count = 0;
unsigned char defender_state = DEF_INACTIVE;
int defender_x = 220;  // Centralizado na rota do passe
int defender_y = 250;  // Força ele a começar abaixo da linha visível (baixo da tela)


// Uma sequência de cores que cria um efeito de brilho/transformação
const unsigned char raster_colors[] = {
    COLOR_GREEN, 13, COLOR_YELLOW, COLOR_WHITE, 
    COLOR_GREEN, 13, COLOR_YELLOW, COLOR_WHITE, 
    COLOR_GREEN, 13, COLOR_YELLOW, COLOR_WHITE, 
};


// Sprites
// Sprites



const unsigned char j1[] = {
   0x00,0xFC,0x00,0x00,0x54,0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,0x54,0x00,0x00,0x98,0x00,0x02,0xAA,0x00,0x02,0xBA,0x00,0x0A,0xBA,0x80,0x08,0xB8,0x80,0x08,0xA8,0x80,0x04,0xA8,0x40,0x0C,0xEC,0xC0,0x00,0xB8,0x00,0x00,0xB8,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x86,
  
};


const unsigned char j1r[64] = {
0x00,0xFC,0x00,0x00,0x54,0x00,0x00,0x54,0x00,0x00,0x74,0x00,0x00,0x54,0x00,0x00,0x98,0x00,0x02,0xAA,0x00,0x02,0xBA,0x00,0x08,0xB8,0x80,0x08,0xB8,0x80,0x04,0xA8,0x40,0x0C,0xA8,0xC0,0x00,0xEC,0x00,0x00,0xB8,0x00,0x00,0xB8,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x00,0xCC,0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x86
};




const unsigned char sprite_defender[64] = {
0x00,0x00,0x00,0x00,0xF0,0x00,0x00,0xFC,0x00,0x00,0x54,0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,0x54,0x00,0x02,0x76,0x00,0x0A,0x9A,0x80,0x0A,0xEE,0x80,0x0A,0xBA,0x80,0x04,0xA8,0x40,0x04,0xFC,0x40,0x0C,0xFC,0xC0,0x00,0xCC,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x44,0x00,0x00,0xCC,0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x82
};


const unsigned char sprite_defender_run[64] = {
0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0xFC,0x00,0x00,0x54,0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,0x54,0x00,0x02,0x76,0x00,0x0A,0xBA,0x80,0x0A,0x9A,0x40,0x06,0xEE,0x40,0x04,0xB8,0x40,0x04,0xFC,0xC0,0x0C,0xFC,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x44,0x00,0x00,0x44,0x00,0x00,0xCC,0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x82


};

const unsigned char j2[64] = {
0x00,0xDC,0x00,0x00,0xDC,0x00,0x00,0x54,0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,0x54,0x00,0x00,0x98,0x00,0x0A,0xEE,0x80,0x0A,0xBA,0x80,0x06,0xAA,0x40,0x04,0xA8,0x40,0x04,0xA8,0x40,0x0C,0xFC,0xC0,0x00,0xFC,0x00,0x00,0xFC,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x87

};

const unsigned char j2r[64] = {
0x00,0xDC,0x00,0x00,0x54,0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,0x54,0x00,0x00,0xB8,0x00,0x00,0xDC,0x00,0x02,0xBA,0x00,0x02,0xAA,0x00,0x02,0xAA,0x00,0x01,0xFD,0x00,0x01,0xFD,0x00,0x03,0xFF,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x00,0x88,0x00,0x00,0xCC,0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x87

};

// Testes
const unsigned char j5[64] = {
0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x74,
 0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,
 0x54,0x00,0x00,0x74,0x00,0x00,0x98,0x00,
 0x02,0x9A,0x00,0x02,0xEE,0x00,0x01,0xB9,
 0x00,0x01,0xA9,0x40,0x01,0xBC,0x40,0x03,
 0xFC,0xC0,0x00,0xFC,0x00,0x00,0xCC,0x00,
 0x00,0x44,0x00,0x00,0x44,0x00,0x00,0x88,
 0x00,0x00,0x88,0x00,0x03,0xCF,0x00,0x87


};


const unsigned char j5r[64] = {
0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x30,
 0x00,0x00,0x74,0x00,0x00,0xDC,0x00,0x00,
 0x74,0x00,0x00,0x54,0x00,0x00,0x74,0x00,
 0x02,0xBA,0x00,0x0A,0x9A,0x80,0x0A,0xAA,
 0x80,0x04,0xEC,0x40,0x04,0xB8,0x70,0x04,
 0xAC,0x50,0x0C,0xFC,0x00,0x00,0xCC,0x00,
 0x00,0x44,0x00,0x00,0x44,0x00,0x00,0x84,
 0x00,0x00,0x88,0x00,0x00,0xCC,0x00,0x87


};


const unsigned char j3[64] = {
0x00,0xFC,0x00,0x00,0xFC,0x00,0x00,0xA8,
 0x00,0x00,0x54,0x00,0x00,0xDC,0x00,0x00,
 0x74,0x00,0x00,0x54,0x00,0x00,0xB6,0x00,
 0x02,0x9A,0x00,0x0A,0xAA,0x80,0x0A,0xB8,
 0x40,0x04,0xA8,0x40,0x04,0xFC,0x40,0x0C,
 0xFC,0xC0,0x00,0xCC,0x00,0x00,0x44,0x00,
 0x00,0x44,0x00,0x00,0x48,0x00,0x00,0x88,
 0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x87


};


const unsigned char j3r[64] = {
0x00,0x30,0x00,0x00,0xFC,0x00,0x00,0xA8,
 0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,
 0x54,0x00,0x00,0x74,0x00,0x02,0x98,0x00,
 0x02,0xAA,0x00,0x02,0xBA,0x00,0x01,0xAA,
 0x00,0x01,0xED,0x00,0x01,0xFD,0x00,0x03,
 0xFF,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,
 0x00,0x44,0x00,0x00,0x48,0x00,0x00,0x88,
 0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x87

};


const unsigned char j4[64] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,
 0x00,0x00,0xFC,0x00,0x00,0x74,0x00,0x00,
 0xDC,0x00,0x00,0x74,0x00,0x00,0x54,0x00,
 0x00,0x74,0x00,0x00,0x98,0x00,0x02,0xAA,
 0x00,0x02,0xFE,0x00,0x01,0xA9,0x00,0x01,
 0xFD,0x00,0x03,0xFF,0x00,0x00,0xCC,0x00,
 0x00,0x44,0x00,0x00,0x88,0x00,0x00,0xCC,
 0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x87


};


const unsigned char j4r[64] = {

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,
 0x00,0x00,0xFC,0x00,0x00,0x74,0x00,0x00,
 0x54,0x00,0x00,0x74,0x00,0x00,0x54,0x00,
 0x00,0xA8,0x00,0x02,0xFE,0x00,0x0B,0xAB,
 0x40,0x04,0xA8,0x40,0x04,0xFC,0xC0,0x0C,
 0xFC,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,
 0x00,0x48,0x00,0x00,0x88,0x00,0x00,0xCC,
 0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x87

};



const unsigned char j6[64] = {
0x00,0xFC,0x00,0x00,0x54,0x00,0x00,0xDC,
0x00,0x00,0x74,0x00,0x00,0xDC,0x00,0x00,
0xFC,0x00,0x02,0xDE,0x00,0x02,0xAA,0x00,
0x02,0xEA,0x00,0x02,0xBA,0x00,0x02,0xAE,
0x00,0x01,0xA9,0x00,0x01,0xBD,0x00,0x03,
0xBF,0x00,0x00,0xFC,0x00,0x00,0xCC,0x00,
0x00,0x44,0x00,0x00,0x48,0x00,0x00,0x88,
0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x87



};


const unsigned char j6r[64] = {

0x00,0xFC,0x00,0x00,0xF4,0x00,0x00,0x54,
0x00,0x00,0xDC,0x00,0x00,0x74,0x00,0x00,
0xDC,0x00,0x00,0xFC,0x00,0x02,0xDE,0x00,
0x02,0x9A,0x00,0x0A,0xEA,0x80,0x08,0xB8,
0x80,0x08,0xAC,0x80,0x04,0xA8,0x40,0x0C,
0xB8,0xC0,0x00,0xFC,0x00,0x00,0xCC,0x00,
0x00,0x44,0x00,0x00,0x48,0x00,0x00,0x88,
0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x87

};


const unsigned char j7[64] = {
0x00,0xF0,0x00,0x03,0xFC,0x00,0x03,0x57,0x00,0x03,0xDF,0x00,0x03,0x77,0x00,0x03,0xDF,0x00,0x00,0xFC,0x00,0x02,0xBA,0x00,0x03,0x9B,0x00,0x01,0xFD,0x00,0x01,0xA9,0x00,0x05,0xA9,0x00,0x0C,0xB5,0x00,0x00,0xAC,0x00,0x00,0xFC,0x00,0x00,0xCC,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x00,0xCC,0x00,0x03,0xCF,0x00,0x00,0x00,0x00,0x87




};


const unsigned char j7r[64] = {

0x00,0xF0,0x00,0x00,0xFC,0x00,0x03,0xDC,0x00,0x03,0xD7,0x00,0x03,0xDF,0x00,0x03,0x77,0x00,0x03,0xDF,0x00,0x02,0xFE,0x00,0x03,0xBB,0x00,0x01,0x99,0x00,0x01,0xFD,0x00,0x01,0xA9,0xC0,0x01,0x79,0x40,0x00,0xAC,0x00,0x00,0xFC,0x00,0x00,0xFC,0x00,0x00,0x4C,0x00,0x00,0x44,0x00,0x00,0x88,0x00,0x00,0xCC,0x00,0x00,0x00,0x00,0x87


};



// Puma

// Frame 0: Seu frame base original (bola completamente cheia com os furos no centro)
const unsigned char ball_frame_0[64] = {
   0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x7E,0x00,0x00,0xDF,0x00,0x00,0xE3,0x00,0x00,0xEF,0x00,0x00,0x7E,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01

};

// Frame 1: Furos movendo 1 pixel para a direita
const unsigned char ball_frame_1[64] = {
	0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x7E,0x00,0x00,0xFF,0x00,0x00,0xDF,0x00,0x00,0xE3,0x00,0x00,0x6E,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01



};

// Frame 2: Furos chegando na borda direita (sem quebrá-la)
const unsigned char ball_frame_2[64] = {
0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x7E,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0xDF,0x00,0x00,0x62,0x00,0x00,0x2C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01


};

// Frame 3: Furos indo para a parte de trás da bola
const unsigned char ball_frame_3[64] = {
0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x7E,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0x5E,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01


};

// Frame 4: Furos terminando de dar a volta
const unsigned char ball_frame_4[64] = {
0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x7E,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0x7E,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01




};

// Frame 5: Furos reaparecendo na lateral esquerda
const unsigned char ball_frame_5[64] = {
 0x00,0x00,0x00,0x00,0x2C,0x00,0x00,0x7E,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0x7E,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01


};

// Frame 6: Furos deslizando novamente em direção ao centro
const unsigned char ball_frame_6[64] = {
   0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x6E,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0x7E,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01



};

// Frame 7: Furos completando a volta antes de voltar pro Frame 0
const unsigned char ball_frame_7[64] = {
 0x00,0x00,0x00,0x00,0x1C,0x00,0x00,0x62,0x00,0x00,0xEF,0x00,0x00,0xFF,0x00,0x00,0xFF,0x00,0x00,0x7E,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01


};

/*
static void clear_screen_c000_with_color(unsigned char color) {
    memset((void*)0xC000, 32, 1000);
    memset((void*)0xD800, color, 1000);
}

*/
/*
static unsigned char saved_sprites[5] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
*/
static unsigned char active_sprite[2];

const unsigned char* const idle_gfx[7] = {
    j1, j2, j3, j4, j5, j6, j7
};

const unsigned char* const run_gfx[7] = {
    j1r, j2r, j3r, j4r, j5r, j6r, j7r
};




static void draw_big_title(void) {
    // Gradiente: Cinza Escuro -> Cinza Claro -> Branco -> Cinza Claro -> Cinza Escuro
    unsigned char colors[] = {5, 13, 7, 13, 5}; 
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

static void wait_vblank(void) {
    // Espera enquanto o chip de vídeo (VIC) está renderizando a parte visível (raster < 255)
    while (VIC.rasterline < 255);
    
    // Espera um pouco mais para garantir que ele saiu da linha 255
    while (VIC.rasterline == 255);
}

static void update_sfx(void) {
    unsigned char vib = 0;
    unsigned int basefreq = 0xB400;

    if (whistle_timer > 0)
    {
        whistle_timer--;

        // Vibrato irregular (simula sopro humano)
        vib++;

        if (vib & 1)
            SID.v2.freq = basefreq + 0x40;
        else if (vib & 2)
            SID.v2.freq = basefreq - 0x30;
        else
            SID.v2.freq = basefreq;

        // Pequeno glide descendente natural
        basefreq -= 2;

        return;
    }

    switch (whistle_state)
    {
        case 1:
        case 4:
            SID.v2.ctrl = 0x00;
            whistle_state = 0;
            break;

        case 2:
            SID.v2.ctrl = 0x00;
            whistle_state = 3;
            whistle_timer = 15;
            basefreq = 0xB400;
            break;

        case 3:
            SID.v2.ctrl = 0x00;
            SID.v2.freq = 0xB400;
            SID.v2.ctrl = 0x41;

            whistle_state = 4;
            whistle_timer = 25;
            break;
    }
}

static void init_whistle_voice(void) {
    SID.v2.ctrl = 0x00;
    SID.v2.ad   = 0x08;
    SID.v2.sr   = 0xA8;
    SID.v2.pw   = 0x0600;   // Pulse estreito
}


static void play_sfx_kick(void) {
    SID.v1.ctrl = 0x00;    
    SID.v1.freq = 0x0AF0;  // Frequência mais alta (traz o brilho de volta)
    SID.v1.ad = 0x04;      // Decay um pouco mais curto para dar o "estalo" rápido
    SID.v1.sr = 0x00;      
    SID.v1.ctrl = 0x11;    // Mantém a onda Triângulo
	setup_sid();
}


static void play_goal_crowd(void) {
    SID.v3.ctrl = 0x00;

    SID.v3.ad = 0x04;   // Attack mais suave
    SID.v3.sr = 0x33;   // Sustain moderado

    SID.v3.freq = 0x0C00;
    SID.v3.ctrl = 0x81;   // Noise curto inicial

    crowd_state = 1;
    crowd_timer = 50;
}


static void __fastcall__  put_text(unsigned char x, unsigned char y, const unsigned char* txt, unsigned char color) {
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
        *color_ptr = color; // Branco para contraste no campo verde
        
        screen_ptr++;
        color_ptr++;
        txt++;
    }
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

static void setup_sid(void) {
    SID.amp = 0x1F;        // Volume máximo (15) + sem filtros
    SID.v1.ad = 0x09;     // Attack/Decay para o chute
    SID.v1.sr = 0x00;     // Sustain/Release
}


static void music_intro_init(void) {
    SID.amp = 0x1F; 
    
    SID.v1.ad = 0x00; SID.v1.sr = 0xD8; // Baixo
    SID.v2.ad = 0x00; SID.v2.sr = 0x16; // Hi-Hat
    SID.v2.pw = 0x0300; 

    // Voz 3: Ajustada para permitir o impacto do tambor
    SID.v3.ad = 0x09; // Ataque rápido para o tambor aparecer
    SID.v3.sr = 0x40; 
}

static void music_intro_update(void) {
    static unsigned int tick = 0;
    static unsigned char step = 0;
    static unsigned char passage = 0;
    static unsigned char loop_count = 0;

    tick++;
    if (tick < (passage == 0 ? 7 : 5)) return; 
    tick = 0;

    // Reset padrão para Voz 1 e 2
    SID.v1.ctrl &= 0xFE; 
    SID.v2.ctrl &= 0xFE;
    VIC.bordercolor = 0;

    // --- LÓGICA DO DRONE VS TAMBOR (VOZ 3) ---
    if (step < 20) {
        // Som ambiente enquanto a melodia toca
        if (step == 0) {
            SID.v3.freq = 0x04AB; 
            SID.v3.ctrl = 0x11; // Triangular suave
        }
    } else {
        // Desliga o drone para preparar os tambores ou silêncio
        SID.v3.ctrl &= 0xFE; 
    }

    switch (step) {
        // --- MELODIA ---
        case 0:  SID.v1.freq = 0x095A; SID.v1.ctrl = 0x21; break;
        case 6:  SID.v1.freq = 0x095A; SID.v1.ctrl = 0x21; break;
        case 8:  SID.v1.freq = 0x0B1F; SID.v1.ctrl = 0x21; break;
        case 10: SID.v1.freq = 0x095A; SID.v1.ctrl = 0x21; break;
        case 12: SID.v1.freq = 0x0851; SID.v1.ctrl = 0x21; break;
        case 14: SID.v1.freq = 0x0767; SID.v1.ctrl = 0x21; break;
        case 18: SID.v1.freq = 0x06FD; SID.v1.ctrl = 0x21; break;

        // --- VIRADA DE TAMBOR COM FLASH (FORÇANDO O SOM) ---
        case 20: 
            if (passage == 0 && loop_count == 1) { 
                SID.v3.freq = 0x0500; 
                SID.v3.ctrl = 0x10; // Gate OFF primeiro
                SID.v3.ctrl = 0x11; // Gate ON (Impacto!)
                VIC.bordercolor = 15; 
            } break;
        case 22: 
            if (passage == 0 && loop_count == 1) { 
                SID.v3.freq = 0x0400; 
                SID.v3.ctrl = 0x10; // Gate OFF
                SID.v3.ctrl = 0x11; // Gate ON
                VIC.bordercolor = 1; 
            } break;

        // --- HI-HAT ESPAÇADO ---
        case 24: case 26: case 28: case 30:
            if (passage == 0 && loop_count == 1) {
                SID.v2.freq = 0x6000; 
                SID.v2.ctrl = 0x41; 
            } break;
    }

    step++;
    if (step > 31) { 
        step = 0; 
        loop_count++;
        if (loop_count > 1) { loop_count = 0; passage = !passage; }
    }
}

static void music_intro_stop(void) {
    SID.v1.ctrl = 0x40;
    SID.v3.ctrl = 0x80;
    SID.amp = 0;
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

static void load_team_sprites(void)
{
    unsigned char i;
    unsigned char *dest;

    
        /*
         * Time formado somente por CPU.
         * Os dois sprites são sorteados a cada partida.
         */
        active_sprite[0] = (unsigned char)(rand() % 7);
        do {
            active_sprite[1] = (unsigned char)(rand() % 7);
        } while (active_sprite[1] == active_sprite[0]);
    

   

    /*
     * Copia os gráficos para os quatro slots VRAM.
     */
    for (i = 0; i < 2; ++i) {
        dest = (unsigned char *)(0xC400 + (i << 7));

        memcpy(dest, idle_gfx[active_sprite[i]], 64);
        memcpy(dest + 64, run_gfx[active_sprite[i]], 64);
    }
}

static void setup_hardware(void) {
    // 1. Configuração de Memória e Charset (Mantida a sua lógica original)
    asm("sei");
    *((unsigned char*)0x01) &= 0xFB; 
    memcpy((void*)0xE000, (void*)0xD000, 2048); // Charset em $E000
    *((unsigned char*)0x01) |= 0x04; 
    asm("cli");
/*
    // 2. CARGA DOS SPRITES 
    // --- Jogador 1 (Slots 16-17) ---
    memcpy((void*)0xC400, SPRITE_DATA, 64);      // Slot 16: Idle
    memcpy((void*)0xC440, jogador_run, 64);      // Slot 17: Correndo

    // --- Jogador 2 (Slots 18-19) ---
    memcpy((void*)0xC480, jogador2, 64);         // Slot 18: Idle
    memcpy((void*)0xC4C0, jogador2_run, 64);     // Slot 19: Correndo
*/
    // --- Bola (Slots 20-22) ---
    memcpy((void*)0xC500, ball_frame_0, 64);     // Slot 20
    memcpy((void*)0xC540, ball_frame_1, 64);     // Slot 21
    memcpy((void*)0xC580, ball_frame_2, 64);     // Slot 22

    // --- NOVO: Carga dos DOIS Frames do Zagueiro ---
    memcpy((void*)0xC5C0, sprite_defender, 64);     // Slot 23: Zagueiro Base / Em pé
    memcpy((void*)0xC600, sprite_defender_run, 64); // Slot 24: Zagueiro Correndo

    // 3. CONFIGURAÇÃO DO VIC-II
    VIC.spr_ena |= 0x0F; // Ativa os Sprites 0, 1, 2 e 3
    
    // Modo Multicolor: Ativa bits 0, 1 e 3 (0x0B). Bola (bit 2) continua Hi-Res.
    *((unsigned char*)0xD01C) = 0x0B; 

    VIC.spr_mcolor0 = 10;    // Cor comum 1
    VIC.spr_mcolor1 = 0;     // Cor comum 2 (Preto)

    // EXPANSÃO DO SPRITE DO ZAGUEIRO (Mantendo os outros como estão no seu jogo)
//    *((unsigned char*)0xD017) |= 0x08; // Expande verticalmente (Y) o Sprite 3
//    *((unsigned char*)0xD01D) |= 0x08; // Expande horizontalmente (X) o Sprite 3
	
	*((unsigned char*)0xD017) = 0x0F; // Expande TODOS os sprites na Vertical (Y)
    *((unsigned char*)0xD01D) = 0x0F; // Expande TODOS os sprites na Horizontal (X)

    // 4. CORES DOS SPRITES
    VIC.spr_color[0] = 7;    // P1 Amarelo
    VIC.spr_color[1] = 2;    // P2 Vermelho
    VIC.spr_color[2] = 1;    // Bola Branca
    VIC.spr_color[3] = 7;    // Zagueiro Verde Escuro

    // 5. PONTEIROS DE MEMÓRIA INICIAIS
    SPR_ZAG[0] = 16; 
    SPR_ZAG[1] = 18; 
    SPR_ZAG[2] = 20; 
    SPR_ZAG[3] = 23; // Começa no slot do zagueiro em pé
    
    // Sincroniza fisicamente na RAM de tela ($C000 + 1016)
    *((unsigned char*)0xC3F8) = 16;
    *((unsigned char*)0xC3F9) = 18;
    *((unsigned char*)0xC3FA) = 20;
    *((unsigned char*)0xC3FB) = 23;

    // Posição Inicial Oculta (Abaixo da tela)
    VIC.spr_pos[3].x = 220;
    VIC.spr_pos[3].y = 250;
    *((unsigned char*)0xD010) &= ~0x08; 

    setup_sid();
}

static void update_players_anim_intro(void) {
    // Alterna o bit a cada 8 ticks do sistema (retorna 0 ou 1)
    unsigned char frame_offset = (PEEK(162) & 8) ? 1 : 0;

    // Atualiza jogadores normais
    SPR_ZAG[0] = 16 + frame_offset;
    SPR_ZAG[1] = 18 + frame_offset;
    *((unsigned char*)0xC3F8) = 16 + frame_offset;
    *((unsigned char*)0xC3F9) = 18 + frame_offset;

    // Gerenciamento e Animação do Zagueiro
    if (defender_state != DEF_INACTIVE) {
        // Se o zagueiro está ativo, ele alterna entre o Slot 23 e 24
        unsigned char def_frame = 23 + frame_offset;
        
        SPR_ZAG[3] = def_frame; 
        *((unsigned char*)0xC3FB) = def_frame; 

        // --- FORÇA A EXPANSÃO DINÂMICA ENQUANTO ELE ESTIVER EM CAMPO ---
        *((unsigned char*)0xD017) |= 0x08; // Liga expansão Y do Sprite 3
        *((unsigned char*)0xD01D) |= 0x08; // Liga expansão X do Sprite 3

        // Tratamento do 9º bit para coordenada X > 255
        if (defender_x > 255) {
            VIC.spr_pos[3].x = (unsigned char)(defender_x - 256);
            *((unsigned char*)0xD010) |= 0x08; 
        } else {
            VIC.spr_pos[3].x = (unsigned char)defender_x;
            *((unsigned char*)0xD010) &= ~0x08; 
        }

        VIC.spr_pos[3].y = (unsigned char)defender_y;
    } else {
        // Se estiver inativo, reseta para o frame em pé (23) e esconde embaixo da tela
        SPR_ZAG[3] = 23;
        *((unsigned char*)0xC3FB) = 23;
        defender_y = 250;
        VIC.spr_pos[3].y = 250;

        // --- DESLIGA A EXPANSÃO DO ZAGUEIRO PARA NÃO GERAR BUGS EM OUTRAS TELAS ---
        *((unsigned char*)0xD017) &= ~0x08; // Desliga expansão Y do Sprite 3
        *((unsigned char*)0xD01D) &= ~0x08; // Desliga expansão X do Sprite 3
    }
}

static void update_ball_animation_intro(void) {
    static unsigned char ball_timer = 0;
    static unsigned char ball_frame = 0;

    // Controla a velocidade da rotação (incrementa o frame a cada 4 ticks)
    ball_timer++;
    if (ball_timer >= 4) {
        ball_timer = 0;
        ball_frame++;
        if (ball_frame >= 3) ball_frame = 0;
    }

    // Slots da bola começam em 20 ($C500)
    SPR_ZAG[2] = 20 + ball_frame;

    // Atualiza posição da bola (bx e by devem ser atualizados no loop da intro)
    VIC.spr_pos[2].x = bx & 0xFF;
    VIC.spr_pos[2].y = by;

 //   // Trata o nono bit (MSB) do Sprite 2 (Bola)
 //   if (bx > 255) 
 //       VIC.spr_hi_x |= 0x04; 
 //   else 
 //       VIC.spr_hi_x &= ~0x04;
}




// Ajuste da função de texto para escrever no Banco 3
static void put_text_b3(unsigned char x, unsigned char y, const char* text, unsigned char color) {
    unsigned int offset = (y * 40) + x;
    while (*text) {
        POKE(SCREEN + offset, get_arena_offset(*text++));
        POKE(TCOLOR_RAM + offset, color);
        offset++;
    }
}

// Desenha caracteres 2x2 no Banco 3
/*
static void draw_big_char_b3(unsigned char x, unsigned char y, unsigned char c_index, unsigned char base_tile) {
    unsigned int pos = SCREEN_ADDR + (y * 40) + x;
    unsigned char t = base_tile + (c_index * 4);

    POKE(pos, t);          
    POKE(pos + 1, t + 1);  
    POKE(pos + 40, t + 2); 
    POKE(pos + 41, t + 3); 
    
    // Pinta o caractere grande de branco para destacar do logo
    POKE(0xD800 + (y * 40) + x, 1);
    POKE(0xD800 + (y * 40) + x + 1, 1);
    POKE(0xD800 + ((y+1) * 40) + x, 1);
    POKE(0xD800 + ((y+1) * 40) + x + 1, 1);
}
*/
/*
static void show_title_b3() {
    // 4GOAL (Posicionado acima do logo central)
    draw_big_char_b3(15, 1, 15, 192); // '2'
    draw_big_char_b3(17, 1, 3, 192); // 'G'
    draw_big_char_b3(19, 1, 5, 192); // 'O'
    draw_big_char_b3(21, 1, 0, 192); // 'A'
    draw_big_char_b3(23, 1, 4, 192); // 'L'
//	draw_big_char_b3(25, 1, 9, 192); // 'I'
//	draw_big_char_b3(26, 1, 1, 192); // '.'
//	draw_big_char_b3(27, 1, 9, 192); // 'I'
	
//	 draw_big_char_b3(14, 4, 6, 192); // 'U'
//    draw_big_char_b3(16, 4, 8, 192); // 'N'
//    draw_big_char_b3(18, 4, 9, 192); // 'I'
//    draw_big_char_b3(20, 4, 10, 192); // 'T'
//    draw_big_char_b3(22, 4, 2, 192); // 'E'
//    draw_big_char_b3(24, 4, 11, 192); // 'D'
	
	
}
*/


static void draw_logo(const unsigned char* map, const unsigned char* color, const unsigned char* charset) {
    int i;

    // 1. Limpa a Color RAM para evitar fantasmas visuais na transição
    memset((void*)0xD800, 0, 1000);

    // 2. Transfere o mapa filtrando o tile 5
    for (i = 0; i < 1000; ++i) {
        unsigned char tile = map[i];
        
        if (tile == 5) {
            ((unsigned char*)SCREEN_ADDR)[i] = 0; 
        } else {
            ((unsigned char*)SCREEN_ADDR)[i] = tile;
        }
        ((unsigned char*)0xD800)[i] = color[i]; 
    }

    // 3. Carrega o Charset específico do logo selecionado em $E000
    memcpy((void*)0xE000, charset, 2048);

    // 4. Garante apontamento do VIC (Banco 3)
    *((unsigned char*)0xDD00) = (*((unsigned char*)0xDD00) & 0xFC); 
    VIC.addr = 0x08; 
}

/*
static void logo(void) {
    int i;

    // 1. Força a limpeza da Color RAM para preto antes de mostrar qualquer coisa
    memset((void*)0xD800, 0, 1000);

    // 2. Transfere o mapa FILTRANDO o caractere '5'
    for (i = 0; i < 1000; ++i) {
        unsigned char tile = map_data[i];
        
        // Se for o tile 5, troca pelo tile 0 (espaço na arena_font)
        if (tile == 5) {
            ((unsigned char*)SCREEN_ADDR)[i] = 0; 
        } else {
            ((unsigned char*)SCREEN_ADDR)[i] = tile;
        }
        
        // Aplica a cor original do asset
        ((unsigned char*)0xD800)[i] = map_colour_data[i]; 
    }

    // 3. Carrega o Charset do logo em $E000
    memcpy((void*)0xE000, charset_data, 2048);

    // 4. Ativa o Banco 3 e aponta o VIC
    *((unsigned char*)0xDD00) = (*((unsigned char*)0xDD00) & 0xFC); 
    VIC.addr = 0x08; 
}


*/




// Função para converter PETSCII (C) para o offset da nossa arena_font
// Space=0, A-Z=1-26, 0-9=27-36
static unsigned char get_arena_offset(unsigned char c) {
    if (c == 32) return 128; // Space
    if (c >= 'A' && c <= 'Z') return 128 + (c - 'A' + 1); // Mapeia A->1, B->2, etc.
    if (c >= '0' && c <= '9') return 128 + (c - '0' + 27); // Mapeia 0->27, 1->28, etc.
	
	// 4. Novos Caracteres Especiais (Seguindo a sequência após o '9')
    if (c == '.') return 128 + 37; // 165
    if (c == ':') return 128 + 38; // 166
    if (c == '/') return 128 + 39; // 167
    if (c == '#') return 128 + 40; // 168 (Bloco para a Moldura)
	if (c == '!') return 128 + 41; // 169 (Ponto de Exclamação)
	
    return 128; // Se não for nada disso, usa o espaço
}



static void update_scroll_b3(const char* message) {
    static unsigned char scroll_pos = 0;
    static unsigned char delay = 0;
    int x;

    // Controla a velocidade (processa a cada 2 frames)
    if (++delay < 2) return;
    delay = 0;

    // Move a linha 24 uma posição para a esquerda
    for (x = 0; x < 39; ++x) {
        POKE(SCREEN + (24 * 40) + x, PEEK(SCREEN + (24 * 40) + x + 1));
        POKE(TCOLOR_RAM + (24 * 40) + x, PEEK(TCOLOR_RAM + (24 * 40) + x + 1));
    }

    // Insere o novo caractere
    POKE(SCREEN + (24 * 40) + 39, get_arena_offset(message[scroll_pos]));
    POKE(TCOLOR_RAM + (24 * 40) + 39, COLOR_WHITE);

    scroll_pos++;
    if (message[scroll_pos] == '\0') scroll_pos = 0;
}

static void update_scroll_b3b(const char* message) {
    static unsigned char scroll_pos = 0;
    static unsigned char delay = 0;
    int x;

    // Controla a velocidade (processa a cada 2 frames)
    if (++delay < 2) return;
    delay = 0;

    // Move a linha 24 uma posição para a esquerda
    for (x = 0; x < 39; ++x) {
        POKE(SCREEN + (23 * 40) + x, PEEK(SCREEN + (23 * 40) + x + 1));
        POKE(TCOLOR_RAM + (23 * 40) + x, PEEK(TCOLOR_RAM + (23 * 40) + x + 1));
    }

    // Insere o novo caractere
    POKE(SCREEN + (23 * 40) + 39, get_arena_offset(message[scroll_pos]));
    POKE(TCOLOR_RAM + (23 * 40) + 39, COLOR_GREEN);

    scroll_pos++;
    if (message[scroll_pos] == '\0') scroll_pos = 0;
}


static void wait_frames(int frames) {
    int f;
    for (f = 0; f < frames; f++) {
        while (PEEK(0xD012) != 255); // Espera o raster
        while (PEEK(0xD012) == 255); // Espera o raster passar
    }
}

// Define as cores fixas finais na Color RAM ($D800)
// "4GOAL" em Verde, "LEAGUE" em Amarelo
/*
static void set_final_title_colors() {
    int x, y;
    
    // Cores para "4GOAL" (Linhas 2-3, X=15 a 24)
    for (y = 2; y <= 3; ++y) {
        for (x = 15; x <= 24; ++x) {
            POKE(0xD800 + (y * 40) + x, COLOR_GREEN);
        }
    }
    
    // Cores para "LEAGUE" (Linhas 5-6, X=14 a 25)
    for (y = 5; y <= 6; ++y) {
        for (x = 14; x <= 25; ++x) {
            POKE(0xD800 + (y * 40) + x, COLOR_YELLOW);
        }
    }
}

*/



static void do_raster_transform(int duration_frames) {
    int f, i;
    unsigned char base_color_idx = 0;

    for (f = 0; f < duration_frames; ++f) {
        // 1. Espera o Raster chegar no topo da tela (linha 0) para sincronizar o frame
        while (PEEK(0xD012) != 0);

        // 2. Espera o Raster chegar perto do título (ex: linha 50)
        while (PEEK(0xD012) < 50);

        // --- INÍCIO DO EFEITO NAS LINHAS DO TÍTULO ---
        // Vamos fazer 40 linhas de efeito
        for (i = 0; i < 40; ++i) {
            // Espera o início da próxima linha para evitar "flicker" horizontal
            while (PEEK(0xD012) == (50 + i)); 
            
            // Aplica a cor da tabela baseada no frame e na linha
            POKE(0xD021, raster_colors[(base_color_idx + (i >> 2)) % 10]);
        }

        // 3. Reseta a cor para o resto da tela
        POKE(0xD021, COLOR_BLACK);

        // 4. Controle de Velocidade: Muda a cor base apenas a cada 4 frames
        if (f % 4 == 0) {
            base_color_idx++;
        }
    }
}


static void update_pass_choreography() {
    int pe_p1 = p1_x + 12; 
    int pe_p2 = p2_x - 4;

    if (defender_state == DEF_INACTIVE) {
        switch (pass_state) {
            case 0:
                bx = pe_p1; by = y_bola_pes; timer++;
                if (timer > 60) { timer = 0; pass_state = 1; ball_speed = 4; }
                break;

            case 1: // Bola indo para a direita
                bx += ball_speed;
                if (bx >= pe_p2) { 
                    bx = pe_p2; pass_state = 2; 
                    pass_count++; 
                    if (pass_count >= 3) { 
                        defender_state = DEF_ENTERING; 
                        defender_y = 240; // Começa subindo logo abaixo da borda
                        defender_x = bx - 10; // Alinha horizontalmente com o centro da bola
                    } 
                }
                break;

            case 2:
                bx = pe_p2; by = y_bola_pes; timer++;
                if (timer > 60) { timer = 0; pass_state = 3; ball_speed = -4; }
                break;

            case 3: // Bola indo para a esquerda
                bx += ball_speed;
                if (bx <= pe_p1) { 
                    bx = pe_p1; pass_state = 0; 
                    pass_count++; 
                    if (pass_count >= 3) { 
                        defender_state = DEF_ENTERING; 
                        defender_y = 240;
                        defender_x = bx - 10; 
                    }
                }
                break;
        }
    } 
    // Zagueiro subindo da parte de baixo da tela
    else if (defender_state == DEF_ENTERING) {
        defender_y -= 2; // Subida ligeiramente mais cadenciada para dar precisão visual

        // AJUSTE DO TOQUE: Permite o zagueiro subir até encostar/sobrepor a bola
        if (defender_y <= (y_bola_pes + 4)) {
            defender_state = DEF_KICKING; 
            // --- DISPARA O SOM DO CHUTE NO FRAME EXATO DO IMPACTO ---
            play_sfx_kick();
        }
    } 
    // Zagueiro chutou! Bola vai para a diagonal e zagueiro desce devagar
    else if (defender_state == DEF_KICKING) {
        bx -= 5; // Isola a bola para longe na diagonal esquerda superior
        by -= 4; 

        defender_y += 1; // Retorna devagar para baixo

        // Quando sumir totalmente na borda inferior
        if (defender_y >= 250) {
            pass_count = 0;
            timer = 0;
            pass_state = 0; 
            bx = pe_p1; // Devolve a bola redondinha no pé do Atacante 1
            by = y_bola_pes;
            defender_state = DEF_INACTIVE;
			load_team_sprites();
        }
    }
}

static unsigned char check_start() {
    // Checa Teclado (qualquer tecla)
    if (PEEK(0xC6) > 0) return 1; 

    // Checa Joystick Porta 2 (Botão Fire - Bit 4)
    // No C64, o bit fica em 0 quando pressionado
    if ((PEEK(0xDC00) & 0x10) == 0) return 1;

    return 0;
}





static void chama_menu(const char* nome) {
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
	
	// 6. O "Pulo do Gato": Inicializa o hardware de E/S
  //  asm("jsr $ff81"); // IOINIT

    asm("cli");

    // 7. Salta para o loop principal do BASIC (depois do cabeçalho de boas-vindas)
    asm("jmp $a480");
}



static void draw_url_box_animated() {
    int x, y, i;
    int left, right, top, bottom;
    int tx, ty;

    unsigned char fill;
    unsigned char block;

    const char *url;

    // --- INICIALIZAÇÃO (tudo no topo!) ---
    fill = get_arena_offset(' ');
    block = get_arena_offset('#');

    left = 7;
    right = 31;
    top = 8;
    bottom = 10;

    tx = 8;
    ty = 9;

    url = "HTTPS://EMMONKS.ITCH.IO";

    // --- 1. CAIXA ANIMADA (linha por linha) ---
    for (y = top; y <= bottom; y++) {

        for (x = left; x <= right; x++) {

            if (y == top || y == bottom || x == left || x == right) {
                POKE(0xC000 + (y * 40) + x, block);
                POKE(0xD800 + (y * 40) + x, 13);
            } else {
                POKE(0xC000 + (y * 40) + x, fill);
                POKE(0xD800 + (y * 40) + x, 11); // DARK GRAY manual
            }
        }

        // delay simples
        for (i = 0; i < 2000; i++) { }
    }

    // --- 2. TEXTO "DIGITANDO" ---
    i = 0;

    while (url[i] != 0) {

        char c[2];
        c[0] = url[i];
        c[1] = 0;

        put_text_b3(tx + i, ty, c, COLOR_YELLOW);

        for (x = 0; x < 800; x++) { }

        i++;
    }

    // --- 3. PISCAR BORDA ---
    for (i = 0; i < 3; i++) {

        // cor clara (LIGHT GREEN = 13)
        for (x = left; x <= right; x++) {
            POKE(0xD800 + (top * 40) + x, 13);
            POKE(0xD800 + (bottom * 40) + x, 13);
        }

        for (y = top; y <= bottom; y++) {
            POKE(0xD800 + (y * 40) + left, 13);
            POKE(0xD800 + (y * 40) + right, 13);
        }

        for (x = 0; x < 4000; x++) { }

        // volta ao verde normal
        for (x = left; x <= right; x++) {
            POKE(0xD800 + (top * 40) + x, 5);
            POKE(0xD800 + (bottom * 40) + x, COLOR_GREEN);
        }

        for (y = top; y <= bottom; y++) {
            POKE(0xD800 + (y * 40) + left, 5);
            POKE(0xD800 + (y * 40) + right, COLOR_GREEN);
        }

        for (x = 0; x < 4000; x++) { }
    }
}

/*
static void play_sfx_whistle(void) {
    SID.v2.ctrl = 0x00;

    SID.v2.freq = 0xB400;      // Pitch base
    SID.v2.ctrl = 0x41;        // Pulse + Gate ON

    whistle_state = 1;
    whistle_timer = 22;
}

*/
static void play_sfx_game_over(void) {
    SID.v2.ctrl = 0x00;

    SID.v2.freq = 0xA800;
    SID.v2.ctrl = 0x41;

    whistle_state = 2;
    whistle_timer = 55;
} 


static void abort_to_menu(void) {
    music_intro_stop();
    silence_sid();
    memset((void*)0xC000, 32, 1000); // 32 é o espaço em branco
    memset((void*)0xD800, 1, 1000);  // Cor branca
    *((unsigned char*)0xD015) = 0x00; // Desliga sprites
    
    memcpy((void*)CHARSET_ADDR, charset2_data, 2048);
    draw_logo(map2_data, map2_colour_data, charset2_data); 
    wait_frames(30); // Pausa menor para resposta imediata
    chama_menu("m1");
}


void main() {
    // Variáveis de controle locais para a nossa "Gag" de transição
    unsigned char logo_switched = 0;
    unsigned char intro_gag_done = 0;
    unsigned char color_shift = 0;
    unsigned char frame_count = 0;
    unsigned char i,j;

    // Array de 8 bytes cravados (potência de 2 para usar máscara de bits em vez de módulo)
    static const unsigned char rainbow[8] = {7, 10, 13, 1, 3, 14, 6, 5};

    const char* scroll_msg = "      2GOAL V1.1 - BY EMMONKS - 2026 - THANKS TO BETA TESTERS: VANESSA GREG  TIAGO AL ALAM   JERONIMO  DIEGO SMITHS  PABLO ROSA AND GLADIMIR      ";
    const char* scroll_msg2 = "      ... WELCOME TO 2GOAL V1.1! ... BORN FROM THE 4GOAL SERIES TO BRING BACK THOSE AWESOME MOMENTS AROUND THE COMMODORE 64 ... GRAB A JOYSTICK, CALL YOUR FRIENDS, AND HAVE AN AMAZING TIME ... THANKS FOR PLAYING, EVERYONE! ... CHEERS ...    ";
	
	bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
	//draw_field();
    init_random();
	setup_hardware();
	load_team_sprites();
     // Limpa a tela para o Banco 3 ($C000) e reseta o charset
    memset((void*)0xC000, 32, 1000); // 32 é o espaço em branco
    memset((void*)0xD800, 1, 1000);  // Cor branca para o texto futuro

	
 // =========================================================================
    // PASSO 1: MOSTRAR LOGO ORIGINAL (4GOAL) E ATIVAR OS ATORES
    // =========================================================================
    draw_logo(map_data, map_colour_data, charset_data); 
    put_text(6, 2, (const unsigned char*)"4GOAL GAME SERIES ARE GREAT...",1);
    wait_frames(90);
    init_whistle_voice();
//    play_sfx_whistle();
    play_sfx_game_over();
    // 4. Aguarda leitura
                    for (i = 0; i < 60; i++) {
                        wait_vblank();
                    update_sfx();
                    }
                    silence_sid();
                    setup_sid();
    
    // Configura e expande os sprites para a animação inicial
    VIC.spr_exp_x = 0x07; 
    VIC.spr_exp_y = 0x07;
    VIC.spr_mcolor0 = 10; // Branco
    VIC.spr_mcolor1 = 9;  // Vermelho claro
    VIC.spr_color[0] = 7; // Amarelo
    VIC.spr_color[1] = 5; // Verde
    
    // --- AJUSTE DE POSIÇÃO: Rebaixando os atores para dar respiro ao logo ---
    y_base = 185;       // Mudou de 160 para 185 (Ganhamos 25 pixels de espaço no topo)
    y_bola_pes = 207;   // Mudou de 182 para 207 (Mantém a proporção da bola nos pés)
    
    p1_x = 100; 
    p2_x = 220; 
    bx = p1_x + 20; 
    by = y_bola_pes;

    VIC.spr_pos[0].x = p1_x & 0xFF; VIC.spr_pos[0].y = y_base;
    VIC.spr_pos[1].x = p2_x & 0xFF; VIC.spr_pos[1].y = y_base;
    VIC.spr_pos[2].x = bx & 0xFF;   VIC.spr_pos[2].y = y_bola_pes;
    VIC.spr_hi_x &= ~0x07;

    // Garante que as variáveis de estado comecem zeradas para a intro
    pass_state = 0;
    pass_count = 0;
    timer = 0;
    defender_state = DEF_INACTIVE;

 //   music_intro_init();
 

    // =========================================================================
    // PASSO 2: LOOP DA COREOGRAFIA DA TRANSFORMAÇÃO (INTERATIVO COM O LOGO)
    // =========================================================================
    while (!intro_gag_done) {
		if (check_start()) { abort_to_menu(); return; }
        if (PEEK(0xD012) == 240) { // Sincronia com o Raster
   //         music_intro_update();
            
            // Executa a física original de passes e subida do zagueiro
            update_pass_choreography();
            update_players_anim_intro();
            update_ball_animation_intro();
            
            // Atualiza posições físicas dos jogadores e bola na tela
            VIC.spr_pos[0].x = p1_x & 0xFF; 
            VIC.spr_pos[1].x = p2_x & 0xFF;
            VIC.spr_pos[2].x = bx & 0xFF;
            
            VIC.spr_hi_x &= ~0x07; 
            if (p1_x > 255) VIC.spr_hi_x |= 0x01;
            if (p2_x > 255) VIC.spr_hi_x |= 0x02;

            // --- GAG VISUAL: CHECAGEM DE IMPACTO NO LOGO ---
            // Quando o zagueiro chuta, a bola ganha velocidade negativa em Y (sobe).
            // A coordenada Y aproximada do meio/base do logo é em torno de 110.
            if (defender_state == DEF_KICKING && !logo_switched) {
                if (by <= 110) { 
                    // IMPACTO! Modifica instantaneamente o cenário para o logo do 2GOAL
                    put_text(6, 2, (const unsigned char*)"                              ",7);
                   // Imprime o texto na linha 2, coluna 13
					put_text(13, 2, (const unsigned char*)"2GOAL IS GOAT!",7);
					put_text(15, 4, (const unsigned char*)"2GOAL v1.1",7);
				//	 draw_logo(map2_data, map2_colour_data, charset2_data); 
                //    logo_switched = 1;

					// Inicializa/reseta as variáveis de controle do efeito
					color_shift = 0;
					frame_count = 0;

					// Loop principal do efeito: vai rodar frame por frame por 3 segundos
					// 180 frames = 3 segundos em NTSC (60Hz) ou ~3.6 segundos em PAL (50Hz)
					for (j = 0; j < 120; j++) {
						
						// Desenha o arco-íris na Color RAM
						for (i = 0; i < 14; i++) {
							// DICA DE RETRO-DEV: Não precisa pular os espaços ("if (i & 1) continue")!
							// No C64, alterar a Color RAM de um caractere de espaço (vazio) não muda nada visualmente, 
							// pois não há pixels ativos para receber a cor. É mais barato computacionalmente colorir tudo direto.
							((unsigned char*)0xD800)[2 * 40 + 13 + i] = rainbow[(color_shift + i) & 7];
							
						}

						// Controla a velocidade da animação do Rainbow (muda a cor a cada 4 frames)
						frame_count++;
						if (frame_count >= 4) { 
							color_shift++;
							frame_count = 0;
						}

						// --- O SEGREDO DO TEMPO: Sincroniza com o hardware ---
						// Força o C64 a esperar exatamente 1 frame antes de continuar o loop j++
						wait_frames(1); 
					}
					
                    wait_frames(90);
                     draw_logo(map2_data, map2_colour_data, charset2_data); 
                    logo_switched = 1;
                    
                    // (Opcional) Se tiver um efeito sonoro de explosão/impacto, chame aqui!
                    play_goal_crowd();
                    // 4. Aguarda leitura
                    for (i = 0; i < 40; i++) {
                        wait_vblank();
                    update_sfx();
                    }
                    silence_sid();
                    setup_sid();
                }
            }

            // --- FIM DA COREOGRAFIA ---
            // Se o logo já mudou e o zagueiro terminou de descer (ficando inativo),
            // a nossa brincadeira da intro terminou com sucesso.
            if (logo_switched && defender_state == DEF_INACTIVE) {
                intro_gag_done = 1;
            }

            while(PEEK(0xD012) == 240); 
        }
    }

    // Pequena pausa estratégica com o novo logo estático na tela para dar impacto
  //  wait_frames(90);
	
	for (i = 0; i < 90; i++) {
        wait_vblank();
        if (check_start()) { abort_to_menu(); return; }
    }

    // 2. TRANSIÇÃO (LIMPEZA)
    // Limpa a tela para o Banco 3 ($C000) e reseta o charset
    memset((void*)0xC000, 32, 1000); // 32 é o espaço em branco
    memset((void*)0xD800, 1, 1000);  // Cor branca para o texto futuro
	
	
	// 1. Carrega o Charset original do LOGO (256 caracteres) em $E000
    memcpy((void*)CHARSET_ADDR, charset_data, 2048);
	
	// 2. Copia as fontes para o lugar CORRETO (CHARSET_ADDR = 0xE000)
    // NÃO COPIE PARA 0xC000, pois lá está o seu mapa/logo!
  //  memcpy((void*)CHARSET_ADDR, arena_font_data, sizeof(arena_font_data));
  //  memcpy((void*)(CHARSET_ADDR + 1024), big_font_data, sizeof(big_font_data));
	
	memcpy((void*)(CHARSET_ADDR + 128 * 8), arena_font_data, sizeof(arena_font_data));
    memcpy((void*)(CHARSET_ADDR + 1536), big_font_data, sizeof(big_font_data));
	
	
    // 3. Configuração Final do VIC para o Banco 3
    *((unsigned char*)0xDD00) &= 0xFC; // Banco 3
    VIC.addr = 0x08;                   // Tela em $C000, Charset em $E000
	
	load_team_sprites();
	

    // 3. Desenha o Título e Textos (Agora no Banco 3)
//    show_title_b3();
	draw_big_title();
	
	// 4. Executa o efeito Raster (Colorido) por 2 segundos
   // do_raster_effect(120); 
	// 2. Faz a transformação suave por 3 segundos (180 frames)
    do_raster_transform(30);
    
    // 5. Finaliza com as cores fixas
  //  set_final_title_colors(); // Verde e Amarelo
	
//	draw_url_box();
	
	draw_url_box_animated();
	 put_text_b3(18, 7, "V1.1", COLOR_WHITE);
    put_text_b3(10, 15, "PRESS FIRE TO START", COLOR_YELLOW);
    
    // O texto que apareceu embaixo na sua foto:
//    put_text_b3(1, 23, "DESENVOLVIDO POR EMMONKS - 2026", COLOR_WHITE);
//	put_text_custom(14, 16, "4GOAL LEAGUE"); 
//    put_text_custom(10, 19, "PRESS FIRE TO START");
//    put_text_custom(2, 2, "ABCDEFGHIJLKMNOPQRSTUVWXYZ0123456789");

    // 5. Loop de animação
	
	
 music_intro_init();



   while(1) {

	// 1. Verifica se o usuário quer iniciar
    if (check_start()) {
		music_intro_stop();
		silence_sid();
        // Limpa a tela para o Banco 3 ($C000) e reseta o charset
    memset((void*)0xC000, 32, 1000); // 32 é o espaço em branco
    memset((void*)0xD800, 1, 1000);  // Cor branca para o texto futuro
    *((unsigned char*)0xD015) = 0x00; // Desliga sprites
    
       // 1. Carrega o Charset original do LOGO (256 caracteres) em $E000
       memcpy((void*)CHARSET_ADDR, charset2_data, 2048);
        draw_logo(map2_data, map2_colour_data, charset2_data); 
        // Pequena pausa estratégica com o novo logo estático na tela para dar impacto
		
    wait_frames(90);
        chama_menu("m1");
//		exit_to_basic_and_load();
        break; 
    }
//	wait_vblank(); 
//	music_intro_update();
    if (PEEK(0xD012) == 240) { // Sincronia com o Raster
	// play_music();
        music_intro_update();
        // 1. Atualiza a lógica do passe (movimento bx)
        update_pass_choreography();
		// Atualiza Lógica de Movimento
      //      update_intro_movement(); 
            
            // Atualiza Animações de Sprites
            update_players_anim_intro();
            update_ball_animation_intro();
			// 3. Atualiza posição final da bola no VIC
        // 1. Aplica posições X (apenas os 8 bits baixos)
    VIC.spr_pos[0].x = p1_x & 0xFF; 
    VIC.spr_pos[1].x = p2_x & 0xFF;
    VIC.spr_pos[2].x = bx & 0xFF;
		
		// 2. TRATAMENTO COMPLETO DO MSB (Registro $D010)
    // Primeiro limpamos os bits dos 3 sprites ativos (bits 0, 1 e 2)
    VIC.spr_hi_x &= ~0x07; 

    // Agora ligamos cada um individualmente se necessário
    if (p1_x > 255) VIC.spr_hi_x |= 0x01; // Bit 0 (Sprite 0)
    if (p2_x > 255) VIC.spr_hi_x |= 0x02; // Bit 1 (Sprite 1)
 //   if (bx > 255)   VIC.spr_hi_x |= 0x04; // Bit 2 (Sprite 2)
		

		
        // Scroll 1: Rápido (em todo frame)
        frame_count1++;
        if (frame_count1 >= SPEED_SCROLL_1) {
            update_scroll_b3(scroll_msg);
            frame_count1 = 0;
        }

        // Scroll 2: Lento e com "atraso"
        // Ele só vai se mover uma vez a cada 3 frames do primeiro
        frame_count2++;
        if (frame_count2 >= SPEED_SCROLL_2) {
            update_scroll_b3b(scroll_msg2);
            frame_count2 = 0;
        }

        // Evita múltiplos disparos no mesmo frame
        while(PEEK(0xD012) == 240); 
    }
}


  // Exemplo: se pressionar espaço ou fogo, inicia o jogo
        // if (cgetc() == ' ') break; 

}


