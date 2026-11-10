/*
Modo Challenge - game_mode == 6
* Falta acertar o bonus de tentativas - > está acumulando, resolvido - Feito
* Verificar no numero de tentativas no game over -> ok
* Verificar a tela de campeao: ok, colocar o numero de gols final para cada jogador -: Feito
* Testar os niveis de progressao: com zagueiros fica bem mais dificil
* Falha no ATTEMPTS quando perde no estagio 1 (sujeira) -> nao apareceu..., perder no primeiro estágio -> n~çao apareceu mais
* Ajustada a transição de estágios quando falha e quando avança -> Feito
* Jogador perdido no campo com P1 somente? Funções de roubo de bola ativadas, usar a flag use_defender e não modo de jogo -> Feito
* Zagueiro chutando para trás, parece ser no roubo de bola o problema -> resolvido, ajustada a função perform_steal -> Feito
* Modo Demo, random: acertar as combinações possíveis paa evitar fantasmas
* Revisar o codigo

*/

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




#define VIC_ADDR_GAME 0x1E       // Valor para usar Charset em $3800 e Screen $0400
#define VIC_ADDR_MENU 0x15       // Valor para usar Charset da ROM (Letras)

#define SPRITE_DATA_ADDR 0xC800  // Logo após o charset
#define SPRITE_BASE_PTR 200      // 200 * 64 = 12800 -> $C800


// Screen Codes
#define FG_CH_HLINE 65
#define FG_CH_VLINE 67
#define CH_NET   72
#define CH_BLANK 32
#define CH_CORNER_UL 68
#define CH_CORNER_UR 69
#define CH_CORNER_DL 70
#define CH_CORNER_DR 71


#define GOAL_MIN 17
#define GOAL_MAX 23        /* 7 tiles */

#define GOAL_TOP 10
#define GOAL_BOT 16        /* 7 tiles */

#define AREA_NORTE_Y 4
#define AREA_SUL_Y   20
#define AREA_OESTE_X 4
#define AREA_LESTE_X 35


#define CH_BASE 128

#define EMPTY    (CH_BASE + 0)
#define HLINE    (CH_BASE + 1)
#define VLINE    (CH_BASE + 2)
#define CTL      (CH_BASE + 3)
#define CTR      (CH_BASE + 4)
#define CBL      (CH_BASE + 5)
#define CBR      (CH_BASE + 6)
#define CROSS    (CH_BASE + 7)

#define CCTL     (CH_BASE + 8)
#define CCTR     (CH_BASE + 9)
#define CCBL     (CH_BASE +10)
#define CCBR     (CH_BASE +11)

#define PENALTY  (CH_BASE +12)
#define CCENTER  (CH_BASE +13)
#define TAREA    (CH_BASE +14)
#define TIAREA   (CH_BASE +15)
#define NET      (CH_BASE +16)

// Exemplo de posições das marcas (ajuste conforme seu draw_field)
#define PENALTY_N_X 176
#define PENALTY_N_Y 80
#define PENALTY_S_X 176
#define PENALTY_S_Y 210
#define PENALTY_W_X 60
#define PENALTY_W_Y 146
#define PENALTY_E_X 290
#define PENALTY_E_Y 146

#define SPR_PLAYER 192


#define SPR_ZAG ((unsigned char*)0xC3F8)


#define FIELD_LEFT    24
#define FIELD_RIGHT   320
#define FIELD_TOP     50
#define FIELD_BOTTOM  229

#define GOAL_N_X 176
#define GOAL_N_Y  40

#define GOAL_S_X 176
#define GOAL_S_Y 210

#define GOAL_W_X  40
#define GOAL_W_Y 146

#define GOAL_E_X 296
#define GOAL_E_Y 146


#define MAX_KICK_POWER 10          // Velocidade máxima da bola
#define MIN_KICK_POWER 3           // Velocidade mínima (toque rápido)

#define PENALTY_PLAYER_DIST 12

// Sugestão: Defina constantes para facilitar o ajuste global
#define HUMAN_STUN 30
#define IA_STUN 60    // IA fica o dobro do tempo parada ao ser desarmada


// Definições de cores padrão do C64
#define C64_YELLOW 7
#define C64_RED    2
#define C64_GREEN  5
#define C64_BLUE   6
#define C64_PINK   10


#define KEEP_IN_BOUNDS(vx, vy) \
    if (vx < 24)  vx = 24; else if (vx > 318) vx = 318; \
    if (vy < 56)  vy = 56; else if (vy > 222) vy = 222;


// Prototypes
/* --- Protótipos de Funções para evitar erros de declaração --- */
static void apply_boundaries(void) ;
static void __fastcall__  apply_net_stretch(unsigned char tx, unsigned char ty, unsigned char mode) ;
//static void clear_menu_area(void) ;
static void clear_screen_c000_with_color(unsigned char color) ;
//static void display_final_score(unsigned char s1, unsigned char s2) ;
static void draw_big_title(void) ;
//static void draw_field(void) ;
static void draw_field(unsigned char style);
static void enter_game_mode(void) ;
static void enter_menu_mode(void);
static void fast_clrscr(unsigned char color) ;
static void handle_ball_logic_v2(void);
static void init_crowd_voice(void);
//static void init_high_scores() ;
static void init_whistle_voice(void) ;
//static void input_name(unsigned int final_score, unsigned char joy_id, unsigned char diff, unsigned char def, unsigned char fld) ;
//static void insert_and_sort(char* new_name, unsigned int new_score, unsigned char diff, unsigned char def, unsigned char fld) ;
//static void __fastcall__  keep_defender_in_field(unsigned int* x, unsigned int* y) ;
//static void __fastcall__  keep_in_bounds(unsigned int *x, unsigned int *y) ;
//static void load_screen(void) ;
//static void move_defender_ia(void) ;
static void move_defender_ia(unsigned char is_south);
static void move_goleiros_ia(void) ;
static void move_goleiros_p2(void) ;
static void perform_steal(unsigned char new_owner) ;
static void play_goal_crowd(void);
static void play_match(void) ;
static void play_sfx_defend(void) ;
static void play_sfx_game_over(void);
static void play_sfx_kick(void);
static void play_sfx_whistle(void);
static void __fastcall__  put_num(unsigned char x, unsigned char y, unsigned char num) ;
static void __fastcall__  put_text(unsigned char x, unsigned char y, const unsigned char* txt) ;
static void read_joysticks(void) ;
static void refresh_menu_visuals(void) ;
static void reset_ball_to_corners(void) ;
static void reset_border_color(void) ;
static void reset_game_state(void) ;
static void setup_hardware(void);
static void setup_sid(void) ;
//static void show_high_scores_logic(void);
//static void show_high_scores(void);
static void show_menu(void);
static void update_ball_animation(void) ;
static void update_dynamic_pointers(void);
static void update_players_anim(void) ;
static void update_sprites(void) ;
static void wait_vblank(void) ;
static void interrupt_raster_handler(void); 
static void display_final_score_individual_v5(void);
static void draw_field_manager(void);
//static void draw_field_checkers(void);
static void move_player_ia_modo5(unsigned char p_idx);
//static void move_player1_ia_modo5(void);
//static void move_player2_ia_modo5(void);
//static void move_player3_ia_modo5(void);
//static void move_player4_ia_modo5(void);
static void calculate_keeper_rating(unsigned char goals_against);
static void __fastcall__ put_num2(unsigned char x, unsigned char y, unsigned int num) ;
//static void separate_ia_players(void);
//static void __fastcall__  apply_smart_kick(unsigned char p_idx);
static void __fastcall__  apply_smart_kick2(unsigned char p_idx);
static void __fastcall__  paint_row_color(unsigned char y, unsigned char x, unsigned char len, unsigned char color);
static unsigned int __fastcall__ keep_x_v5(unsigned int x);
static unsigned char __fastcall__ keep_y_v5(unsigned char y);
static unsigned char get_fixed_player_color(unsigned char p_idx);
static void bounce_ball_at_walls(void) ;
//static void reset_keeper_memory(void);
//static void display_stats_mode3(void);
static void separate_all_players(void);

static void show_game_over_confirm(void);
static void show_interstage_screen(void);

static void show_tournament_champion(void);
static void draw_champion_title(unsigned char color_offset);
//static void setup_match_colors(unsigned char h_id, unsigned char a_id);

//static void prepare_tournament_teams(void);
static void init_random(void);
static void sorteia_personalidades(void);

static void music_init(void);
static void music_play(void);
static void music_stop(void);

//extern void __fastcall__ chama_menu_asm(const char* nome);


// Tabelas de ajuste por Dificuldade [Fácil (0), Normal (1), Difícil (2)]
static const unsigned char IA_DIST_Y[]      = {20, 40, 60};  // Distância para perseguição total
static const unsigned char IA_TIMER_MASK[]  = {7,  3,  1};  // Máscara de frames (7 = a cada 8 frames, 1 = a cada 2 frames)
static const unsigned char IA_CHUTE_BASE[]  = {3,  5,  7};  // Força base do bico para frente
static const unsigned char IA_CHUTE_RAND[]  = {1,  3,  3};  // Variação aleatória da força
static const unsigned char IA_WAIT_TIME[]   = {50, 35, 15};  // Tempo de travamento após o chute

/*
// Tabelas de ajuste por Dificuldade [Fácil (0), Normal (1), Difícil (2)]
static const unsigned char IA_DIST_Y[]      = {20, 40, 60};  // Distância para perseguição total
static const unsigned char IA_TIMER_MASK[]  = {7,  3,  1};  // Máscara de frames (7 = a cada 8 frames, 1 = a cada 2 frames)
static const unsigned char IA_CHUTE_BASE[]  = {5,  7,  9};  // Força base do bico para frente
static const unsigned char IA_CHUTE_RAND[]  = {2,  3,  4};  // Variação aleatória da força
static const unsigned char IA_WAIT_TIME[]   = {40, 25, 10};  // Tempo de travamento após o chute
*/
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

/* T invertido - TAREA */
0,0,0,255,255,24,24,24,

/* T normal - TIAREA */
24,24,24,255,255,0,0,0,


/* 16 - rede da goleira */
0,  36,  0,  36,  0,  36,  0,  0

};



/* --- Globais --- */
unsigned int px, py, bx, by;
int bdx, bdy, score, timer;
unsigned char i, joy;
int gx[4], gy[4], g_dir[4];
//unsigned char possession = 0; // 0: solta, 1: com jogador, 2-5: com robôs
//unsigned int kick_timer = 0;  // Contador para os 2 segundos
//unsigned char last_joy = 0;   // Para detectar o clique do botão
unsigned char game_mode = 1;      // <--- ADICIONADO AQUI
unsigned char ball_possession = 0; 
unsigned int possession_timer = 0; 
unsigned char kick_cooldown = 0; 
unsigned char kick_cooldown_b = 0; 
unsigned char difficulty_boost = 0; 
unsigned char goal_delay = 0; 
unsigned char g_level[4] = {0,0,0,0}; // Inicializado
unsigned int p2x, p2y;      // Posição do Player 2
unsigned int p3x, p3y;      // Posição do Player 3
unsigned int p4x, p4y;      // Posição do Player 4
unsigned char joy1, joy2;   // Leitura das duas portas de joystick
unsigned char joy3, joy4;
//unsigned char ball_owner = 0; // 0: Ninguém, 1: P1, 2: P2
//unsigned char p2_defended = 0;
unsigned char s1_final = 0; // Renomeei para evitar confusão com outras variáveis
unsigned char s2_final = 0;
unsigned char last_touch = 0;        // 1 para P1, 2 para P2
unsigned char after_touch_timer = 0; // Contador do efeito
unsigned char field_type = 0; // 0: Grama, 1: Molhado, 2: Neve
unsigned char selected_field_option = 0; // 0: Grama, 1: Molhado, 2: Neve, 3: Aleatório
unsigned char use_defender = 0; // 0: Não, 1: Sim
unsigned int dx, dy;                   // Posição do Zagueiro
unsigned int dx_b, dy_b;                   // Posição do Zagueiro
//int d_bdx, d_bdy;               // Auxiliares de movimento
unsigned char d_level = 0;      // Nível da IA do zagueiro
unsigned char spr_active_p1       = 1;
unsigned char spr_active_p2       = 0;
unsigned char spr_active_ball     = 1;
//unsigned char spr_active_defender = 0;
unsigned char spr_hix_mask;
//unsigned char defender_stuck = 0;
//unsigned char tries = 0;
unsigned char kick_power = 0;      // Acumulador de força
unsigned char defender_wait = 0; // Timer de "paralisia" inicial da IA
unsigned char defender_wait_b = 0; // Timer de "paralisia" inicial da IA
//unsigned char menu_option = 0; // 0: Start, 1: Field, 2: Mode, etc.
unsigned char last_joy_state = 0; // Para evitar que o menu "corra" muito rápido
unsigned char joy_now = 0;
unsigned int hs_timer = 0;  // Contador de frames
//unsigned char in_hs_screen = 0; // 0 = Menu, 1 = Tela de High Score
//unsigned char current_mode = 0; // 0 = Rock, 1 = Carnaval
//unsigned char current_song = 0; 
//unsigned char step = 0;
//unsigned char loops = 0;
//unsigned int  tick = 0;
//unsigned char mira_pos = 0;  
//signed char mira_vel = 4;    
unsigned char difficulty_level = 1; // 0: Easy, 1: Normal, 2: Hard
//unsigned char anim_timer = 0;
unsigned char num_players = 1; // 1, 2, 3 ou 4 jogadores humanos
unsigned char joy_bits[4]; // Adicione esta linha no escopo global
unsigned char s3_final, s4_final;
//unsigned char defender_kick_count = 0; // Contador de afastamentos seguidos

//unsigned char stun_timer = 0;
//unsigned char stunned_player = 0;
unsigned char p2_is_moving = 0; // 0 = parado, 1 = em movimento

unsigned char p1_is_moving = 0;
//unsigned char defender_is_moving = 0; // Nova 
unsigned char p3_is_moving = 0;
unsigned char p4_is_moving = 0;
unsigned char p5_is_moving = 0;
unsigned char p6_is_moving = 0;
unsigned char player_type[5]; // player_type[2] é o P2, etc.
static unsigned int saves_p2 = 0;
static unsigned int keeper_score = 0;
unsigned char stun_timer_p1 = 0;
unsigned char stun_timer_p2 = 0;
unsigned char stun_timer_p3 = 0;
unsigned char stun_timer_p4 = 0;
static unsigned char shot_delay_p1 = 0; 
static unsigned char shot_delay_p2 = 0; 
static unsigned char shot_delay_p3 = 0; 
static unsigned char shot_delay_p4 = 0; 
unsigned char gold_goal_active = 0;
//unsigned char wall_hit_count = 0; // Conta chutes seguidos na parede
//unsigned char last_hit_side = 0;  // 1=Norte, 2=Sul, 3=Oeste, 4=Leste
unsigned char wall_insist_count = 0;
//unsigned char last_kick_dir_x = 0;
//unsigned char last_second = 0; // Armazena o último segundo processado
//unsigned char redraw_time = 0; // Flag para redesenhar
unsigned char is_overtime = 0;
unsigned char golden_score_triggered = 0;
unsigned char enemy_color = 2; // Default vermelho

unsigned char sys_tick = 0;
unsigned char player_pers[5]; /* Usamos 5 para ignorar o índice 0 e usar 1 a 4 direto */

// --- 0. ADICIONAR ESTA VARIÁVEL GLOBAL OU ESTÁTICA ---
//static unsigned char defender_flee_timer = 0;
 
 // Demo
unsigned char is_demo_mode = 0; 
unsigned int demo_frame_counter;

 
 unsigned char ia_frame_toggle = 0; 

 unsigned char current_half = 1; // 1 = 1st Half, 2 = 2nd Half
 
 unsigned char ot_base1, ot_base2, ot_base3, ot_base4;
unsigned char e1, e2, e3, e4;   /* placares efetivos para decidir o vencedor */
unsigned char eff[4];					 

/*
typedef struct {
    char name[4]; // 3 letras + null terminator
    unsigned int score;
	unsigned char difficulty; // 0:E, 1:N, 2:H
    unsigned char has_defender; // 0:No, 1:Yes
    unsigned char field; // 0:G, 1:W, 2:F
} HighScore;

HighScore topScores[5];
*/

//unsigned long menu_timer = 0; // Para contar os 30 segundos
//unsigned char showing_hs = 0; // Flag se está na tela de recordes


unsigned char last_goal_tx = 0;
unsigned char last_goal_ty = 0;


unsigned char whistle_timer = 0;
unsigned char whistle_state = 0; // 0: desligado, 1: curto, 2: longo(final), 3: intervalo, 4: curto(final)


unsigned char crowd_timer = 0;
unsigned char crowd_state = 0; // 0: desligado, 1: curto, 2: longo(final), 3: intervalo, 4: curto(final)

unsigned char field_style = 0; // 0 = Checkers, 1 = Solid

static unsigned char goal_text_backup[7];
static unsigned char goal_color_backup[7];


//static unsigned char last_p2_state = 0xFF;

static unsigned char deflect_cooldown = 0; 
static unsigned char keeper_recovery[4] = {0, 0, 0, 0};
static signed char last_keeper = -1; // -1 = bola neutra
static unsigned char inter_keeper_passes[4][4] = {0};

// Dificuldade para cada um dos 10 times (IA Speed/Reaction)
const unsigned char tournament_levels[] = {0, 0, 0, 1, 1, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2}; 

// Cores dos times adversários (evitando amarelo e a cor do gramado)
// Gramado 0 (Verde): Adversários podem ser 2(Verm), 7(Amarelo-P1), 1(Branco), 10(Verm. Claro)
// Usaremos cores que contrastam com Verde, Azul e Cinza.
//const unsigned char enemy_colors[] = {2, 6, 4, 8, 10, 12, 14, 15, 9, 1}; 

unsigned char tournament_stage = 0; // Estágio atual (0-9)

unsigned char tournament_opponents[15]; // 10 oponentes para os 10 estágios

unsigned char s1_total = 0; // Renomeei para evitar confusão com outras variáveis
unsigned char s2_total = 0;

unsigned char winner = 0;
unsigned char is_tie = 0;

unsigned int challenge_bonus_threshold = 30;
unsigned char challenge_human_count = 1;
unsigned char challenge_bonuses_given = 0;

static const signed char lut_kick_br[9][4][3] = {
    // Zona 0 (Sup. Esq) - Alvos: Goleira Norte (Direita) e Oeste (Baixo)
    {{ 1, 0, 0}, { 1, 1, 1}, { 0, 1, 0}, { 1, 1,-1}}, 
    
    // Zona 1 (Norte Central) - Alvos: Goleira Sul (Baixo) e Diagonais para Leste/Oeste
    {{ 0, 1, 0}, { 1, 1, 1}, {-1, 1,-1}, { 0, 1, 1}},

    // Zona 2 (Sup. Dir) - Alvos: Goleira Norte (Esquerda) e Leste (Baixo)
    {{-1, 0, 0}, {-1, 1,-1}, { 0, 1, 0}, {-1, 1, 1}},

    // Zona 3 (Oeste Central) - Alvos: Goleira Leste (Direita) e Diagonais para Norte/Sul
    {{ 1, 0, 0}, { 1,-1, 1}, { 1, 1,-1}, { 1, 0, 1}},

    // Zona 4 (Centro) - Alvos: As 4 Goleiras (Chutes diretos)
    {{ 1, 0, 0}, {-1, 0, 0}, { 0, 1, 0}, { 0,-1, 0}},

    // Zona 5 (Leste Central) - Alvos: Goleira Oeste (Esquerda) e Diagonais para Norte/Sul
    {{-1, 0, 0}, {-1,-1,-1}, {-1, 1, 1}, {-1, 0,-1}},

    // Zona 6 (Inf. Esq) - Alvos: Goleira Sul (Direita) e Oeste (Cima)
    {{ 1, 0, 0}, { 1,-1,-1}, { 0,-1, 0}, { 1,-1, 1}},

    // Zona 7 (Sul Central) - Alvos: Goleira Norte (Cima) e Diagonais para Leste/Oeste
    {{ 0,-1, 0}, { 1,-1,-1}, {-1,-1, 1}, { 0,-1,-1}},

    // Zona 8 (Inf. Dir) - Alvos: Goleira Sul (Esquerda) e Leste (Cima)
    {{-1, 0, 0}, {-1,-1, 1}, { 0,-1, 0}, {-1,-1,-1}}
};

// --- TABELAS DE ESTÁGIOS DO MODO CHALLENGE (18 ESTÁGIOS) ---
// Formato: { num_adversarios, dificuldade (0=Easy, 1=Normal, 2=Hard), zagueiro (0=No, 1=Yes), min_gols }

const unsigned char challenge_stage_1p[18][4] = {
    {1, 0, 0, 3}, {1, 1, 0, 2}, {1, 2, 0, 1},
    {1, 0, 1, 3}, {1, 1, 1, 2}, {1, 2, 1, 1},
    {2, 0, 0, 3}, {2, 1, 0, 2}, {2, 2, 0, 1},
    {2, 0, 1, 3}, {2, 1, 1, 2}, {2, 2, 1, 1},
    {3, 0, 0, 3}, {3, 1, 0, 2}, {3, 2, 0, 1},
    {3, 0, 1, 3}, {3, 1, 1, 2}, {3, 2, 1, 1}
};

const unsigned char challenge_stage_2p[18][4] = {
    {1, 0, 0, 4}, {1, 1, 0, 3}, {1, 2, 0, 2},
    {1, 0, 1, 4}, {1, 1, 1, 3}, {1, 2, 1, 2},
    {2, 0, 0, 4}, {2, 1, 0, 3}, {2, 2, 0, 2},
    {2, 0, 1, 4}, {2, 1, 1, 3}, {2, 2, 1, 2},
    {3, 0, 0, 4}, {3, 1, 0, 3}, {3, 2, 0, 2},
    {3, 0, 1, 4}, {3, 1, 1, 3}, {3, 2, 1, 2}
};

// Variáveis de controle globais para o Challenge
unsigned char challenge_attempts = 2;
unsigned char current_min_goals = 0;



// Variável global para controlar a direção da curva da IA

signed char ai_curve_dir = 0;      // Controla a direção da curva (1, -1 ou 0)

// Suporte para até 4 jogadores (índices 1 a 4)
unsigned char has_boost[5] = {0, 0, 0, 0, 0};    // 1 = Pronto para usar
unsigned char boost_timer[5] = {0, 0, 0, 0, 0};  // Contador de frames do boost ativo
unsigned char p_idx_check = 0 ; // Declare aqui se ainda não foi declarada

 unsigned char is_pass = 0; // Flag para proteger o passe
 
 
// --- VARIÁVEIS GLOBAIS (DATA/BSS) ---
unsigned char home_team = 0;  // ID do Time A (0-9)
unsigned char away_team = 1;  // ID do Time B (0-9)
unsigned char p1_color, p2_color, p3_color, p4_color; // Cores finais dos sprites
 
/* --- Assets --- */

/* --- Mensagens do Menu (Screen Codes) --- */
/* --- Assets de Texto --- */
//const unsigned char msg_op1[]    = {49, 32, 45, 32, 49, 32, 16, 12, 1, 25, 5, 18, 0};
//const unsigned char msg_op2[]    = {50, 32, 45, 32, 50, 32, 16, 12, 1, 25, 5, 18, 19, 32, 24, 32, 3, 54, 52, 0};
//const unsigned char msg_op3[]    = {51, 32, 45, 32, 49, 32, 16, 12, 1, 25, 5, 18, 32, 24, 32, 50, 32, 16, 12, 1, 25, 5, 18, 0};
const unsigned char msg_prompt[] = {15, 16, 3, 1, 15, 58, 0};
//const unsigned char TXT_S[]      = {19, 3, 15, 18, 5, 58, 0 }; 
//const unsigned char TXT_T[]      = {20, 9, 13, 5, 58, 0 };      
//const unsigned char TXT_AGAIN[]  = {10, 15, 7, 1, 18, 32, 4, 5, 14, 15, 22, 15, 63, 0 }; 
//const unsigned char TXT_FINAL_SCORE[] = {6, 9, 14, 1, 12, 32, 19, 3, 15, 18, 5, 58, 0 };
/* --- Globais Adicionais (Certifique-se de que estão no topo) --- */


const unsigned char msg_titulo[] = "training & challenge v1.1"; // "2GOAL - v"
//const unsigned char msg_op4[]    = {52, 32, 45, 32, 53, 32, 7, 15, 1, 12, 19, 32, 23, 9, 14, 0}; // "4 - 5 GOALS WIN"
//const unsigned char msg_op5[] = "5 - 4 PLAYERS ALL VS ALL";
//const unsigned char TXT_P1_WIN[] = {16, 12, 1, 25, 5, 18, 32, 49, 32, 23, 9, 14, 19, 0};      // "PLAYER 1 WINS"
//const unsigned char TXT_P2_WIN[] = {16, 12, 1, 25, 5, 18, 32, 50, 32, 23, 9, 14, 19, 0};      // "PLAYER 2 WINS"
const unsigned char msg_field[] = "choose field: 1-gra 2-wet 3-fro 4-rand";
const unsigned char msg_line[]  = "--------------------------------------";
// No topo, com as outras strings:
//const unsigned char msg_def[] = "use defender: 1-no 2-yes";
const unsigned char msg_credits[] = "by eduardo maronas monks - 2026";
// Atualizado de "REJOGAR" para "PLAY AGAIN" (ajustado nos screen codes)
const unsigned char TXT_NAV[] = {1, 32, 45, 32, 16, 12, 1, 25, 32, 1, 7, 1, 9, 14, 32, 32, 13, 32, 45, 32, 13, 5, 14, 21, 0}; // "A - PLAY AGAIN  M - MENU"
// G=7, A=1, M=13, E=5, [espaço]=32, O=15, V=22, E=5, R=18, !=33 (ou 122 dependendo do seu charset)
//const unsigned char TXT_GAME_OVER[] = {7, 1, 13, 5, 32, 15, 22, 5, 18, 33, 0};
//const unsigned char msg_fire[] = "press fire to start";

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



// Goleiros

const unsigned char sprite_goleiroN[64] = {
0x02,0xAA,0x80,0x0A,0xAA,0xA0,0x2A,0x96,0xA8,0x2A,0x96,0xA8,0x20,0x55,0x08,0x10,0x55,0x04,0x10,0x55,0x04,0x10,0x3C,0x04,0x3C,0x00,0x3C,0x30,0x00,0x0C,0x30,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82


};

const unsigned char sprite_goleiroS[64] = {
0x30,0x00,0x0C,0x30,0x00,0x0C,0x3C,0x00,0x3C,0x10,0x3C,0x04,0x10,0x55,0x04,0x10,0x55,0x04,0x10,0x55,0x04,0x2A,0x96,0xA8,0x2A,0x96,0xA8,0x0A,0xAA,0xA0,0x02,0xAA,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82


};

//const unsigned char sprite_goleiroE[64] = {
//0x00,0x00,0x00,0x00,0xFA,0x80,0x00,0x3A,0x80,0x00,0x0A,0xA0,0x00,0x00,0xA0,0x00,0x01,0xA0,0x00,0x05,0x60,0x00,0x35,0x60,0x00,0x35,0x60,0x00,0x05,0x60,0x00,0x01,0xA0,0x00,0x00,0xA0,0x00,0x0A,0xA0,0x00,0x3A,0x80,0x00,0xFA,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82
//};


//const unsigned char sprite_goleiroW[64] = {
//0x00,0x00,0x00,0x2A,0xF0,0x00,0x2A,0xC0,0x00,0xAA,0x00,0x00,0xA0,0x00,0x00,0xA4,0x00,0x00,0x95,0x00,0x00,0x95,0xC0,0x00,0x95,0xC0,0x00,0x95,0x00,0x00,0xA4,0x00,0x00,0xA0,0x00,0x00,0xAA,0x00,0x00,0x2A,0xC0,0x00,0x2A,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82
//};


 
// Lembre-se: CH_BASE + 8 até + 11
const unsigned char net_deform_tiles[] = {
    // CCTL (Curva para fora/cima-esquerda)
    0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80,
    // CCTR (Curva para fora/cima-direita)
    0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01,
    // CCBL (Curva para fora/baixo-esquerda)
    0x80, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03,
    // CCBR (Curva para fora/baixo-direita)
    0x01, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0
};

// Referências externas para os dados no assets.s
//extern const unsigned char charset_data[];
//extern const unsigned char map_data[];
//extern const unsigned char map_colour_data[];

// Revisar
// Movimentacao da bola
//#define BALL_FRAMES 3
#define SPR_PTR ((unsigned char*)0xC3F8)
//#define SPR_BALL_INDEX 5     // sprite VIC usado pela bola
//#define SPR_BALL       250   // sprite base (frame 0)

unsigned char ball_anim = 0;
unsigned char ball_anim_delay = 0;



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

// Modo playoff
unsigned char v1_total, v2_total, v3_total, v4_total;
unsigned char win_p = 0;
unsigned char playoff_matches = 0;
unsigned char playoff_target_wins = 0;
unsigned char playoff_game = 0;

static unsigned char saved_sprites[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
static unsigned char active_sprite[4];

const unsigned char* const idle_gfx[6] = { j1, j2, j3, j4, j5, j6 };
const unsigned char* const run_gfx[6]  = { j1r, j2r, j3r, j4r, j5r, j6r };

static void clear_saved_sprites(void) {
    unsigned char i;
    for (i = 0; i < 4; ++i) {
        saved_sprites[i] = 0xFF;
    }
}



static void load_individual_sprites(void) {
    unsigned char i, j, r, valid;
    unsigned char* dest;

    // ==========================================
    // PASSO 1: RESOLVER OS HUMANOS PRIMEIRO
    // ==========================================
    for (i = 0; i < 4; ++i) {
        if (player_type[i + 1] == 1) { 
            // É Humano. Se ainda não tem sprite, sorteia um único.
            if (saved_sprites[i] > 5) {
                do {
                    r = (unsigned char)(rand() % 6);
                    valid = 1;
                    
                    // Verifica se algum outro humano já possui este sprite salvo
                    for (j = 0; j < 4; ++j) {
                        if (i != j && saved_sprites[j] == r) {
                            valid = 0;
                            break;
                        }
                    }
                } while (!valid);
                saved_sprites[i] = r;
            }
            // Transfere o sprite fixo do humano para o array ativo da partida
            active_sprite[i] = saved_sprites[i];
        } else {
            // É CPU. Marcamos com 0xFF para sinalizar que o slot está vago
            active_sprite[i] = 0xFF; 
        }
    }


    // ==========================================
    // PASSO 2: RESOLVER AS CPUS (Preencher os vagos)
    // ==========================================
    for (i = 0; i < 4; ++i) {
        if (player_type[i + 1] != 1) { 
            // É CPU. Precisa de um sprite que não esteja em uso por NINGUÉM.
            do {
                r = (unsigned char)(rand() % 6);
                valid = 1;
                
                // Valida contra todos os sprites já ativos na partida (Humanos e outras CPUs)
                for (j = 0; j < 4; ++j) {
                    if (i != j && active_sprite[j] == r) {
                        valid = 0;
                        break;
                    }
                }
            } while (!valid);
            
            // Atribui o sprite único à CPU
            active_sprite[i] = r;
        }
    }


    // ==========================================
    // PASSO 3: PROTEÇÃO DE PONTEIRO
    // ==========================================
    for (i = 0; i < 4; ++i) {
        if (active_sprite[i] > 5) active_sprite[i] = 0;
    }


    // ==========================================
    // PASSO 4: CÓPIA PARA A VRAM
    // ==========================================
    for (i = 0; i < 4; ++i) {
        dest = (unsigned char*)(0xF000 + (i << 7));
        
        memcpy(dest,      idle_gfx[active_sprite[i]], 64);
        memcpy(dest + 64, run_gfx[active_sprite[i]],  64);
    }
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


static void draw_champion_title(unsigned char color_offset) {
    // Array estático no RODATA
    static const unsigned char rainbow[] = {7, 10, 13, 1, 3, 14, 6}; 
    
    // 2GOAL corrigido (matriz perfeitamente alinhada para 28 colunas exatas)
    static const unsigned char art_4goal[20] = {
        0xF3, 0xCF, 0x3C, 0x80, // "XXXX  XXXX  XXXX  XXXX  X   "
        0x12, 0x09, 0x24, 0x80, // "   X  X     X  X  X  X  X   " (Espaçamento corrigido)
        0xF2, 0xC9, 0x3C, 0x80, // "XXXX  X XX  X  X  XXXX  X   "
        0x82, 0x49, 0x24, 0x80, // "X     X  X  X  X  X  X  X   "
        0xF3, 0xCF, 0x24, 0xF0  // "XXXX  XXXX  XXXX  X  X  XXXX"
    };

    // CHAMPION mantido intacto (já estava perfeito)
    static const unsigned char art_champion[25] = {
        0xF4, 0xBD, 0x17, 0xBB, 0xDA, 
        0x84, 0xA5, 0xB4, 0x92, 0x56, 
        0x87, 0xBD, 0x57, 0x92, 0x52, 
        0x84, 0xA5, 0x14, 0x12, 0x52, 
        0xF4, 0xA5, 0x14, 0x3B, 0xD2  
    };

    register unsigned char val;
    unsigned char row, col;
    unsigned char line_color;
    
    unsigned char *scr_ptr;
    unsigned char *col_ptr;
    const unsigned char *data_ptr;

    // --- 1. Desenha "2GOAL" ---
    scr_ptr = SCREEN + 46; // Volta a usar (1 * 40) + 6 para centralizar certinho
    col_ptr = ((unsigned char*)0xD800) + 46;
    data_ptr = art_4goal;

    for (row = 0; row < 5; ++row) {
        line_color = rainbow[(row + color_offset) % 7];
        
        for (col = 0; col < 28; ++col) {
            if ((col & 7) == 0) val = *data_ptr++;
            
            if (val & 0x80) {
                *scr_ptr = 160;
                *col_ptr = line_color;
            } else {
                *scr_ptr = 32; // Limpa o fundo onde não tem pixel para evitar lixo
            }
            
            val <<= 1;
            scr_ptr++; 
            col_ptr++;
        }
        // Pula os espaços restantes da tela (40 - 28 colunas = 12)
        scr_ptr += 12; 
        col_ptr += 12;
    }

    // --- 2. Desenha "CHAMPION" ---
    scr_ptr = SCREEN + 320; // (8 * 40) + 0
    col_ptr = ((unsigned char*)0xD800) + 320;
    data_ptr = art_champion;

    for (row = 0; row < 5; ++row) {
        line_color = rainbow[(row + color_offset) % 7];
        
        for (col = 0; col < 39; ++col) {
            if ((col & 7) == 0) val = *data_ptr++;
            
            if (val & 0x80) {
                *scr_ptr = 160;
                *col_ptr = line_color;
            } else {
                *scr_ptr = 32; 
            }
            
            val <<= 1;
            scr_ptr++; 
            col_ptr++;
        }
        // Pula o espaço restante da tela (40 - 39 colunas desenhadas = 1)
        scr_ptr += 1; 
        col_ptr += 1;
    }
}

static void show_tournament_champion(void) {
    unsigned char frame_count = 0;
    unsigned char color_shift = 0;
  //  const unsigned char* name_ptr;

    fast_clrscr(1); 
    VIC.bordercolor = 0;
    VIC.bgcolor0 = 0;     // Trava o fundo em PRETO

        music_init(); 
    // Reposicionados para baixo do título de duas linhas
 //   name_ptr = (const unsigned char*)team_names[home_team];
    
 //   put_text(13, 14, name_ptr);
     put_text(12, 14, (const unsigned char*)"P1 GOALS: ");
    put_num2(22, 14, s1_total); // gols do P1
  if ( player_type[2] == 1)
  {
    put_text(12, 16, (const unsigned char*)"P2 GOALS: ");
    put_num2(22, 16, s2_total); // gols do P2
   } 
    put_text(10, 18, (const unsigned char*)"CHALLENGE COMPLETED!");
    put_text(13, 20, (const unsigned char*)"CONGRATULATIONS");
    put_text(11, 24, (const unsigned char*)"PRESS FIRE TO EXIT");

    while (!(~CIA1.pra & 0x10)) { 
        wait_vblank();
        music_play();
       

        frame_count++;
        if (frame_count > 180) { // Um pouco mais rápido para dar energia
            color_shift++;
            frame_count = 0;
             draw_champion_title(color_shift);
     //       VIC.bordercolor++; 
        }
    }

   // Reset de segurança antes de sair
   music_stop();
    VIC.bordercolor = 0;
    VIC.bgcolor0 = 0;
    fast_clrscr(1);
    while ((~CIA1.pra & 0x10)); 
}

static void show_game_over_confirm(void) {
    unsigned char selection = 0; // 0 = Retry, 1 = Quit
    
    fast_clrscr(1);
	put_text(15, 4,  (const unsigned char*)"GAME OVER");
	put_text(1, 5, msg_line);
     put_text(12, 6, (const unsigned char*)"P1 GOALS: ");
    put_num2(22, 6, s1_total); // gols do P1
  if ( player_type[2] == 1)
  {
    put_text(12, 8, (const unsigned char*)"P2 GOALS: ");
    put_num2(22, 8, s2_total); // gols do P2
   } 
    put_text(1, 9, msg_line);
  //  put_text(15, 8,  (const unsigned char*)"GAME OVER");
  //  put_text(12, 12, (const unsigned char*)"RETRY CHALLENGE");
    put_text(12, 14, (const unsigned char*)"PRESS FIRE TO MENU");

   // Aguarda o jogador soltar e pressionar o botão de disparo (Fire)
    while (1) {
        while ((~CIA1.pra & 0x10) || (~CIA1.prb & 0x10)) {
            wait_vblank();
        }
        while (!((~CIA1.pra & 0x10) || (~CIA1.prb & 0x10))) {
            wait_vblank();
        }
        break;
    }
}

static void show_interstage_screen(void) {
    unsigned char name_len = 0;
    unsigned char x_pos = 0;
 //   const unsigned char* name_ptr;
    unsigned char i;
    
	VIC.bordercolor = 0; 
    VIC.bgcolor0 = 0;    
    fast_clrscr(1); // Limpa a tela
    


    // Título e Informações do Estágio
    put_text(13, 3, (const unsigned char*)"CHALLENGE MODE");
    put_text(12, 5, (const unsigned char*)"STAGE: ");
    put_num(19, 5, tournament_stage + 1); // Mostra de 1 a 18
    
    put_text(12, 7, (const unsigned char*)"TARGET: ");
    put_num(20, 7, current_min_goals);
  
    put_text(23, 7, (const unsigned char*)" GOALS");

    put_text(12, 9, (const unsigned char*)"ATTEMPTS: ");
    put_num(22, 9, challenge_attempts); // Mostra 2 ou 1 tentativa
put_text(1, 10, msg_line);
     put_text(12, 11, (const unsigned char*)"P1 GOALS: ");
    put_num2(22, 11, s1_total); // gols do P1
  if ( player_type[2] == 1)
  {
    put_text(12, 13, (const unsigned char*)"P2 GOALS: ");
    put_num2(22, 13, s2_total); // gols do P2
   } 
    put_text(1, 14, msg_line);
  
    // Loop de pisca ajustado para a nova linha Y=19
    for (i = 0; i < 60; i++) {
        paint_row_color(7, 12, 17, (i & 8) ? 1 : 7);
        wait_vblank(); 
    }

    paint_row_color(7, 12, 17, 7);


    put_text(10, 19, (const unsigned char*)"PRESS FIRE TO START");
    
    // Feedback visual da cor do time principal
 //   ((unsigned char*)0xD800)[5 * 40 + 19] = team_colors[home_team];
    
    // Aguarda o botão Fire ser solto e pressionado para começar
    while ((~CIA1.pra & 0x10)); 
    while (!(~CIA1.pra & 0x10)); 
    
    fast_clrscr(1);
}

static void play_sfx_hit_human(void)
{
    SID.v1.ctrl = 0x80;      // Gate OFF

    SID.v1.freq = 0x0A00;    // Frequência um pouco maior

    SID.v1.ad = 0x04;        // Ataque instantâneo
    SID.v1.sr = 0x04;        // Release curtíssimo

    SID.v1.ctrl = 0x81;      // Gate ON
}


// ==========================
// MÚSICA 4GOAL - TAMBORES DE MARCAÇÃO (CC65)
// ==========================

// Variáveis compactadas para economizar Zero Page/RAM
static unsigned int play_counter = 0;
static unsigned int pause_counter = 0;
static unsigned char is_paused = 0;

static unsigned char tick = 0;
static unsigned char step = 0;
static unsigned char passage = 0;
static unsigned char loop_count = 0;

// Tabela de frequências da melodia compactada (substitui o switch/case)
// 0xFFFF indica que não há nota a tocar neste step
static const unsigned int intro_notes[32] = {
    0x095A, 0, 0, 0, 0, 0, 0x095A, 0,     // 0 a 7
    0x0B1F, 0, 0x095A, 0, 0x0851, 0, 0x0767, 0, // 8 a 15
    0, 0, 0x06FD, 0, 0, 0, 0, 0,          // 16 a 23
    0, 0, 0, 0, 0, 0, 0, 0                // 24 a 31 (Hi-Hats tratados à parte)
};

static void music_init(void) {
    SID.amp = 0x1F; 
    
    SID.v1.ad = 0x00; SID.v1.sr = 0xD8; // Baixo
    SID.v2.ad = 0x00; SID.v2.sr = 0x16; // Hi-Hat
    SID.v2.pw = 0x0300; 

    SID.v3.ad = 0x09; // Ataque rápido para o tambor
    SID.v3.sr = 0x40; 

    play_counter = 0;
    pause_counter = 0;
    is_paused = 0;
    tick = 0;
    step = 0;
    passage = 0;
    loop_count = 0;
    whistle_state = 0;
    whistle_timer = 0;
}

static void music_play(void) {
    // --- 1. GERENCIA OS APITOS A CADA FRAME ---
    if (whistle_state > 0) {
        if (--whistle_timer == 0) {
            SID.v2.ctrl = 0x00; 
            
            if (whistle_state == 2) { 
                is_paused = 1;
                pause_counter = 0;
                SID.amp = 0x00;
            }
            whistle_state = 0;
        }
        return; 
    }

    // --- 2. CONTROLE DE PAUSA (8 SEGUNDOS) ---
    if (is_paused) {
        if (++pause_counter >= 960) { 
            is_paused = 0;          
            pause_counter = 0;
            play_counter = 0;       
            SID.amp = 0x1F;         
            
            play_sfx_whistle();     
            return;                 
        }
        return;
    }

    // --- 3. CONTROLE DE EXECUÇÃO (8 SEGUNDOS) ---
    if (++play_counter >= 1060) { 
        is_paused = 1;          
        pause_counter = 0;
        SID.amp = 0x00;         
        
        SID.v1.ctrl = 0x40; 
        SID.v2.ctrl = 0x00;
        SID.v3.ctrl = 0x80; 
        VIC.bordercolor = 0;
        
        play_sfx_game_over(); 
        return;
    }

    // --- 4. LÓGICA DA MÚSICA INTRO ---
    tick++;
    if (tick < (passage == 0 ? 7 : 5)) return; 
    tick = 0;

    SID.v1.ctrl &= 0xFE; 
    SID.v2.ctrl &= 0xFE;
    VIC.bordercolor = 0;

    // --- LÓGICA DO DRONE (VOZ 3) ---
    if (step < 20) {
        if (step == 0) {
            SID.v3.freq = 0x04AB; 
            SID.v3.ctrl = 0x11; 
        }
    } else {
        SID.v3.ctrl &= 0xFE; 
    }

    // --- MELODIA VIA TABELA (Economiza muito espaço de código) ---
    if (intro_notes[step] != 0) {
        SID.v1.freq = intro_notes[step];
        SID.v1.ctrl = 0x21;
    }

    // --- VIRADAS DE TAMBOR E HI-HAT ---
    if (passage == 0 && loop_count == 1) {
        if (step == 20) { 
            SID.v3.freq = 0x0500; 
            SID.v3.ctrl = 0x10; 
            SID.v3.ctrl = 0x11; 
            VIC.bordercolor = 15; 
        } else if (step == 22) { 
            SID.v3.freq = 0x0400; 
            SID.v3.ctrl = 0x10; 
            SID.v3.ctrl = 0x11; 
            VIC.bordercolor = 1; 
        } else if ((step >= 24) && (step <= 30) && ((step & 1) == 0)) {
            SID.v2.freq = 0x6000; 
            SID.v2.ctrl = 0x41; 
        }
    }

    step++;
    if (step > 31) { 
        step = 0; 
        loop_count++;
        if (loop_count > 3) { 
            loop_count = 0; 
            passage = !passage; 
        }
    }
}

static void music_stop(void) {
    SID.v1.ctrl = 0x40;
    SID.v3.ctrl = 0x80;
    SID.amp = 0x00;
    music_init();
}

// Funcionando bem
#ifndef ABS_DIFF
#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))
#endif


static void move_defender_ia(unsigned char is_south) {
    // 1. Eixo Y para 8-bits: Y na tela nunca passa de 255. 
    // Usar unsigned char salva ciclos vitais de CPU no 6502.
    static unsigned char defends_north, moving, wait_val, dist_y;
    static unsigned char curr_y, target_y; 
    static unsigned int curr_x; // Apenas o X precisa de 16-bits (pode passar de 255)
    
    unsigned char r,forca; // Para armazenar o rand() uma única vez
    unsigned char voltar;

    // CACHE LOCAL
    if (is_south) {
        curr_x = dx_b; 
        curr_y = (unsigned char)dy_b; 
        wait_val = defender_wait_b;
        defends_north = (current_half == 2);
    } else {
        curr_x = dx; 
        curr_y = (unsigned char)dy; 
        wait_val = defender_wait;
        defends_north = (current_half == 1);
    }

    if (goal_delay > 0 || wait_val > 0) {
        if (wait_val > 0) wait_val--;
        moving = 0;
    } 
    else {
        // 2. MEGA OTIMIZAÇÃO LÓGICA:
        // Se a bola passou do meio de campo (146), não importa se ela está fora (>200 ou <90).
        // Podemos resumir 'bola_fora || !bola_no_meu_campo' a uma única checagem simples!
        voltar = defends_north ? (by > 146) : (by <= 146);

        if (voltar) {
            target_y = defends_north ? 80 : 192; // Reutilizamos target_y (removemos return_y)
            
            if (curr_x < 160) curr_x++; else if (curr_x > 160) curr_x--;
            if (curr_y < target_y) curr_y++; else if (curr_y > target_y) curr_y--;
            
            moving = (curr_x != 160 || curr_y != target_y); 
        } 
        else {
            // 3. ABS INLINE: Evita que macros genéricas de ABS façam cast indesejado para 16-bits
            dist_y = (curr_y > by) ? (curr_y - by) : (by - curr_y);
            moving = 0;

            if (dist_y < IA_DIST_Y[difficulty_level] || (timer & IA_TIMER_MASK[difficulty_level]) == 0) {
                target_y = by;
                
                if (defends_north) {
                    if (target_y > 146) target_y = 146; 
                } else {
                    if (target_y < 146) target_y = 146; 
                }

                if (curr_x < bx) curr_x++; else if (curr_x > bx) curr_x--;
                if (curr_y < target_y) curr_y++; else if (curr_y > target_y) curr_y--;
                
                moving = (curr_x != bx || curr_y != target_y);
            }

            // Checagem de distância X com ABS inline para o momento do chute
            if (kick_cooldown == 0 && dist_y < 12 && ((curr_x > bx) ? (curr_x - bx) : (bx - curr_x)) < 12) {
                ball_possession = 0;
                last_touch = is_south ? 6 : 5; 
                
                // 4. OTIMIZAÇÃO DO RAND(): Chamamos apenas UMA VEZ e fatiamos os bits
                r = rand(); 
                
                // Bits 0 e 1 (0 a 3)
                bdx = (curr_x < 160) ? (signed char)(1 + (r & 3)) : -(signed char)(1 + (r & 3));

                // Bits 2 e 3 (deslocamos para a direita para pegar novos valores aleatórios)
                forca = IA_CHUTE_BASE[difficulty_level] + ((r >> 2) & IA_CHUTE_RAND[difficulty_level]);

                if (defends_north) {
                    bdy = (forca < 3) ? 3 : forca; 
                    if (by <= curr_y + 14) by = curr_y + 16; 
                } else {
                    bdy = -(signed char)forca; 
                    if (bdy > -3 || bdy == 0) bdy = -3; 
                    if (by >= curr_y - 14) by = curr_y - 16; 
                }
                
                wait_val = IA_WAIT_TIME[difficulty_level]; 
                
                // Bits 4 a 7 (gera valor de 0 a 15)
                kick_cooldown = 30 + ((r >> 4) & 15); 
                
                play_sfx_kick();
            }
        }
    }

   // 4. LIMITES INLINE (Meio Campo e Bordas)
    // Substitui completamente a lenta keep_defender_in_field
    
    // Trava do meio-campo
    if (defends_north) {
        if (curr_y > 146) curr_y = 146; 
    } else {
        if (curr_y < 146) curr_y = 146; 
    }

    // Trava das bordas (Clamping) direto nas variáveis locais (super rápido)
    if (curr_x < FIELD_LEFT) curr_x = FIELD_LEFT;
    else if (curr_x > FIELD_RIGHT) curr_x = FIELD_RIGHT; // else evita teste duplo

    if (curr_y < FIELD_TOP) curr_y = FIELD_TOP;
    else if (curr_y > FIELD_BOTTOM) curr_y = FIELD_BOTTOM;
    
    // 5. GRAVAÇÃO DO CACHE (Agora direta, sem ponteiros temporários)
    if (is_south) {
        dx_b = curr_x;
        dy_b = curr_y;
        defender_wait_b = wait_val;
        p6_is_moving = moving;
    } else {
        dx = curr_x;
        dy = curr_y;
        defender_wait = wait_val;
        p5_is_moving = moving;
    }
}


static void sorteia_personalidades(void) {
    unsigned char i;
    init_random();
    for (i = 1; i < 5; ++i) {
        // No modo 5 (jogo individual), a personalidade é sempre 1 (Ofensiva)
     //   if (game_mode == 5) {
            player_pers[i] = 3; 
      //  } else {
            player_pers[i] = rand() & 0x03; 
       // }
    }
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

// Modo 5


static void interrupt_raster_handler(void) {
    // O raster do goleiro agora usa EXCLUSIVAMENTE o Slot 5,
    // liberando o Slot 1 e 4 para os zagueiros sem conflito visual.
    
    if (VIC.rasterline < 100) {
        // Goleiro Norte
        VIC.spr_pos[5].x = gx[0] & 0xFF;
        VIC.spr_pos[5].y = gy[0];
        SPR_ZAG[5] = 16;
        VIC.rasterline = 150; 
        
        // Troca a cor baseada no tempo
        if (current_half == 2) {
            VIC.spr_color[5] = 4; // 2º Tempo: Time B defende o Norte
        } else {
            VIC.spr_color[5] = 0; // 1º Tempo: Time A defende o Norte
        }
    } else {
        // Goleiro Sul
        VIC.spr_pos[5].x = gx[1] & 0xFF;
        VIC.spr_pos[5].y = gy[1];
        SPR_ZAG[5] = 17;
        VIC.rasterline = 30; 
        
        // Troca a cor baseada no tempo
        if (current_half == 2) {
            VIC.spr_color[5] = 0; // 2º Tempo: Time A defende o Sul
        } else {
            VIC.spr_color[5] = 4; // 1º Tempo: Time B defende o Sul
        }
    }
    
    VIC.irr = 0x01; 
    asm("jmp $ea31"); 
}


static void read_joysticks(void) {
    // Declarações obrigatórias no início do bloco
    unsigned char row, joy_mask;
    unsigned char raw3, raw4; // Mantidas para o modo 5

// 1. DESLIGA O KERNAL (Impede que o SO suje a leitura)
    asm("sei"); 

    // 2. Força o teclado a não interferir no Joystick 1
    CIA1.pra = 0xFF; 
    
    // Pequeno delay para o hardware do C64 respirar
    asm("nop"); 
    asm("nop");
	
    joy2 = (~CIA1.pra) & 0x1F;
    joy1 = (~CIA1.prb) & 0x1F;
	// 4. RELIGA AS INTERRUPÇÕES (Para não travar o C64)
    asm("cli");
	
    joy3 = 0;
    joy4 = 0;
    joy_mask = ~joy1; 


		
		// Substitua o bloco do Adaptador por:
CIA2.ddrb = 0x80; 
CIA2.prb |= 0x80;
raw3 = CIA2.prb;
CIA2.prb &= 0x7F;
raw4 = CIA2.prb;

joy3 = (raw3 ^ 0x0F) & 0x0F;
if (!(raw4 & 0x20)) joy3 |= 0x10;
joy4 = (raw4 ^ 0x0F) & 0x0F;
if (!(raw3 & 0x10)) joy4 |= 0x10;

        // --- LEITURA DE TECLADO OTIMIZADA ---
		
	if ((player_type[3] == 1 || player_type[4] == 1) && (joy3 == 0 && joy4 == 0)) {
        asm("sei");
        
        CIA1.pra = 0xFD; row = (~CIA1.prb) & joy_mask;
        if (row & 0x02) joy3 |= 0x01; 
        if (row & 0x04) joy3 |= 0x04; 
        if (row & 0x20) joy3 |= 0x02;
        
        CIA1.pra = 0xFB; row = (~CIA1.prb) & joy_mask;
        if (row & 0x04) joy3 |= 0x08;
        
        CIA1.pra = 0x7F; row = (~CIA1.prb) & joy_mask;
        if (row & 0x40) joy3 |= 0x10;

        CIA1.pra = 0xEF; row = (~CIA1.prb) & joy_mask;
        if (row & 0x02) joy4 |= 0x01; 
        if (row & 0x04) joy4 |= 0x04; 
        if (row & 0x20) joy4 |= 0x02;
        
        CIA1.pra = 0xDF; row = (~CIA1.prb) & joy_mask;
        if (row & 0x04) joy4 |= 0x08; 
        if (row & 0x02) joy4 |= 0x10;

        CIA1.pra = 0xFF; 
        asm("cli");
    }

    joy_bits[0] = joy2; joy_bits[1] = joy1; joy_bits[2] = joy3; joy_bits[3] = joy4;


	// --- BOOST OTIMIZADO (SEM LOOP) ---


// Jogador 1 
if (player_type[1] == 1 && ball_possession != 1 && (joy2 & 0x10) && has_boost[1]) {
    has_boost[1] = 0; 
    boost_timer[1] = 10;
}

// Jogador 2 (Verifica se é humano no Joy 1)
if (player_type[2] == 1 && ball_possession != 2 && (joy1 & 0x10) && has_boost[2]) {
    has_boost[2] = 0; 
    boost_timer[2] = 10;
}

// Jogador 3 (Só processa se houver 3+ jogadores e for humano)
if (num_players >= 3 && player_type[3] == 1) {
    if (ball_possession != 3 && (joy3 & 0x10) && has_boost[3]) {
        has_boost[3] = 0; 
        boost_timer[3] = 10;
    }
}

// Jogador 4 (Só processa se houver 4 jogadores e for humano)
if (num_players == 4 && player_type[4] == 1) {
    if (ball_possession != 4 && (joy4 & 0x10) && has_boost[4]) {
        has_boost[4] = 0; 
        boost_timer[4] = 10;
    }
}

    p1_is_moving = (joy2 & 0x0F); 
    p2_is_moving = (joy1 & 0x0F);
    p3_is_moving = (joy3 & 0x0F); 
    p4_is_moving = (joy4 & 0x0F);
}



static void setup_hardware(void) {
    asm("sei");
    *((unsigned char*)0x01) &= 0xFB; 
    memcpy((void*)CHARSET_ADDR, (void*)0xD000, 2048); 
    *((unsigned char*)0x01) |= 0x04; 
    asm("cli");

    memcpy((void*)(CHARSET_ADDR + 1024), field_charset, sizeof(field_charset));
    
    // --- 1. CARGA DOS DADOS EM ÁREA SEGURA ($C400 em diante) ---
    // No Banco 3 ($C000), o endereço $C400 dividido por 64 resulta no Slot 16.
    memcpy((void*)0xC400, sprite_goleiroN, 64); // Slot 16
    memcpy((void*)0xC440, sprite_goleiroS, 64); // Slot 17
  //  memcpy((void*)0xC480, sprite_goleiroW, 64); // Slot 18
  //  memcpy((void*)0xC4C0, sprite_goleiroE, 64); // Slot 19
  /*  
    // Jogador 1 e 2 em $F000 (Slot 192)
    memcpy((void*)SPRITE_ADDR, SPRITE_DATA, 64);       
	
	// NOVO: Jogador - Frame Corrida em $F040 (Slot 193)
	memcpy((void*)(SPRITE_ADDR + 64), jogador_run, 64);
	
	// --- Jogador 2 e Animação ---
	memcpy((void*)0xF080, jogador2, 64);      // Slot 194 (Jogador 2)
	memcpy((void*)0xF0C0, jogador2_run, 64);  // Slot 195 (Jogador 2 Run)
*/
    // --- BOLA NOS SLOTS 246 até 253 (8 Frames) ---
    memcpy((void*)0xFD80, ball_frame_0, 64); // Slot 246
    memcpy((void*)0xFDC0, ball_frame_1, 64); // Slot 247
    memcpy((void*)0xFE00, ball_frame_2, 64); // Slot 248
    memcpy((void*)0xFE40, ball_frame_3, 64); // Slot 249
    memcpy((void*)0xFE80, ball_frame_4, 64); // Slot 250
    memcpy((void*)0xFEC0, ball_frame_5, 64); // Slot 251
    memcpy((void*)0xFF00, ball_frame_6, 64); // Slot 252
    memcpy((void*)0xFF40, ball_frame_7, 64); // Slot 253
    
    // Zagueiro em $FFC0 (Slot 255)
//    memcpy((void*)0xFFC0, sprite_defender, 64); 
	
	// NOVO: Zagueiro correndo em $FF80 (Slot 254)
//	memcpy((void*)0xFF80, sprite_defender_run, 64);
	
	memcpy((void*)0xC480, sprite_defender, 64);     // Slot 18 (Parado) - ANTES $FFC0
    memcpy((void*)0xC4C0, sprite_defender_run, 64); // Slot 19 (Correndo) - ANTES $FF80
   
    // --- 2. CONFIGURAÇÃO DE MULTICOLOR ---
    // Sprites 1, 2, 3, 4 (Goleiros) e 7 (Zagueiro) ligados.
    // Sprites 0 (P1), 5 (Bola) e 6 (P2) desligados (Hi-Res).
    
// --- CONFIGURAÇÃO DE MULTICOLOR ---
    VIC.spr_mcolor0 = 10;  
    VIC.spr_mcolor1 = 0; 
    VIC.spr_ena = 0xFF;
	
	if (game_mode == 5 || game_mode == 7 || game_mode == 6) {
        // MODO 5: Todos multicolor, exceto Sprite 3 (Bola)
        // Bits: 76543210 -> 11110111 = 0xF7
        *((unsigned char*)0xd01c) = 0xF7; 

	
       
	
        // Ponteiros Iniciais Modo 5
        SPR_ZAG[0] = 192; // P1
        SPR_ZAG[6] = 194; // P2
        SPR_ZAG[2] = 196; // P3
        SPR_ZAG[7] = 198; // P4
        SPR_ZAG[3] = 246; // Bola (Mudou do 5 para o 3!)
  //      SPR_ZAG[4] = 18;  // Goleiro Inicial (Oeste)
        SPR_ZAG[5] = 16;  // Goleiro Inicial (Norte)
		// Zagueiros (Sprites 1 e 4) apontando para o NOVO Slot 18 (Parado)
    SPR_ZAG[1] = 18;  // Antes provavelmente 255
    SPR_ZAG[4] = 18;  // Antes provavelmente 255
		
		asm("sei"); // Desliga interrupções para configurar

		// Aponta o vetor de interrupção ($0314) para sua função
		// Se o compilador for cc65, use:
		*((unsigned int*)0x0314) = (unsigned int)interrupt_raster_handler;

		VIC.imr = 0x01;      // Habilita interrupção de raster
		VIC.ctrl1 &= 0x7F;   // Limpa o 8º bit da linha de raster (total < 256)
		VIC.rasterline = 30; // Primeiro disparo na linha 30 (Goleiro Norte)

		asm("cli"); // Religa interrupções
	
} 
    setup_sid();
}





static void update_ball_animation(void) {
    signed char offset = 0;
    unsigned char speed_comp;

    // 1. Bola Parada (Sem posse E sem velocidade)
    if (ball_possession == 0 && bdx == 0 && bdy == 0) {
        SPR_PTR[3] = 246; // Frame 0
        ball_anim = 0;
        ball_anim_delay = 0;
    } else {
        // Se estiver com posse, força um valor fixo de velocidade para animar a bola girando
        speed_comp = (ball_possession != 0) ? 6 : ((bdx < 0) ? -bdx : bdx) + ((bdy < 0) ? -bdy : bdy);
        if (speed_comp > 24) speed_comp = 24; // CLAMP
        
        // 3. Controle de frames baseado no acúmulo
        ball_anim_delay += speed_comp;

        if (ball_anim_delay >= 8) {  
            ball_anim_delay -= 8;   
            ball_anim++;
            ball_anim &= 0x07;   
            SPR_PTR[3] = 246 + ball_anim;
        }

        // 4. Efeito de Oscilação (Quique na grama)
        if (speed_comp > 4) {
            offset = (*(unsigned char*)162) & 0x01;  
            if (ball_anim & 0x01) offset = -offset;  
        }
    }

    // 5. Atualização Direta de Posição (Sprite 3)
    VIC.spr_pos[3].x = bx & 0xFF;
    VIC.spr_pos[3].y = by - offset;  

    // 6. Bit de Alta (MSB) Otimizado via Bitwise (Testa direto o byte alto)
    if (bx & 0x100) {
        VIC.spr_hi_x |= 0x08; 
    } else {
        VIC.spr_hi_x &= ~0x08;
    }
}


static void update_players_anim(void) {
    unsigned char anim = (sys_tick & 8) ? 193 : 192;
    unsigned char anim2 = (sys_tick & 8) ? 195 : 194;
    unsigned char anim_zag = (sys_tick & 8) ? 18 : 19;  // Zagueiros (NOVO)
    // NOVO: Controle de animação para P3 e P4
    unsigned char anim3 = (sys_tick & 8) ? 197 : 196; // P3
    unsigned char anim4 = (sys_tick & 8) ? 199 : 198; // P4

    // P1 - Humano
    SPR_ZAG[0] = (p1_is_moving) ? anim : 192;

    // P2 - (Seja Humano ou IA)
    // Se P2 for IA, p2_is_moving deve ser setado pela lógica da IA no move_ia()
    SPR_ZAG[6] = (p2_is_moving) ? anim2 : 194;

//    if ((game_mode == 5 || game_mode == 7 || game_mode == 8)) {
        // P3
        SPR_ZAG[2] = (p3_is_moving) ? anim3 : 196;
        // P4
        SPR_ZAG[7] = (p4_is_moving) ? anim4 : 198;
//    }
        // --- ANIMAÇÃO DOS ZAGUEIROS ---
    // Zagueiros só estão presentes nos modos 6 e 7
if (use_defender==1) {
        // Zagueiro Norte (P5) - usa o ponteiro de sprite 4
        SPR_ZAG[4] = (p5_is_moving) ? anim_zag : 18;
        
        // Zagueiro Sul (P6) - usa o ponteiro de sprite 1
        SPR_ZAG[1] = (p6_is_moving) ? anim_zag : 18;
    }
}



static void update_sprites(void) {
    // --- MODO 5 DEFINITIVO ---
    // Fixos sempre ativos: 
    // P1(S0), Zagueiro Sul(S1), Bola(S3), Zagueiro Norte(S4), Goleiro Raster(S5)
    // Binário: 00111011 -> 0x3B
    unsigned char mask = 0x3B; 

    // Adiciona jogadores extras conforme o número de participantes
    if (num_players >= 3) mask |= 0x04; // + P3 (S2)
    if (num_players >= 2) mask |= 0x40; // + P2 (S6)
    if (num_players == 4) mask |= 0x80; // + P4 (S7)

    VIC.spr_ena = mask;
}


static void enter_game_mode(void) {
    unsigned char i;

    // 1. Seleciona Bank 3 ($C000 - $FFFF)
    *((unsigned char*)0xDD00) &= 0xFC;
    
    // 2. VIC aponta para Screen $C000 e Charset $D000
    VIC.addr = 0x0E;

   // --- PONTEIROS DE SPRITES ---
    SPR_ZAG[0] = 192; // P1
    SPR_ZAG[1] = 18;  // Zagueiro Sul
    SPR_ZAG[2] = (player_type[3] != 255) ? 196 : 0; // P3 / Oponente
    SPR_ZAG[3] = 246; // BOLA
    SPR_ZAG[4] = 18;  // Zagueiro Norte
    SPR_ZAG[5] = 16;  // Goleiro
    
    // P2 (Slot 6) e P4 (Slot 7)
    SPR_ZAG[6] = (player_type[2] != 255 && player_type[2] != 2) ? 194 : 0;
    SPR_ZAG[7] = (player_type[4] != 255) ? 198 : 0;

    // Ativa Multicolor para todos, exceto Bola (S3) -> 0xF7
    *((unsigned char*)0xd01c) = 0xF7;

    // Controle de ativação lógica do P2
    if (player_type[2] == 255 || player_type[2] == 2) {
        spr_active_p2 = 0;
        VIC.spr_pos[6].x = 0;
        VIC.spr_pos[6].y = 0;
    } else {
        spr_active_p2 = 1;
    }

    // Zeramento de posições iniciais
    for (i = 0; i < 8; i++) {
        if (i == 1 || i == 4 || i == 5) continue;
        VIC.spr_pos[i].x = 0;
        VIC.spr_pos[i].y = 0;
    }

    VIC.spr_mcolor0 = 10;
    VIC.spr_mcolor1 = 0;
    VIC.spr_hi_x = 0;

    load_individual_sprites(); 
    update_sprites();
}

//#ifndef ABS_DIFF
//#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))
//#endif

static void move_goleiros_ia(void) {
    // CC65 Otimização: Variáveis locais estáticas (evitam o stack)
    static unsigned char final_level, dist_bola, forca_ia;
    static unsigned char block_count = 0;
    static unsigned int dist_y;
    static int target_8; 
    
    if (goal_delay) return;
    
    // Otimização: Validação global de X feita uma única vez antes do processamento
    if (bx < 26 || bx > 325) return; 

    // =========================================================
    // GOLEIRO 0 (i = 0) - Loop Desenrolado para eliminar indexação de array
    // =========================================================
    final_level = g_level[0] + difficulty_boost;
    if (final_level > 3) final_level = 3; 

    dist_bola = (unsigned char)ABS_DIFF(gx[0], bx); 
    dist_y = ABS_DIFF(gy[0], by);
    
    if (dist_y > 40) block_count = 0; 
    if (dist_y > 60) dist_bola += 20; 

    if (((timer & (3 - final_level)) == 0) || dist_bola < 70) {
        forca_ia = (dist_bola < 50 || final_level > 0) ? 3 : 2;
        target_8 = (int)bx + (bdx << 2);
        
        if (bx < 106) {
            target_8 += 10;
        } else if (bx >= 212) {
            target_8 -= 10;
        }

        if (target_8 < 126) target_8 = 126;
        else if (target_8 > 218) target_8 = 218;

        if (gx[0] < target_8) {
            if ((target_8 - gx[0]) <= forca_ia) gx[0] = target_8;
            else gx[0] += forca_ia;
        } else if (gx[0] > target_8) {
            if ((gx[0] - target_8) <= forca_ia) gx[0] = target_8;
            else gx[0] -= forca_ia;
        }
    }

    // Colisão Goleiro 0 (Ifs combinados para reduzir saltos condicionais)
    if ((unsigned char)ABS_DIFF(gx[0], bx) < 14 && (unsigned char)ABS_DIFF(gy[0], by) < 14) {
        ball_possession = 0; 
        after_touch_timer = 0; 
        block_count++; 

        if (block_count > 1) {
            bdx = (gx[0] > 170) ? 3 : -3;
        } else {
            bdx = (timer & 3) - 1; 
        }

        by = gy[0] + 16; 
        bdy = 4;  
        kick_cooldown = 14; 
        play_sfx_defend();
    }

    // =========================================================
    // GOLEIRO 1 (i = 1) - Loop Desenrolado
    // =========================================================
    final_level = g_level[1] + difficulty_boost;
    if (final_level > 3) final_level = 3; 

    dist_bola = (unsigned char)ABS_DIFF(gx[1], bx); 
    dist_y = ABS_DIFF(gy[1], by);
    
    if (dist_y > 40) block_count = 0; 
    if (dist_y > 60) dist_bola += 20; 

    if (((timer & (3 - final_level)) == 0) || dist_bola < 70) {
        forca_ia = (dist_bola < 50 || final_level > 0) ? 3 : 2;
        target_8 = (int)bx + (bdx << 2);
        
        if (bx < 106) {
            target_8 += 10;
        } else if (bx >= 212) {
            target_8 -= 10;
        }

        if (target_8 < 126) target_8 = 126;
        else if (target_8 > 218) target_8 = 218;

        if (gx[1] < target_8) {
            if ((target_8 - gx[1]) <= forca_ia) gx[1] = target_8;
            else gx[1] += forca_ia;
        } else if (gx[1] > target_8) {
            if ((gx[1] - target_8) <= forca_ia) gx[1] = target_8;
            else gx[1] -= forca_ia;
        }
    }

    // Colisão Goleiro 1
    if ((unsigned char)ABS_DIFF(gx[1], bx) < 14 && (unsigned char)ABS_DIFF(gy[1], by) < 14) {
        ball_possession = 0; 
        after_touch_timer = 0; 
        block_count++; 

        if (block_count > 1) {
            bdx = (gx[1] > 170) ? 3 : -3;
        } else {
            bdx = (timer & 3) - 1; 
        }

        by = gy[1] - 16; 
        bdy = -4; 
        kick_cooldown = 14; 
        play_sfx_defend();
    }
}



// Função auxiliar para evitar repetição de código de cor
static void __fastcall__  paint_row_color(unsigned char y, unsigned char x, unsigned char len, unsigned char color) {
    unsigned int color_addr = 0xD800 + (y * 40) + x;
    for(i = 0; i < len; i++) {
        ((unsigned char*)color_addr)[i] = color;
    }
}


static void display_final_score_individual_v5(void) {
    unsigned char max_gols = 0;
    unsigned char row, i, p_color;
    unsigned char scores[4];
    unsigned char wins[4];
    
    // OTIMIZAÇÃO DE STRINGS
    static const unsigned char str_spaces[] = "                              ";
    static const unsigned char str_dashes[] = "------------------------------";
    static const unsigned char str_draw[]   = "DRAW GAME!";
    static unsigned char player_str[] = "PLAYER 0  :"; 

    scores[0] = s1_final; scores[1] = s2_final;
    scores[2] = s3_final; scores[3] = s4_final;

    if (game_mode == 6) {
        s1_total = s1_total + s1_final;
        s2_total = s2_total + s2_final;
    }

    // Gols que valem para decidir a partida
    eff[0] = scores[0]; eff[1] = scores[1];
    eff[2] = scores[2]; eff[3] = scores[3];

    if (is_overtime) {
        // Golden Score: só contam os gols feitos depois do empate
        eff[0] = scores[0] - ot_base1;
        eff[1] = scores[1] - ot_base2;
        eff[2] = scores[2] - ot_base3;
        eff[3] = scores[3] - ot_base4;
    }

    // PRÉ-CÁLCULO: Descobre o vencedor da partida antes de desenhar
    winner = 0;
    is_tie = 0;
    max_gols = 0;
    for (i = 0; i < num_players; i++) {
        if (eff[i] > max_gols) {
            max_gols = eff[i];
            winner = i + 1;
            is_tie = 0;
        } else if (eff[i] == max_gols && max_gols > 0) {
            is_tie = 1;
        }
    }


    // ATUALIZA VITÓRIAS MODO 7: Se houve um vencedor (sem empate), soma a vitória
    if (game_mode == 7 && max_gols > 0 && is_tie == 0) {
		playoff_game++;
        if (winner == 1) v1_total++;
        else if (winner == 2) v2_total++;
        else if (winner == 3) v3_total++;
        else if (winner == 4) v4_total++;
    }
    
    // Coloca num array para facilitar o loop de desenho
    wins[0] = v1_total; wins[1] = v2_total; 
    wins[2] = v3_total; wins[3] = v4_total;

    // 1. LIMPEZA TOTAL DA ÁREA
    for (row = 3; row < 22; row++) {
        put_text(5, row, str_spaces);
    }
    
    if (game_mode == 7) 
	{		
	put_text(11, 3, (const unsigned char*)"MATCH & PLAYOFF");
	put_text(14, 4, (const unsigned char*)"GAME: ");
	 put_num(20, 4, playoff_game);
	}
    else put_text(13, 3, (const unsigned char*)"MATCH STATS");

    // --- LISTA DE ARTILHEIROS (Modos 1 a 4 jogadores) ---
    for (i = 1; i <= num_players; i++) {
        if (i == 2 && player_type[2] == 2) continue; // Pula o P2 se for o Goleiro IA
        
        p_color = get_fixed_player_color(i);
        if (i == 4 && field_type == 1) p_color = 1; // Campo molhado para P4

        player_str[7] = '0' + i; 
        row = 5 + i; // Calcula a linha: P1=6, P2=7, etc.
        
        put_text(5, row, player_str);
        
        // NO MODO 7, mostra os gols e também as vitórias ao lado
        if (game_mode == 7) {
            put_num(16, row, scores[i-1]);
            put_text(20, row, (const unsigned char*)"WINS:");
            put_num(26, row, wins[i-1]);
        } else {
            put_num(18, row, scores[i-1]);
        }
        paint_row_color(row, 5, 23, p_color);
    }

    // --- SEÇÃO DO GOLEIRO (P2) --- (Mantida exatamente como você fez)
    if (player_type[2] == 2) {
        calculate_keeper_rating(scores[0] + scores[1] + scores[2] + scores[3]);
        p_color = (field_type == 1) ? 4 : 4; 

        put_text(5, 11, str_dashes);
        put_text(11, 12, (const unsigned char*)"P2 KEEPER SKILLS");
        paint_row_color(12, 11, 16, p_color);
        
        put_text(7, 14, (const unsigned char*)"SAVES........."); put_num2(22, 14, saves_p2);
        put_text(7, 15, (const unsigned char*)"RATING........"); put_num2(22, 15, keeper_score);

        if (keeper_score >= 150)      put_text(13, 17, (const unsigned char*)"* LEGENDARY *");
        else if (keeper_score >= 80)  put_text(13, 17, (const unsigned char*)"PRO KEEPER");
        else if (keeper_score >= 30)  put_text(13, 17, (const unsigned char*)"GOOD EFFORT");
        else                          put_text(12, 17, (const unsigned char*)"NEED PRACTICE");
        
        if (num_players == 2) {
            if (scores[0] > 0) {
                put_text(8, 20, (const unsigned char*)"WINNER: PLAYER 1");
                paint_row_color(20, 8, 17, VIC.spr_color[0]);
            } else {
                put_text(13, 20, str_draw);
                put_text(10, 21, (const unsigned char*)"GOALIE DEFENDED!");
                paint_row_color(20, 13, 9, p_color);
                paint_row_color(21, 10, 16, p_color);
            }
            goto wait_end;
        }
    }

    // --- ANÚNCIO FINAL DA PARTIDA ---
    if (max_gols == 0) {
        put_text(10, 20, (const unsigned char*)"NO GOALS SCORED   ");
    } else if (is_tie) {
        put_text(13, 20, str_draw);
    } else {
        put_text(8, 20, (const unsigned char*)"MATCH WINNER: PLAYER ");
        put_num(29, 20, winner);
        paint_row_color(20, 8, 23, get_fixed_player_color(winner));
    }

wait_end:
    for(i = 0; i < 250; i++) wait_vblank();
}

static void __fastcall__ apply_smart_kick2(unsigned char p_idx) {
    register unsigned char zone;
    register unsigned char pers;
    unsigned char option;
    signed char temp_power;
    unsigned char ataca_sul;  
    
    pers = player_pers[p_idx];

    init_random();  

    /* CÁLCULO DA ZONA DO CAMPO (Otimizado com operador ternário em cascata) */
    zone = (bx < 106) ? 0 : ((bx < 212) ? 1 : 2);
    if (by > 66) {
        zone += 3;
        if (by > 132) zone += 3;
    }

    /* SELEÇÃO DE OPÇÃO DA LUT */
    option = rand() & 0x03;
    if (difficulty_level > 1) option |= 0x02;

    temp_power = 8 + difficulty_level;
    if (pers == 1) temp_power += 2;  
    if (temp_power > 12) temp_power = 12;

    bdx = (signed char)lut_kick_br[zone][option][0] * temp_power;
    bdy = (signed char)lut_kick_br[zone][option][1] * temp_power;
    ai_curve_dir = lut_kick_br[zone][option][2];

    /* LÓGICA DE DIREÇÃO: CHUTAR PARA NORTE OU SUL */
    /* Define o alvo baseado na metade do campo em que a bola (ou o jogador) está.
       Considerando as zonas 66 e 132, o meio do campo Y deve ser aproximadamente 99. */
    ataca_sul = (by > 99) ? 1 : 0; 

    /* Inverte a polaridade do eixo Y caso o chute natural da LUT esteja para o lado errado */
    if (ataca_sul) {
        /* Chutando para o SUL (Y positivo) */
        if (bdy < 0) {
            bdy = -bdy;
            ai_curve_dir = -ai_curve_dir;
        } else if (bdy == 0) {
            bdy = temp_power;
        }
    } else {
        /* Chutando para o NORTE (Y negativo) */
        if (bdy > 0) {
            bdy = -bdy;
            ai_curve_dir = -ai_curve_dir;
        } else if (bdy == 0) {
            bdy = -temp_power;
        }
    }

    /* DESVIO DE CHUTES CENTRAIS (Evitar chutar reto no goleiro) */
    /* Se o chute estiver saindo 100% reto (bdx == 0) e o jogador 
       estiver na zona central perto dos gols (zone 1 ou zone 7) */
    if (bdx == 0 && (zone == 1 || zone == 7)) {
        /* Desvia a bola para o canto direito ou esquerdo, usando metade da força do chute.
           O operador (>> 1) é uma divisão por 2 ultra-rápida. */
        bdx = (rand() & 1) ? (temp_power >> 1) : -(temp_power >> 1);
    }

    after_touch_timer = 60 + (difficulty_level << 3);
    if (pers == 3) after_touch_timer >>= 1;

    kick_power = temp_power;
}

static unsigned char get_fixed_player_color(unsigned char p_idx) {
    switch(p_idx) {
        case 1: return 7; // P1
        case 2: return 2;    // P2
        case 3: return 1;  // P3 (Ajuste para Verde se preferir)
        case 4: return 10;   // P4
        default: return 1;         // Branco
    }
}



static void fast_clrscr(unsigned char color) {
    // 32 é o código do espaço vazio (sem gramado)
    memset(SCREEN, 32, 1000); 
    // Força a cor branca em tudo para o texto aparecer
    memset((void*)0xD800, color, 1000); 
}


static void __fastcall__ put_num2(unsigned char x, unsigned char y, unsigned int num) {
    unsigned int offset = y * 40 + x;
    unsigned char* screen_ptr;
    unsigned char digit;

    // Detecta o banco ativo (Mantendo sua lógica original)
    if (VIC.addr < 0x10) {
        screen_ptr = (unsigned char*)(0xC000 + offset);
    } else {
        screen_ptr = (unsigned char*)(0x0400 + offset);
    }

    // Centena
    digit = (num / 100);
    screen_ptr[0] = digit + 48;
    
    // Dezena
    digit = (num % 100) / 10;
    screen_ptr[1] = digit + 48;
    
    // Unidade
    digit = num % 10;
    screen_ptr[2] = digit + 48;

    // Atualiza a Color RAM para os 3 dígitos
    ((unsigned char*)0xD800)[offset] = 1;     
    ((unsigned char*)0xD800)[offset + 1] = 1; 
    ((unsigned char*)0xD800)[offset + 2] = 1; 
}

static void reset_border_color(void) {
    if (field_type == 1)      VIC.bordercolor = 14; // Azul claro (Molhado)
    else if (field_type == 2) VIC.bordercolor = 15; // Cinza claro (Neve)
    else                      VIC.bordercolor = 13; // Verde claro (Grama)
}



static void setup_sid(void) {
    SID.amp = 0x1F;        // Volume máximo (15) + sem filtros
    SID.v1.ad = 0x09;     // Attack/Decay para o chute
    SID.v1.sr = 0x00;     // Sustain/Release
}


static void __fastcall__  put_num(unsigned char x, unsigned char y, unsigned char num) {
    unsigned int offset = y * 40 + x;
    unsigned char* screen_ptr;

    // Detecta o banco ativo:
    // Se VIC.addr for menor que 0x10, estamos no Bank 3 ($C000)
    // Se for 0x14 ou similar, estamos no Bank 0 ($0400)
    if (VIC.addr < 0x10) {
        screen_ptr = (unsigned char*)(0xC000 + offset);
    } else {
        screen_ptr = (unsigned char*)(0x0400 + offset);
    }

    // Escreve os caracteres (Screencodes) diretamente no ponteiro calculado
    screen_ptr[0] = (num / 10) + 48; // Dezena
    screen_ptr[1] = (num % 10) + 48; // Unidade

    // A Color RAM ($D800) é global e não muda com os bancos
    ((unsigned char*)0xD800)[offset] = 1;     // Branco
    ((unsigned char*)0xD800)[offset + 1] = 1; // Branco
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



static void update_sfx(void) {
    static unsigned char vib = 0;
    static unsigned int basefreq = 0xB400;

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


static void play_sfx_whistle(void) {
    SID.v2.ctrl = 0x00;

    SID.v2.freq = 0xB400;      // Pitch base
    SID.v2.ctrl = 0x41;        // Pulse + Gate ON

    whistle_state = 1;
    whistle_timer = 22;
}


static void play_sfx_game_over(void) {
    SID.v2.ctrl = 0x00;

    SID.v2.freq = 0xA800;
    SID.v2.ctrl = 0x41;

    whistle_state = 2;
    whistle_timer = 55;
}


static void init_whistle_voice(void) {
    SID.v2.ctrl = 0x00;
    SID.v2.ad   = 0x08;
    SID.v2.sr   = 0xA8;
    SID.v2.pw   = 0x0600;   // Pulse estreito
}

static void init_crowd_voice(void) {
    SID.v3.ctrl = 0x00;

    SID.v3.ad = 0x02;
    SID.v3.sr = 0x11;

	SID.v3.freq = 0x0C00;


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
    unsigned char* scr_ptr = SCREEN + 45; 
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
                    scr_ptr[x] = 87;
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
    put_text(6, 7, msg_titulo);
    put_text(4, 8, msg_credits);
    put_text(1, 9, msg_line);
    
   // Menu direto com apenas 2 opções
    put_text(10, 11, (const unsigned char*)"1- TRAINING");
    put_text(10, 12, (const unsigned char*)"2- CHALLENGE");
	put_text(10, 13, (const unsigned char*)"3- PLAYOFF");
	put_text(10, 14, (const unsigned char*)"4- EXIT TO MENU");

    // Instrução movida para a linha 17 para dar respiro
    put_text(12, 16, (const unsigned char*)"SELECT GAME MODE");
    
    // A linha divisória (msg_line) será desenhada pela show_menu 
    // na linha 18 conforme os submenus aparecem.
}


static void show_demo(void) {
    unsigned char i;

    *(unsigned char*)198 = 0; // Limpa o buffer de teclado uma última vez
    // 1. Sinaliza o sistema que estamos em modo de demonstração
    is_demo_mode = 1;
    demo_frame_counter = 0;
    
    // 2. Configurações da Partida (Exibição/Versus)
    game_mode = 5; 
    num_players = 4;
//	num_players = (rand() % 4) + 1;
	use_defender = rand() % 2;
    difficulty_level = 2; // Normal para a IA não ser boba nem perfeita
    gold_goal_active = 0; // Partida por tempo
    
    // 3. Define todos os 4 slots como CPU (0 = 'C')
    for (i = 1; i <= 4; i++) {
        player_type[i] = 0; 
    }

    // 4. Sorteio de Times (0 a 11)
    // Usando o rasterline ou SID noise como semente caso precise de mais aleatoriedade
    //home_team = rand() % 16;
    //away_team = (home_team + 1 + (rand() % 15)) % 16; // Garante que away != home

    // 5. Configuração Visual do Campo (Sempre Checker para o Demo)
    field_type = rand() % 3;   // 0: Grass, 1: Wet, 2: Frozen
    field_style = rand() % 3;           // 0: Checker (Quadriculado)
    selected_field_option = field_type; // Sincroniza com a variável global

    // 6. Preparação Técnica
 //   setup_match_colors(home_team, away_team);
    
    // Define os IDs dos times para a lógica de "is_human_team" não bugar
  //  for(i = 0; i < 16; i++) is_human_team[i] = 0; 
    // No demo, ninguém é humano, mas os times precisam ser processados pela IA
//    player_team[1] = home_team; player_team[2] = home_team;
//    player_team[3] = away_team; player_team[4] = away_team;

	sorteia_personalidades();
    // 7. Execução
    play_match();

    // 8. Cleanup: Ao sair da play_match (seja por tempo ou tecla), limpa as flags
    is_demo_mode = 0;
    
    // Para todos os sons para não voltar ao menu com ruído
    silence_sid(); 
    setup_sid();
	music_init();
	*(unsigned char*)198 = 0; // Limpa o buffer de teclado uma última vez
}

static void show_menu(void) {
    unsigned char menu_sel = 0;
    unsigned char old_sel = 255; // Força o redesenho na primeira execução
    unsigned char key;
    unsigned char joy1_now;
//	unsigned char total_active;
    unsigned char p_idx;
	unsigned char max_opt;
	unsigned char menu_level = 0; // 0 = Novo Menu, 1 = Outros Modos
    
    hs_timer = 0;
    refresh_menu_visuals();
    last_joy_state = ~CIA1.pra;
	
	is_overtime = 0;
	golden_score_triggered = 0;
	gold_goal_active = 0;
	tournament_stage = 0;
	is_demo_mode = 0;
    s1_total=0;
    s2_total=0;
	v1_total = 0;
	v2_total = 0;
	v3_total = 0;
	v4_total = 0;
	playoff_game = 0;
    challenge_attempts = 2;
	challenge_bonuses_given = 0;
	music_init();
	
	clear_saved_sprites();
	if (game_mode == 6) {
    unsigned char tournament_stage = 0;    tournament_stage = 0; 
    }

    // --- PARTE 1: MODO DE JOGO ---
    while(1) {
		
		music_play();
        wait_vblank();
        joy_now = ~CIA1.pra;
        joy1_now = ~CIA1.prb;
        
        key = 0; 
        if (kbhit()) {
            key = cgetc();
            hs_timer = 0;
        }
        
        
        // Reset se houver qualquer movimento nos joysticks
		if (joy_now != last_joy_state || joy1_now != 0) {
			hs_timer = 0;
		}

		hs_timer++;
 		// Atalho manual: Tecla 'G' para forçar o demo imediatamente
        if (key == 'g' || key == 'G') {
			music_stop();
		//	wait_vblank();
			silence_sid();
			setup_sid();
			show_demo();
            refresh_menu_visuals();
			music_init();
            old_sel = 255; hs_timer = 0;
            last_joy_state = ~CIA1.pra;
            continue; 
        }
		
		
			// Se passar de ~30 segundos (1800 frames em 60Hz ou 1500 em 50Hz)
        if (hs_timer > 1800) {
			music_stop();
		//	wait_vblank();
			silence_sid();
			setup_sid();
            show_demo();
            // Ao voltar do demo, precisamos redesenhar o menu
            refresh_menu_visuals();
			music_init();
            old_sel = 255; 
            hs_timer = 0;
            last_joy_state = ~CIA1.pra;
            continue;
        }
		
		 // Reset se houver qualquer movimento nos joysticks
		if (joy_now != last_joy_state || joy1_now != 0) {
			hs_timer = 0;
			
		}

      // OTIMIZAÇÃO: Só desenha as setas se menu_sel mudou
    if (menu_sel != old_sel) {
        if (old_sel != 255) put_text(8, 11 + old_sel, (const unsigned char*)" "); 
        put_text(8, 11 + menu_sel, (const unsigned char*)">");
        old_sel = menu_sel;
    }

    // NAVEGAÇÃO: Aumentado para 3 (permite selecionar as linhas 0, 1, 2 e 3)
    if ((joy_now & 1) && !(last_joy_state & 1) && menu_sel > 0) menu_sel--;
    if ((joy_now & 2) && !(last_joy_state & 2) && menu_sel < 3) menu_sel++;
    
    // BOTÃO FIRE (JOYSTICK)
    if ((joy_now & 0x10) && !(last_joy_state & 0x10)) {
        if (menu_sel == 3) {
//            chama_menu("m1"); // Chama o buffer do teclado para carregar o Menu Principal
            silence_sid();
			setup_sid();  
//			chama_menu_asm("m1");
            chama_menu("m1"); // Chama o buffer do teclado para carregar o Menu Principal
            return; 
        } else {
			silence_sid();
			setup_sid();
            game_mode = menu_sel + 5; // Opções 0, 1 e 2 resultam em game_mode 5, 6 e 7
            break; 
        }
    }

    // TECLAS DE ATALHO (TECLADO)
    if (key >= '1' && key <= '3') {
		silence_sid();
	    setup_sid();
        game_mode = (key - '0') + 4; // '1'->5, '2'->6, '3'->7
        break;
    }
    
    // Atalho tecla 4 para sair
    if (key == '4') {
       // chama_menu("m1");
	   silence_sid();
			setup_sid();
//		chama_menu_asm("m1");
            chama_menu("m1"); // Chama o buffer do teclado para carregar o Menu Principal
        return;
    }

    last_joy_state = joy_now;
}

	VIC.bordercolor = 0;
// --- PARTE 1.5: CONFIGURAÇÃO MODO 5 (COMPACTO) ---
// --- PARTE 1.5: CONFIGURAÇÃO MODO 5 (COMPACTO) ---
 // --- PARTE 1.5: CONFIGURAÇÃO MODO 5 (COMPACTO) E MODO 7 (PLAYOFF) ---
   if (game_mode == 5 || game_mode == 7) {
        // 1. Define o mínimo de jogadores baseado no modo
        unsigned char min_p = (game_mode == 7) ? 2 : 1;
        
        put_text(1, 15, msg_line);
        // Deixei o espaço em branco no final porque o loop 'while' abaixo já vai desenhar o < 1P > ou < 2P > na primeira iteração
        put_text(1, 16, (const unsigned char*)"TOTAL PLAYERS:             "); 
        
        // 2. Inicia já com o mínimo correto
        num_players = min_p; 
        old_sel = 255;
        last_joy_state = ~CIA1.pra;

        // --- ETAPA A: Quantos jogadores no total? ---
        while(1) {
            wait_vblank(); 
            joy_now = ~CIA1.pra;
            
            if (num_players != old_sel) {
                if (num_players == 1)      put_text(21, 16, (const unsigned char*)"< 1P >");
                else if (num_players == 2) put_text(21, 16, (const unsigned char*)"< 2P >");
                else if (num_players == 3) put_text(21, 16, (const unsigned char*)"< 3P >");
                else                       put_text(21, 16, (const unsigned char*)"< 4P >");
                old_sel = num_players;
            }

            // Aumenta até 4
            if ((joy_now & 8) && !(last_joy_state & 8) && num_players < 4) num_players++;
            
            // 3. Diminui até esbarrar no limite 'min_p' que definimos no início
            if ((joy_now & 4) && !(last_joy_state & 4) && num_players > min_p) num_players--;
            
            if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
            last_joy_state = joy_now;
        }

        // --- ETAPA B: Definir tipo de cada jogador (P1 até P4) ---
        for(i=0; i<15; i++) wait_vblank(); 
        
        for (p_idx = 1; p_idx <= 4; p_idx++) {
            if (p_idx > num_players) {
                player_type[p_idx] = 255;
                continue; 
            }

            {
                unsigned char col = ((p_idx - 1) % 2) * 19 + 1; 
                unsigned char row = 17 + ((p_idx - 1) / 2);     

                unsigned char p_label[] = "P1:   CPU  ";
                p_label[1] = p_idx + 48;
                
                put_text(col, row, p_label);
                
                player_type[p_idx] = (p_idx == 1) ? 1 : 0; 
                old_sel = 255;

                while ((~CIA1.pra) & 0x10) { 
                    wait_vblank(); 
                }
                last_joy_state = ~CIA1.pra;

                while(1) {
                    wait_vblank(); 
                    joy_now = ~CIA1.pra;
                    
                    if (player_type[p_idx] != old_sel) {
                        // O P2 só tem a opção de Goleiro ('G') se for no Modo 5
                        if (p_idx == 2 && game_mode == 5) {
                            if (player_type[2] == 0)      put_text(col + 4, row, (const unsigned char*)"<C> H  G");
                            else if (player_type[2] == 1) put_text(col + 4, row, (const unsigned char*)" C <H> G");
                            else if (player_type[2] == 2) put_text(col + 4, row, (const unsigned char*)" C  H <G>");
                        } else {
                            if (player_type[p_idx] == 0)  put_text(col + 4, row, (const unsigned char*)"<C> H  ");
                            else                          put_text(col + 4, row, (const unsigned char*)" C <H> ");
                        }
                        old_sel = player_type[p_idx];
                    }
                    
                    if ((joy_now & 0x10) && !(last_joy_state & 0x10)) {
                        break;
                    }

                    // Define o limite máximo baseado no modo de jogo
                    max_opt = (p_idx == 2 && game_mode == 5) ? 2 : 1;

                    if ((joy_now & 8) && !(last_joy_state & 8) && player_type[p_idx] < max_opt) {
                        player_type[p_idx]++;
                    } else if ((joy_now & 4) && !(last_joy_state & 4) && player_type[p_idx] > 0) {
                        player_type[p_idx]--;
                    }

                    last_joy_state = joy_now;
                }
                for(i=0; i<12; i++) wait_vblank(); 
            }
        }

        // --- ETAPA C: CONFIGURAÇÃO EXCLUSIVA MODO 7 (PLAYOFF) ---
        if (game_mode == 7) {
			 put_text(1, 18, msg_line); // Linha separadora
            put_text(1, 19, (const unsigned char*)"MATCHES (BO):       < 3 >");
            playoff_matches = 3; 
            old_sel = 255;
            
            while ((~CIA1.pra) & 0x10) wait_vblank(); // Aguarda soltar o botão
            last_joy_state = ~CIA1.pra;

            while(1) {
                wait_vblank();
                joy_now = ~CIA1.pra;

                if (playoff_matches != old_sel) {
                    if (playoff_matches == 3)      put_text(21, 19, (const unsigned char*)"< 3 >");
                    else if (playoff_matches == 5) put_text(21, 19, (const unsigned char*)"< 5 >");
                    else if (playoff_matches == 7) put_text(21, 19, (const unsigned char*)"< 7 >");
                    old_sel = playoff_matches;
                }

                // Direita aumenta (3 -> 5 -> 7)
                if ((joy_now & 8) && !(last_joy_state & 8) && playoff_matches < 7) playoff_matches += 2;
                // Esquerda diminui (7 -> 5 -> 3)
                if ((joy_now & 4) && !(last_joy_state & 4) && playoff_matches > 3) playoff_matches -= 2;

                if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
                last_joy_state = joy_now;
            }
            
            // Variável de alvo de vitórias já é definida automaticamente aqui
            // 3 partidas = 2 vitórias / 5 partidas = 3 vitórias / 7 partidas = 4 vitórias
            playoff_target_wins = (playoff_matches / 2) + 1;
            
            for(i=0; i<12; i++) wait_vblank();
        }
    }
    
    // --- PARTE 1.6: CONFIGURAÇÃO MODO 6 (CHALLENGE) ---
    else if (game_mode == 6) {
        put_text(1, 15, msg_line);
        put_text(1, 16, (const unsigned char*)"HUMAN PLAYERS:      < 1P > ");
        
        num_players = 1; 
        old_sel = 255;
        last_joy_state = ~CIA1.pra;

        // --- ETAPA ÚNICA: Escolher 1P (P1 Humano + P2 CPU) ou 2P (P1 e P2 Humanos) ---
        while(1) {
            wait_vblank(); 
            joy_now = ~CIA1.pra;
            
            if (num_players != old_sel) {
                if (num_players == 1)      put_text(21, 16, (const unsigned char*)"< 1P > ");
                else                       put_text(21, 16, (const unsigned char*)"< 2P > ");
                old_sel = num_players;
            }

            if ((joy_now & 8) && !(last_joy_state & 8) && num_players < 2) num_players++;
            if ((joy_now & 4) && !(last_joy_state & 4) && num_players > 1) num_players--;
            
            if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
            last_joy_state = joy_now;
        }

        // --- CONFIGURAÇÃO AUTOMÁTICA DOS 4 SLOTS DO MODO 6 ---
        player_type[1] = 1;                          
        player_type[2] = (num_players == 2) ? 1 : 0; 
        
        player_type[3] = 0;                          
        player_type[4] = 0;           
       // selected_field_option = 3;
       // field_style = 3;


        for(i = 0; i < 15; i++) wait_vblank();    
        goto Finaliza;   
    }

	
	// --- Etapa C: Escolha do Objetivo (TEMPO ou 5 GOLS) ---
    for(i=0; i<15; i++) wait_vblank(); // Limpa buffer do botão
    
    put_text(1, 18, msg_line); // Linha separadora
    put_text(1, 19, (const unsigned char*)"MATCH TYPE:      < TIME >");
    gold_goal_active = 0; 
    old_sel = 255;
    last_joy_state = ~CIA1.pra;

    while(1) {
        wait_vblank(); 
        joy_now = ~CIA1.pra;
        
        if (gold_goal_active != old_sel) {
            if (gold_goal_active == 0) 
                put_text(18, 19, (const unsigned char*)"<  TIME  >");
            else 
                put_text(18, 19, (const unsigned char*)"< 5 GOALS >");
            old_sel = gold_goal_active;
        }

        // Navegação
        if ((joy_now & 12) && !(last_joy_state & 12)) { // Esquerda ou Direita
            gold_goal_active = !gold_goal_active; 
        }
        
        if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
        last_joy_state = joy_now;
    }
	
	  // --- PARTE 2: ZAGUEIRO ---
    put_text(1, 20, msg_line);
    put_text(1, 21, (const unsigned char*)"USE DEFENDER:  ");
    menu_sel = 0; old_sel = 255;
    last_joy_state = ~CIA1.pra;

    while(1) {
        wait_vblank();
        joy_now = ~CIA1.pra;

        if (menu_sel != old_sel) {
            // Usa setas para indicar a escolha
            put_text(15, 21, (menu_sel == 0) ? (const unsigned char*)"> NO " : (const unsigned char*)"  NO ");
            put_text(21, 21, (menu_sel == 1) ? (const unsigned char*)"> YES" : (const unsigned char*)"  YES");
            old_sel = menu_sel;
        }

        if ((joy_now & 4) && !(last_joy_state & 4)) menu_sel = 0;
        if ((joy_now & 8) && !(last_joy_state & 8)) menu_sel = 1;
        if ((joy_now & 0x10) && !(last_joy_state & 0x10)) {
            use_defender = menu_sel;
            break;
        }
        last_joy_state = joy_now;
    }

   

// --- PARTE 3: CONFIGURAÇÃO DO CAMPO (MESMA LINHA) ---
    put_text(1, 22, msg_line);
    put_text(1, 23, (const unsigned char*)"FIELD: ");
    put_text(18, 23, (const unsigned char*)" STYLE: ");
    
    // Sub-etapa A: TIPO (PISO)
    selected_field_option = 0; old_sel = 255;
    last_joy_state = ~CIA1.pra;
    while(1) {
        wait_vblank(); joy_now = ~CIA1.pra;
        if (selected_field_option != old_sel) {
            // Opção atual com setas
            if (selected_field_option == 0)      put_text(7, 23, (const unsigned char*)"< GRASS  >");
            else if (selected_field_option == 1) put_text(7, 23, (const unsigned char*)"<  WET   >");
            else if (selected_field_option == 2) put_text(7, 23, (const unsigned char*)"< FROZEN >");
            else                                 put_text(7, 23, (const unsigned char*)"< RANDOM >");
            
            // Style sem setas (foco está no Type)
            if (field_style == 0)      put_text(26, 23, (const unsigned char*)" CHECKER ");
            else if (field_style == 1) put_text(26, 23, (const unsigned char*)"   BARS  ");
            else if (field_style == 2) put_text(26, 23, (const unsigned char*)"   SOLID ");
            else                       put_text(26, 23, (const unsigned char*)"  RANDOM ");
            
            old_sel = selected_field_option;
        }
        if ((joy_now & 4) && !(last_joy_state & 4) && selected_field_option > 0) selected_field_option--;
        if ((joy_now & 8) && !(last_joy_state & 8) && selected_field_option < 3) selected_field_option++;
        if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
        last_joy_state = joy_now;
    }

    // Sub-etapa B: ESTILO (PADRÃO)
    field_style = 0; old_sel = 255;
    last_joy_state = ~CIA1.pra;
    while(1) {
        wait_vblank(); joy_now = ~CIA1.pra;
        if (field_style != old_sel) {
            // Type sem setas (foco saiu daqui)
            if (selected_field_option == 0)      put_text(7, 23, (const unsigned char*)"  GRASS   ");
            else if (selected_field_option == 1) put_text(7, 23, (const unsigned char*)"   WET    ");
            else if (selected_field_option == 2) put_text(7, 23, (const unsigned char*)"  FROZEN  ");
            else                                 put_text(7, 23, (const unsigned char*)"  RANDOM  ");

            // Style agora com setas (0: CHECKER, 1: BARS, 2: SOLID, 3: RANDOM)
            if (field_style == 0)      put_text(25, 23, (const unsigned char*)"< CHECKER >");
            else if (field_style == 1) put_text(25, 23, (const unsigned char*)"<  BARS   >");
            else if (field_style == 2) put_text(25, 23, (const unsigned char*)"<  SOLID  >");
            else                       put_text(25, 23, (const unsigned char*)"< RANDOM  >");
            
            old_sel = field_style;
        }
        if ((joy_now & 4) && !(last_joy_state & 4) && field_style > 0) field_style--;
        if ((joy_now & 8) && !(last_joy_state & 8) && field_style < 3) field_style++;
        if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
        last_joy_state = joy_now;
    }


    // --- PARTE 4: DIFICULDADE (CARROSSEL) ---
    put_text(1, 24, (const unsigned char*)"DIFFICULTY:   < NORMAL >");
    difficulty_level = 1; old_sel = 255;
    last_joy_state = ~CIA1.pra;
    while(1) {
        wait_vblank(); joy_now = ~CIA1.pra;
        if (difficulty_level != old_sel) {
            put_text(15, 24, (const unsigned char*)"        "); 
            if (difficulty_level == 0)      put_text(15, 24, (const unsigned char*)"<  EASY  >");
            else if (difficulty_level == 1) put_text(15, 24, (const unsigned char*)"< NORMAL >");
            else                            put_text(15, 24, (const unsigned char*)"<  HARD  >");
            old_sel = difficulty_level;
        }
        if ((joy_now & 4) && !(last_joy_state & 4) && difficulty_level > 0) difficulty_level--;
        if ((joy_now & 8) && !(last_joy_state & 8) && difficulty_level < 2) difficulty_level++;
        if ((joy_now & 0x10) && !(last_joy_state & 0x10)) break;
        last_joy_state = joy_now;
    }
    Finaliza:
    play_sfx_whistle();
 }





// Ficou boa!
static void play_sfx_kick(void) {
    SID.v1.ctrl = 0x00;    
    SID.v1.freq = 0x0AF0;  // Frequência mais alta (traz o brilho de volta)
    SID.v1.ad = 0x04;      // Decay um pouco mais curto para dar o "estalo" rápido
    SID.v1.sr = 0x00;      
    SID.v1.ctrl = 0x11;    // Mantém a onda Triângulo
}



static void play_sfx_defend(void) {
    SID.v1.ctrl = 0x80;    // Seleciona Ruído (Noise)
    
    // 1. Frequência mais baixa para um som mais "grave"
    SID.v1.freq = 0x1200;  
    
    // 2. Attack 0 (instantâneo) e Decay 5 (mais curto que o chute)
    SID.v1.ad = 0x05;      
    
    // 3. Sustain e Release curtos para não sobrar "cauda" no som
    SID.v1.sr = 0x00;      

    // OPCIONAL: Configuração básica de filtro (se você já tiver o filtro ativo globalmente)
    // SID.flt_ctrl = 0x11; // Filtra a Voz 1 com Low Pass
    // SID.flt_cutoff = 0x4000; // Corta as frequências agudas do ruído
    
    SID.v1.ctrl = 0x81;    // Dispara (Gate ON)
}


static void draw_field_manager(void) {
    // Passa field_style como parâmetro (ex: 0 para checkers/quadrados, 1 para sólido)
    draw_field(field_style);
}

static void draw_field(unsigned char style) {
    unsigned int offset = 0;
    unsigned int row;
    unsigned char x, y, x_cnt, y_cnt;
    unsigned char color_dark, color_light, current_color, row_start_color;

    enter_game_mode();

    // Seleção de cores baseada no tipo de gramado
    if (field_type == 1) {      /* WET */
        color_dark = 6; color_light = 14; 
    } else if (field_type == 2) { /* FROZEN */
        color_dark = 11; color_light = 12; 
    } else {                    /* GRASS */
        color_dark = 5; color_light = 13; 
    }

    // --- 1. PREENCHIMENTO DO GRAMADO (VARIÁVEL POR ESTILO) ---
    if (style == 0) {
        /* Estilo Checkers (Quadrados) */
        row_start_color = color_dark;
        y_cnt = 0;

        for (y = 0; y < 25; ++y) {
            current_color = row_start_color;
            x_cnt = 0;

            for (x = 0; x < 40; ++x) {
                SCREEN[offset] = 160; 
                COLOR_RAM[offset] = current_color;
                offset++;

                if (++x_cnt == 5) { 
                    x_cnt = 0;
                    current_color = (current_color == color_dark) ? color_light : color_dark;
                }
            }

            if (++y_cnt == 5) { 
                y_cnt = 0;
                row_start_color = (row_start_color == color_dark) ? color_light : color_dark;
            }
        }
    } else if (style == 1) {
        /* Estilo Barras Horizontais */
        current_color = color_dark;
        y_cnt = 0;

        for (y = 0; y < 25; ++y) {
            for (x = 0; x < 40; ++x) {
                SCREEN[offset] = 160; 
                COLOR_RAM[offset] = current_color;
                offset++;
            }

            if (++y_cnt == 5) { 
                y_cnt = 0;
                current_color = (current_color == color_dark) ? color_light : color_dark;
            }
        }
    } else {
        /* Estilo Sólido (Liso, sem checkers ou barras) */
        for (y = 0; y < 25; ++y) {
            for (x = 0; x < 40; ++x) {
                SCREEN[offset] = 160; 
                COLOR_RAM[offset] = color_dark; // Apenas a cor principal
                offset++;
            }
        }
    }

    // --- 2. MOLDURA EXTERNA E REDES (COMPARTILHADO) ---
    for (x = 0; x < 40; x++) {
        current_color = (x >= 15 && x <= 23) ? NET : HLINE;
        SCREEN[x] = current_color;         COLOR_RAM[x] = 1;       /* Norte */
        SCREEN[960 + x] = current_color;    COLOR_RAM[960 + x] = 1; /* Sul */
    }

    for (y = 0; y < 25; y++) {
        row = y * 40;
        SCREEN[row] = VLINE;      COLOR_RAM[row] = 1;       /* Oeste */
        SCREEN[row + 39] = VLINE; COLOR_RAM[row + 39] = 1; /* Leste */
    }

    SCREEN[0] = CTL; SCREEN[39] = CTR; SCREEN[960] = CBL; SCREEN[999] = CBR;

    /* --- 3. LINHA DE MEIO-DE-CAMPO --- */
    for (x = 1; x < 39; x++) {
        SCREEN[480 + x] = HLINE;
        COLOR_RAM[480 + x] = 1;
    }
    SCREEN[480 + 19] = PENALTY; COLOR_RAM[480 + 19] = 1;

    /* --- 4. ÁREAS --- */
    
    // ÁREA NORTE
    for (x = 13; x <= 25; x++) { SCREEN[120 + x] = HLINE; COLOR_RAM[120 + x] = 1; }
    for (y = 1; y < 3; y++) {
        SCREEN[y * 40 + 13] = VLINE; COLOR_RAM[y * 40 + 13] = 1;
        SCREEN[y * 40 + 25] = VLINE; COLOR_RAM[y * 40 + 25] = 1;
    }
    SCREEN[133] = CBL; SCREEN[145] = CBR;
    SCREEN[13] = TAREA; COLOR_RAM[13] = 1;
    SCREEN[25] = TAREA; COLOR_RAM[25] = 1;

    // ÁREA SUL
    for (x = 13; x <= 25; x++) { SCREEN[840 + x] = HLINE; COLOR_RAM[840 + x] = 1; }
    for (y = 22; y < 24; y++) {
        SCREEN[y * 40 + 13] = VLINE; COLOR_RAM[y * 40 + 13] = 1;
        SCREEN[y * 40 + 25] = VLINE; COLOR_RAM[y * 40 + 25] = 1;
    }
    SCREEN[853] = CTL; SCREEN[865] = CTR;
    SCREEN[960 + 13] = TIAREA; COLOR_RAM[960 + 13] = 1;
    SCREEN[960 + 25] = TIAREA; COLOR_RAM[960 + 25] = 1;

    /* --- 5. MARCAS DE PÊNALTI --- */
    SCREEN[219] = PENALTY; COLOR_RAM[219] = 1;
    SCREEN[779] = PENALTY; COLOR_RAM[779] = 1;
}


// Defender
/*
static void __fastcall__  keep_defender_in_field(unsigned int* x, unsigned int* y) {

    if (*x < FIELD_LEFT)  *x = FIELD_LEFT;
    if (*x > FIELD_RIGHT) *x = FIELD_RIGHT;

    if (*y < FIELD_TOP)    *y = FIELD_TOP;
    if (*y > FIELD_BOTTOM) *y = FIELD_BOTTOM;
}
*/
// MACRO ULTRA OTIMIZADA (Bounding Box Sem Subtração)
// 1. Sem variáveis temporárias (poupa ciclos de memória/Zero Page)
// 2. Com trava de segurança contra underflow (x > 2) para evitar que
//    o jogador seja arremessado para o outro lado da tela (65535).
#define CHECK_COLLISION_FAST(x1, y1, x2, y2) do { \
    if ((y1) + 12 > (y2) && (y1) < (y2) + 12) { \
        if ((x1) + 12 > (x2) && (x1) < (x2) + 12) { \
            if ((x1) > (x2)) { (x1) += 2; if ((x2) > 2) (x2) -= 2; } \
            else { if ((x1) > 2) (x1) -= 2; (x2) += 2; } \
            if ((y1) > (y2)) { (y1) += 2; if ((y2) > 2) (y2) -= 2; } \
            else { if ((y1) > 2) (y1) -= 2; (y2) += 2; } \
        } \
    } \
} while(0)



static void separate_all_players(void) {
 //   unsigned int diff_x; 
 //   unsigned char diff_y;
//    unsigned char gx, gy; // Flags locais de 8 bits (o CC65 mapeia direto para registradores/zero-page)

   

    if (ia_frame_toggle & 1) {
        CHECK_COLLISION_FAST(px, py, p2x, p2y);
        CHECK_COLLISION_FAST(p3x, p3y, p4x, p4y);
        CHECK_COLLISION_FAST(px, py, p3x, p3y);
    } else {
        CHECK_COLLISION_FAST(p2x, p2y, p4x, p4y);
        CHECK_COLLISION_FAST(px, py, p4x, p4y);
        CHECK_COLLISION_FAST(p2x, p2y, p3x, p3y);
    }
    
    // --- COLISÕES DOS ZAGUEIROS ---
     if (use_defender == 1) {
        if (ia_frame_toggle & 1) {
            CHECK_COLLISION_FAST(dx, dy, px, py);
            CHECK_COLLISION_FAST(dx, dy, p3x, p3y);
            CHECK_COLLISION_FAST(dx_b, dy_b, p2x, p2y);
            CHECK_COLLISION_FAST(dx_b, dy_b, p4x, p4y);
        } else {
            CHECK_COLLISION_FAST(dx, dy, p2x, p2y);
            CHECK_COLLISION_FAST(dx, dy, p4x, p4y);
            CHECK_COLLISION_FAST(dx_b, dy_b, px, py);
            CHECK_COLLISION_FAST(dx_b, dy_b, p3x, p3y);
        }
    }
    
 
}




static void move_player_ia_modo5(unsigned char p_idx) {
    // Declarações static para evitar overhead de stack no CC65
    static unsigned char pers;
	//, mask, current_trig, current_delay;
    static int tx, ty, curr_x, curr_y;
    static unsigned char joy_out;
    unsigned char antecip;
    
    // Arrays de lookup baseados no p_idx (0 a 3) para evitar if/else lentos
    static const unsigned char def_trig[4]  = {85, 80, 75, 80};
    static const unsigned char poss_min[4]  = {150, 150, 120, 150};
    static const unsigned char rand_mask[4] = {3, 3, 1, 7};
    static const unsigned char trig_base[4] = {80, 70, 60, 60};

    // 1. Leitura inicial do estado e verificação de stun
    switch (p_idx) {
        case 1:
            if (stun_timer_p1) { stun_timer_p1--; p1_is_moving = 0; joy2 = 0; return; }
            curr_x = px; curr_y = py;
            break;
        case 2:
            if (stun_timer_p2) { stun_timer_p2--; p2_is_moving = 0; joy1 = 0; return; }
            curr_x = p2x; curr_y = p2y;
            break;
        case 3:
            if (stun_timer_p3) { stun_timer_p3--; p3_is_moving = 0; joy3 = 0; return; }
            curr_x = p3x; curr_y = p3y;
            break;
        case 4:
            if (stun_timer_p4) { stun_timer_p4--; p4_is_moving = 0; joy4 = 0; return; }
            curr_x = p4x; curr_y = p4y;
            break;
    }

    pers = player_pers[p_idx];

    if (ball_possession == p_idx) {
        joy_out = 0;

        // 1. Lógica de Chute (Sem passes, apenas finalização)
        // Todos ganham o mesmo tempo mínimo de condução antes de pensar em chutar.
        // Valores base unificados, mas você pode atrelar isso à 'pers' se quiser.
        if (possession_timer >= 120) { 
            // 25% de chance de chutar a cada verificação (mesma regra para todos)
        //    if ((rand() & 3) == 0) { 
            if ((rand() & 1) == 0) {
                possession_timer = 0; 
                last_touch = p_idx; 
                joy_out = 0x10; // FOGO! (Chute)
                goto apply_state;
            }
        }

        // 2. Definir o Alvo (Qualquer Goleira)
        // A IA vai escolher a goleira com base em onde ela está no campo agora,
        // garantindo que ela sempre ataque a área de perigo mais acessível.
        
        // Exemplo assumindo que o centro do campo é X=170, Y=138:
        if (pers == 1) {
            // Personalidade 1: Vai sempre buscar o centro da pequena área mais próxima
            tx = 170;
            ty = (curr_y < 138) ? 70 : 220; // Ataca a goleira de cima ou de baixo
        } 
        else if (pers == 2) {
            // Personalidade 2: Joga pelas pontas/laterais buscando o gol
            tx = (curr_x < 170) ? 60 : 280; // Ataca a goleira da esquerda ou direita
            ty = (curr_y < 138) ? 90 : 200;
        } 
        else {
            // Personalidade 3+: Pega a rota mais curta e direta baseada na posição atual
            if (curr_x < 115) { tx = 60; ty = 138; }       // Muito na esquerda, ataca gol esquerdo
            else if (curr_x > 225) { tx = 280; ty = 138; } // Muito na direita, ataca gol direito
            else { tx = 170; ty = (curr_y < 138) ? 70 : 220; } // Pelo meio, ataca cima/baixo
        }

        // Sinaliza movimento (pois ele está conduzindo a bola)
        if (p_idx == 1) p1_is_moving = 1;
        else if (p_idx == 2) p2_is_moving = 1;
        else if (p_idx == 3) p3_is_moving = 1;
        else p4_is_moving = 1;

    } 
    else { 
        // --- SEM POSSE DE BOLA: MODO CAÇADOR ---
        if (p_idx == 1) p1_is_moving = 1;
        else if (p_idx == 2) p2_is_moving = 1;
        else if (p_idx == 3) p3_is_moving = 1;
        else p4_is_moving = 1;

        // A perseguição agora é idêntica para todos os IDs.
        // A diferença na "inteligência" de interceptação vem SÓ da personalidade.
        // pers == 1 (Craque): lê 2 frames à frente da bola.
        // pers != 1 (Comum): lê apenas 1 frame à frente.
        antecip = (pers == 1) ? 2 : 1;
        
        tx = bx + (bdx << antecip);
        ty = by + (bdy << antecip);
        
        // Opcional: Se quiser que a pers==1 seja AINDA mais letal (como o antigo P3), 
        // adicione um delta extra na linha abaixo para ele "fechar" o ângulo:
        // if (pers == 1) { tx += bdx; ty += bdy; } 
    }

    // Lógica de Movimentação Unificada
    joy_out = 0;
    if ((int)curr_x < tx) { curr_x++; joy_out = 8; }
    else if ((int)curr_x > tx) { curr_x--; joy_out = 4; }
    
    if ((int)curr_y < ty) { curr_y++; joy_out |= 2; }
    else if ((int)curr_y > ty) { curr_y--; joy_out |= 1; }
    
    if (curr_x < 12) curr_x = 12; else if (curr_x > 334) curr_x = 334;
    if (curr_y < 28) curr_y = 28; else if (curr_y > 240) curr_y = 240;

apply_state:
    // 2. Escrita final do estado de volta nas globais apropriadas
    switch (p_idx) {
        case 1:
            px = curr_x; py = curr_y; joy2 = joy_out;
            break;
        case 2:
            p2x = curr_x; p2y = curr_y; joy1 = joy_out;
            break;
        case 3:
            p3x = curr_x; p3y = curr_y; joy3 = joy_out;
            break;
        case 4:
            p4x = curr_x; p4y = curr_y; joy4 = joy_out;
            break;
    }
}





static void reset_ball_to_corners(void) {
    unsigned char pos;
    init_random();
    pos = rand() % 5;

    // 1. POSICIONAMENTO DA BOLA
    switch(pos) {
        case 0: bx = 164; by = 144; break; // Centro            
        case 1: bx = 40;  by = 70;  break; // Canto Sup. Esq.
        case 2: bx = 310; by = 70;  break; // Canto Sup. Dir.
        case 3: bx = 40;  by = 220; break; // Canto Inf. Esq.
        case 4: bx = 310; by = 220; break; // Canto Inf. Dir.
    }

    // 2. TELETRANSPORTE DOS JOGADORES (Inversão por Tempo de Jogo)
    if (current_half == 2) {
        // --- SEGUNDO TEMPO: Lados Invertidos ---
        // Time A (P1 e P2) assume o lado Sul do mapa
        px = 120;  py = 160;  // P1 (Sul-Oeste)
        p2x = 200; p2y = 160; // P2 (Sul-Leste)
        
        // Time B (P3 e P4) assume o lado Norte do mapa
        p3x = 120; p3y = 120; // P3 (Norte-Oeste)
        p4x = 200; p4y = 120; // P4 (Norte-Leste)
        
        // CORREÇÃO: Coordenadas FÍSICAS dos goleiros permanecem estritas aos seus quadrantes!
        gx[0] = 176; gy[0] = 58;  // Slot do Norte continua fisicamente na goleira Norte (58)
        gx[1] = 176; gy[1] = 234; // Slot do Sul continua fisicamente na goleira Sul (234)
        
        dx = 160;   dy = 190;     // Zagueiro Norte (Time A) defende embaixo
        dx_b = 160; dy_b = 90;    // Zagueiro Sul (Time B) defende em cima
    } else {
        // --- PRIMEIRO TEMPO: Configuração Padrão ---
        px = 120;  py = 120;  // P1 (Norte-Oeste)
        p2x = 200; p2y = 120; // P2 (Norte-Leste)
        
        p3x = 120; p3y = 160; // P3 (Sul-Oeste)
        p4x = 200; p4y = 160; // P4 (Sul-Leste)
        
        gx[0] = 176; gy[0] = 58;  // Goleiro A (Norte)
        gx[1] = 176; gy[1] = 234; // Goleiro B (Sul)
        
        dx = 160;   dy = 90;      // Zagueiro Norte defende em cima
        dx_b = 160; dy_b = 190;   // Zagueiro Sul defende embaixo
    }

    // 3. RESET DE ESTADOS
    bdx = 0; 
    bdy = 0;
    ball_possession = 0;
   
    kick_power = 0;
    after_touch_timer = 0;
    kick_cooldown = 60; // Janela em frames para os sprites se estabilizarem
}

static void reset_game_state(void) {
    unsigned char j; // Declaração no topo

    // 1. Limpeza de Score e Timers Globais
    score = 0;
    timer = 0; 
    goal_delay = 0;
    possession_timer = 0;
    kick_cooldown = 60; 
	s1_final = 0; s2_final = 0; s3_final = 0; s4_final = 0;
//	tournament_stage = 0;

    // 2. Limpeza de Estados do Chute
    kick_power = 0;
    after_touch_timer = 0;
    last_touch = 0;
   
    defender_wait = 100; // Proteção da IA que adicionamos

    // 3. Reset dos Goleiros
    for (j = 0; j < 2; j++) {
        g_dir[j] = 1;
    }

    // 4. Posicionamento
    reset_ball_to_corners();
} // Chave de fechamento essencial aqui!


static void apply_boundaries(void) {
    // CACHE LOCAL: Avaliamos as condições de 16 bits uma única vez no topo.
    // O CC65 armazena estas flags de 8 bits em registradores/zeropage.
    unsigned char is_left = (bx <= 14 || bx > 60000U);
    unsigned char is_right = (bx >= 326 && bx < 60000U);
    unsigned char is_north = (by <= 56 || by > 60000U);
    unsigned char is_south = (by >= 232);
    unsigned char temp_abs;

    // --- TRATAMENTO ESPECÍFICO DE QUINAS (VÉRTICES) ---
    if (is_left && is_north) {
        bx = 16;          
        by = 58;
        bdx = 2;          
        bdy = 2;
        after_touch_timer = 0;
        return;
    }
    
    if (is_right && is_south) {
        bx = 324;          
        by = 230;
        bdx = -2;          
        bdy = -2;
        after_touch_timer = 0;
        return;
    }

    // --- LATERAIS (EIXO X) ---
    if (is_left) {
        if (by >= 115 && by <= 177) {
            if (bx <= 5) { bx = 5; bdx = 0; } 
        } else {
            bx = 14;
            if (bdx < 0) {
                bdx = -(bdx / 2);
                if (bdx == 0) bdx = 1;
            }
            after_touch_timer = 0;
        }
    }
    else if (is_right) {
        if (by >= 115 && by <= 177) {
            if (bx >= 341) { bx = 341; bdx = 0; } 
        } else {
            bx = 326;
            if (bdx > 0) {
                bdx = -(bdx / 2);
                if (bdx == 0) bdx = -1;
            }
            after_touch_timer = 0;
        }
    }

    // --- NORTE (Cima) ---
    if (is_north) {
        if (bx >= 132 && bx <= 184) {      
            if (by < 44) {
                by = 44; 
                if (goal_delay > 0) { bdx = 0; bdy = 0; }
            }
        } else {  
            by = 56; 
            // OTIMIZAÇÃO: Substituição de abs() e divisão por 3 por multiplicação rápida
            temp_abs = (bdy < 0) ? -bdy : bdy;
            bdy = (temp_abs * 86) >> 8; 
            if (bdy == 0) bdy = 1; 
            after_touch_timer = 0;
        }
    }
    
    // --- SUL (Baixo) ---
    else if (is_south) {
        if (bx >= 120 && bx <= 192) {     
            if (by > 250) { 
                by = 250; 
                if (goal_delay > 0) { bdx = 0; bdy = 0; }
            }
        } 
        else {  
            by = 232; 
            // OTIMIZAÇÃO: Equivalente para o Sul mantendo o sinal negativo
            temp_abs = (bdy < 0) ? -bdy : bdy;
            bdy = -((temp_abs * 86) >> 8);
            if (bdy == 0) bdy = -1; 
            after_touch_timer = 0;
        }
    }
}

static void __fastcall__  apply_net_stretch(unsigned char tx, unsigned char ty, unsigned char mode) {
    // Escolhe o tile central (se mode for 0, volta para NET)
    unsigned char t_center = (mode == 1) ? (CH_BASE + 13) : NET; // 13 = CCENTER

    // 1. GOLEIRAS NORTE OU SUL (ty é 0 ou 24)
    if (ty == 0 || ty == 24) {
        // Só deforma se estiver dentro dos 7 tiles da goleira (16 a 22)
        if (tx >= 15 && tx <= 23) {
            SCREEN[ty * 40 + tx] = t_center;
            // Deforma os vizinhos laterais se não forem as traves
            if (tx > 15) SCREEN[ty * 40 + tx - 1] = (mode == 1) ? CCTR : NET;
            if (tx < 23) SCREEN[ty * 40 + tx + 1] = (mode == 1) ? CCTL : NET;
        }
    }

}


static void perform_steal(unsigned char new_owner) {
    unsigned char old_owner = ball_possession; 
    unsigned char is_p1_stealing_from_ia = 0;
/*    
    // --- 1. VERIFICAÇÃO DE FOGO AMIGO (Simplificada para menos saltos de CPU) ---
    if ((game_mode | 1) == 7) { // Truque: cobre o modo 6 e 7 de uma só vez (6 | 1 = 7)
        if (new_owner <= 2) {
            if (old_owner <= 2 || old_owner == 5) return;
        } else if (new_owner <= 4) {
            if ((old_owner >= 3 && old_owner <= 4) || old_owner == 6) return;
        } else if (new_owner == 5 && old_owner <= 2) {
            return;
        } else if (new_owner == 6 && (old_owner == 3 || old_owner == 4)) {
            return;
        }
    }
*/
    // Otimização: Acesso direto ao array evitado se old_owner for 1
    if (new_owner == 1 && old_owner >= 2 && old_owner <= 4) {
        if (player_type[old_owner] == 0) {
            is_p1_stealing_from_ia = 1;
        }
    }

   // play_sfx_kick();      
   play_sfx_hit_human();
   
    ball_possession = 0;   
    possession_timer = 0;

    // --- 2. GERENCIAMENTO DE ATORDOAMENTO (STUN Otimizado) ---
    // Usamos operador ternário encadeado direto na atribuição para evitar blocos if-else
    if (old_owner == 1) {
        stun_timer_p1 = 30; 
    } else if (old_owner >= 2 && old_owner <= 4) {
        unsigned char stun_val = is_p1_stealing_from_ia ? 65 : 60;
        if (old_owner == 2) stun_timer_p2 = stun_val;
        else if (old_owner == 3) stun_timer_p3 = stun_val;
        else stun_timer_p4 = stun_val;
    }

   // --- 3. REAÇÃO FÍSICA DA BOLA ---
    // --- 3. REAÇÃO FÍSICA DA BOLA ---
    if (new_owner >= 5) { 
        unsigned char defends_north = (new_owner == 5) ? (current_half == 1) : (current_half == 2);
        
        kick_cooldown = 20; // Aumentei um pouco para evitar o ping-pong imediato
        
        // Força do chute (4 a 7)
        bdy = 3 + (rand() & 3); 
        
        // Se defende o Norte, DEVE chutar pro Sul (positivo).
        // Se defende o Sul, DEVE chutar pro Norte (negativo).
        if (!defends_north) {
            bdy = -(signed char)bdy; 
        }
        
        bdx = (rand() & 3) - 1; 
    }
    else if (is_p1_stealing_from_ia) {
        kick_cooldown = 15; 
        bdx = (px < bx) ? 6 : -6; 
        bdy = (py < by) ? 6 : -6;
    } 
    else {
        // Uso de variáveis locais qualificadas para o CC65 alocar em registradores
        static signed char spd;
        spd = (field_type == 1) ? 4 : 3;
        kick_cooldown = (field_type == 1) ? 25 : 20; 
        
        bdx = (rand() & 7) - 3;
        bdy = (rand() & 7) - 3;
        
        if ((unsigned char)(bdx + 1) <= 2) bdx = (rand() & 1) ? spd : -spd;
        if ((unsigned char)(bdy + 1) <= 2) bdy = (rand() & 1) ? spd : -spd;
    }

    bx += (bdx << 1); 
    by += (bdy << 1);
}

// MACRO para calcular diferença absoluta sem chamar a função pesada abs() da biblioteca C.
// Como usamos apenas para distâncias, a matemática não assinada resolve o problema e é compilada inline!
//#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

static void handle_ball_logic_v2(void) {
    // Transformar variáveis locais em static evita o uso pesado da "software stack" do CC65.
    static int last_dir_x[5] = {0, 1, -1, 1, -1};
    static int last_dir_y[5] = {0, 0, 0, 0, 0};
    
    static int bx_offset, by_offset;
    static unsigned char char_at_ball, tx, ty, j, rx, i;
    static unsigned int tx_p, ty_p; 
    static unsigned char joy_possession = 0; 
    static unsigned char p_idx_check, steal_range;
    static unsigned char quem_chutou, is_multi;
    
    // Novo contador estático para substituir o timer % 12 (módulo)
    static unsigned char friction_timer = 0;

    quem_chutou = ball_possession;
    is_multi = (game_mode == 5 || game_mode == 7 || game_mode == 6);

    // Otimização: Divisão substituída por bitshift (>> 3)
    tx = (bx >= 24) ? ((bx - 24) >> 3) : 0;
    ty = (by >= 50) ? ((by - 50) >> 3) : 0;

    init_crowd_voice();

    // --- DETECÇÃO DE GOL PRIORITÁRIA ---
    if (bx >= 104 && bx <= 214 && (by <= 52 || by >= 234)) {
        if (goal_delay == 0) {
            score++;
            goal_delay = 50;
            
            last_goal_tx = tx; 
            if (last_goal_tx < 13) last_goal_tx = 13;
            else if (last_goal_tx > 23) last_goal_tx = 23;
            
            last_goal_ty = (by <= 52) ? 0 : 24;
            apply_net_stretch(last_goal_tx, last_goal_ty, 1);
            play_goal_crowd();
            VIC.bordercolor = 1;
        }
        bdx = 0; bdy = 0; kick_power = 0;
        char_at_ball = NET; 
    } else {
        if (tx > 39) tx = 39;
        if (ty > 24) ty = 24;

        // Otimização: ty * 40 convertido para shifts: (ty * 32) + (ty * 8)
        char_at_ball = SCREEN[((unsigned int)ty << 5) + ((unsigned int)ty << 3) + tx];
        if (char_at_ball != NET) char_at_ball = 32; 
    }
    
    if (goal_delay > 0) {
        if (goal_delay == 50) { 
            for (i = 0; i < 7; i++) {
                // Otimização: 12 * 40 + 17 pré-calculado para 497
                goal_text_backup[i] = SCREEN[497 + i];
                goal_color_backup[i] = ((unsigned char*)0xD800)[497 + i];
            }
        }

        if (goal_delay > 20) { 
            play_goal_crowd(); 
            put_text(17, 12, (const unsigned char*)" GOAL! ");
            for(i = 0; i < 7; i++) ((unsigned char*)0xD800)[497 + i] = 1; 
        }
        else if (goal_delay == 20) { 
            for (i = 0; i < 7; i++) {
                SCREEN[497 + i] = goal_text_backup[i];
                ((unsigned char*)0xD800)[497 + i] = goal_color_backup[i];
            }
        }

        if (--goal_delay == 0) {
            apply_net_stretch(last_goal_tx, last_goal_ty, 0);
            reset_border_color(); 
            reset_ball_to_corners(); 
            ball_possession = 0; 
            after_touch_timer = 0;
            kick_cooldown = 40; 
        }
        return; 
    }

    if (char_at_ball == NET) {
        if (goal_delay == 0) {
            score++;
            play_goal_crowd();
            VIC.bordercolor = 1;
            goal_delay = 50;
            last_goal_tx = tx;
            last_goal_ty = ty;
            apply_net_stretch(tx, ty, 1); 
        }
        bdx = 0; bdy = 0; kick_power = 0; after_touch_timer = 0;
        return;
    }

    if (kick_cooldown > 0) kick_cooldown--;
    if (kick_cooldown_b > 0) kick_cooldown_b--;

    // ==============================================================
    // BOLA LIVRE (SEM POSSE)
    // ==============================================================
    if (ball_possession == 0) {
        if (after_touch_timer > 0) {
            after_touch_timer--;
            if (last_touch >= 2 && ai_curve_dir != 0) {
                // Otimização: bitwise AND substitui módulo (% 2)
                if ((after_touch_timer & 1) == 0) by += ai_curve_dir;
            }
            // Otimização: bitwise AND substitui % 4
            if (field_type == 1 && ((timer & 3) == 0)) return;
            
            joy_possession = joy_bits[last_touch - 1];
            
   if ((timer & 1) == 0) { // Pode manter o timer se preferir uma curva mais suave, porém larga
    // Note os limites alterados (16) e o incremento (+= 2 e -= 2)
    if (joy_possession & 1) { if (bdy > -16 && bdy <= 0) bdy -= 1; } 
    else if (joy_possession & 2) { if (bdy < 16 && bdy >= 0) bdy += 1; }
    
    if (joy_possession & 4) { if (bdx > -16 && bdx <= 0) bdx -= 1; } 
    else if (joy_possession & 8) { if (bdx < 16 && bdx >= 0) bdx += 1; }
}
        }
        
        if (by < 70 || by > 220 || bx < 40 || bx > 310) {
            if (bdx > 6) bdx = 6; else if (bdx < -6) bdx = -6;
            if (bdy > 6) bdy = 6; else if (bdy < -6) bdy = -6;
        }

        bx += bdx; by += bdy;
                
        apply_boundaries(); 
        
        // Otimização: Modulo de atrito (% 12 ou % 30) substituído por um contador estático
        friction_timer++;
        if (friction_timer >= (field_type == 2 ? 30 : 12)) {
            friction_timer = 0;
            if (bdx > 0) bdx--; else if (bdx < 0) bdx++;
            if (bdy > 0) bdy--; else if (bdy < 0) bdy++;
        }
        
        if (bdx == 0 && bdy == 0 && goal_delay == 0) {
            if (bx < 24) { bx = 25; bdx = 1; } 
            else if (bx > 318) { bx = 317; bdx = -1; }
            
            if (by < 56) { by = 57; bdy = 1; } 
            else if (by > 222) { by = 221; bdy = -1; }
        }

        if (kick_cooldown == 0 && goal_delay == 0) {
            // INTERCEPTAÇÃO COM BOOST
            for (i = 1; i <= 4; ++i) {
                if (player_type[i] == 1 && boost_timer[i] > 0) {
                    // Otimização: Trocar ternários encadeados por um switch rápido (gera jump table)
                    switch(i) {
                        case 1: tx_p = px; ty_p = py; break;
                        case 2: tx_p = p2x; ty_p = p2y; break;
                        case 3: tx_p = p3x; ty_p = p3y; break;
                        case 4: tx_p = p4x; ty_p = p4y; break;
                    }

                    // Otimização: Uso do ABS_DIFF elimina a chamada de função abs()
                    if (ABS_DIFF(tx_p, bx) < 14 && ABS_DIFF(ty_p, by) < 14) {
                        bdx = (bx > tx_p) ? 5 : -5;
                        bdy = (by > ty_p) ? 5 : -5;
                        after_touch_timer = 0; ai_curve_dir = 0; boost_timer[i] = 0; 
                        play_sfx_defend(); 
                        bx += (bdx << 1); by += (bdy << 1); // << 1 é mais rápido que * 2
                        kick_cooldown = 10;
                        return; 
                    }
                }
            }
            
            // COLISÃO COM GOLEIROS
            for (j = 0; j < 4; j++) {
                if (is_multi) {
                    if (j == 0 && by > 144) continue; 
                    if (j == 1 && by < 144) continue; 
                }
                rx = (j == 3) ? 18 : 16;
                // Otimização: Remoção das chamadas abs() pesadas
                if (ABS_DIFF(gx[j], bx) < rx && ABS_DIFF(gy[j], by) < 16) {
                    play_sfx_defend();
                    if (j == 3) { bx = gx[3] - 16; bdx = -4; bdy = (rand() % 5) - 2; } 
                    else if (j == 2) { bx = gx[2] + 16; bdx = 4; bdy = (rand() % 5) - 2; } 
                    else if (j == 0) { by = gy[0] + 16; bdy = 4; bdx = (rand() % 5) - 2; } 
                    else if (j == 1) { by = gy[1] - 16; bdy = -4; bdx = (rand() % 5) - 2; }
                    return; 
                }
            }

            // CAPTURA DE BOLA PELOS JOGADORES (Sem abs!)
            if (ABS_DIFF(px, bx) < 16 && ABS_DIFF(py, by) < 16) {
                ball_possession = 1; last_touch = 1;
            } 
            else if (is_multi && ABS_DIFF(p2x, bx) < 16 && ABS_DIFF(p2y, by) < 16) {
                ball_possession = 2; last_touch = 2;
            }
            else if (is_multi) {
                if (ABS_DIFF(p3x, bx) < 16 && ABS_DIFF(p3y, by) < 16) {
                    ball_possession = 3; last_touch = 3;
                }
                else if (ABS_DIFF(p4x, bx) < 16 && ABS_DIFF(p4y, by) < 16) {
                    ball_possession = 4; last_touch = 4;
                }
            }
            
            if (ball_possession != 0) {    
                possession_timer = 120;
                bdx = 0; bdy = 0; after_touch_timer = 0;
                kick_power = 0; 
                
                if (last_dir_x[ball_possession] == 0 && last_dir_y[ball_possession] == 0) {
                    last_dir_x[ball_possession] = (ball_possession == 1 || ball_possession == 3) ? 1 : -1;
                    last_dir_y[ball_possession] = 0;
                }

                if (player_type[ball_possession] == 1) {
                    has_boost[ball_possession] = 1;
                }
            }
        }
    } 
    // ==============================================================
    // JOGADOR COM POSSE DE BOLA
    // ==============================================================
    else { 
        // Otimização: Acesso direto ao array ao invés de cascata de if/else
        joy_possession = joy_bits[ball_possession - 1]; 
        
        if ((joy_possession & 15) != 0) {
            last_dir_x[ball_possession] = 0;
            last_dir_y[ball_possession] = 0;

            if (joy_possession & 1) last_dir_y[ball_possession] = -1;
            if (joy_possession & 2) last_dir_y[ball_possession] = 1; 
            if (joy_possession & 4) last_dir_x[ball_possession] = -1;
            if (joy_possession & 8) last_dir_x[ball_possession] = 1; 
            
            if (ball_possession == 1) {
                px += last_dir_x[1]; py += last_dir_y[1];
                if (px < 15) px = 15; else if (px > 325) px = 325;
                if (py < 45) py = 45; else if (py > 235) py = 235;
            }
            else if (ball_possession == 2) {
                p2x += last_dir_x[2]; p2y += last_dir_y[2];
                if (p2x < 15) p2x = 15; else if (p2x > 325) p2x = 325;
                if (p2y < 45) p2y = 45; else if (p2y > 235) p2y = 235;
            }
            else if (ball_possession == 3) {
                p3x += last_dir_x[3]; p3y += last_dir_y[3];
            }
            else if (ball_possession == 4) {
                p4x += last_dir_x[4]; p4y += last_dir_y[4];
            }
        }

        bx_offset = (last_dir_x[ball_possession] * 10) + ((possession_timer & 1) * last_dir_x[ball_possession]);
        by_offset = 12 + (last_dir_y[ball_possession] * 7);

        if (ball_possession == 1) { bx = px + bx_offset; by = py + by_offset; }
        else if (ball_possession == 2) { bx = p2x + bx_offset; by = p2y + by_offset; }
        else if (ball_possession == 3) { bx = p3x + bx_offset; by = p3y + by_offset; }
        else if (ball_possession == 4) { bx = p4x + bx_offset; by = p4y + by_offset; }
        
        if (is_multi) {
            VIC.spr_pos[3].x = bx & 255; 
            VIC.spr_pos[3].y = by;
        } else {
            VIC.spr_pos[5].x = bx & 255; 
            VIC.spr_pos[5].y = by;
        }

        if (kick_cooldown == 0) {
            if (is_multi) {    
                for(j=0; j<4; j++) {
                    p_idx_check = j + 1; 
                    if (p_idx_check == ball_possession) continue;
                    
                    switch(j) {
                        case 0: tx_p = px; ty_p = py; break;
                        case 1: tx_p = p2x; ty_p = p2y; break;
                        case 2: tx_p = p3x; ty_p = p3y; break;
                        case 3: tx_p = p4x; ty_p = p4y; break;
                    }
                    
                    steal_range = (player_type[p_idx_check] == 0) ? 8 : 12;
                    
                    if ((unsigned char)(tx_p - bx + steal_range) < (steal_range << 1) && 
                        (unsigned char)(ty_p - by + steal_range) < (steal_range << 1)) { 
                        
                        perform_steal(p_idx_check); 
                        
                        if (player_type[p_idx_check] == 1) { 
                            kick_cooldown = 15; possession_timer = 120; 
                        } else { 
                            kick_cooldown = 40; possession_timer = 100; 
                        }
                        break; 
                    }
                }

                if (use_defender==1) {
                    if (ball_possession != 0) {
                        if (ball_possession != 5 && ABS_DIFF(dx, bx) < 12 && ABS_DIFF(dy, by) < 12) perform_steal(5);
                        if (ball_possession != 6 && ABS_DIFF(dx_b, bx) < 12 && ABS_DIFF(dy_b, by) < 12) perform_steal(6);
                    }
                }
            }
        }
        
        if (possession_timer > 0) possession_timer--;
        
        if (ball_possession != 0 && is_multi && player_type[ball_possession] == 0 && possession_timer <= 75) {
            kick_power = 1; 
        }
        
        if (joy_possession & 0x10) {  
            if ((possession_timer & 3) == 0 && kick_power < 9) kick_power++;
            if (possession_timer == 0) kick_power = 1; 
        }
        else if (kick_power > 0 && ball_possession != 0) {
            quem_chutou = ball_possession;
            is_pass = 0; 
            
            if (is_multi && player_type[quem_chutou] == 0) {
                apply_smart_kick2(quem_chutou);
                bx += (bdx << 1); by += (bdy << 1);
                ball_possession = 0; 
                if (!is_pass) { kick_cooldown = 12; after_touch_timer = 20; }
                kick_power = 0;
            } else {
                bdx = last_dir_x[quem_chutou];
                bdy = last_dir_y[quem_chutou];
                
                if (bdx == 0 && bdy == 0) bdx = (quem_chutou == 1 || quem_chutou == 3) ? 1 : -1;
                if (kick_power < 3) kick_power = 3;
                
                bdx *= (int)kick_power; 
                bdy *= (int)kick_power;
                
                ball_possession = 0;
                kick_cooldown = 10;
                kick_power = 0;
                after_touch_timer = 20;
            }

            if (is_multi && player_type[quem_chutou] == 0 && (bdx > 1 || bdx < -1)) {
                if (bdx > 0) bdx += 2; else bdx -= 2;
            }
            
            if (!is_pass) {
                if (bdx > 0 && bx < 320) bx += 8;  
                else if (bdx < 0 && bx > 15) bx -= 8;

                if (bdy > 0 && by < 230) by += 8;
                else if (bdy < 0 && by > 55) by -= 8;
                
                kick_cooldown = 10;
            }
            
            play_sfx_kick(); 
            kick_power = 0; 
  //          spr_active_defender = use_defender;
        }
    }
     
    update_ball_animation();
    
    if (is_multi) {
        VIC.spr_pos[3].x = bx & 255; 
        VIC.spr_pos[3].y = by;
        if (bx > 255) VIC.spr_hi_x |= 0x08; else VIC.spr_hi_x &= ~0x08;
    } else {
        VIC.spr_pos[5].x = bx & 255; 
        VIC.spr_pos[5].y = by;
        if (bx > 255) VIC.spr_hi_x |= 0x20; else VIC.spr_hi_x &= ~0x20;
    }
}

static void clear_screen_c000_with_color(unsigned char color) {
    memset((void*)0xC000, 32, 1000);
    memset((void*)0xD800, color, 1000);
}



static void move_goleiros_p2(void) {
    signed char dx = 0;

    if (joy1 & 4) dx = -2; // Esquerda
    if (joy1 & 8) dx = 2;  // Direita

    gx[0] += dx; 
    gx[1] += dx;

    gy[0] = 58;   // Norte
    gy[1] = 232;  // Sul

    // [ALTERADO] TRAVAS DE BORDA (Limites do gol aumentados: 136-8=128 e 200+8=208)
    if (gx[0] < 128) gx[0] = 128; else if (gx[0] > 208) gx[0] = 208;
    if (gx[1] < 128) gx[1] = 128; else if (gx[1] > 208) gx[1] = 208;
    
    if (keeper_recovery[0] > 0) keeper_recovery[0]--;
    if (keeper_recovery[1] > 0) keeper_recovery[1]--;

    for (i = 0; i < 2; i++) {
        if (keeper_recovery[i] > 0) continue;
        
        if (abs(gx[i] - bx) < 18 && abs(gy[i] - by) < 16) {
            if (bx < 32 || bx > 328 || by < 52 || by > 240) continue;

            if (last_keeper != -1 && last_keeper != i) {
                inter_keeper_passes[last_keeper][i]++;
                inter_keeper_passes[i][last_keeper]++;
            }

            if (last_keeper != -1 && inter_keeper_passes[last_keeper][i] >= 2) {
                inter_keeper_passes[last_keeper][i] = 0;
                inter_keeper_passes[i][last_keeper] = 0;
                
                bdx = (bx > 160) ? 12 : -12; 
                bdy = (by > 144) ? 12 : -12;
            } else {
                if (joy1 & 4) bdx = -7; else if (joy1 & 8) bdx = 7; else bdx = (bx > gx[i]) ? 6 : -6;
                if (joy1 & 1) bdy = -7; else if (joy1 & 2) bdy = 7; else bdy = (by > gy[i]) ? 6 : -6;
            }

            last_keeper = i; 

            bx += bdx;
            by += bdy;
            
            if (bx < gx[i]) bx -= 8; else bx += 8;
            if (by < gy[i]) by -= 8; else by += 8;

            keeper_recovery[i] = 15; 
            deflect_cooldown = 15;   

            if (ball_possession != 2) {
                ball_possession = 0; 
                play_sfx_defend();
                saves_p2++;
            }
            kick_cooldown = 30; 
            break; 
        }
    }
}



static void bounce_ball_at_walls(void) {
    // 1. Paredes Norte e Sul (Unificadas para economizar ROM e ciclos)
    if (by < 52 || by > 228) {  
        if (bx < 128 || bx > 224) { 
            bdy = -bdy; 
            by = (by < 52) ? 52 : 228; 
        }  
    }

    // 2. Laterais Oeste e Leste (Sem vãos, rebote direto na parede)
    if (bx < 26) {  
        bx = 26; 
        bdx = -bdx;  
    }
    else if (bx > 318) {
        bx = 318; 
        bdx = -bdx;  
    }
}



static void play_match(void) {
    unsigned char last_touch = 0; 
    unsigned char old_score;
   // unsigned char temp_hi_x;
   // Encontra quem fez mais gols
        unsigned char max_g = 0;
        unsigned char win_p = 0;
        unsigned char tie = 0;
//		unsigned char ia_frame_toggle = 0; // Para intercalar processamento de IA
   
   unsigned char char_da_linha = 64; // Exemplo: ajuste para o seu código
 //   unsigned char moldura_rep[7];
unsigned char spd,k; // Variável temporária para velocidade
unsigned char gs_text_backup[13]; 
unsigned char gs_color_backup[13];
// Calcula o total de cada time
unsigned char gols_time_a = 0;
unsigned char gols_time_b = 0;
//static unsigned char screen_backup[35];
//static unsigned char color_backup[35];
unsigned char spr_mask;

	
	
	init_whistle_voice(); 
	
	
	VIC.addr = 0x10;
    
    // --- 1. RESET DE VARIÁVEIS ---
    score = 0; s1_final = 0; s2_final = 0; s3_final = 0; s4_final = 0;
    goal_delay = 0; // CRUCIAL para a IA não começar travada
    kick_cooldown = 0;
    ball_possession = 0;
	difficulty_boost = 0; // Reset do boost no início
	saves_p2 = 0;
    keeper_score = 0;
	shot_delay_p1 = 0;
	shot_delay_p2 = 0;
    shot_delay_p3 = 0;
    shot_delay_p4 = 0;
	possession_timer = 0;
	is_overtime = 0;
    golden_score_triggered =0;
	// Calcula o total de cada time
    gols_time_a = 0;
    gols_time_b = 0;
		ot_base1 = 0; ot_base2 = 0; ot_base3 = 0; ot_base4 = 0;
	e1 = 0; e2 = 0; e3 = 0; e4 = 0;


// Desativa a varredura automática de teclado do sistema operacional
*((unsigned char*)0x0289) = 0;


// --- RESET DE BOOST (EXCLUSIVO MODO 5) ---
if (game_mode == 5  || game_mode == 6 || game_mode == 7 ) {
    for (i = 0; i < 5; i++) {
        has_boost[i] = 1;
        boost_timer[i] = 0;
    }
}
	

	// --- RESET DE JOYSTICK PARA EVITAR CHUTE FANTASMA ---
    joy_now = 0;
    last_joy_state = 0xff; // Força um estado neutro inicial
	
	dx = 100; dy = 140; // Posição inicial do zagueiro
    d_level = rand() % 3;
	
	
	
	// 2. Espera ativa: Só começa o jogo quando o botão FIRE for solto
    // Isso evita que o clique no menu vire um chute no jogo
    while ((~CIA1.pra & 0x10) || (~CIA1.prb & 0x10)) {
        wait_vblank(); // Aguarda o jogador tirar o dedo do botão
    }
	
	// 3. Delay de estabilização (aprox. 0.5 segundos)
    for(i = 0; i < 30; i++) wait_vblank();
	


spr_active_p1 = 1;
spr_active_ball = 1;
//spr_active_p2 = (game_mode == 2 || game_mode == 4);
//spr_active_defender = use_defender;

  

	// 1. FORÇAR HABILITAÇÃO
 //   VIC.spr_ena |= 0x80; // Liga TODOS os 8 sprites
	
	// --- NOVO SORTEIO DE DIFICULDADE BASEADO NA ESCOLHA ---
	for (i = 0; i < 4; i++) {
		if (difficulty_level == 0)      g_level[i] = rand() % 2; // Easy: 0 ou 1
		else if (difficulty_level == 2) g_level[i] = 2 + (rand() % 2); // Hard: 2 ou 3
		else                            g_level[i] = rand() % 4; // Normal: 0 a 3
	}
   
	// --- Altere a configuração inicial do Timer para incluir o reset do current_half ---
    current_half = 1; // Garante que começa sempre no 1º tempo

    // Configura o Timer
    if ((game_mode == 5 && gold_goal_active == 1) || (game_mode == 7 && gold_goal_active == 1) || (game_mode == 6 && gold_goal_active == 1)) timer = 9999; 
    else timer = 2250; 

    // Coordenadas padrão de jogabilidade
    px = 120; py = 120; 
    p2x = 200; p2y = 120; 
    p3x = 120; p3y = 160; 
    p4x = 200; p4y = 160; 
    bx = 164; by = 144;
    gx[0] = 176; gy[0] = 58; 
    gx[1] = 176; gy[1] = 234; 
	dx = 160; dy = 90;
	dx_b = 160; dy_b = 190;
 
// ---------------- 1. PONTEIROS DE SPRITES (MODO 5) ----------------
    // IMPORTANTE: O valor no ponteiro é Endereço_Real / 64
    SPR_ZAG[0] = 192; // P1 ($F000)
    SPR_ZAG[1] = 18;  // Zagueiro Sul Parado ($C480)
    SPR_ZAG[2] = 196; // P3 ($F000)
    SPR_ZAG[3] = 246; // Bola ($FD80)
    SPR_ZAG[4] = 18;  // Zagueiro Norte Parado ($C480)
    SPR_ZAG[5] = 16;  // Goleiro Inicial Norte ($C400) - Raster assume depois

    // Controle do P2 / Goleiro Fantasma
    if (player_type[2] == 2 || player_type[2] == 255) {
        SPR_ZAG[6] = 0;        // Zera o ponteiro
        spr_active_p2 = 0;
        VIC.spr_pos[6].x = 0;
        VIC.spr_pos[6].y = 0;
        p2x = 0; p2y = 0;      // Evita colisões invisíveis
    } else {
        SPR_ZAG[6] = 194;      // P2 ($F080)
        spr_active_p2 = 1;
    }
    
    SPR_ZAG[7] = 198; // P4 ($F080)

    // ---------------- 2. MÁSCARA DE SPRITES E FANTASMAS ----------------
    // Base do Modo 5: P1(S0), Zag Sul(S1), Bola(S3), Zag Norte(S4), Goleiro(S5)
    // Binário: 0011 1011 -> 0x3B
    spr_mask = 0x3B; 

    if (spr_active_p2) spr_mask |= 0x40; // Liga P2 (Slot 6)

    // Proteção extra: liga apenas se tiver jogadores
    if (num_players >= 3) {
        spr_mask |= 0x04; // Liga P3 (Slot 2)
    } else {
        SPR_ZAG[2] = 0;
        VIC.spr_pos[2].x = 0; VIC.spr_pos[2].y = 0;
    }

    if (num_players == 4) {
        spr_mask |= 0x80; // Liga P4 (Slot 7)
    } else {
        SPR_ZAG[7] = 0;
        VIC.spr_pos[7].x = 0; VIC.spr_pos[7].y = 0;
    }

    VIC.spr_ena = spr_mask;

    // Posição inicial dos Zagueiros
    VIC.spr_pos[1].x = 160; VIC.spr_pos[1].y = 190;
    VIC.spr_pos[4].x = 160; VIC.spr_pos[4].y = 90;

    // ---------------- 3. CORES GERAIS DOS SPRITES ----------------
    VIC.spr_color[0] = get_fixed_player_color(1);  // P1 Amarelo
    VIC.spr_color[2] = get_fixed_player_color(3); // P3
    VIC.spr_color[6] = get_fixed_player_color(2);  // P2 Vermelho
    VIC.spr_color[7] = get_fixed_player_color(4); // P4
 //   VIC.spr_color[1] = 1;  // Zagueiro Sul (Púrpura)
 //   VIC.spr_color[4] = 4;  // Zagueiro Norte (Púrpura)
 //   VIC.spr_color[5] = 5;  // Goleiro Vertical (Verde)

    // ---------------- 4. CONFIGURAÇÃO DE CAMPO E CLIMA ----------------
    if (selected_field_option == 3) {
        field_type = rand() % 3; // Sorteia entre 0, 1 e 2
    } else {
        field_type = selected_field_option;
    }
    
    // Sorteio do Estilo do Campo
    if (field_style == 3) {
        field_style = rand() % 3; // Checker, Bars, Solid
    }

    if (game_mode == 6) {
        unsigned char r = rand() % 10;
        
        if (r < 6)      field_type = 0; // 60% de chance (Grass)
        else if (r < 8) field_type = 1; // 20% de chance (Wet)
        else            field_type = 2; // 20% de chance (Frozen)

        field_style = rand() % 3;
        selected_field_option = field_type;
    }
	
    draw_field_manager();

    // Aplicação das cores do Clima
    if (field_type == 1) {        // MOLHADO
        VIC.bgcolor0 = 6;         // Azul escuro
        VIC.bordercolor = 14;     // Azul claro
        VIC.spr_color[3] = 1;     // Bola
        VIC.spr_color[1] = 5;     // Zagueiro muda de cor na chuva
		    VIC.spr_color[4] = 13;     // Zagueiro muda de cor na chuva
	 VIC.spr_color[1] = 5;
    } else if (field_type == 2) { // NEVE
        VIC.bgcolor0 = 12;        // Cinza médio
        VIC.bordercolor = 15;     // Cinza claro
    //    VIC.spr_color[7] = 4;     // Preto
        VIC.spr_color[3] = 7;     // Bola
        VIC.spr_color[1] = 5;
		    VIC.spr_color[4] = 13;     // Zagueiro muda de cor na chuva
    } else {                      // GRAMA
        VIC.bgcolor0 = 5;         // Verde
        VIC.bordercolor = 13;     // Verde claro
    //    VIC.spr_color[7] = 4;     // Preto
        VIC.spr_color[3] = 1;     // Bola
        VIC.spr_color[1] = 14;
		VIC.spr_color[4] = 6;
    }

    // ---------------- 5. EXIBIÇÃO DO MODO GOLDEN SCORE ----------------
    if (gold_goal_active == 1) {
        // Coluna 1, Linha 0: Canto superior esquerdo
        put_text(36, 0, (const unsigned char*)"GS"); 
    }

    // ---------------- 6. PREPARAÇÃO FINAL DA PARTIDA ----------------
    dx = 160; // Centro do campo (X)
    dy = 140; // Centro do campo (Y)
    play_sfx_whistle();

    // ---------------- 7. CONFIGURAÇÃO DA INTERRUPÇÃO DE RASTER ----------------
    asm("sei"); 
    *((unsigned int*)0x0314) = (unsigned int)interrupt_raster_handler;
    VIC.imr = 0x01;      
    VIC.ctrl1 &= 0x7F;   
    VIC.rasterline = 30; 
    asm("cli");

// --- DESENHO ESTÁTICO DA UI (FORA DO LOOP) ---
if (game_mode == 5  || game_mode == 6 || game_mode == 7) {
    if (num_players >= 3) put_text(5, 0, (const unsigned char*)"P3:");
    if (num_players == 4) put_text(28, 0, (const unsigned char*)"P4:");
    put_text(5, 24, (const unsigned char*)"P1:");
	if (player_type[2] == 2) {
		if (num_players >= 2) put_text(28, 24, (const unsigned char*)"P2:GK");
	} else {
		if (num_players >= 2) put_text(28, 24, (const unsigned char*)"P2:");
	}
  // --- NOVO: CORES DOS JOGADORES NO MODO 5 ---
    if ((game_mode == 5 || game_mode == 6 || game_mode == 7)) {
        unsigned char c_temp;
        
        // P1: (linha 24, coluna 5, 3 caracteres)
        c_temp = get_fixed_player_color(1); 
        ((unsigned char*)0xD800)[24 * 40 + 5] = c_temp;
        ((unsigned char*)0xD800)[24 * 40 + 6] = c_temp;
        ((unsigned char*)0xD800)[24 * 40 + 7] = c_temp;

        if (num_players >= 2) {
            // P2: (linha 24, coluna 28, 3 ou 5 caracteres)
            c_temp = get_fixed_player_color(2);
			
            ((unsigned char*)0xD800)[24 * 40 + 28] = c_temp;
            ((unsigned char*)0xD800)[24 * 40 + 29] = c_temp;
            ((unsigned char*)0xD800)[24 * 40 + 30] = c_temp;
            
  //          // Se for P2:GK, colore os dois caracteres a mais ("GK")
            if (player_type[2] == 2 && field_type == 1) { 
                ((unsigned char*)0xD800)[24 * 40 + 31] = 5;
                ((unsigned char*)0xD800)[24 * 40 + 32] = 5;
            }
			else {
				 ((unsigned char*)0xD800)[24 * 40 + 31] = 4;
                ((unsigned char*)0xD800)[24 * 40 + 32] = 4;
			}
        }

        if (num_players >= 3) {
            // P3: (linha 0, coluna 5, 3 caracteres)
            c_temp = get_fixed_player_color(3);
            ((unsigned char*)0xD800)[0 * 40 + 5] = c_temp;
            ((unsigned char*)0xD800)[0 * 40 + 6] = c_temp;
            ((unsigned char*)0xD800)[0 * 40 + 7] = c_temp;
        }

        if (num_players == 4) {
            // P4: (linha 0, coluna 28, 3 caracteres)
            c_temp = get_fixed_player_color(4);
            ((unsigned char*)0xD800)[0 * 40 + 28] = c_temp;
            ((unsigned char*)0xD800)[0 * 40 + 29] = c_temp;
            ((unsigned char*)0xD800)[0 * 40 + 30] = c_temp;
        }
    }  
   	
} 

// Desenha o "T:" do tempo uma única vez
if (gold_goal_active == 0) put_text(35, 0, (const unsigned char*)"T:");
if (game_mode == 6) {
    put_text(35, 24, (const unsigned char*)"S");
    put_num(36, 24,tournament_stage + 1);
}
if (is_demo_mode) {

			put_text(18, 11, (const unsigned char*)"DEMO"); 
			
//			put_text(5, 13, (const unsigned char*)team_names[home_team]);
//			put_text(19, 13, (const unsigned char*)"VS");
//			put_text(23, 13, (const unsigned char*)team_names[away_team]);
		}

update_sprites();

	if (num_players >= 2 && player_type[2] != 2) {
    VIC.spr_pos[6].x = p2x & 0xFF; VIC.spr_pos[6].y = p2y;
} else {
    VIC.spr_pos[6].y = 0; // Esconde
}
 
 

// Loop principal

    while(1) {
	
		
        while(VIC.rasterline != 250); // Sincroniza com o frame
// Teste
//		VIC.bordercolor = 1;
		sys_tick++;
		

        // TESTE FORÇADO NO LOOP
 //   *((unsigned char*)0xd01c) = 0xF7;
 
 // --- BLOCO MODO DEMO (ADICIONAR AQUI) ---
        if (is_demo_mode) {
            demo_frame_counter++;
            
            // 1. Condição de saída por tempo (30 segundos = 1500 a 1800 frames dependendo da região)
           // if (demo_frame_counter > 1800) break; 
            if (demo_frame_counter > 3600) break;

            if (kbhit()) {
                k = cgetc();
            // SÓ ABORTA se a tecla NÃO for 1, 2 ou 3
            if (k < '1' || k > '3') {
               break; 
             }
        // Se for 1, 2 ou 3, o cgetc() acima já limpou o buffer, 
        // então o jogo continua normalmente.
           }

        // Saída pelo botão de fogo (Joy 2)
        if (!(CIA1.pra & 0x10)) {
           break; 
        }
      }
//VIC.bordercolor = 0;
		read_joysticks();
//VIC.bordercolor = 1;		

		update_sfx();

   
  
   
//	VIC.bordercolor = 1;
	// Boost Timer sem loop 'for' (Otimização extra)
    if (boost_timer[1] > 0) boost_timer[1]--;
    if (boost_timer[2] > 0) boost_timer[2]--;
    if (boost_timer[3] > 0) boost_timer[3]--;
    if (boost_timer[4] > 0) boost_timer[4]--;

        old_score = score;
		ia_frame_toggle++; // Incrementa a cada frame (0-255)


 #define MOVE_HUMAN(joy, px, py, p_id) \
    if (ball_possession != p_id) { \
        spd = (boost_timer[p_id] > 0) ? 3 : 2; \
        if (joy & 1) py -= spd; \
        if (joy & 2) py += spd; \
        if (joy & 4) px -= spd; \
        if (joy & 8) px += spd; \
    }  


// P2
if (player_type[2] != 255 && player_type[2] != 2) {
    if (player_type[2] == 1) {
        MOVE_HUMAN(joy1, p2x, p2y, 2);
    } else if (player_type[2] == 0) {
        move_player_ia_modo5(2);
    }
} else {
    p2x = 0; p2y = 0;
}

// --- 1. MOVIMENTAÇÃO DOS JOGADORES ---
if (player_type[1] == 1) {
    MOVE_HUMAN(joy2, px, py, 1);
} else if (player_type[1] == 0) {
     move_player_ia_modo5(1);
}



// P3
if (player_type[3] != 255) {
    if (player_type[3] == 1) {
        MOVE_HUMAN(joy3, p3x, p3y, 3);
    } else if (player_type[3] == 0) {
        move_player_ia_modo5(3);
    }
} else {
    p3x = 0; p3y = 0;
}

// P4
if (player_type[4] != 255) {
    if (player_type[4] == 1) {
        MOVE_HUMAN(joy4, p4x, p4y, 4);
    } else if (player_type[4] == 0) {
        move_player_ia_modo5(4);
    }
} else {
    p4x = 0; p4y = 0;
}

       if (use_defender==1) {
  //       move_defender_south_ia();
  //      move_defender_north_ia();
		move_defender_ia(0); // Zagueiro Norte
        move_defender_ia(1); // Zagueiro Sul
   }

separate_all_players();

// --- LIMITES FÍSICOS ---
KEEP_IN_BOUNDS(px, py);
if (player_type[2] != 255 && player_type[2] != 2) { KEEP_IN_BOUNDS(p2x, p2y); }
if (player_type[3] != 255) { KEEP_IN_BOUNDS(p3x, p3y); }
if (player_type[4] != 255) { KEEP_IN_BOUNDS(p4x, p4y); }

if (ball_possession != 0) last_touch = ball_possession;

update_players_anim();
handle_ball_logic_v2();
bounce_ball_at_walls();

// --- ATUALIZAÇÃO DE COORDENADAS DOS SPRITES ---
VIC.spr_pos[0].x = (unsigned char)px; VIC.spr_pos[0].y = py;
VIC.spr_pos[3].x = (unsigned char)bx; VIC.spr_pos[3].y = by;

// P2 (Slot 6)
if (player_type[2] == 255 || player_type[2] == 2) {
    VIC.spr_pos[6].x = 0; VIC.spr_pos[6].y = 0;
} else {
    VIC.spr_pos[6].x = (unsigned char)p2x; VIC.spr_pos[6].y = p2y;
}

// P3 (Slot 2)
if (player_type[3] != 255) {
    VIC.spr_pos[2].x = (unsigned char)p3x; VIC.spr_pos[2].y = p3y;
} else {
    VIC.spr_pos[2].x = 0; VIC.spr_pos[2].y = 0;
}

// P4 (Slot 7)
if (player_type[4] != 255) {
    VIC.spr_pos[7].x = (unsigned char)p4x; VIC.spr_pos[7].y = p4y;
} else {
    VIC.spr_pos[7].x = 0; VIC.spr_pos[7].y = 0;
}

// Zagueiros
if (use_defender == 1) {
    VIC.spr_pos[1].x = (unsigned char)dx_b; VIC.spr_pos[1].y = dy_b;
    VIC.spr_pos[4].x = (unsigned char)dx;   VIC.spr_pos[4].y = dy;
} else {
    VIC.spr_pos[1].x = 0; VIC.spr_pos[1].y = 0;
    VIC.spr_pos[4].x = 0; VIC.spr_pos[4].y = 0;
}

// --- ATUALIZAÇÃO DO SPR_HI_X ---
spr_hix_mask = 0;



if (px & 0x100) spr_hix_mask |= 0x01; // P1 (Slot 0)
if (bx & 0x100) spr_hix_mask |= 0x08; // Bola (Slot 3)

if (player_type[2] != 255 && player_type[2] != 2) {
    if (p2x & 0x100) spr_hix_mask |= 0x40; // P2 (Slot 6)
}
if (player_type[3] != 255) {
    if (p3x & 0x100) spr_hix_mask |= 0x04; // P3 (Slot 2)
}
if (player_type[4] != 255) {
    if (p4x & 0x100) spr_hix_mask |= 0x80; // P4 (Slot 7)
}

if (use_defender == 1) {
    if (dx_b & 0x100) spr_hix_mask |= 0x02; // Zagueiro Sul (Slot 1)
    if (dx   & 0x100) spr_hix_mask |= 0x10; // Zagueiro Norte (Slot 4)
}

VIC.spr_hi_x = spr_hix_mask;
 

 // Atualização de placar
    if (score > old_score) {
        if (game_mode == 8 ) {
            
            // Checa se o gol foi na metade superior (Norte) ou inferior (Sul)
            if (by < 100) {
                // -------------------------------------------------------
                // GOL NA GOLEIRA NORTE 
                // -------------------------------------------------------
                if (current_half == 1) {
                    // 1º TEMPO: Alvo do TIME B (Gols vão para s3 ou s4)
                    if (last_touch == 1 || last_touch == 2 || last_touch == 5) { s3_final++; } // Contra
                    else {
                        if (last_touch == 3) s3_final++;
                        else if (last_touch == 4) s4_final++;
                    }
                } else {
                    // 2º TEMPO: Alvo do TIME A (Gols vão para s1 ou s2)
                    if (last_touch == 3 || last_touch == 4 || last_touch == 6) { s1_final++; } // Contra
                    else {
                        if (last_touch == 1) s1_final++;
                        else if (last_touch == 2) s2_final++;
                    }
                }
            } 
            else {
                // -------------------------------------------------------
                // GOL NA GOLEIRA SUL
                // -------------------------------------------------------
                if (current_half == 1) {
                    // 1º TEMPO: Alvo do TIME A (Gols vão para s1 ou s2)
                    if (last_touch == 3 || last_touch == 4 || last_touch == 6) { s1_final++; } // Contra
                    else {
                        if (last_touch == 1) s1_final++;
                        else if (last_touch == 2) s2_final++;
                    }
                } else {
                    // 2º TEMPO: Alvo do TIME B (Gols vão para s3 ou s4)
                    if (last_touch == 1 || last_touch == 2 || last_touch == 5) { s3_final++; } // Contra
                    else {
                        if (last_touch == 3) s3_final++;
                        else if (last_touch == 4) s4_final++;
                    }
                }
            }
            
            if (is_overtime == 1) {
                break; 
            }
        } 
        else if (game_mode == 5 || game_mode == 6 || game_mode == 7) {
            if (last_touch == 1) s1_final++;
            else if (last_touch == 2) s2_final++;
            else if (last_touch == 3) s3_final++;
            else if (last_touch == 4) s4_final++;
			
			// ADICIONE ESTE BLOCO AQUI: Se estiver no Golden Score, qualquer gol encerra a partida!
            if (is_overtime == 1) {
                for(i = 0; i < 100; i++) {
                    wait_vblank(); 
                    update_sfx();
                }
                silence_sid();
                setup_sid();
                break; 
            }
        }
    }
// IA dos Goleiros Modo 5
              if (player_type[2] == 2) move_goleiros_p2();
            else  move_goleiros_ia();
    
 //   VIC.bordercolor = 1;
	
//}	

// --- 7. UI, TEMPO E GATILHOS DE DIFICULDADE ---
        // --- 7. UI, TEMPO E GATILHOS DE DIFICULDADE ---
		if (sys_tick % 50 == 0) {
	
			if (game_mode == 5 || game_mode == 6 || game_mode == 7) {
        // --- PARTE SUPERIOR (P3 e P4) ---
		if (num_players >= 3) {
          //put_text(5, 0, (const unsigned char*)"P3:"); 
          put_num(8, 0, s3_final);
		}
		if (num_players == 4) {
          //put_text(28, 0, (const unsigned char*)"P4:"); 
          put_num(31, 0, s4_final);
		}

        // --- PARTE INFERIOR (P1 e P2) ---
        //put_text(5, 24, (const unsigned char*)"P1:"); 
        put_num(8, 24, s1_final);
		if (num_players >= 2) {
               // put_text(28, 24, (const unsigned char*)"P2:"); 
			   if (player_type[2] != 2) {
                put_num(31, 24, s2_final);
			   }
		}
	
		
    } 

			// Invertido: SÓ desenha o cronômetro se NÃO for modo GS (Modo 4 ou Modo 5 com Gold Goal)
			if (game_mode != 4 && gold_goal_active == 0 && golden_score_triggered == 0) {
			//	put_text(35, 0, (const unsigned char*)"T:"); 
				put_num(37, 0, (timer / 50));
			}
			else if (golden_score_triggered == 1) {
        put_text(37, 0, (const unsigned char*)"GS"); 
    }

		
		}

       if ((game_mode == 5 && gold_goal_active == 1) || (game_mode == 7 && gold_goal_active == 1) || (game_mode == 6 && gold_goal_active == 1)) {
		    unsigned char win_condition_met = 0;
			
			if (sys_tick % 50 == 0) {
		//	gols_time_a = s1_final + s2_final;
		//	gols_time_b = s3_final + s4_final;
			
			
			// Lógica para Modo 5 (Individual)
			if (game_mode == 5 || game_mode == 7) {
				// 1. Verifica condição de vitória apenas para jogadores válidos na partida
    if ((s1_final >= 5) || 
        (num_players >= 2 && player_type[2] != 2 && s2_final >= 5) || 
        (num_players >= 3 && s3_final >= 5) || 
        (num_players >= 4 && s4_final >= 5)) {
        
        win_condition_met = 1;

        // Placar P1 (Sempre presente)
        put_num(8, 24, s1_final);

        // Placar P2: Apenas se houver >= 2 jogadores E P2 não for Goleiro
        if (num_players >= 2 && player_type[2] != 2) {
            put_num(31, 24, s2_final);
        }

        // Placar P3: Apenas se houver >= 3 jogadores
        if (num_players >= 3) {
            put_num(8, 0, s3_final);
        }

        // Placar P4: Apenas se houver 4 jogadores
        if (num_players >= 4) {
            put_num(31, 0, s4_final);
        }
    }
			}
			// Lógica para Modo 7 (Time)
			else if (game_mode == 8 ) {
				if (gols_time_a >= 5 || gols_time_b >= 5) {
					win_condition_met = 1;
					put_num(8, 24, s1_final);
					put_num(31, 24, s2_final);
					put_num(8, 0, s3_final);
					put_num(31, 0, s4_final);

				}
			}
			
			}
			if (win_condition_met) {
				
				// Delay para celebração
				for(i = 0; i < 100; i++) {
					wait_vblank(); 
					update_sfx();
				}
				
				silence_sid();
				setup_sid();
				break; // SAI DO LOOP APENAS SE ATINGIR 5 GOLS
			  }
			

			// Se ainda não chegou em 5 gols, apenas decrementa o timer (que começou em 9999)
			timer--; 
			
			// Gatilho de dificuldade extra no final
			if ((s1_final >= 4 || s2_final >= 4 || s3_final >= 4 || s4_final >= 4) && difficulty_boost == 0) {
				difficulty_boost = 1;
				play_sfx_whistle();
			}
	

        } else {
        
            // Lógica para os outros modos (por tempo)
            if (golden_score_triggered == 1) {
                if (timer > 0) {
                    timer--;
                } else {
                    timer = 9999;
                }
            } 
            else {
                // Tempo regulamentar normal
                if (timer > 0) {
                    timer--;
                } 
                else {
                    // --- O TEMPO REGULAMENTAR ZEROU NESTE FRAME! ---
    //                gols_time_a = s1_final + s2_final;
    //                gols_time_b = s3_final + s4_final;

                    if (current_half == 1) {
                        // -----------------------------------------------------------
                        // TRANSICÃO DO 1º PARA O 2º TEMPO (2nd Half)
                        // -----------------------------------------------------------
                        current_half = 2;
                        timer = 2250; // Reseta o cronômetro para o segundo tempo
                        
                        // 1. Apito de fim de primeiro tempo
                        play_sfx_whistle();
                        
                        // 2. Backup e exibição do texto "2ND HALF!" centralizado
                        for (i = 0; i < 13; i++) {
                            gs_text_backup[i] = SCREEN[12 * 40 + 13 + i]; 
                            gs_color_backup[i] = ((unsigned char*)0xD800)[12 * 40 + 13 + i];
                        }
                        
                        put_text(15, 12, (const unsigned char*)"2ND HALF");
                        for(i = 0; i < 13; i++) ((unsigned char*)0xD800)[12 * 40 + 13 + i] = 1; // Branco

                        // Pausa de ~2 segundos (100 frames) para o jogador ver o anúncio
                        for(i = 0; i < 80; i++) {
                            wait_vblank();
                            update_sfx();
                        }

                        // Restaura o gramado
                        for (i = 0; i < 13; i++) {
                            SCREEN[12 * 40 + 13 + i] = gs_text_backup[i];
                            ((unsigned char*)0xD800)[12 * 40 + 13 + i] = gs_color_backup[i];
                        }

                        // 3. REPOSICIONAMENTO DOS JOGADORES (Lados Invertidos)
                        px = 120; py = 160;   // P1 vai para o Sul
                        p2x = 200; p2y = 160; // P2 vai para o Sul
                        p3x = 120; p3y = 120; // P3 vai para o Norte
                        p4x = 200; p4y = 120; // P4 vai para o Norte
                        bx = 164; by = 144;   // Bola no centro

                        // --- RESET COMPLETO DA BOLA E TRAVA DE CHUTE ---
                        bx = 164; by = 144;   // Bola no centro
                        bdx = 0;  bdy = 0;    // TRAVA: Zera a velocidade e os vetores de chutes vazados
                        ball_possession = 0;  // Ninguém começa grudado na bola
                        kick_cooldown = 30;   // Cooldown inicial para o segundo tempo começar limpo
                        
                         dx = 160; dy = 190;   // Zagueiro do Time A (ex-Norte) agora defende o Sul
                        dx_b = 160; dy_b = 90; // Zagueiro do Time B (ex-Sul) agora defende o Norte

                        // --- ATUALIZAÇÃO DOS ZAGUEIROS (Slots 1 e 4) CORES E POSIÇÕES ---
 

if (use_defender==0) {
    // --- ESCONDE OS ZAGUEIROS NO MODO 5 (Força posição 0,0) ---
    VIC.spr_pos[1].x = 0;  VIC.spr_pos[1].y = 0; // Slot 1 em 0,0
    VIC.spr_pos[4].x = 0;  VIC.spr_pos[4].y = 0; // Slot 4 em 0,0
}

                        play_sfx_whistle(); // Apito de início do 2º tempo
                        
                        // Continua o loop principal (não dá break!)
                    } // <--- AQUI CHAVE ADICIONADA CORRETAMENTE PARA FECHAR O 'if (current_half == 1)'
                    else {
						
						unsigned char is_tied = 0;

    // Define a regra de empate de acordo com o número de jogadores ativos
   // Define a regra de empate (válida para o Modo 5 e Modo 7)
    if (game_mode == 7) {
        unsigned char max_score = 0;
        unsigned char leaders_count = 0;
        unsigned char p_scores[4];
        unsigned char p_idx;

        p_scores[0] = s1_final;
        p_scores[1] = s2_final;
        p_scores[2] = s3_final;
        p_scores[3] = s4_final;

        // 1. Descobre qual é a maior pontuação entre os jogadores ativos
        for (p_idx = 0; p_idx < num_players; p_idx++) {
            if (p_scores[p_idx] > max_score) {
                max_score = p_scores[p_idx];
            }
        }

        // 2. Conta quantos jogadores estão empatados com essa pontuação máxima
        for (p_idx = 0; p_idx < num_players; p_idx++) {
            if (p_scores[p_idx] == max_score) {
                leaders_count++;
            }
        }

        // 3. Define se deu empate na liderança
        if (num_players == 1) {
            is_tied = 1; // Sozinho sempre vai pro Golden Score (treino, etc)
        } else if (leaders_count > 1) {
            is_tied = 1; // Pelo menos 2 pessoas dividem a liderança
        } else {
            is_tied = 0; // Alguém é o líder isolado
        }
    }
                        // Se já estava no segundo tempo (current_half == 2)
                    if ((game_mode == 7) && is_tied) {
        // Todos os placares empatados: Ativação do Golden Score

   
                            // --- ATIVAÇÃO DO GOLDEN SCORE ---
                            golden_score_triggered = 1;
                            is_overtime = 1; 
							ot_base1 = s1_final;
                            ot_base2 = s2_final;
                            ot_base3 = s3_final;
                            ot_base4 = s4_final;
                            timer = 9999;
                            
                            if (gold_goal_active == 0) {
                                difficulty_boost = 1;
                            }
                            
                            for (i = 0; i < 13; i++) {
                                gs_text_backup[i] = SCREEN[12 * 40 + 13 + i]; 
                                gs_color_backup[i] = ((unsigned char*)0xD800)[12 * 40 + 13 + i];
                            }
                            
                            put_text(13, 12, (const unsigned char*)"GOLDEN SCORE!");
                            for(i = 0; i < 13; i++) ((unsigned char*)0xD800)[12 * 40 + 13 + i] = 1;

                            play_sfx_whistle();
                            for(i = 0; i < 100; i++) wait_vblank(); 

                            for (i = 0; i < 13; i++) {
                                SCREEN[12 * 40 + 13 + i] = gs_text_backup[i];
                                ((unsigned char*)0xD800)[12 * 40 + 13 + i] = gs_color_backup[i];
                            }
                        } 
                        else {
                            // Fim de jogo definitivo
                            break; 
                        }
                    }
                }
            }
        }
    } // Fim do while(1)
			
		if (golden_score_triggered == 0 ) {
           put_num(36,0, 0); // Força o "00" na tela
		}
    VIC.spr_ena = 0;
//    fast_clrscr(1);
	
	// --- NO FINAL DA PLAY_MATCH ---
	asm("sei");
	*((unsigned int*)0x0314) = 0xea31; // Restaura o sistema original
	VIC.imr = 0;                       // Desliga interrupção de vídeo
	asm("cli");
	
	// ==========================================
    // CORREÇÃO: RESTAURAR O TECLADO PARA O MENU / F1
    // ==========================================
    *((unsigned char*)0x0289) = 0x80;  // Restaura o padrão do sistema (ou 128)
    
    // Limpa completamente qualquer lixo ou tecla presa no buffer do CC65
    while (kbhit()) {
        cgetc();
    }

     play_sfx_game_over();
	 
	 // Espera o apito terminar processando a máquina de estados
    while(whistle_state != 0) {
        while(VIC.rasterline != 250);
        update_sfx();
    }
    silence_sid(); // Mata o áudio antes de sair
	setup_sid();
    VIC.spr_ena = 0;
	
	 

if ((game_mode == 5 || game_mode == 6 || game_mode == 7) && !(is_demo_mode)) {

    /* No Golden Score, só contam os gols feitos depois do empate */
    if (is_overtime) {
        e1 = s1_final - ot_base1;
        e2 = s2_final - ot_base2;
        e3 = s3_final - ot_base3;
        e4 = s4_final - ot_base4;
    } else {
        e1 = s1_final;
        e2 = s2_final;
        e3 = s3_final;
        e4 = s4_final;
    }

    max_g = e1;
    win_p = 1;
    tie = 0;

    if (e2 > max_g) { max_g = e2; win_p = 2; tie = 0; }
    else if (e2 == max_g) tie = 1;

    if (e3 > max_g) { max_g = e3; win_p = 3; tie = 0; }
    else if (e3 == max_g) tie = 1;

    if (e4 > max_g) { max_g = e4; win_p = 4; tie = 0; }
    else if (e4 == max_g) tie = 1;

    put_text(11, 12, (const unsigned char*)"                    ");

    if (max_g == 0) {
        put_text(13, 12, (const unsigned char*)"NO GOALS SCORED ");
    }
    else if (tie) {
        put_text(15, 12, (const unsigned char*)"DRAW GAME");
    }
    else {
        put_text(13, 12, (const unsigned char*)"PLAYER    WINS");
        put_num(20, 12, win_p);
    }
}

// --- PARTE 2: FINALIZAÇÃO (Pausa, limpeza e áudio) ---
// Roda incondicionalmente no final da partida, independentemente do modo

// Mata o áudio antes de sair
silence_sid(); 
setup_sid();

// Pequeno delay para o jogador ler quem venceu antes de seguir
for(i = 0; i < 100; i++) wait_vblank(); 

// Limpa os textos da tela central
put_text(5, 12, (const unsigned char*)"            ");
put_text(13, 12, (const unsigned char*)"               ");

// Delay estendido antes de trocar de tela
for(i = 0; i < 250; i++) {
    while(VIC.rasterline != 250); 
}

}
	

// Revisar!
static void calculate_keeper_rating(unsigned char goals_against) {
    int calculation;
    
    // Cada defesa vale 10, cada gol tira 15
    calculation = (saves_p2 * 10) - (goals_against * 15);
    
    // Bônus por não tomar gols (se o jogo durou o tempo mínimo)
    if (goals_against == 0 && saves_p2 > 0) calculation += 50;

    // Garante que não seja negativo
    if (calculation < 0) keeper_score = 0;
    else keeper_score = (unsigned int)calculation;
}




void main(void) {
    unsigned char key;
    unsigned char post_match_sel = 0; 
   // unsigned char challenge_human_count = 1; 
    
    
    // init_high_scores(); // Inicializa os dados uma vez
    init_random();
    // load_screen();       // Splash screen
    setup_hardware();   // Prepara os assets
    music_init();
    
    while(1) {
        show_menu(); // Configura modo, campo, etc.

       // Captura a escolha do menu AGORA, antes que num_players seja modificado pelos estágios
        if (game_mode == 6) {
            challenge_human_count = num_players; 
        }
        
        while(1) {
            
            if (game_mode == 6) {
        unsigned char num_opps, diff, def, goals;
        unsigned char human_count = num_players; // Salva se o usuário escolheu 1P ou 2P humanos no menu
         unsigned int current_total_goals = (challenge_human_count == 2) ? (s1_total + s2_total) : s1_total;
		 unsigned int goal_step = (challenge_human_count == 2) ? 50 : 30;
		 // Quantos bônus o jogador deve ter no total até agora
                unsigned int total_deserved_bonuses = current_total_goals / goal_step;
              
       
       challenge_bonus_threshold = (challenge_human_count == 2) ? 50 : 30;

        // --- VERIFICAÇÃO DE BÔNUS DE TENTATIVAS ---
                
            //    while (current_total_goals >= challenge_bonus_threshold) {
            //        challenge_attempts++;
            //        challenge_bonus_threshold += (challenge_human_count == 2) ? 50 : 30;
            //    }
				
				  
                // Se o total merecido for maior do que o que já foi dado, concede a diferença
                if (total_deserved_bonuses > challenge_bonuses_given) {
                    unsigned char new_bonuses = total_deserved_bonuses - challenge_bonuses_given;
                    challenge_attempts += new_bonuses;
                    challenge_bonuses_given = total_deserved_bonuses;
                }

        // Seleciona a tabela correta baseada no número de jogadores humanos
        if (challenge_human_count == 1) {
            num_opps = challenge_stage_1p[tournament_stage][0];
            diff     = challenge_stage_1p[tournament_stage][1];
            def      = challenge_stage_1p[tournament_stage][2];
            goals    = challenge_stage_1p[tournament_stage][3];
        } else {
            num_opps = challenge_stage_2p[tournament_stage][0];
            diff     = challenge_stage_2p[tournament_stage][1];
            def      = challenge_stage_2p[tournament_stage][2];
            goals    = challenge_stage_2p[tournament_stage][3];
        }

        // Atribui as configurações do estágio para o motor do jogo
        current_min_goals = goals;
        difficulty_level  = diff;
        use_defender      = def;
	

        // Configuração dos slots e cálculo correto do total de entidades ativas (num_players)
        if (challenge_human_count == 2) {
            player_type[1] = 1; // P1 Humano
            player_type[2] = 1; // P2 Humano
            player_type[3] = (num_opps >= 1) ? 0 : 255;
            player_type[4] = (num_opps >= 2) ? 0 : 255;
            num_players = 2 + num_opps; // 2 humanos + oponentes da CPU
        } else {
            player_type[1] = 1;                         // P1 Humano
            player_type[2] = (num_opps >= 1) ? 0 : 255; // P2 CPU
            player_type[3] = (num_opps >= 2) ? 0 : 255; // P3 CPU
            player_type[4] = (num_opps >= 3) ? 0 : 255; // P4 CPU
            num_players = 1 + num_opps; // 1 humano + oponentes da CPU (2, 3 ou 4 total)
        }

        // Exibe a tela com o Estágio, Target e Tentativas ANTES do jogo
        show_interstage_screen();
    }

            sorteia_personalidades();
            play_match(); // O jogo acontece aqui

            // --- INÍCIO DA TRANSIÇÃO RÁPIDA ---
            VIC.spr_ena = 0; // Desliga sprites imediatamente

            // 2. Preparação para a tela de Score Final
            memset((void*)0x0400, 32, 1000); 

           // --- DESVIO PARA MODO 5, 6 ou 7 ---
            if (game_mode == 5 || game_mode == 6 || game_mode == 7) {
                display_final_score_individual_v5(); 
            } 
            
            // --- LÓGICA DE VERIFICAÇÃO PÓS-PARTIDA ---
            if (game_mode == 6) {
                unsigned char total_scored = (challenge_human_count == 2) ? (s1_final + s2_final) : s1_final;
                unsigned char stage_passed = 0;
                
                // Validação de acordo com a quantidade de jogadores humanos
                if (challenge_human_count == 1) {
                    if (total_scored >= current_min_goals && (winner == 1 || is_tie == 1)) stage_passed = 1;
                } else {
                    if (total_scored >= current_min_goals && (winner == 1 || winner == 2 || is_tie == 1)) stage_passed = 1;
                }

                if (stage_passed) {
                    put_text(13, 14, (const unsigned char*)"STAGE CLEARED!");
                    tournament_stage++;
                    
                    if (tournament_stage >= 18) {
                        show_tournament_champion(); 
                        tournament_stage = 0;
                        challenge_attempts = 2;
                        challenge_bonuses_given = 0;
                        break; 
                    }
                    put_text(9, 16, (const unsigned char*)"A - NEXT STAGE  M - MENU");
                } else {
                    challenge_attempts--;
                    put_text(13, 14, (const unsigned char*)"STAGE FAILED!");
                    if (challenge_attempts == 0) {
                        show_game_over_confirm();
                        tournament_stage = 0;
                        challenge_attempts = 2;
                        challenge_bonus_threshold = (challenge_human_count == 2) ? 50 : 30;
                        challenge_bonuses_given = 0;
                        break; 
                    }
                    put_text(9, 16, (const unsigned char*)"A - SAME STAGE  M - MENU");         
                }
            } 
            else if (game_mode == 7) {
                // --- VERIFICA SE HÁ UM CAMPEÃO NO PLAYOFF ---
                unsigned char champ = 0;
                if (v1_total >= playoff_target_wins) champ = 1;
                else if (v2_total >= playoff_target_wins) champ = 2;
                else if (v3_total >= playoff_target_wins) champ = 3;
                else if (v4_total >= playoff_target_wins) champ = 4;

                if (champ > 0) {
                    // Limpa o meio da tela e anuncia
                    put_text(6, 13, (const unsigned char*)"    PLAYOFF CHAMPION: P     ");
                    put_num(29, 13, champ);
                    paint_row_color(13, 6, 28, get_fixed_player_color(champ));
                    
                    put_text(10, 14, (const unsigned char*)"PRESS BUTTON TO MENU");
                    
                    // Zera o campeonato para a próxima vez
                    v1_total = 0; v2_total = 0; v3_total = 0; v4_total = 0;
                    
                    // Espera apertar o botão (evitando pulo acidental)
                    while ((~CIA1.pra) & 0x10) wait_vblank(); 
                    while (!((~CIA1.pra) & 0x10)) wait_vblank(); 
                    
                    break; // Sai do loop da partida e volta ao menu inicial
                } else {
                    // Sem campeão ainda, libera para jogar a próxima partida do Playoff
                    put_text(8, 16, (const unsigned char*)"A - NEXT MATCH  M - MENU");
					
                }
            }
            else {
                // Para os demais modos (diferentes de 6 e 7)
                put_text(9, 16, TXT_NAV); 
            }
            
            // Aqui continua a sua rotina de escolha pós-partida existente (seletores de A/M)
            post_match_sel = 0;
            last_joy_state = ~CIA1.pra;
            
            // 3. Loop de escolha pós-partida
  
            while(1) {
                wait_vblank();
                joy_now = ~CIA1.pra;

                // Desenha apenas os seletores
                put_text(8, 16, (post_match_sel == 0) ? (const unsigned char*)"*" : (const unsigned char*)" ");
                put_text(24, 16, (post_match_sel == 1) ? (const unsigned char*)"*" : (const unsigned char*)" ");

                if ((joy_now & 4) && !(last_joy_state & 4)) post_match_sel = 0;
                if ((joy_now & 8) && !(last_joy_state & 8)) post_match_sel = 1;

                if ((joy_now & 0x10) && !(last_joy_state & 0x10)) {
                    key = (post_match_sel == 0) ? 'A' : 'M';
                    break;
                }

                if (kbhit()) {
                    key = cgetc();
                    if (key == 'a' || key == 'A' || key == 'm' || key == 'M') break;
                }
                last_joy_state = joy_now;
            }
            
            // 4. Saída do Loop de Partida
            if (key == 'm' || key == 'M') {
                break; // Volta para o Menu Principal
            }
            
            // Se for 'A', reseta o estado e vai para o próximo estágio/partida
            reset_game_state(); 
        }
    }
}

