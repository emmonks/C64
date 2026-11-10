/*
2GOAL - C64 LEAGUE - v1.1
15/08/2026 - ajustar as funções e remover código do game_mode==5
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
static void clear_screen_c000_with_color(unsigned char color) ;
static void draw_big_title(void) ;
static void draw_field(unsigned char style);
static void enter_game_mode(void) ;
//static void enter_menu_mode(void);
static void fast_clrscr(unsigned char color) ;
static void handle_ball_logic_v2(void);
static void init_crowd_voice(void);
static void init_whistle_voice(void) ;
//static void __fastcall__  keep_defender_in_field(unsigned int* x, unsigned int* y) ;
static void move_defender_ia(unsigned char is_south);
static void move_goleiros_ia(void) ;
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
//static void reset_game_state(void) ;
static void setup_hardware(void);
static void setup_sid(void) ;
static void show_menu(void);
static void update_ball_animation(void) ;
//static void update_dynamic_pointers(void);
static void update_players_anim(void) ;
//static void update_sprites(void) ;
static void wait_vblank(void) ;
static void interrupt_raster_handler(void); 
static void display_final_score_individual_v5(void);
static void draw_field_manager(void);
//static void move_player1_ia_modo5(void);
//static void move_player2_ia_modo5(void);
//static void move_player3_ia_modo5(void);
//static void move_player4_ia_modo5(void);
static void move_player_ia_modo5(unsigned char pid);
//static void __fastcall__ put_num2(unsigned char x, unsigned char y, unsigned int num) ;
static void __fastcall__  apply_smart_kick2(unsigned char p_idx);
static void __fastcall__  paint_row_color(unsigned char y, unsigned char x, unsigned char len, unsigned char color);
static unsigned int __fastcall__ keep_x_v5(unsigned int x);
static unsigned char __fastcall__ keep_y_v5(unsigned char y);
static unsigned char get_fixed_player_color(unsigned char p_idx);
static void bounce_ball_at_walls(void) ;
static void separate_all_players(void);
static void show_tournament_champion(unsigned char champion_idx);
//static void draw_champion_title(unsigned char color_offset);
static void setup_match_colors(unsigned char h_id, unsigned char a_id);
//static void prepare_tournament_teams(void);
static void init_random(void);
//static void sorteia_personalidades(void);
// C64 League
static void generate_league_fixtures(void);
static void init_league(void);
static void league_manager_loop(void);
static void show_league_table(void);
static void simulate_cpu_match(void) ;
static void update_league_results(unsigned char f_idx);
static void sort_league_table(void);
static void show_last_match_result(void);
static void reset_league(void);
//static void init_random(void);
static void show_league_status(void);
static void sorteia_personalidades(void);
//static void show_demo(void);
//static void show_bet_mode(void);
static void highlight_line(unsigned char x, unsigned char y, unsigned char length, unsigned char color);

//extern void __fastcall__ chama_menu_asm(const char* nome);

static void silence_sid(void);
static void chama_menu(const char* nome);
//static void init_human_sprites(void);

// Tabelas de ajuste por Dificuldade [Fácil (0), Normal (1), Difícil (2)]
static const unsigned char IA_DIST_Y[]      = {20, 40, 60};  // Distância para perseguição total
static const unsigned char IA_TIMER_MASK[]  = {7,  3,  1};  // Máscara de frames (7 = a cada 8 frames, 1 = a cada 2 frames)
static const unsigned char IA_CHUTE_BASE[]  = {3,  5,  7};  // Força base do bico para frente
static const unsigned char IA_CHUTE_RAND[]  = {1,  1,  3};  // Variação aleatória da força
static const unsigned char IA_WAIT_TIME[]   = {50, 35, 15};  // Tempo de travamento após o chute

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
unsigned char game_mode = 8;      // <--- ADICIONADO AQUI
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
unsigned char s1_final = 0; // Renomeei para evitar confusão com outras variáveis
unsigned char s2_final = 0;
unsigned char last_touch = 0;        // 1 para P1, 2 para P2
unsigned char after_touch_timer = 0; // Contador do efeito
unsigned char field_type = 0; // 0: Grama, 1: Molhado, 2: Neve
unsigned char selected_field_option = 0; // 0: Grama, 1: Molhado, 2: Neve, 3: Aleatório
unsigned char use_defender = 0; // 0: Não, 1: Sim
unsigned int dx, dy;                   // Posição do Zagueiro
unsigned int dx_b, dy_b;                   // Posição do Zagueiro
//unsigned char d_level = 0;      // Nível da IA do zagueiro
//unsigned char spr_active_p1       = 1;
//unsigned char spr_active_p2       = 0;
//unsigned char spr_active_ball     = 1;
//unsigned char spr_active_defender = 0;
unsigned char spr_hix_mask;
unsigned char kick_power = 0;      // Acumulador de força
unsigned char defender_wait = 0; // Timer de "paralisia" inicial da IA
unsigned char defender_wait_b = 0; // Timer de "paralisia" inicial da IA
//unsigned char last_joy_state = 0; // Para evitar que o menu "corra" muito rápido
//unsigned char joy_now = 0;
//unsigned int hs_timer = 0;  // Contador de frames
unsigned char difficulty_level = 1; // 0: Easy, 1: Normal, 2: Hard
unsigned char num_players = 1; // 1, 2, 3 ou 4 jogadores humanos
unsigned char joy_bits[4]; // Adicione esta linha no escopo global
unsigned char s3_final, s4_final;
unsigned char p2_is_moving = 0; // 0 = parado, 1 = em movimento
unsigned char p1_is_moving = 0;
unsigned char p3_is_moving = 0;
unsigned char p4_is_moving = 0;
unsigned char p5_is_moving = 0;
unsigned char p6_is_moving = 0;
unsigned char player_type[5]; // player_type[2] é o P2, etc.
//static unsigned int saves_p2 = 0;
//static unsigned int keeper_score = 0;
unsigned char stun_timer_p1 = 0;
unsigned char stun_timer_p2 = 0;
unsigned char stun_timer_p3 = 0;
unsigned char stun_timer_p4 = 0;
static unsigned char shot_delay_p1 = 0; 
static unsigned char shot_delay_p2 = 0; 
static unsigned char shot_delay_p3 = 0; 
static unsigned char shot_delay_p4 = 0; 
unsigned char gold_goal_active = 0;
//unsigned char wall_insist_count = 0;
//unsigned char is_overtime = 0;
unsigned char golden_score_triggered = 0;
//unsigned char enemy_color = 2; // Default vermelho

unsigned char sys_tick = 0;
unsigned char player_pers[5]; /* Usamos 5 para ignorar o índice 0 e usar 1 a 4 direto */


 
 unsigned char ia_frame_toggle = 0; 

 unsigned char current_half = 1; // 1 = 1st Half, 2 = 2nd Half



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




unsigned char tournament_opponents[15]; // 10 oponentes para os 10 estágios

// C64 League
// Funcoes do C64 LEAGUE

unsigned char current_turn_display = 1;
unsigned char gols_time_a = 0;
unsigned char gols_time_b = 0;

// LEAGUE
#define MAX_TEAMS 20

// Tabela de Classificação
unsigned char league_points[MAX_TEAMS];
unsigned char league_goals_for[MAX_TEAMS];
unsigned char league_goals_agst[MAX_TEAMS];
unsigned char league_wins[MAX_TEAMS]; // Opcional, para estatísticas

// Configurações da Liga
unsigned char team_difficulty[MAX_TEAMS]; // 1 a 4
unsigned char is_human_team[MAX_TEAMS];   // 0 = CPU, 1 = Humano
unsigned char current_round = 0;          // Rodada atual (0 a 21 se for turno/returno)

unsigned char league_total_teams = 12; // 4 a 12
unsigned char league_total_turns = 1;  // 1 ou 2

static unsigned int league_entropy = 0; // Global de 16 bits

unsigned char champion_idx = 0; 

// Estrutura de um Jogo (Fixture)
typedef struct {
    unsigned char home;
    unsigned char away;
    unsigned char played; // 0 = não, 1 = sim
} fixture;

// Para 12 times (turno único), são 132 jogos. 
// 132 * 3 bytes = 396 bytes. Cabe folgado na RAM.
fixture league_fixtures[132];
unsigned char total_league_games;
unsigned char current_fixture_idx = 0; // Índice do jogo atual (0 a 131)
unsigned char league_rank[12];         // Índices dos times ordenados para a tabela

// Variáveis globais para armazenar o último resultado simulado
static unsigned char last_h_score, last_a_score;
static unsigned char last_h_idx = 255, last_a_idx = 255;
static unsigned char league_active_teams[12]; // Guarda os IDs dos times sorteados

unsigned char player_team[5];  // Armazena o ID do time (0-10) para cada jogador (P1-P4)
unsigned char partner_of[5];   // Armazena quem é a dupla de quem 

unsigned char original_player_type[5];

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




// Variável global para controlar a direção da curva da IA

signed char ai_curve_dir = 0;      // Controla a direção da curva (1, -1 ou 0)

// Suporte para até 4 jogadores (índices 1 a 4)
unsigned char has_boost[5] = {0, 0, 0, 0, 0};    // 1 = Pronto para usar
unsigned char boost_timer[5] = {0, 0, 0, 0, 0};  // Contador de frames do boost ativo
unsigned char p_idx_check = 0 ; // Declare aqui se ainda não foi declarada

 unsigned char is_pass = 0; // Flag para proteger o passe
 
 // Times
 // Times
 // --- CONSTANTES (RODATA) ---
const char* team_names[] = {
    "GIANA UNITED ", // 12 + 1
    "TURRICAN FC  ", // 11 + 2
    "WIZBALL CITY ", // 12 + 1
    "REAL ARKANOID", // 13 (Exato)
    "BAYERN ELITE ", // 12 + 1
    "URIDIUM AC   ", // 10 + 3
    "LAST NINJAS  ", // 11 + 2
    "IK+ COUNTY   ", // 10 + 3
    "COMMANDO AS  ", // 11 + 2
    "PIRATES! FC  ", // 11 + 2
    "AFC MANIACS  ",  // 12 + 1 (Mantive o espaço inicial se for proposital)
    "LODE RUNNERS ", // 13 (Exato)
    "CYBERDYNE FC ", // 13 (Exato)
    "EPYX UNITED  ", // 13 (Exato)
    "INVADERS KS  ", // 13 (Exato)
    "BOUNDERS SC  ", // 13 (Exato)
	"PARADROID AC ",
    "BUBBLE ROVERS",
    "IMPOSSIBLE FC",
    "S.E.R.K.ERS  "
};

// Cores base de cada time
const unsigned char team_colors[] = {
    1,  // Giana (Branco)
    10, // Turrican (Verme. Claro)
    3,  // Wizball (Ciano)
    7,  // Arkanoid (Amarelo)
    8,  // Bayern Elite (Laranja)
    4,  // Uridium (Roxo)
    0, // Last Ninja (Cinza Médio)
    9,  // IK+ (Marrom)
    11, // Commando (Cinza Escuro)
    3,  // Pirates (Ciano)
    6,  //AFC Maniacs
    1,  //LODE RUNNERS 
    14,  //CYBERDYNE FC
    6,  //EPYX UNITED
    4,  //INVADERS KS
    2,  //BOUNDERS SC
	12, // PARADROID AC
    13, // BUBBLE ROVERS
    11, // IMPOSSIBLE FC
    14   // SUMMER GAMES
};

static const unsigned char color_family[16] = {
    1, // 0: Black
    2, // 1: White
    3, // 2: Red
    4, // 3: Cyan
    5, // 4: Purple
    6, // 5: Green
    4, // 6: Blue
    7, // 7: Yellow
    3, // 8: Orange (Família do Vermelho)
    3, // 9: Brown  (Família do Vermelho)
    3, // 10: Light Red (Família do Vermelho)
    1, // 11: Dark Grey
    1, // 12: Medium Grey
    6, // 13: Light Green
    4, // 14: Light Blue
    2  // 15: Light Grey
};
 
// --- VARIÁVEIS GLOBAIS (DATA/BSS) ---
unsigned char home_team = 0;  // ID do Time A (0-9)
unsigned char away_team = 1;  // ID do Time B (0-9)
unsigned char p1_color, p2_color, p3_color, p4_color; // Cores finais dos sprites
 
/* --- Assets --- */

/* --- Mensagens do Menu (Screen Codes) --- */
/* --- Assets de Texto --- */
const unsigned char msg_prompt[] = {15, 16, 3, 1, 15, 58, 0};
const unsigned char msg_titulo[] = "c64 league v1.1"; // "2GOAL - v"
//const unsigned char msg_field[] = "choose field: 1-gra 2-wet 3-fro 4-rand";
const unsigned char msg_line[]  = "--------------------------------------";
const unsigned char msg_credits[] = "by eduardo maronas monks - 2026";
// Atualizado de "REJOGAR" para "PLAY AGAIN" (ajustado nos screen codes)
//const unsigned char TXT_NAV[] = {1, 32, 45, 32, 16, 12, 1, 25, 32, 1, 7, 1, 9, 14, 32, 32, 13, 32, 45, 32, 13, 5, 14, 21, 0}; // "A - PLAY AGAIN  M - MENU"

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
/*
const unsigned char sprite_goleiroN[64] = {
0x02,0xAA,0x80,0x0A,0xAA,0xA0,0x2A,0x69,0xA8,0x2A,0x55,0xA8,0x20,0x55,0x08,0x20,0x55,0x08,0x20,0x55,0x08,0x20,0x3C,0x08,0x3C,0x00,0x3C,0x30,0x00,0x0C,0x30,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82
};

const unsigned char sprite_goleiroS[64] = {
0x30,0x00,0x0C,0x30,0x00,0x0C,0x3C,0x00,0x3C,0x20,0x3C,0x08,0x20,0x55,0x08,0x20,0x55,0x08,0x20,0x55,0x08,0x2A,0x55,0xA8,0x2A,0x69,0xA8,0x0A,0xAA,0xA0,0x02,0xAA,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82
};
*/

const unsigned char sprite_goleiroN[64] = {
0x02,0xAA,0x80,0x0A,0xAA,0xA0,0x2A,0x96,0xA8,0x2A,0x96,0xA8,0x20,0x55,0x08,0x10,0x55,0x04,0x10,0x55,0x04,0x10,0x3C,0x04,0x3C,0x00,0x3C,0x30,0x00,0x0C,0x30,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82


};

const unsigned char sprite_goleiroS[64] = {
0x30,0x00,0x0C,0x30,0x00,0x0C,0x3C,0x00,0x3C,0x10,0x3C,0x04,0x10,0x55,0x04,0x10,0x55,0x04,0x10,0x55,0x04,0x2A,0x96,0xA8,0x2A,0x96,0xA8,0x0A,0xAA,0xA0,0x02,0xAA,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82


};



 
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

#define SPR_PTR ((unsigned char*)0xC3F8)

unsigned char ball_anim = 0;
unsigned char ball_anim_delay = 0;



// BOLA DE FUTEBOL (8x7 pixels) - 8 FRAMES DE ROTAÇÃO HORIZONTAL

// BOLA DE FUTEBOL (8x7 pixels) - 8 FRAMES DE ROTAÇÃO HORIZONTAL


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

static unsigned char saved_sprites[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
static unsigned char active_sprite[4];

const unsigned char* const idle_gfx[7] = { j1, j2, j3, j4, j5, j6,j7 };
const unsigned char* const run_gfx[7]  = { j1r, j2r, j3r, j4r, j5r, j6r,j7r };


static void clear_saved_sprites(void) {
    unsigned char i;
    for (i = 0; i < 4; ++i) {
        saved_sprites[i] = 0xFF;
    }
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
	


    asm("cli");

    // 7. Salta para o loop principal do BASIC (depois do cabeçalho de boas-vindas)
    asm("jmp $a480");
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

// C64 LEAGUE

// Modo 8

static void update_league_results(unsigned char f_idx) {
    unsigned char h = league_fixtures[f_idx].home; // Mandante
    unsigned char a = league_fixtures[f_idx].away; // Visitante
    unsigned char goals_h, goals_a;

 //   if (player_team[1] == h) { 
	if (home_team == h) {
        // Mandante está na Esquerda
        goals_h = s1_final + s2_final;
        goals_a = s3_final + s4_final;
    } else {
        // Mandante está na Direita (Inverteu a posição no campo, mas não o controle!)
        goals_h = s3_final + s4_final;
        goals_a = s1_final + s2_final;
    }

    // Salva para exibição do "Last Match" antes de processar
    last_h_score = goals_h;
    last_a_score = goals_a;
    last_h_idx = h;
    last_a_idx = a;

    // Estatísticas de Gols (GF e GA)
    league_goals_for[h] += goals_h;
    league_goals_agst[h] += goals_a;
    league_goals_for[a] += goals_a;
    league_goals_agst[a] += goals_h;

    // --- CONTABILIZAÇÃO DE PONTOS ---
    if (goals_h > goals_a) {
        // Vitória Mandante
        league_points[h] += 3;
        league_wins[h]++;
    } 
    else if (goals_a > goals_h) {
        // Vitória Visitante (IMPORTANTE: Garante pontos fora de casa)
        league_points[a] += 3;
        league_wins[a]++;
    } 
    else {
        // Empate
        league_points[h] += 1;
        league_points[a] += 1;
    }

	if (is_human_team[h] || is_human_team[a]) {
    // Acumula os gols e o estado atual do contador de frames (VIC.rasterline)
    // Somar o rasterline garante que até o MILISEGUNDO em que o jogo acabou mude a semente.
    league_entropy += (goals_h * 10) + (goals_a * 5) + VIC.rasterline;
}

    league_fixtures[f_idx].played = 1;
}


static void sort_league_table(void) {
    unsigned char i, j, temp;
    unsigned char t1, t2; 
    int s1, s2; 
    unsigned char should_swap;

    // 1. Inicializa o ranking com as POSIÇÕES (0 a N) do array league_active_teams
    for (i = 0; i < league_total_teams; i++) {
        league_rank[i] = i; 
    }

    // 2. Bubble Sort limpo (sem goto)
    for (i = 0; i < league_total_teams - 1; i++) {
        for (j = 0; j < league_total_teams - i - 1; j++) {
            
            t1 = league_active_teams[league_rank[j]];
            t2 = league_active_teams[league_rank[j + 1]];

            should_swap = 0;

            // Critério 1: Pontos (maior pontuação fica acima)
            if (league_points[t1] < league_points[t2]) {
                should_swap = 1;
            } 
            else if (league_points[t1] == league_points[t2]) {
                // Critério 2: Vitórias (mais vitórias fica acima)
                if (league_wins[t1] < league_wins[t2]) {
                    should_swap = 1;
                } 
                else if (league_wins[t1] == league_wins[t2]) {
                    // Critério 3: Saldo de Gols (maior saldo fica acima)
                    s1 = (int)league_goals_for[t1] - (int)league_goals_agst[t1];
                    s2 = (int)league_goals_for[t2] - (int)league_goals_agst[t2];
                    if (s1 < s2) {
                        should_swap = 1;
                    }
                }
            }

            // Executa a troca de posições no ranking
            if (should_swap) {
                temp = league_rank[j];
                league_rank[j] = league_rank[j + 1];
                league_rank[j + 1] = temp;
            }
        }
    }
}

/*
static void sort_league_table(void) {
    unsigned char i, j, temp;
    unsigned char r1, r2, t1, t2; 
    int s1, s2; 

    // 1. Inicializa o ranking com as POSIÇÕES (0 a N) do array league_active_teams
    for (i = 0; i < league_total_teams; i++) {
        league_rank[i] = i; 
    }

    // 2. Bubble Sort
    for (i = 0; i < league_total_teams - 1; i++) {
        for (j = 0; j < league_total_teams - i - 1; j++) {
            
            r1 = league_rank[j];
            r2 = league_rank[j + 1];
            
            // BUSCA O ID REAL DO TIME (0-11) PARA COMPARAR OS DADOS
            t1 = league_active_teams[r1];
            t2 = league_active_teams[r2];

            // Critério 1: Pontos
            if (league_points[t1] < league_points[t2]) goto swap;
            if (league_points[t1] > league_points[t2]) continue;

            // Critério 2: Vitórias
            if (league_wins[t1] < league_wins[t2]) goto swap;
            if (league_wins[t1] > league_wins[t2]) continue;

            // Critério 3: Saldo de Gols
            s1 = (int)league_goals_for[t1] - (int)league_goals_agst[t1];
            s2 = (int)league_goals_for[t2] - (int)league_goals_agst[t2];
            if (s1 < s2) goto swap;
            
            continue;

        swap:
            temp = league_rank[j];
            league_rank[j] = league_rank[j + 1];
            league_rank[j + 1] = temp;
        }
    }
}
*/

static void simulate_cpu_match(void) {
    unsigned char h = league_fixtures[current_fixture_idx].home; // ID do Mandante
    unsigned char a = league_fixtures[current_fixture_idx].away; // ID do Visitante
    unsigned char gh, ga;
	
	gh = 0; ga = 0;

    // 1. NÃO reatribuímos player_team[]. Eles ficam onde estavam no menu.
    // Forçamos uma nova semente a cada jogo simulado usando o Raster
   
	
    // Simulamos 6 "momentos de perigo" no jogo
    for(i = 0; i < 5; i++) {
		 srand(*(unsigned char*)0xD41B + league_entropy + VIC.rasterline);
        // Se o time mandante vencer o sorteio baseado na força dele
        // Adicionamos +2 na dificuldade para garantir que o 0x0 seja raro
        if ((rand() % 10) < (team_difficulty[h] + 2)) {
            gh++;
        }
        // O mesmo para o visitante
        if ((rand() % 10) < (team_difficulty[a] + 1)) {
            ga++;
        }
    }

    // 3. Agora distribuímos esses gols nos slots certos (S1..S4)
    // Se o time mandante (h) for o time que está no Slot 1 (Esquerda)
    if (player_team[1] == h) {
        s1_final = gh; s2_final = 0; // Lado A = Mandante
        s3_final = ga; s4_final = 0; // Lado B = Visitante
    } else {
        // Se o mandante não está no Slot 1, ele obrigatoriamente está no Slot 3 (Direita)
        // porque em um jogo de liga, um dos dois times TEM que ser o P1 ou P3 original.
        s1_final = ga; s2_final = 0; // Lado A = Visitante
        s3_final = gh; s4_final = 0; // Lado B = Mandante
    }

    // Atualiza as globais de placar para a tela de fim de jogo
    gols_time_a = s1_final + s2_final;
    gols_time_b = s3_final + s4_final;
}

static void show_last_match_result(void) {
    if (last_h_idx == 255) return; // Se não houve jogo ainda, não mostra nada

    put_text(1, 19, (const unsigned char*)"LAST:");
    put_text(06, 19, (const unsigned char*)team_names[last_h_idx]);
    put_num(20, 19, last_h_score);
    put_text(22, 19, (const unsigned char*)"-");
    put_num(23, 19, last_a_score);
    put_text(26, 19, (const unsigned char*)team_names[last_a_idx]);
}


static void show_league_status(void) {
    unsigned char games_per_round = league_total_teams / 2;
    unsigned char rounds_per_turn = league_total_teams - 1;
    unsigned char current_round_display;
 //   unsigned char current_turn_display;

    // Calculando o Turno (1 ou 2)
    if (current_fixture_idx < (total_league_games / 2)) {
        current_turn_display = 1;
    } else {
        current_turn_display = 2;
    }

    // Calculando a Rodada (1 a N)
    // O +1 é porque o índice começa em 0, mas para o humano a rodada 1 é a primeira.
    current_round_display = (current_fixture_idx / games_per_round) + 1;

    // Se for o segundo turno, queremos que a rodada resete para 1 ou continue?
    // Geralmente em ligas de 2 turnos, a rodada volta a 1 no Turno 2:
    if (current_turn_display == 2 && league_total_turns == 2) {
        current_round_display -= rounds_per_turn;
    }

    // Exibição na tela (ajuste a linha conforme sua UI)
    if (league_total_turns == 2) {
        put_text(10, 17, (const unsigned char*)"TURN ");
        put_num(15, 17, current_turn_display);
        put_text(18, 17, (const unsigned char*)"- ROUND ");
        put_num(26, 17, current_round_display);
    } else {
        put_text(12, 17, (const unsigned char*)"ROUND ");
        put_num(18, 17, current_round_display);
    }
}


static void show_league_table(void) {
    unsigned char i, t_idx;
    

    sort_league_table(); 

    // Limpeza de tela (VIC configurado)
    fast_clrscr(1);
    clear_screen_c000_with_color(COLOR_WHITE);
        
    put_text(1, 1, (const unsigned char*)"#  TEAM          P   W   GF  GA");
    put_text(1, 2, (const unsigned char*)"-------------------------------");

    for (i = 0; i < league_total_teams; i++) {
        // O league_rank[i] diz qual posição do active_teams está em 1º, 2º...
        t_idx = league_active_teams[league_rank[i]]; 
        
        if (is_human_team[t_idx]) {
            put_text(0, 3+i, (const unsigned char*)"*");
        }
        
        put_num(1, 3+i, i + 1); 
        put_text(4, 3+i, (const unsigned char*)team_names[t_idx]); 
        
        put_num(18, 3+i, league_points[t_idx]);
        put_num(22, 3+i, league_wins[t_idx]);
        put_num(26, 3+i, league_goals_for[t_idx]);
        put_num(30, 3+i, league_goals_agst[t_idx]);
    }
}

void show_next_match_info(void) {
    unsigned char h = league_fixtures[current_fixture_idx].home;
    unsigned char a = league_fixtures[current_fixture_idx].away;

    put_text(1, 21, (const unsigned char*)"NEXT:");
    put_text(6, 21, (const unsigned char*)team_names[h]);
    put_text(20, 21, (const unsigned char*)"VS");
    put_text(24, 21, (const unsigned char*)team_names[a]);
}

static void league_manager_loop(void) {
 //   unsigned char original_player_type[5];
    unsigned char i, j;
 //   unsigned int high_score;
    unsigned char h_idx, a_idx;
    unsigned char base_difficulty;
    unsigned char play_again = 1;
    unsigned char replay_choice = 1;
    unsigned char joy_pressed = 0;
	unsigned char liga_home = h_idx; 
    unsigned char liga_away = a_idx;

    // 1. Backup do Menu
    base_difficulty = difficulty_level;
    for(i = 1; i <= 4; i++) {
        original_player_type[i] = player_type[i];
    }
    
    while (play_again) {
        reset_league(); 
        
        while (current_fixture_idx < total_league_games) {
            VIC.bordercolor = 0;
            VIC.bgcolor0 = 0;
            fast_clrscr(1);
            clear_screen_c000_with_color(COLOR_WHITE);
            
            h_idx = league_fixtures[current_fixture_idx].home;
            a_idx = league_fixtures[current_fixture_idx].away;
			liga_home = h_idx; 
            liga_away = a_idx;
            
// --- NOVA LÓGICA DE ATRIBUIÇÃO DE LADOS E ATIVAÇÃO ---
            for(i = 1; i <= 4; i++) player_type[i] = 0;

            if (original_player_type[1] == 1 && (player_team[1] == h_idx || player_team[1] == a_idx)) {
                if (player_team[1] == a_idx) {
                    home_team = a_idx; 
                    away_team = h_idx;
                } else {
                    home_team = h_idx;
                    away_team = a_idx;
                }
                player_type[1] = original_player_type[1];
                player_type[2] = original_player_type[2];
                
                if (player_team[3] == away_team || player_team[4] == away_team) {
                    player_type[3] = original_player_type[3];
                    player_type[4] = original_player_type[4];
                }
            } 
            else if (original_player_type[3] == 1 && (player_team[3] == h_idx || player_team[3] == a_idx)) {
                if (player_team[3] == h_idx) {
                    home_team = a_idx; 
                    away_team = h_idx;
                } else {
                    home_team = h_idx;
                    away_team = a_idx;
                }
                player_type[3] = original_player_type[3];
                player_type[4] = original_player_type[4];
            }
            else {
                home_team = h_idx;
                away_team = a_idx;
            }

            // --- INFORMAÇÕES PRÉ-JOGO ---
            show_league_table();
            show_league_status();
            show_next_match_info(); 
            show_last_match_result(); 
            
            put_text(10, 23, (const unsigned char*)"PRESS FIRE TO START");
            while(!(~CIA1.pra & 0x10)) {
				// Forçamos uma nova semente a cada jogo simulado usando o Raster
				srand(*(unsigned char*)0xD41B + VIC.rasterline);		
			}
            for(i=0; i<15; i++) wait_vblank();

            game_mode = 8;
            
            // --- AJUSTE DE DIFICULDADE ---
            if (base_difficulty == 0) difficulty_level = rand() % 2; 
            else if (base_difficulty == 1) difficulty_level = 1; 
            else difficulty_level = 2;

            // --- EXECUÇÃO DA PARTIDA ---
            if (is_human_team[home_team] || is_human_team[away_team]) {
                num_players = 4; 
				sorteia_personalidades();
	//			show_match_announcement(home_team, away_team);
                play_match(); 
          //      display_final_score_individual_v5();
				// 2. A CORREÇÃO: Verificar se houve inversão para salvar o placar certo
    last_h_idx = liga_home; 
    last_a_idx = liga_away;

    if (home_team == liga_home) {
        // Não houve inversão: Home da liga jogou como Home no motor
        last_h_score = gols_time_a; 
        last_a_score = gols_time_b;
    } else {
        // HOUVE INVERSÃO: Home da liga jogou como Away no motor (P3/P4 no lado esquerdo)
        // Invertemos os scores para que a tabela da liga receba os valores corretos
        last_h_score = gols_time_b; 
        last_a_score = gols_time_a;
    }
	display_final_score_individual_v5();
     //           last_h_idx = home_team; last_a_idx = away_team;
     //           last_h_score = gols_time_a; 
     //           last_a_score = gols_time_b;
            } else {
                simulate_cpu_match(); 
                last_h_idx = home_team; last_a_idx = away_team;
                last_h_score = gols_time_a;
                last_a_score = gols_time_b;
            }

            update_league_results(current_fixture_idx);
			sort_league_table();
            current_fixture_idx++;
			
			// --- NOVO BLOCO: Pausa para ver o último resultado ---
            if (current_fixture_idx == total_league_games) {
                // Estamos no último jogo! Precisamos mostrar a tabela e o LAST MATCH
                // antes de ir para a tela de "LEAGUE FINISHED"
                fast_clrscr(1);
                clear_screen_c000_with_color(COLOR_WHITE);
                
                show_league_table();      // Mostra a classificação final
                show_league_status();
                show_last_match_result(); // Mostra o placar do último jogo (ex: 10 vs 7)
                // Na tela de "FINAL MATCH DONE", você pode fazer isso:
// Na tela de "FINAL MATCH DONE"
for(j = 0; j < 10; j++) { // Pisca 10 vezes
    
    // 1. Pinta a linha inteira (ou os 15 primeiros caracteres do nome) de BRANCO
    for(i = 0; i < 14; i++) {
        ((unsigned char*)0xD800)[3 * 40 + 4 + i] = 1; // 1 = Branco
    }
    for(i = 0; i < 8; i++) wait_vblank(); // Pausa curta

    // 2. Pinta a mesma linha de AMARELO
    for(i = 0; i < 14; i++) {
        ((unsigned char*)0xD800)[3 * 40 + 4 + i] = 7; // 7 = Amarelo
    }
	
    for(i = 0; i < 8; i++) wait_vblank(); // Pausa curta
}

                put_text(10, 23, (const unsigned char*)"FINAL MATCH DONE!");
                put_text(8, 24, (const unsigned char*)"PRESS FIRE TO FINISH");
                
                // Espera o Fire para o jogador absorver o resultado
                while(!(~CIA1.pra & 0x10)); 
                for(i=0; i<20; i++) wait_vblank();
            }
			
			
			
        } // Fim do while (current_fixture_idx...)

        // --- FIM DA LIGA ---
		sort_league_table();
   
       champion_idx = league_active_teams[league_rank[0]];
		

        fast_clrscr(1);
        clear_screen_c000_with_color(COLOR_WHITE);
        show_league_table();   
        put_text(12, 21, (const unsigned char*)"LEAGUE FINISHED!");
        put_text(8, 23, (const unsigned char*)"PRESS FIRE TO CEREMONY");

        while(~CIA1.pra & 0x10); 
        while(!(~CIA1.pra & 0x10)); 
        for(i=0; i<20; i++) wait_vblank();
        
        show_tournament_champion(champion_idx);
        
        // --- SELEÇÃO DE REPLAY ---
        replay_choice = 1; joy_pressed = 0;
        while(1) {
            put_text(10, 20, (const unsigned char*)"LEAGUE FINISHED!");
            put_text(10, 21, (const unsigned char*)"PLAY AGAIN?");
            
            if (replay_choice == 1) {
                put_text(14, 23, (const unsigned char*)"[ YES ]");
                put_text(22, 23, (const unsigned char*)"   NO   ");
            } else {
                put_text(14, 23, (const unsigned char*)"  YES  ");
                put_text(22, 23, (const unsigned char*)"[ NO  ]");
            }

            if (!((~CIA1.pra) & 0x0C)) joy_pressed = 0;
            if (!joy_pressed) {
                if ((~CIA1.pra) & 0x04) { replay_choice = 1; joy_pressed = 1; }
                if ((~CIA1.pra) & 0x08) { replay_choice = 0; joy_pressed = 1; }
            }

            if ((~CIA1.pra) & 0x10) {
                play_again = replay_choice;
                if (play_again == 1) {
                    reset_league(); // <--- CHAME AQUI! Agora a nova liga começa limpa.
                }

                for(i = 0; i < 25; i++) wait_vblank(); 
                break;
            }
            wait_vblank();
        }
        if (play_again == 0) break;
    } 
    difficulty_level = base_difficulty;
}


static void reset_league(void) {
    unsigned char i;
    
    // 1. Voltamos o cursor para o primeiro jogo da lista de confrontos (fixtures)
    current_fixture_idx = 0; 
	league_entropy = 0;

    // 2. LIMPAMOS OS DADOS DE TODOS OS 12 TIMES POSSÍVEIS
    // Isso garante que, não importa qual time foi sorteado na primeira vez,
    // os pontos dele agora serão zero.
    for (i = 0; i < 20; i++) {
        league_points[i] = 0;
        league_wins[i] = 0;
        league_goals_for[i] = 0;
        league_goals_agst[i] = 0;
    }
    
    // 3. Resetamos o ranking para a ordem original (0, 1, 2, 3...)
    // Isso limpa a "bagunça" visual da tabela anterior
    for (i = 0; i < league_total_teams; i++) {
        league_rank[i] = i; 
    }

    // 4. Limpamos os dados da interface e placares globais
    last_h_idx = 255; 
    last_a_idx = 255;
    last_h_score = 0;
    last_a_score = 0;
    gols_time_a = 0;
    gols_time_b = 0;

    // 5. IMPORTANTE: Reiniciar o status de "já jogado" de cada partida
    // Se não fizer isso, o loop da liga vai achar que todos os jogos já aconteceram
    for (i = 0; i < total_league_games; i++) {
        league_fixtures[i].played = 0;
    }
	
	 for(i = 1; i <= 4; i++) {
        player_type[i] = original_player_type[i];
    }
}


static void init_league(void) {
    unsigned char i;
    
    // 1. Limpa as tabelas de todos os times possíveis
    for (i = 0; i < 20; i++) {
        league_points[i] = 0;
        league_goals_for[i] = 0;
        league_goals_agst[i] = 0;
        league_wins[i] = 0;
        // is_human_team e team_difficulty já foram definidos no show_menu
    }

    current_round = 0;
	champion_idx = 0;
  //  high_score = 0;
    
    // 2. Gera os jogos (Fixtures)
    generate_league_fixtures();
//	init_human_sprites();
}

static void generate_league_fixtures(void) {
    unsigned char round, i, j, t1, t2, temp, r, r_temp;
    static unsigned char all_teams[20]; // Pool de sobra
    unsigned char teams[12];     // Lista final da liga
    unsigned char selected_count = 0;
    unsigned char f_idx = 0;
    unsigned char already_in;


	
    // 1. PRIMEIRO: Adiciona os times que os humanos escolheram
    for (i = 1; i <= 4; i++) {
        if (player_type[i] == 1) { // Se o slot é Humano
            // Verifica se este time já não foi adicionado (evita duplicar se 2 players escolherem o mesmo time)
            already_in = 0;
            for (j = 0; j < selected_count; j++) {
                if (teams[j] == player_team[i]) { already_in = 1; break; }
            }
            
            if (!already_in && selected_count < league_total_teams) {
                teams[selected_count] = player_team[i];
                selected_count++;
            }
        }
    }

    // 2. SEGUNDO: Prepara a lista de times que SOBRARAM
    temp = 0; // Usado como contador de sobra aqui
    for (i = 0; i < 20; i++) {
        already_in = 0;
        for (j = 0; j < selected_count; j++) {
            if (teams[j] == i) { already_in = 1; break; }
        }
        if (!already_in) {
            all_teams[temp] = i;
            temp++;
        }
    }

    // 3. TERCEIRO: Embaralha apenas os times que sobraram (Fisher-Yates na sobra)
    if (temp > 1) {
        for (i = temp - 1; i > 0; i--) {
            r = rand() % (i + 1);
            r_temp = all_teams[i]; // Use uma variável local r_temp ou reuse 't1'
            all_teams[i] = all_teams[r];
            all_teams[r] = r_temp;
        }
    }

    // 4. QUARTO: Completa as vagas restantes da liga com os sorteados
    while (selected_count < league_total_teams && temp > 0) {
        teams[selected_count] = all_teams[temp - 1];
        selected_count++;
        temp--;
    }

	// --- ADICIONE ESTE TRECHO AQUI ---
    // Salva os times sorteados na global para a tabela saber quem exibir
    for (i = 0; i < league_total_teams; i++) {
        league_active_teams[i] = teams[i];
    }
	
    // --- AGORA GERA OS CONFRONTOS (ROUND ROBIN) ---
    // (A partir daqui o código é idêntico ao seu original)
    for (round = 0; round < (league_total_teams - 1); round++) {
        for (i = 0; i < (league_total_teams / 2); i++) {
            t1 = teams[i];
            t2 = teams[league_total_teams - 1 - i];
            
            league_fixtures[f_idx].home = t1;
            league_fixtures[f_idx].away = t2;
            league_fixtures[f_idx].played = 0;
            f_idx++;
        }
        // Rotação simples
        temp = teams[league_total_teams - 1];
        for (i = league_total_teams - 1; i > 1; i--) teams[i] = teams[i-1];
        teams[1] = temp;
    }
    total_league_games = f_idx;

    if (league_total_turns == 2) {
        unsigned char t1_count = total_league_games;
        for (i = 0; i < t1_count; i++) {
            league_fixtures[f_idx].home = league_fixtures[i].away;
            league_fixtures[f_idx].away = league_fixtures[i].home;
            league_fixtures[f_idx].played = 0;
            f_idx++;
        }
        total_league_games = f_idx;
    }
}


static void display_final_score_individual_v5(void) {
    unsigned char winner = 0;
    unsigned char max_gols = 0;
    unsigned char is_tie = 0;
   // unsigned char total_match_goals;
    unsigned char row;
    unsigned char p_color;
	unsigned char gols_time_a = 0;
    unsigned char gols_time_b = 0;
	const unsigned char* name_ptr; // Declarada aqui no topo
    unsigned char i;               // Para o loop wait_end
//	unsigned char actual_opponent = tournament_opponents[tournament_stage];

    // 1. LIMPEZA TOTAL DA ÁREA (Cobre toda a largura para limpar Golden Score)
    for (row = 3; row < 22; row++) {
        put_text(5, row, (const unsigned char*)"                               ");
    }
	
	if (game_mode == 8) {
        put_text(13, 3, (const unsigned char*)"MATCH STATS");
        
        gols_time_a = s1_final + s2_final;
        gols_time_b = s3_final + s4_final;

        // --- TIME A (Home Team / Lado Esquerdo) ---
        // Pegamos a cor que foi enviada para o Sprite 0 (P1)
        

    if (current_turn_display == 1) {
       // --- TIME A (Home Team) ---
	   p_color = p1_color;
        name_ptr = (const unsigned char*)team_names[home_team];
        put_text(5, 7, name_ptr); 
        put_num(22, 7, gols_time_a);
        paint_row_color(7, 5, 16, p_color); // Cor dinâmica do seu time

        // --- TIME B (Away Team) ---
     
            // No modo friendly (Modo 7), usa o que foi escolhido no menu
            name_ptr = (const unsigned char*)team_names[away_team];
            // --- TIME B (Away Team / Lado Direito) ---
        // Pegamos a cor que foi enviada para o Sprite 2 (P3)
        // Isso já inclui o fallback de Amarelo/Vermelho ou Branco
        p_color = p3_color;
            //p_color = team_colors[away_team];
     
//        name_ptr = (const unsigned char*)team_names[away_team];
        put_text(5, 9, name_ptr); 
        put_num(22, 9, gols_time_b);
        paint_row_color(9, 5, 16, p_color); // Cor dinâmica do adversário
	}
	else
	{
		// --- TIME A (Home Team) ---
		p_color = p3_color;
        name_ptr = (const unsigned char*)team_names[away_team];
        put_text(5, 7, name_ptr); 
        put_num(22, 7, gols_time_b);
        paint_row_color(7, 5, 16, p_color); // Cor dinâmica do seu time

        // --- TIME B (Away Team) ---
     
            // No modo friendly (Modo 7), usa o que foi escolhido no menu
            name_ptr = (const unsigned char*)team_names[home_team];
            // --- TIME B (Away Team / Lado Direito) ---
        // Pegamos a cor que foi enviada para o Sprite 2 (P3)
        // Isso já inclui o fallback de Amarelo/Vermelho ou Branco
        p_color = p1_color;
            //p_color = team_colors[away_team];
     
//        name_ptr = (const unsigned char*)team_names[away_team];
        put_text(5, 9, name_ptr); 
        put_num(22, 9, gols_time_a);
        paint_row_color(9, 5, 16, p_color); // Cor dinâmica do adversário
	}

        put_text(4, 12, (const unsigned char*)"-----------------------------------");
        
        if (gols_time_a > gols_time_b) {
            // Se o jogador venceu
            put_text(11, 14, (const unsigned char*)team_names[home_team]);
            p_color = p1_color;
            put_text(11, 15, (const unsigned char*)"  WINS!"); // Ajuste o offset se o nome for longo
            paint_row_color(14, 11, 19, p_color);
        } else if (gols_time_b > gols_time_a) {
            // Se o adversário venceu
            put_text(11, 14, (const unsigned char*)team_names[away_team]);
            p_color = p3_color;
            put_text(11, 15, (const unsigned char*)"  WINS!");
            paint_row_color(14, 11, 19, p_color);
        } else {
            put_text(13, 14, (const unsigned char*)"DRAW GAME!");
        }
        goto wait_end;
    }
    

    put_text(13, 3, (const unsigned char*)"MATCH STATS");

    // --- LISTA DE ARTILHEIROS (Nomes completos e cores dinâmicas) ---
    
    // PLAYER 1 (Sprite 0)
    p_color = get_fixed_player_color(1);
    put_text(5, 6, (const unsigned char*)"PLAYER 1  :"); put_num(18, 6, s1_final);
    paint_row_color(6, 5, 15, p_color);

    max_gols = s1_final;
    winner = 1;
    is_tie = 0;

    // PLAYER 2 (Sprite 1)
    if (num_players >= 2 && player_type[2] != 2) {
        p_color = get_fixed_player_color(2);
        put_text(5, 7, (const unsigned char*)"PLAYER 2  :"); put_num(18, 7, s2_final);
        paint_row_color(7, 5, 15, p_color);
        if (s2_final > max_gols) { max_gols = s2_final; winner = 2; is_tie = 0; }
        else if (s2_final == max_gols) is_tie = 1;
    }

    // PLAYER 3 (Sprite 2)
    if (num_players >= 3) {
        p_color = get_fixed_player_color(3);
        put_text(5, 8, (const unsigned char*)"PLAYER 3  :"); put_num(18, 8, s3_final);
        paint_row_color(8, 5, 15, p_color);
        if (s3_final > max_gols) { max_gols = s3_final; winner = 3; is_tie = 0; }
        else if (s3_final == max_gols) is_tie = 1;
    }

    // PLAYER 4 (Sprite 4)
    if (num_players == 4) {
        p_color = get_fixed_player_color(4);
        put_text(5, 9, (const unsigned char*)"PLAYER 4  :"); put_num(18, 9, s4_final);
         if (field_type == 1) { 
        // CAMPO MOLHADO (WET)
            p_color = 1; // Cor branca
         }    
        paint_row_color(9, 5, 15, p_color);
        if (s4_final > max_gols) { max_gols = s4_final; winner = 4; is_tie = 0; }
        else if (s4_final == max_gols) is_tie = 1;
    }

    // --- ANÚNCIO FINAL ---
    if (max_gols == 0) {
        put_text(10, 20, (const unsigned char*)"NO GOALS SCORED   ");
    } else if (is_tie) {
        put_text(13, 20, (const unsigned char*)"DRAW GAME!");
    } else {
        put_text(8, 20, (const unsigned char*)"TOP SCORER: PLAYER ");
        put_num(27, 20, winner);
        p_color = get_fixed_player_color(winner);
        paint_row_color(20, 8, 21, p_color);
    }

wait_end:
    for(i=0; i<250; i++) wait_vblank();
}


static void highlight_line(unsigned char x, unsigned char y, unsigned char length, unsigned char color) {
    unsigned int offset = y * 40 + x;
    unsigned char* color_ptr = (unsigned char*)(0xD800 + offset);
    
    while (length--) {
        *color_ptr = color;
        color_ptr++;
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
    put_text(12, 7, msg_titulo);
    put_text(4, 8, msg_credits);
    put_text(1, 9, msg_line);
    
   // Menu direto com apenas 2 opções
//    put_text(10, 11, (const unsigned char*)"1- 4 PLAYERS ALL VS ALL");
//    put_text(10, 12, (const unsigned char*)"2- 4 PLAYERS TOURNAMENT");
	put_text(10, 11, (const unsigned char*)"1- C64 LEAGUE");		
//	put_text(10, 12, (const unsigned char*)"2- FRIENDLY");
	put_text(10, 12, (const unsigned char*)"2- EXIT TO MENU");

    // Instrução movida para a linha 17 para dar respiro
    put_text(12, 17, (const unsigned char*)"SELECT GAME MODE");
    
    // A linha divisória (msg_line) será desenhada pela show_menu 
    // na linha 18 conforme os submenus aparecem.
}

static void show_menu(void) {
    unsigned char menu_sel = 0;
    unsigned char old_sel = 255; // Força o redesenho na primeira execução
    unsigned char key;
    unsigned char joy1_now;
    unsigned char p_idx;
    unsigned char menu_level = 0; // 0 = Novo Menu, 1 = Outros Modos
    unsigned char i, joy_now, last_joy_state, joy_pressed; // joy_pressed adicionado
    unsigned char selected_count;
    unsigned char action; // Usado para agrupar as saídas do menu principal
    unsigned char c_str[2] = {0, 0}; // String buffer centralizado
    
    // Arrays para otimização do Field / Style / Difficulty (Economia enorme de ifs e RODATA)
    static const unsigned char* const f_type_sel[] = {(const unsigned char*)"< GRASS  > ", (const unsigned char*)"<  WET   >", (const unsigned char*)"< FROZEN >", (const unsigned char*)"< RANDOM >"};
    static const unsigned char* const f_type_unsel[] = {(const unsigned char*)"  GRASS   ", (const unsigned char*)"   WET    ", (const unsigned char*)"  FROZEN  ", (const unsigned char*)"  RANDOM  "};
    static const unsigned char* const f_style_sel[] = {(const unsigned char*)"< CHECKER >", (const unsigned char*)"<  BARS  >", (const unsigned char*)"<  SOLID  >", (const unsigned char*)"< RANDOM  >"};
    static const unsigned char* const f_style_unsel[] = {(const unsigned char*)" CHECKER ", (const unsigned char*)"   BARS  ", (const unsigned char*)"   SOLID ", (const unsigned char*)"  RANDOM "};
    static const unsigned char* const diff_strs[] = {(const unsigned char*)"<  EASY  >", (const unsigned char*)"< NORMAL >", (const unsigned char*)"<  HARD  >"};
    static const unsigned char str_blank40[] = "                                        ";

    music_init();
    
    golden_score_triggered = 0;
    gold_goal_active = 0;
    difficulty_level = 0;
    
    refresh_menu_visuals();
    last_joy_state = ~CIA1.pra;
	
	clear_saved_sprites();
    
    // --- PARTE 1: MODO DE JOGO ---
    while(1) {
        music_play();
        wait_vblank();
        joy_now = ~CIA1.pra;
        joy1_now = ~CIA1.prb;
        joy_pressed = joy_now & ~last_joy_state; // Calcula apenas as teclas que acabaram de ser pressionadas
        
        key = 0; 
        
        // OTMIZAÇÃO: Só desenha as setas se menu_sel mudou
        if (menu_sel != old_sel) {
            if (old_sel != 255) put_text(8, 11 + old_sel, (const unsigned char*)" "); 
            put_text(8, 11 + menu_sel, (const unsigned char*)">");
            old_sel = menu_sel;
        }

        if ((joy_pressed & 1) && menu_sel > 0) menu_sel--;
        if ((joy_pressed & 2) && menu_sel < 1) menu_sel++;
        
        // Agrupamento de ações de saída para não repetir código do SID
        action = 0;
        if (joy_pressed & 0x10) action = (menu_sel == 0) ? 1 : 2;
        else if (key == '1')    action = 1;
        else if (key == '2')    action = 2;
        
        if (action) {
            music_stop();
            silence_sid();
            setup_sid();
            if (action == 1) {
                game_mode = 8;
                break;
            } else {
                chama_menu("m1"); 
                return;
            }
        }
        last_joy_state = joy_now;
    }
VIC.bordercolor = 0;
    // Modo 8 - LIGA
    if (game_mode == 8) {
        // Reutilizamos variáveis globais já existentes
        unsigned char team_taken[20];
        unsigned char is_paired[5];
        unsigned char can_human, partner; 
        unsigned char humans_assigned = 1; // P1 já é humano
        
        static unsigned char m_buf[] = "1P PARTNER:         <  >"; 
        
        num_players = 1; // Usando num_players como 'num_humans'
        
        // 1. Reset das estatísticas dos 16 times
        for(i = 0; i < 20; i++) {
            league_points[i] = 0;
            league_goals_for[i] = 0;
            league_goals_agst[i] = 0;
            league_wins[i] = 0;
            is_human_team[i] = 0;    
            team_taken[i] = 0;       
        }

        // 2. Reset do gerenciamento de progresso
        current_round = 0;
        current_fixture_idx = 0;     
        total_league_games = 0;      

        // 3. Reset de pareamento e slots (P1-P4)
        for(i = 0; i < 5; i++) {
            is_paired[i] = 0;
            partner_of[i] = 0;
            player_type[i] = 0;      
            player_team[i] = 255;    
        }
        
        put_text(1, 18, msg_line);
        
        // --- ETAPA A: NÚMERO DE HUMANOS ---
        old_sel = 255;
        last_joy_state = ~CIA1.pra;
        while(1) {
            wait_vblank(); 
            joy_now = ~CIA1.pra;
            joy_pressed = joy_now & ~last_joy_state;
            
            if (num_players != old_sel) {
                put_text(1, 19, (const unsigned char*)"HUMAN PLAYERS:         <   >");
                c_str[0] = num_players + '0';
                put_text(27, 19, c_str); 
                old_sel = num_players;
            }
            
            if ((joy_pressed & 8) && num_players < 4) num_players++;
            if ((joy_pressed & 4) && num_players > 1) num_players--;
            if (joy_pressed & 0x10) break;
            
            last_joy_state = joy_now;
        }

        // --- ETAPA B e C: REGRAS DE PAREAMENTO (C/H) ---
        selected_count = 0; 
        for(i=1; i<=4; i++) {
            player_type[i] = 0; 
            is_paired[i] = 0;
            partner_of[i] = 0;
        }

        // REGRA 1: Se são 4 jogadores, pareamento é fixo
        if (num_players == 4) {
            for(i=1; i<=4; i++) {
                player_type[i] = 1;
                is_paired[i] = 1;
            }
            partner_of[1]=2; partner_of[2]=1;
            partner_of[3]=4; partner_of[4]=3;
            selected_count = 4;
        }

        for(p_idx = 1; p_idx <= 4; p_idx++) {    
            if (is_paired[p_idx] && player_team[p_idx] != 255) continue;
            if (selected_count >= num_players && num_players < 4) {
                 if (!is_paired[p_idx]) break; 
            }

            // --- SUB-ETAPA: MENU DE PARTNER ---
            if (!is_paired[p_idx] && num_players < 4) {
                player_type[p_idx] = 1; 
                selected_count++; 
                
                can_human = (p_idx == 1 && num_players >= 2); 
                menu_sel = 0; 
                old_sel = 255;
                last_joy_state = ~CIA1.pra; 
                
                while(1) {
                    wait_vblank(); 
                    joy_now = ~CIA1.pra;
                    joy_pressed = joy_now & ~last_joy_state;
                    
                    if (menu_sel != old_sel) {
                        put_text(1, 20, (const unsigned char*)" P PARTNER:             <   >           ");
                        highlight_line(1, 20, 10, 7);
                        c_str[0] = p_idx + '0';
                        put_text(2, 20, c_str); 
                        put_text(26, 20, (menu_sel == 0) ? (const unsigned char*)"C" : (const unsigned char*)"H");
                        old_sel = menu_sel;
                    }

                    if (joy_pressed & 0x0C) {
                        if (can_human) menu_sel = 1 - menu_sel;
                        else menu_sel = 0; 
                    }

                    if (joy_pressed & 0x10) {
                        if (menu_sel == 1) {
                            partner = 2;
                            player_type[partner] = 1;
                            selected_count++; 
                        } else {
                            if (p_idx == 1) {
                                partner = 2; 
                                player_type[partner] = 0; 
                                if (num_players == 3) {
                                    player_type[3] = 1; player_type[4] = 1;
                                    partner_of[3] = 4; partner_of[4] = 3;
                                    is_paired[3] = 1; is_paired[4] = 1;
                                    selected_count = 3;
                                }
                            } else {
                                partner = 4;
                                player_type[partner] = 0; 
                            }
                        }
                        is_paired[p_idx] = 1;
                        is_paired[partner] = 1;
                        partner_of[p_idx] = partner;
                        partner_of[partner] = p_idx;
                        highlight_line(1, 20, 10, 1);
                        break; 
                    }
                    last_joy_state = joy_now;
                }
            }

            // --- SELEÇÃO DE TIME ---
            put_text(1, 21, str_blank40); // Utilizando a constante limpa-linha
            home_team = 0; 
            while(team_taken[home_team] || home_team == player_team[1]) home_team++;
            
            old_sel = 255; 
            last_joy_state = ~CIA1.pra; 

            while(1) {
                wait_vblank(); 
                joy_now = ~CIA1.pra;
                joy_pressed = joy_now & ~last_joy_state;

                if (home_team != old_sel) {
                    put_text(1, 21, (const unsigned char*)" P  TEAM:       <             >         ");
                    highlight_line(1, 21, 10, 7);
                    c_str[0] = p_idx + '0';            
                    put_text(2, 21, c_str); 
                    put_text(18, 21, (const unsigned char*)team_names[home_team]);
                    old_sel = home_team;
                }

                // Navegação com do-while iterativo e limpo
                if (joy_pressed & 0x0C) {
                    do {
                        if (joy_now & 0x08) home_team = (home_team + 1) % 20; 
                        else home_team = (home_team + 15) % 20;              
                    } while(team_taken[home_team]);
                }

                if (joy_pressed & 0x10) {
                    team_taken[home_team] = 1;
                    is_human_team[home_team] = 1;
                    player_team[p_idx] = home_team;
                    if (partner_of[p_idx] > 0) {
                        player_team[partner_of[p_idx]] = home_team;
                    }
                    highlight_line(1, 21, 10, 1);
                    break;
                }
                last_joy_state = joy_now;
            }
        }

        // --- ETAPA D: NÚMERO DE TIMES NA LIGA ---
        for(i=0; i<15; i++) wait_vblank();
        old_sel = 255;
        last_joy_state = ~CIA1.pra;
        put_text(1, 21, str_blank40);
        
        while(1) {
            wait_vblank(); 
            joy_now = ~CIA1.pra;
            joy_pressed = joy_now & ~last_joy_state;
            
            if (league_total_teams != old_sel) {
                put_text(1, 21, (const unsigned char*)"LEAGUE TEAMS:          <  >");
                put_num(25, 21, league_total_teams); 
                old_sel = league_total_teams;
            }
            if ((joy_pressed & 8) && league_total_teams < 12) league_total_teams += 2;
            if ((joy_pressed & 4) && league_total_teams > 4)  league_total_teams -= 2;
            if (joy_pressed & 0x10) break;
            
            last_joy_state = joy_now;
        }

        // --- ETAPA E: NÚMERO DE TURNOS ---
        for(i=0; i<15; i++) wait_vblank();
        old_sel = 255;
        last_joy_state = ~CIA1.pra;
        
        while(1) {
            wait_vblank(); 
            joy_now = ~CIA1.pra;
            joy_pressed = joy_now & ~last_joy_state;
            
            if (league_total_turns != old_sel) {
                put_text(1, 22, (const unsigned char*)"LEAGUE TURNS:          <  >");
                c_str[0] = league_total_turns + '0'; 
                put_text(26, 22, c_str); 
                old_sel = league_total_turns;
            }
            if (joy_pressed & 12) {
                league_total_turns = (league_total_turns == 1) ? 2 : 1;
            }
            if (joy_pressed & 0x10) break;
            
            last_joy_state = joy_now;
        }

        // --- FINALIZAÇÃO ---
        for(i=0; i < league_total_teams; i++) {
            srand(*(unsigned char*)0xD41B + VIC.rasterline);
            if (!is_human_team[i]) {
                team_difficulty[i] = (rand() % 4) + 1;
            }
        }

        for(i = 0; i < 20; i++) is_human_team[i] = 0;
        
        for(i = 1; i <= 4; i++) {
            if (player_type[i] == 1 && player_team[i] != 255) {
                is_human_team[player_team[i]] = 1;
            }
        }
    }
        
//    put_text(1, 20, str_blank40);
     put_text(1, 21, msg_line);
     // --- PARTE 2: ZAGUEIRO ---
 //   put_text(1, 20, msg_line);
    put_text(1, 22, (const unsigned char*)"USE DEFENDER:  ");
    menu_sel = 0; old_sel = 255;
    last_joy_state = ~CIA1.pra;

    while(1) {
        wait_vblank();
        joy_now = ~CIA1.pra;

        if (menu_sel != old_sel) {
            // Usa setas para indicar a escolha
            put_text(15, 22, (menu_sel == 0) ? (const unsigned char*)"> NO " : (const unsigned char*)"  NO ");
            put_text(21, 22, (menu_sel == 1) ? (const unsigned char*)"> YES  " : (const unsigned char*)"  YES  ");
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

    // --- PARTE 3: CONFIGURAÇÃO DO CAMPO ---
 //   put_text(1, 21, msg_line);
    put_text(1, 22, (const unsigned char*)"FIELD: ");
    put_text(18, 22, (const unsigned char*)" STYLE: ");
    
    // Sub-etapa A: TIPO (PISO) - Utilizando array de strings!
    selected_field_option = 0; old_sel = 255;
    last_joy_state = ~CIA1.pra;
    while(1) {
        wait_vblank(); 
        joy_now = ~CIA1.pra;
        joy_pressed = joy_now & ~last_joy_state;
        
        if (selected_field_option != old_sel) {
            put_text(7, 22, f_type_sel[selected_field_option]);
            put_text(26, 22, f_style_unsel[field_style]);
            old_sel = selected_field_option;
        }
        if ((joy_pressed & 4) && selected_field_option > 0) selected_field_option--;
        if ((joy_pressed & 8) && selected_field_option < 3) selected_field_option++;
        if (joy_pressed & 0x10) break;
        
        last_joy_state = joy_now;
    }

    // Sub-etapa B: ESTILO (PADRÃO)
    field_style = 0; old_sel = 255;
    last_joy_state = ~CIA1.pra;
    while(1) {
        wait_vblank(); 
        joy_now = ~CIA1.pra;
        joy_pressed = joy_now & ~last_joy_state;
        
        if (field_style != old_sel) {
            put_text(7, 22, f_type_unsel[selected_field_option]);
            put_text(25, 22, f_style_sel[field_style]);
            old_sel = field_style;
        }
        if ((joy_pressed & 4) && field_style > 0) field_style--;
        if ((joy_pressed & 8) && field_style < 3) field_style++;
        if (joy_pressed & 0x10) break;
        
        last_joy_state = joy_now;
    }

    // --- PARTE 4: DIFICULDADE (CARROSSEL) ---
    put_text(1, 24, (const unsigned char*)"DIFFICULTY:   < NORMAL >");
    difficulty_level = 1; old_sel = 255;
    last_joy_state = ~CIA1.pra;
    
    while(1) {
        wait_vblank(); 
        joy_now = ~CIA1.pra;
        joy_pressed = joy_now & ~last_joy_state;
        
        if (difficulty_level != old_sel) {
            put_text(15, 24, (const unsigned char*)"        "); 
            put_text(15, 24, diff_strs[difficulty_level]);
            old_sel = difficulty_level;
        }
        
        if ((joy_pressed & 4) && difficulty_level > 0) difficulty_level--;
        if ((joy_pressed & 8) && difficulty_level < 2) difficulty_level++;
        if (joy_pressed & 0x10) break;
        
        last_joy_state = joy_now;
    }
    
    // Finalização
    if (game_mode == 8) {
        play_sfx_whistle();
        init_league(); 
        return; 
    } else {
        play_sfx_whistle();
    }
}






// Funcionando bem
#ifndef ABS_DIFF
#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))
#endif

static void move_defender_ia(unsigned char is_south) {
    static unsigned char defends_north, moving, wait_val, dist_y, dist_x;
    static unsigned int curr_x, target_x;
    static unsigned char curr_y, target_y;
    static unsigned char bola_fora, forca, near_midfield;
    static unsigned char touch_1, touch_2;

    // 1. LEITURA E CACHE OTIMIZADO
    if (is_south) {
        curr_x = dx_b;
        curr_y = (unsigned char)dy_b;
        wait_val = defender_wait_b;
        defends_north = (current_half == 2);
        touch_1 = 3;
        touch_2 = 4;
    } else {
        curr_x = dx;
        curr_y = (unsigned char)dy;
        wait_val = defender_wait;
        defends_north = (current_half == 1);
        touch_1 = 1;
        touch_2 = 2;
    }

    // 2. EARLY EXIT (Saída rápida para poupar CPU massivamente)
    if (goal_delay > 0 || wait_val > 0) {
        if (wait_val > 0) wait_val--;
        
        if (is_south) {
            defender_wait_b = wait_val;
            p6_is_moving = 0;
        } else {
            defender_wait = wait_val;
            p5_is_moving = 0;
        }
        return; // Sai da função, ignorando todo o resto!
    } 

    moving = 0; // Reseta flag de movimento

    // Evitando ternários (CC65 processa ifs puros de forma muito mais enxuta)
    if (defends_north) {
        bola_fora = (by > 200);
        target_y = 80; // Reaproveitamos target_y para atuar como return_y
    } else {
        bola_fora = (by < 90);
        target_y = 192;
    }

    // 3. ESTRUTURA PRINCIPAL
    if (bola_fora || (ball_possession && (last_touch == touch_1 || last_touch == touch_2))) {
        // Ao invés de avaliar se mudou no final, setamos a flag de movimento na própria ação
        if (curr_x < 160) { curr_x++; moving = 1; } 
        else if (curr_x > 160) { curr_x--; moving = 1; }
        
        if (curr_y < target_y) { curr_y++; moving = 1; } 
        else if (curr_y > target_y) { curr_y--; moving = 1; }
    } 
    else {
        // Cálculo absoluto inline (mais rápido que chamar macro)
        dist_y = (curr_y > by) ? (curr_y - by) : (by - curr_y);

        if (dist_y < IA_DIST_Y[difficulty_level] || (timer & IA_TIMER_MASK[difficulty_level]) == 0) {
            target_y = by;
            if (defends_north) {
                if (target_y > 146) target_y = 146; 
            } else {
                if (target_y < 146) target_y = 146; 
            }

            if (curr_x < bx) { curr_x++; moving = 1; } 
            else if (curr_x > bx) { curr_x--; moving = 1; }
            
            if (curr_y < target_y) { curr_y++; moving = 1; } 
            else if (curr_y > target_y) { curr_y--; moving = 1; }
        }

       if (kick_cooldown == 0) {
            if (dist_y < 12) {
                dist_x = (curr_x > bx) ? (curr_x - bx) : (bx - curr_x);
                if (dist_x < 12) {
                    
                    ball_possession = 0;
                    last_touch = is_south ? 6 : 5; 
                    
                    near_midfield = defends_north ? (curr_y > 116) : (curr_y < 176);
                    
                    // Reduzimos o "chutão" aleatório para 25% (rand() & 3 == 0) para focar em passes
                    if (near_midfield && ((rand() & 3) == 0)) {
                        bdx = (curr_x < 160) ? (2 + (rand() & 3)) : -(2 + (rand() & 3));
                        forca = IA_CHUTE_BASE[difficulty_level] + (rand() & IA_CHUTE_RAND[difficulty_level]) + 1;
                        bdy = defends_north ? forca : -(signed char)forca;
                    } else {
                        // === PASSE INTELIGENTE VETORIAL ===
                        
                        // 1. Define o alvo (o companheiro mais avançado no ataque)
                        if (!is_south) {
                            if (p2y > py) { target_x = p2x; target_y = p2y; } // p2 está mais avançado
                            else          { target_x = px;  target_y = py;  } // p1 está mais avançado
                        } else {
                            if (p4y < p3y) { target_x = p4x; target_y = p4y; }
                            else           { target_x = p3x; target_y = p3y; }
                        }
                        
                        // 2. Calcula velocidade X (Distância / 16 usando Bitshift LSR, super rápido)
                        if (target_x > curr_x) {
                            dist_x = (target_x - curr_x) >> 4;
                            bdx = (dist_x > 5) ? 5 : dist_x; // Limita velocidade máxima
                        } else {
                            dist_x = (curr_x - target_x) >> 4;
                            bdx = (dist_x > 5) ? -5 : -(signed char)dist_x;
                        }
                        
                        // 3. Calcula velocidade Y com a mesma lógica vetorial
                        if (target_y > curr_y) {
                            forca = (target_y - curr_y) >> 4;
                            if (forca > 6) forca = 6; // Limite máximo
                            if (forca < 2) forca = 2; // Força mínima para a bola não parar no meio do caminho
                            bdy = forca;
                        } else {
                            forca = (curr_y - target_y) >> 4;
                            if (forca > 6) forca = 6;
                            if (forca < 2) forca = 2;
                            bdy = -(signed char)forca;
                        }
                    }

                    // Move a bola fisicamente para fora do zagueiro para evitar "prender" a bola
                    if (defends_north) {
                        if (by <= curr_y + 14) by = curr_y + 16; 
                    } else {
                        if (by >= curr_y - 14) by = curr_y - 16; 
                    }
                    
                    wait_val = IA_WAIT_TIME[difficulty_level]; 
                    kick_cooldown = 30 + (rand() & 15); 
                    play_sfx_kick();
                }
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

    // Verifica se estamos no segundo tempo
    if (current_half == 2) {
        int gols_time1 = s1_final + s2_final;
        int gols_time2 = s3_final + s4_final;

        // Se o Time 1 (P1 e P2) está perdendo por 3 ou mais gols
        if (gols_time2 - gols_time1 >= 3) {
            player_pers[1] = 3; // Mais agressiva
            player_pers[2] = 3; // Mais agressiva
            player_pers[3] = rand() & 0x03;
            player_pers[4] = rand() & 0x03;
            return;
        }
        // Se o Time 2 (P3 e P4) está perdendo por 3 ou mais gols
        else if (gols_time1 - gols_time2 >= 3) {
            player_pers[1] = rand() & 0x03;
            player_pers[2] = rand() & 0x03;
            player_pers[3] = 3; // Mais agressiva
            player_pers[4] = 3; // Mais agressiva
            return;
        }
    }

    // Comportamento padrão (sorteio aleatório para todos)
    for (i = 1; i < 5; ++i) {
        player_pers[i] = rand() & 0x03;
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
 //   if (game_mode == 8 ) {
        // --- TORNEIO: RASTER GOLEIRO USA O SLOT 5 ---
        if (VIC.rasterline < 100) {
            VIC.spr_pos[5].x = gx[0] & 0xFF;
            VIC.spr_pos[5].y = gy[0];
            VIC.spr_color[5] = p1_color; 
            SPR_ZAG[5] = 16;
            VIC.rasterline = 150; 
            // Troca apenas a cor baseada no tempo
            if (current_half == 2) {
                VIC.spr_color[5] = p3_color; // 2º Tempo: Time B defende o Norte
            } else {
                VIC.spr_color[5] = p1_color; // 1º Tempo: Time A defende o Norte
            }
        } else {
            VIC.spr_pos[5].x = gx[1] & 0xFF;
            VIC.spr_pos[5].y = gy[1];
            VIC.spr_color[5] = p3_color; 
            SPR_ZAG[5] = 17;
            VIC.rasterline = 30; 
            // Troca apenas a cor baseada no tempo
            if (current_half == 2) {
                VIC.spr_color[5] = p1_color; // 2º Tempo: Time A defende o Sul
            } else {
                VIC.spr_color[5] = p3_color; // 1º Tempo: Time B defende o Sul
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




static void setup_match_colors(unsigned char h_id, unsigned char a_id) {
    unsigned char color_h = team_colors[h_id];
    unsigned char color_a = team_colors[a_id];
    unsigned char f_dark, f_light;

    // Identifica as cores do gramado atual para checar contraste
    if (field_type == 1) {          /* WET */
        f_dark = 6; f_light = 14; 
    } else if (field_type == 2) { /* FROZEN */
        f_dark = 11; f_light = 12; 
    } else {                    /* GRASS */
        f_dark = 5; f_light = 13; 
    }

    // 1. REGRA DE CONFLITO ENTRE TIMES (Checa se pertencem à mesma família de cor)
    if (h_id == a_id || color_family[color_h] == color_family[color_a]) {
        color_h = 7; // Amarelo (Família 7)
        color_a = 2; // Vermelho (Família 3)
    }

    // 2. REGRA DE CONTRASTE COM O CAMPO (Anti-Camuflagem)
    if (color_h == f_dark || color_h == f_light) {
        color_h = (field_type == 1) ? 1 : 7; 
    }

    if (color_a == f_dark || color_a == f_light) {
        color_a = (color_h == 7) ? 2 : 7; 
    }

    // 3. REGRA DE CONFLITO PÓS-CAMPO (Garante que ajustes do campo não geraram famílias iguais)
    if (color_family[color_h] == color_family[color_a]) {
        color_a = (color_h == 7) ? 2 : 7;
    }

    // 4. ATRIBUIÇÃO AOS SPRITES
    VIC.spr_color[0] = color_h; 
    VIC.spr_color[6] = color_h; 

    VIC.spr_color[2] = color_a; 
    VIC.spr_color[7] = color_a;

    p1_color = color_h;
    p2_color = color_h;
    p3_color = color_a;
    p4_color = color_a;
}




static void load_team_sprites(void) {
    unsigned char i;
    unsigned char* dest;

    // ==========================================
    // TIME A: P1 (player_type[1]) e P2 (player_type[2])
    // ==========================================
    if (player_type[1] == 1 || player_type[2] == 1) {
        // Se ainda não foi definido ou há lixo na RAM, sorteia e trava os dois do time
        if (saved_sprites[0] == 0xFF || saved_sprites[0] > 6) {
            saved_sprites[0] = (unsigned char)(rand() % 7);
            do {
                saved_sprites[1] = (unsigned char)(rand() % 7);
            } while (saved_sprites[1] == saved_sprites[0]);
        }
        active_sprite[0] = saved_sprites[0];
        active_sprite[1] = saved_sprites[1];
    } else {
        // Time 100% CPU: Sorteia novos a cada partida
        active_sprite[0] = (unsigned char)(rand() % 7);
        do { 
            active_sprite[1] = (unsigned char)(rand() % 7); 
        } while (active_sprite[1] == active_sprite[0]);
    }


    // ==========================================
    // TIME B: P3 (player_type[3]) e P4 (player_type[4])
    // ==========================================
    if (player_type[3] == 1 || player_type[4] == 1) {
        if (saved_sprites[2] == 0xFF || saved_sprites[2] > 6) {
            saved_sprites[2] = (unsigned char)(rand() % 7);
            do {
                saved_sprites[3] = (unsigned char)(rand() % 7);
            } while (saved_sprites[3] == saved_sprites[2]);
        }
        active_sprite[2] = saved_sprites[2];
        active_sprite[3] = saved_sprites[3];
    } else {
        // Time 100% CPU: Sorteia novos a cada partida
        active_sprite[2] = (unsigned char)(rand() % 7);
        do { 
            active_sprite[3] = (unsigned char)(rand() % 7); 
        } while (active_sprite[3] == active_sprite[2]);
    }


    // ==========================================
    // TRAVA DE SEGURANÇA ABSOLUTA (FAILSAFE)
    // ==========================================
    if (active_sprite[0] == active_sprite[1]) {
        active_sprite[1] = (active_sprite[0] + 1) % 7;
    }
    if (active_sprite[2] == active_sprite[3]) {
        active_sprite[3] = (active_sprite[2] + 1) % 7;
    }

    // Proteção final de ponteiro para os 4 slots
    for (i = 0; i < 4; ++i) {
        if (active_sprite[i] > 5) active_sprite[i] = 0;
    }


    // ==========================================
    // CÓPIA PARA A VRAM (0xF000, 0xF080, 0xF100, 0xF180)
    // ==========================================
    for (i = 0; i < 4; ++i) {
        dest = (unsigned char*)(0xF000 + (i << 7));
        
        memcpy(dest,      idle_gfx[active_sprite[i]], 64);
        memcpy(dest + 64, run_gfx[active_sprite[i]],  64);
    }
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
//load_team_sprites();
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
	
//	if (game_mode == 8) {
        // MODO 5: Todos multicolor, exceto Sprite 3 (Bola)
        // Bits: 76543210 -> 11110111 = 0xF7
        *((unsigned char*)0xd01c) = 0xF7; 

	// 6. Cores
 //       if (game_mode == 8) {
//			setup_match_colors(home_team, away_team);
            // MODO 7: Cores por Time
           // Agora aplicamos as cores que a função calculou
  //      VIC.spr_color[0] = p1_color; // P1 (Time A)
  //      VIC.spr_color[6] = p2_color; // P2 (Time A)
   //     VIC.spr_color[2] = p3_color; // P3 (Time B)
  //      VIC.spr_color[7] = p4_color; // P4 (Time B)
//		}
		
	/*	
		 else {
        // Cores individuais (Cor 3 do Multicolor)
        VIC.spr_color[0] = 7;  // P1 Amarelo
        VIC.spr_color[6] = 2;  // P2 Vermelho
        VIC.spr_color[2] = 1;  // P3 Branco
        VIC.spr_color[7] = 13; // P4 Verde
		 }
	*/	 
		 
  //      VIC.spr_color[4] = 14; // Goleiro Dinâmico 1
  //      VIC.spr_color[5] = 14; // Goleiro Dinâmico 2
  //      VIC.spr_color[3] = 7;  // BOLA (Hi-Res)

        // Ponteiros Iniciais Modo 5
 //       SPR_ZAG[0] = 192; // P1
 //       SPR_ZAG[6] = 194; // P2
 //       SPR_ZAG[2] = 196; // P3
 //       SPR_ZAG[7] = 198; // P4
 //       SPR_ZAG[3] = 246; // Bola (Mudou do 5 para o 3!)
  //      SPR_ZAG[4] = 18;  // Goleiro Inicial (Oeste)
 //       SPR_ZAG[5] = 16;  // Goleiro Inicial (Norte)
		// Zagueiros (Sprites 1 e 4) apontando para o NOVO Slot 18 (Parado)
 //   SPR_ZAG[1] = 18;  // Antes provavelmente 255
 //   SPR_ZAG[4] = 18;  // Antes provavelmente 255
		
		asm("sei"); // Desliga interrupções para configurar

		// Aponta o vetor de interrupção ($0314) para sua função
		// Se o compilador for cc65, use:
		*((unsigned int*)0x0314) = (unsigned int)interrupt_raster_handler;

		VIC.imr = 0x01;      // Habilita interrupção de raster
		VIC.ctrl1 &= 0x7F;   // Limpa o 8º bit da linha de raster (total < 256)
		VIC.rasterline = 30; // Primeiro disparo na linha 30 (Goleiro Norte)

		asm("cli"); // Religa interrupções
	
//} 
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
/*

static void update_sprites(void) {
    // --- MODOS DE TORNEIO (6 e 7) --- Tem Zagueiros!
    
        // Fixos sempre ativos: 
        // P1(S0), Zagueiro Sul(S1), Bola(S3), Zagueiro Norte(S4), Goleiro Raster(S5)
        // Binário: 00111011 -> 0x3B (Nota: O comentário anterior 0x1B estava errado)
        unsigned char mask = 0x3B; 

        if (num_players >= 3) mask |= 0x04; // + P3 (S2)
        if (num_players >= 2) mask |= 0x40; // + P2 (S6)
        if (num_players == 4) mask |= 0x80; // + P4 (S7)

        VIC.spr_ena = mask;
    
    
}
*/
static void enter_game_mode(void) {
    int i; // Declarando variável para o loop

    // 1. Seleciona Bank 3 ($C000 - $FFFF)
    *((unsigned char*)0xDD00) = (*((unsigned char*)0xDD00) & 0xFC);
    
    // 2. VIC aponta para Screen $C000 e Charset $D000
    VIC.addr = 0x0E;

 //   if (game_mode == 8) {
        
        // --- PONTEIROS COMUNS A TODOS OS MULTIPLAYERS ---
        // --- PONTEIROS COMUNS A TODOS OS MULTIPLAYERS ---
  //      SPR_ZAG[0] = 192; // P1
        
        // CORREÇÃO: Só dá gráfico ao P2 se ele for jogador de linha
       
  //          SPR_ZAG[6] = 194; // P2
       
        
  //      SPR_ZAG[2] = 196; // P3
  //      SPR_ZAG[7] = 198; // P4

        // AJUSTE: BOLA mapeada para o novo Slot Base 20 ($C500) para suportar 8 frames Hi-Res
        SPR_ZAG[3] = 246;  // ANTES 250
        
       // if (game_mode == 8 ) {
            // --- MODOS DE TORNEIO (Tem Zagueiros!) ---
            
            // AJUSTE JÁ FEITO POR VOCÊ: Zagueiros mapeados para o Slot 18 ($C480)
            SPR_ZAG[1] = 18; // Zagueiro Sul (Sprite 1)
            SPR_ZAG[4] = 18; // Zagueiro Norte (Sprite 4)
            
            SPR_ZAG[5] = 16; // Goleiro Dinâmico (Raster cuidará dele no S5, Slot 16)
            
            VIC.spr_color[1] =  1; // Cor Zagueiro Sul
            VIC.spr_color[4] =  7; // Cor Zagueiro Norte
            VIC.spr_color[5] = 14; // Cor Goleiro Dinâmico
            
            // Ativa Multicolor para todos, EXCETO Bola(S3)
            // Bit 3 é 0 (Hi-Res), outros são 1 (Multicolor) -> 11110111 = 0xF7
            *((unsigned char*)0xd01c) = 0xF7; 
     //   } 
		
		/*
		else {
            // --- MODO 5 TRADICIONAL (Tem Goleiro, sem Zagueiros) ---
            SPR_ZAG[1] = 16; // Goleiro Dinâmico no Sprite 1 (Slot 16)
            VIC.spr_color[1] = 14;
            
            // Mantém a configuração Multicolor padrão (Bola Hi-Res)
            *((unsigned char*)0xd01c) = 0xF7; 
        }
		*/
/*
        // 6. Cores Gerais dos Jogadores
        if (game_mode == 8 ) {
            VIC.spr_color[0] = 7; 
            VIC.spr_color[6] = 7; 
            VIC.spr_color[2] = 2; 
            VIC.spr_color[7] = 2; 
        } 
 */       
   //     if (game_mode == 8) {
   //         VIC.spr_color[2] = enemy_color; // enemy_color deve estar definida globalmente
   //         VIC.spr_color[7] = enemy_color;
   //     }
    //    VIC.spr_color[3] = 1; // Cor da BOLA (Branca, Hi-Res)
	//    VIC.spr_color[3] = 7; // Cor da BOLA (Branca, Hi-Res)
    /*    
        // P2 desativado se for "None"
        if (player_type[2] == 2) {
            spr_active_p2 = 0; // spr_active_p2 deve estar definida globalmente
            VIC.spr_pos[6].x = 0;
            VIC.spr_pos[6].y = 0;
        } else {
	*/		
     //       spr_active_p2 = (num_players >= 2);
    //    }

        // CORREÇÃO CRÍTICA DO ZERAMENTO:
        // Não zere a posição dos Zagueiros (S1, S4) e Goleiro Raster (S5) nos Modos 6/7.
        for (i=0; i<8; i++) {
       //     if ((game_mode == 8 ) && (i == 1 || i == 4 || i == 5)) continue;
            // Opcional: Manter posição da bola (S3) se necessário
            VIC.spr_pos[i].x = 0;
            VIC.spr_pos[i].y = 0;
        }
 //   } 

    // 5. Cores Compartilhadas para Multicolor (Comum a todos os modos)
    VIC.spr_mcolor0 = 10; // Cor Multicolor 1
    VIC.spr_mcolor1 = 0;  // Cor Multicolor 2
    
    VIC.spr_hi_x = 0; // Zera todos os bits de X > 255
 //   init_human_sprites();
    load_team_sprites();
	 if (use_defender == 1 )
		{	
			VIC.spr_ena = 0xFF;
		}
		else
		{
			VIC.spr_ena = 0xED;
	}

    // Chama a função que seta a visibilidade correta (VIC.spr_ena)
   // update_sprites();
}

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



static void __fastcall__ apply_smart_kick2(unsigned char p_idx) {
    register unsigned char zone;
    register unsigned char pers;
    register unsigned char target_y; 
    unsigned char option;
    signed char temp_power;
    unsigned int target_x;
    unsigned char pass_mask;
    unsigned char ataca_sul;  
     unsigned char is_team_a;
         
    
    static const unsigned char player_to_vic_slot[5] = { 0, 0, 6, 2, 4 };
    static const unsigned char partners[5] = { 0, 2, 1, 4, 3 };

    pers = player_pers[p_idx];

    init_random();  

    /* LÓGICA DE PASSE */
 //   if ((game_mode & 0x07) >= 6) {  
        is_team_a = (p_idx == 1 || p_idx == 2);
        ataca_sul = (current_half == 1) ? is_team_a : !is_team_a;
        
   //     if (ataca_sul ? (by <= 146) : (by > 146)) {
   //         goto pula_passe;  
   //     }

   //     if (ataca_sul ? (by > 190) : (by < 60)) {
   //     goto pula_passe;  
   // }

//    if (ataca_sul ? (by >= 146) : (by <= 122)) {
  if (ataca_sul ? (by > 130) : (by < 162)) {
    goto pula_passe;  
    }

//        pass_mask = (p_idx == 2) ? 0x03 : ((pers == 2) ? 0x01 : 0x03);
     // Aumenta a probabilidade de tentar o passe (0x01 = 50% de chance, 0x00 = 100%)
    pass_mask = (pers == 2) ? 0x00 : 0x01;
        
        if ((rand() & pass_mask) == 0) {
            unsigned char partner_logical_idx = partners[p_idx];
            unsigned char partner_vic_slot = player_to_vic_slot[partner_logical_idx];
            
            target_y = VIC.spr_pos[partner_vic_slot].y;

            if (target_y > 0) {
                unsigned int abs_dx;
                unsigned char abs_dy;

                target_x = VIC.spr_pos[partner_vic_slot].x;
                if (VIC.spr_hi_x & (1 << partner_vic_slot)) target_x += 256;

                abs_dx = (target_x > bx) ? (target_x - bx) : (bx - target_x);
                abs_dy = (target_y > by) ? (target_y - by) : (by - target_y);

                bdx = (target_x > bx) ? 4 : -4;
                bdy = (target_y > by) ? 4 : -4;

                if (abs_dx > (abs_dy << 1)) {
                    bdy >>= 1;  
                } else if (abs_dy > (abs_dx << 1)) {
                    bdx >>= 1;  
                }
                
                kick_power = 4;
                is_pass = 1;
                after_touch_timer = 0;
                kick_cooldown = 15;
                return;  
            }
        }
   // }
pula_passe:

    /* CÁLCULO DA ZONA DO CAMPO (Otimizado com operador ternário em cascata) */
    zone = (bx < 106) ? 0 : ((bx < 212) ? 1 : 2);
    if (by > 66) {
        zone += 3;
        if (by > 132) zone += 3;
    }

    /* SELEÇÃO DE OPÇÃO DA LUT */
    option = rand() & 0x03;
    if (difficulty_level > 1) option |= 0x02;

    temp_power = 5 + difficulty_level;
    if (pers == 1) temp_power += 2;  
    if (temp_power > 9) temp_power = 9;

    bdx = (signed char)lut_kick_br[zone][option][0] * temp_power;
    bdy = (signed char)lut_kick_br[zone][option][1] * temp_power;
    ai_curve_dir = lut_kick_br[zone][option][2];

    // --- LÓGICA DE DIREÇÃO POR TEMPO E TIME (Modos 6 e 7) ---
  //  if ((game_mode & 0x07) >= 6) {
        // Aproveita o cálculo de ataca_sul feito no início da função
        if (ataca_sul) {
            if (bdy < 0) {
                bdy = -bdy;
                ai_curve_dir = -ai_curve_dir;
            } else if (bdy == 0) {
                bdy = temp_power;
            }
        } else {
            if (bdy > 0) {
                bdy = -bdy;
                ai_curve_dir = -ai_curve_dir;
            } else if (bdy == 0) {
                bdy = -temp_power;
            }
        }

        
        // Se o chute estiver saindo 100% reto (bdx == 0) e o jogador 
        // estiver na zona central perto dos gols (zone 1 ou zone 7)
        if (bdx == 0 && (zone == 1 || zone == 7)) {
            // Desvia a bola para o canto direito ou esquerdo, usando metade da força do chute.
            // O operador (>> 1) é uma divisão por 2 ultra-rápida.
            bdx = (rand() & 1) ? (temp_power >> 1) : -(temp_power >> 1);
//            bdx = (rand() & 1) ? (temp_power >> 2) : -(temp_power >> 2);
        }

/*
    } 
    
    else {
        // Modo 5 (Individual): Segue a direção natural da LUT
        if (bdy == 0) {
            bdy = (rand() & 1) ? 2 : -2;
        }
    }
 */   
    after_touch_timer = 60 + (difficulty_level << 3);
    if (pers == 3) after_touch_timer >>= 1;

    kick_power = temp_power;
}

static unsigned char get_fixed_player_color(unsigned char p_idx) {
    switch(p_idx) {
        case 1: return C64_YELLOW; // P1
        case 2: return C64_RED;    // P2
        case 3: return C64_PINK;  // P3 (Ajuste para Verde se preferir)
        case 4: return C64_BLUE;   // P4
        default: return 1;         // Branco
    }
}



static void fast_clrscr(unsigned char color) {
    // 32 é o código do espaço vazio (sem gramado)
    memset(SCREEN, 32, 1000); 
    // Força a cor branca em tudo para o texto aparecer
    memset((void*)0xD800, color, 1000); 
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


    // Filtro pra suavizar
 //   SID.flt_cutoff = 0x0600;
 //   SID.flt_ctrl   = 0x34;
 //   SID.flt_res    = 0x88;
}


static void play_goal_crowd(void) {
    SID.v3.ctrl = 0x00;   // Limpa o registro

    // A = 0 (Attack instantâneo: causa o efeito de explosão/estouro)
    // D = A (~750ms: o som cai levemente após o impacto inicial)
    SID.v3.ad = 0x0A;   

    // S = A (Sustain alto, mantém a vibração forte)
    // R = A (~750ms: Release moderado para o som não cortar seco nem durar muito)
    SID.v3.sr = 0xAA;   

    // Frequência: 0x1000 costuma ser o "sweet spot" para soar 
    // como um rugido caótico em vez de chiado de TV (que seria > 0x2000)
    SID.v3.freq = 0x1000; 

    // Dispara o som (Noise + Gate)
    SID.v3.ctrl = 0x81;   

    crowd_state = 1;
    crowd_timer = 40; // Mantém o som no pico por um tempo um pouco menor
}
/*
static void play_goal_crowd(void) {
    SID.v3.ctrl = 0x00;

    SID.v3.ad = 0x04;   // Attack mais suave
    SID.v3.sr = 0x33;   // Sustain moderado

    SID.v3.freq = 0x0C00;
    SID.v3.ctrl = 0x81;   // Noise curto inicial

    crowd_state = 1;
    crowd_timer = 50;
}
*/

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

// 1. TABELAS DE LOOKUP CONSTANTES (Fora da função)
// Ficam na ROM/RODATA. O 6502 acessa em apenas 4 ciclos (LDA abs,X)
static const unsigned char LUT_TRIGGER[5] = {0, 80, 85, 75, 85};
static const unsigned char LUT_PANIC[5]   = {0, 100, 100, 80, 95};
static const unsigned char LUT_PMASK[5]   = {0, 3, 3, 1, 1};
static const int LUT_OFFSET[5]            = {0, 30, 20, -30, -20};

static void move_player_ia_modo5(unsigned char pid) {
    static unsigned char pers, is_support, team, my_joy;
    static unsigned int my_x, my_y, mate_x, mate_y;
    static int tx, ty; 
    static unsigned char trigger_frame, current_sd;
    static unsigned int dist_me, dist_mate;
    static unsigned char mate_has_ball, partner_closer;
    static unsigned char panic_time, shift_amt;

    // =========================================================
    // 1. CHECAGEM DE STUN (Usando Switch - vira Jump Table)
    // =========================================================
    switch (pid) {
        case 1: if (stun_timer_p1) { stun_timer_p1--; p1_is_moving = 0; joy2 = 0; return; } break;
        case 2: if (stun_timer_p2) { stun_timer_p2--; p2_is_moving = 0; joy1 = 0; return; } break;
        case 3: if (stun_timer_p3) { stun_timer_p3--; p3_is_moving = 0; joy3 = 0; return; } break;
        case 4: if (stun_timer_p4) { stun_timer_p4--; p4_is_moving = 0; joy4 = 0; return; } break;
    }

    pers = player_pers[pid];
    
    // Matemática branchless (sem saltos) incrivelmente rápida para o 6502:
    is_support = (pid & 1) == 0; // Par (2,4) = 1 (True), Ímpar (1,3) = 0 (False)
    team = (pid + 1) >> 1;       // (1+1)/2=1 | (2+1)/2=1 | (3+1)/2=2 | (4+1)/2=2

    // =========================================================
    // 2. CARREGAR GLOBAIS
    // =========================================================
    switch (pid) {
        case 1: my_x = px;  my_y = py;  mate_x = p2x; mate_y = p2y; current_sd = shot_delay_p1; break;
        case 2: my_x = p2x; my_y = p2y; mate_x = px;  mate_y = py;  current_sd = shot_delay_p2; break;
        case 3: my_x = p3x; my_y = p3y; mate_x = p4x; mate_y = p4y; current_sd = shot_delay_p3; break;
        case 4: my_x = p4x; my_y = p4y; mate_x = p3x; mate_y = p3y; current_sd = shot_delay_p4; break;
    }

    // =========================================================
    // 3. LÓGICA PRINCIPAL UNIFICADA
    // =========================================================
    if (ball_possession == pid) {
        my_joy = 0;
        
        // Uso das Lookup Tables para evitar múltiplos if/else
        trigger_frame = LUT_TRIGGER[pid];
        panic_time    = LUT_PANIC[pid];

        if (possession_timer >= panic_time) { 
            unsigned char rand_val = (*(unsigned char*)162);
            unsigned char panic_mask = LUT_PMASK[pid];
            
            if ((rand_val & panic_mask) == 0) {
                possession_timer = 0; 
                last_touch = pid; 
                my_joy = 0x10; 
                goto apply_joy;
            }
            trigger_frame = (team == 2 ? 20 : 40) + (rand_val & 31); 
        }

        if (possession_timer == trigger_frame) {
            unsigned char rand_val = (*(unsigned char*)162);
            unsigned char mask = 0x3F;
            
            switch (pid) {
                case 2: mask = (pers == 1 ? 0x1F : 0x3F); break;
                case 3: mask = 0x7F; break;
                case 4: mask = (pers == 1 ? 0x7F : 0x3F); break;
            }
            
            current_sd = rand_val & mask;
            
            switch (pid) {
                case 1: shot_delay_p1 = current_sd; break;
                case 2: shot_delay_p2 = current_sd; break;
                case 3: shot_delay_p3 = current_sd; break;
                case 4: shot_delay_p4 = current_sd; break;
            }
        }

        if (possession_timer <= current_sd) {
            possession_timer = 0; last_touch = pid; my_joy = 0x10; goto apply_joy;
        }

        // Coordenadas Alvo Com a Bola
        switch (pid) {
            case 1:
                ty = (current_half == 2) ? 70 : 220;
                if (pers == 1) { tx = 170; }
                else if (pers == 2) { tx = (my_x < 170) ? 280 : 60; ty = (ty == 220) ? 200 : 90; }
                else { tx = 170; ty = (ty == 220) ? 180 : 110; }
                break;
            case 2:
                ty = (current_half == 2) ? 80 : 210;
                tx = (my_x < 170) ? 260 : 80;
                break;
            case 3:
                ty = (current_half == 2) ? 220 : 70;
                tx = 170;
                break;
            case 4:
                ty = (current_half == 2) ? 210 : 80;
                tx = (my_x < 170) ? 260 : 80;
                break;
        }
    } 
    else {
        // Sem a bola
        // Mágica Bitwise: Descobre parceiro cruzando bits (1 vira 2, 3 vira 4, etc). Sem branches!
        mate_has_ball = (ball_possession == (pid ^ (team == 1 ? 3 : 7)));

        if (mate_has_ball) {
            // Outra mágica: Equipe atacando p/ sul = se equipe atual é igual ao half atual.
            unsigned char team_ataca_sul = (team == current_half); 

            tx = (mate_x < 170) ? mate_x + 55 : mate_x - 55;
            ty = team_ataca_sul ? (mate_y + 35) : (mate_y - 35);
        }
        // Checagem branchless se adversário está com a bola
        else if (difficulty_level == 2 && ball_possession > 0 && ((ball_possession + 1) >> 1) != team) {
            if (team == 1) {
                tx = (ball_possession == 3) ? p3x : p4x;
                ty = (ball_possession == 3) ? p3y : p4y;
            } else {
                tx = (ball_possession == 1) ? px : p2x;
                ty = (ball_possession == 1) ? py : p2y;
            }
        } 
        else {
            // Bola Solta
            dist_me = (my_x > bx ? my_x - bx : bx - my_x) + (my_y > by ? my_y - by : by - my_y);
            dist_mate = (mate_x > bx ? mate_x - bx : bx - mate_x) + (mate_y > by ? mate_y - by : by - mate_y);

            partner_closer = is_support ? (dist_mate <= dist_me) : (dist_mate < dist_me);

            if (partner_closer) {
                tx = bx + LUT_OFFSET[pid]; // Tabela de Lookup substitui ternário quádruplo
                ty = (by < 120) ? by + 45 : by - 45;
            } else {
                switch (pid) {
                    case 1:
                        shift_amt = (pers == 1) ? 2 : 1;
                        tx = bx + (bdx << shift_amt); ty = by + (bdy << shift_amt);
                        break;
                    case 2: 
                        tx = bx + (bdx << 1); ty = by + (bdy << 1); 
                        break;
                    case 3:
                        shift_amt = (pers == 1) ? 2 : 1;
                        tx = bx + (bdx << shift_amt) + bdx; ty = by + (bdy << shift_amt) + bdy;
                        break;
                    case 4:
                        shift_amt = (pers == 1) ? 1 : 0;
                        tx = bx + (bdx << shift_amt); ty = by + (bdy << shift_amt);
                        break;
                }
            }
        }
    }

    // Aplica o movimento
    my_joy = 0;
    if ((int)my_x < tx) { my_x++; my_joy = 8; }
    else if ((int)my_x > tx) { my_x--; my_joy = 4; }
    
    if ((int)my_y < ty) { my_y++; my_joy |= 2; }
    else if ((int)my_y > ty) { my_y--; my_joy |= 1; }
    
    if (my_x < 12) my_x = 12; else if (my_x > 334) my_x = 334;
    if (my_y < 28) my_y = 28; else if (my_y > 240) my_y = 240;

apply_joy:
    // =========================================================
    // 4. DEVOLVER LOCAIS PARA AS GLOBAIS
    // =========================================================
    switch (pid) {
        case 1: px = my_x;  py = my_y;  joy2 = my_joy; p1_is_moving = 1; break;
        case 2: p2x = my_x; p2y = my_y; joy1 = my_joy; p2_is_moving = 1; break;
        case 3: p3x = my_x; p3y = my_y; joy3 = my_joy; p3_is_moving = 1; break;
        case 4: p4x = my_x; p4y = my_y; joy4 = my_joy; p4_is_moving = 1; break;
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
     if (new_owner >= 5) { 
        unsigned char defends_north = (new_owner == 5) ? (current_half == 1) : (current_half == 2);
        
        kick_cooldown = 20; // Aumentei um pouco para evitar o ping-pong imediato
        
        // Força do chute (4 a 7)
        bdy = 2 + (rand() & 3); 
        
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
    is_multi = (game_mode == 8);

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

            if (use_defender == 1 ) {
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
     //       spr_active_defender = use_defender;
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



static void show_tournament_champion(unsigned char champion_idx) {
    // 1. Variáveis locais estáticas evitam o pesado stack em software (sp) do CC65
    static unsigned char frame_count;
    static unsigned char color_shift;
    static unsigned char i;
    
    static const unsigned char rainbow[8] = {7, 10, 13, 1, 3, 14, 6, 5};

    // Inicialização manual necessária já que agora são static
    frame_count = 0;
    color_shift = 0;

    clear_screen_c000_with_color(COLOR_WHITE); 
    VIC.bordercolor = 0;
    VIC.bgcolor0 = 0;
    music_init(); 
    
    put_text(13, 6, (const unsigned char*)"C H A M P I O N");
 //   draw_micro_flag(champion_idx, 27, 14);
    
    // 2. Remoção da variável name_ptr e alocação direta (menos uso de stack)
    put_text(13, 14, (const unsigned char*)team_names[champion_idx]); 
    
    put_text(13, 17, (const unsigned char*)"LEAGUE WINNER!");
    put_text(13, 19, (const unsigned char*)"CONGRATULATIONS");
    put_text(11, 22, (const unsigned char*)"PRESS FIRE TO EXIT");

    while (1) { 
        wait_vblank();
        music_play();
        
        // 3. OTIMIZAÇÃO CRÍTICA: Operador de negação bit a bit (~) no CC65 promove
        // para int de 16-bits, gerando instruções extras (EOR #$FF). 
        // Testar '== 0' compila para um enxuto lda/and/beq.
        if ((CIA1.pra & 0x10) == 0) break; 

        // 4. Salto de 2 em 2 no loop elimina o 'if (i & 1) continue;' 
        // e todo o branching interno que ele gerava.
        for (i = 0; i < 15; i += 2) {
            // 5. Hardcode do offset da Color RAM: 0xD800 + (6 * 40) + 13 = 0xD8FD.
            // O CC65 nem sempre otimiza essa matemática em tempo de compilação.
            ((unsigned char*)0xD8FD)[i] = rainbow[(color_shift + i) & 7];
        }

        if (++frame_count > 180) { 
            color_shift++;
            frame_count = 0;
        }
    }

    // Aplicação da mesma lógica enxuta de leitura do botão
    while ((CIA1.pra & 0x10) == 0) {
        wait_vblank();
    }
    
    music_stop();
    VIC.bordercolor = 0;
    VIC.bgcolor0 = 0;
    clear_screen_c000_with_color(COLOR_WHITE);
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
   
 //  unsigned char char_da_linha = 64; // Exemplo: ajuste para o seu código
 //   unsigned char moldura_rep[7];
unsigned char spd;
//,k; // Variável temporária para velocidade
unsigned char gs_text_backup[13]; 
unsigned char gs_color_backup[13];
// Calcula o total de cada time
unsigned char gols_time_a = 0;
unsigned char gols_time_b = 0;
static unsigned char screen_backup[35];
static unsigned char color_backup[35];
unsigned char len_h,total_len,start_x,len_a;

	
	
	init_whistle_voice(); 
	
	
	VIC.addr = 0x10;
    
    // --- 1. RESET DE VARIÁVEIS ---
    score = 0; s1_final = 0; s2_final = 0; s3_final = 0; s4_final = 0;
    goal_delay = 0; // CRUCIAL para a IA não começar travada
    kick_cooldown = 0;
    ball_possession = 0;
	difficulty_boost = 0; // Reset do boost no início
//	saves_p2 = 0;
 //   keeper_score = 0;
	shot_delay_p1 = 0;
	shot_delay_p2 = 0;
    shot_delay_p3 = 0;
    shot_delay_p4 = 0;
	possession_timer = 0;
//	is_overtime = 0;
    golden_score_triggered =0;
	// Calcula o total de cada time
    gols_time_a = 0;
    gols_time_b = 0;


// Desativa a varredura automática de teclado do sistema operacional
*((unsigned char*)0x0289) = 0;
    
// --- RESET DE BOOST (EXCLUSIVO MODO 5) ---
//if (game_mode == 8  ) {
    for (i = 0; i < 5; i++) {
        has_boost[i] = 1;
        boost_timer[i] = 0;
    }
//}
	

	// --- RESET DE JOYSTICK PARA EVITAR CHUTE FANTASMA ---
 //   joy_now = 0;
 //   last_joy_state = 0xff; // Força um estado neutro inicial
	
//	dx = 100; dy = 140; // Posição inicial do zagueiro
//    d_level = rand() % 3;
	
	
	
	// 2. Espera ativa: Só começa o jogo quando o botão FIRE for solto
    // Isso evita que o clique no menu vire um chute no jogo
    while ((~CIA1.pra & 0x10) || (~CIA1.prb & 0x10)) {
        wait_vblank(); // Aguarda o jogador tirar o dedo do botão
    }
	
	// 3. Delay de estabilização (aprox. 0.5 segundos)
    for(i = 0; i < 30; i++) wait_vblank();
	
// teste
//VIC.spr_color[7] = 2;      // vermelho
//VIC.spr_pos[7].x = 160;
//VIC.spr_pos[7].y = 120;

//VIC.spr_ena |= 0x80;      // garante ligado

//spr_active_p1 = 1;
//spr_active_ball = 1;
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
  //  if (  (game_mode == 8 && gold_goal_active == 1)) timer = 9999; 
  //  else 
		
	timer = 2250; 

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
 
/* 
    // --- 2. PONTEIROS DE SPRITE (FIXO PARA EVITAR BORRÃO) ---

*/
// IMPORTANTE: O valor no ponteiro é Endereço_Real / 64
//    SPR_ZAG[0] = 192; // P1 ($F000)
//    SPR_ZAG[1] = 18;  // Goleiro N ($C400) -> ANTES ESTAVA 246 (ERRO)
//    SPR_ZAG[2] = 196;  // Goleiro S ($C440)
//    SPR_ZAG[3] = 0;  // Goleiro O ($C480)
//    SPR_ZAG[4] = 18;  // Goleiro L ($C4C0)
//    SPR_ZAG[5] = 193; // Bola ($F040)
    // CORREÇÃO: Garante que P2 não tenha gráfico se for GK
 //   if (player_type[2] == 2 || (game_mode == 5 && num_players < 2)) {
     //   SPR_ZAG[6] = 0;
  //  } else {
//        SPR_ZAG[6] = 194; // P2
 //   }
 //   SPR_ZAG[7] = 198; // Zagueiro ($FFC0)

    

 //   VIC.spr_ena |= 0x80;
    // Cores
 //   VIC.spr_color[0] = 7; VIC.spr_color[1] = 4; VIC.spr_color[2] = 4;
  //  VIC.spr_color[3] = 7; 
//	VIC.spr_color[4] = 4; VIC.spr_color[5] = 1;
 //   VIC.spr_color[6] = 2; 

// --- NOVO: CONTROLE DE FANTASMAS ---
    // Começamos definindo quem REALMENTE deve estar ligado.
    // No seu código: S0=P1, S3=Bola (no Modo 5), S4/S5=Goleiros.
    
 //   if ( game_mode == 8) {
        // Modo 5: Liga P1(0), P3(2), Bola(3), Goleiros(4,5), P2(6), P4(7)
        // Binário: %11111101 (Slot 1 fica desligado, pois você usa Raster para ele)
 //       VIC.spr_ena = 0xFD; 
		
		
        
/*		  
        // Proteção extra: se num_players for menor que 4, desliga os excedentes
        if (num_players < 4) {
            VIC.spr_ena &= ~0x80; // Desliga P4 (Slot 7)
            VIC.spr_pos[7].x = 0; VIC.spr_pos[7].y = 0;
        }
        if (num_players < 3) {
            VIC.spr_ena &= ~0x04; // Desliga P3 (Slot 2)
            VIC.spr_pos[2].x = 0; VIC.spr_pos[2].y = 0;
        }
  //  }
*/

  if (selected_field_option == 3) {
        field_type = rand() % 3; // Sorteia entre 0, 1 e 2
    } else {
        field_type = selected_field_option;
    }
  // Sorteio do Estilo do Campo (Field Style)
    if (field_style == 3) {
        field_style = rand() % 3; // Sorteia entre 0, 1 e 2 (Checker, Bars, Solid)
    }

   draw_field_manager();
   /*
   // --- EXIBIÇÃO DO MODO GOLDEN SCORE (ESTÁTICA) ---
    if ((game_mode == 7 && gold_goal_active == 1)) {
        // Coluna 1, Linha 0: Canto superior esquerdo
        put_text(36, 0, (const unsigned char*)"GS"); 
    }
*/
    // 2. Configuração Visual baseada no campo
    if (field_type == 1) {      // MOLHADO
        VIC.bgcolor0 = 6;       // Azul escuro
        VIC.bordercolor = 14;   // Azul claro
		VIC.spr_color[3] = 1;
/*		
		if ( game_mode == 8) {
        // No Modo 5, os goleiros estão nestes slots:
        VIC.spr_color[1] = 5; // Goleiro Vertical (Verde)
        VIC.spr_color[4] = 5; // Goleiro Oeste (Verde)
        VIC.spr_color[1] = 5; // Goleiro Leste / Raster (Verde)
        
        // Garante que a bola (Slot 3) continue branca no molhado
        VIC.spr_color[3] = 1; 
		VIC.spr_color[3] = 8;
    } 
*/	
//	else {
//		VIC.spr_color[7] = 5;   // 13
//		VIC.spr_color[1] = 5; 
//		VIC.spr_color[2] = 5;
   //     VIC.spr_color[3] = 7;
//		VIC.spr_color[4] = 5;
//	}
    } else if (field_type == 2) { // NEVE
        VIC.bgcolor0 = 12;       // Cinza médio
        VIC.bordercolor = 15;   // Cinza
		VIC.spr_color[7] = 4; // Preto
		VIC.spr_color[3] = 7;
    } else {                    // GRAMA
        VIC.bgcolor0 = 5;       // Verde
        VIC.bordercolor = 13;   // Verde claro
		VIC.spr_color[7] = 4; // Preto
		VIC.spr_color[3] = 1;
    }

 //   draw_field();
	
	dx = 160; // Centro do campo (X)
dy = 140; // Centro do campo (Y)
	play_sfx_whistle();



// --- Substitua a parte dos ponteiros (SPR_ZAG) por esta ---
// --- SUBSTITUA A PARTE DOS PONTEIROS NA PLAY_MATCH POR ESTA ---
//if (game_mode == 8) {
    // Ponteiros Iniciais COMUNS
 //   SPR_ZAG[0] = 192; // P1
 //   SPR_ZAG[2] = 196; // P3
 //   SPR_ZAG[3] = 246; // BOLA (No Slot 3)
 //   SPR_ZAG[6] = 194; // P2
 //   SPR_ZAG[7] = 198; // P4
 //   SPR_ZAG[5] = 16; // Goleiro Dinâmico N/S (Raster)
/*
    if (use_defender==1) {
        // --- MODOS DE TORNEIO (Têm Zagueiros) ---
        SPR_ZAG[1] = 18; // Zagueiro Sul (Slot C480)
        SPR_ZAG[4] = 18; // Zagueiro Norte (Slot C480)
      
        
        VIC.spr_color[1] = 1; 
        VIC.spr_color[4] = 7; 
        
        // Posição inicial dos Zagueiros
        VIC.spr_pos[1].x = 160; VIC.spr_pos[1].y = 190;
        VIC.spr_pos[4].x = 160; VIC.spr_pos[4].y = 90;
        
        // Liga a máscara base correta (S0, S1, S3, S4, S5)
        VIC.spr_ena = 0x3B; 
    } 

	else {
        // --- MODO 5 TRADICIONAL (Goleiros, sem Zagueiros) ---
        SPR_ZAG[1] = 0;  // Desativado
        SPR_ZAG[4] = 0;  // Desativado
		VIC.spr_pos[1].x = 0; 
        VIC.spr_pos[1].y = 0; // Força na borda invisível
		VIC.spr_pos[4].x = 0; 
        VIC.spr_pos[4].y = 0; // Força na borda invisível
//        SPR_ZAG[5] = 16; // Goleiro Dinâmico N/S (Raster)
        
        // Liga a máscara base correta (S0, S3, S5)
        VIC.spr_ena = 0x29; 
    }
*/

    // --- BLOCO CORRIGIDO DE CORES ---
//    if (game_mode == 8) {
        setup_match_colors(home_team, away_team);
        VIC.spr_color[1] = p3_color;  
        VIC.spr_color[4] = p1_color;  
//    } 
	/*
	else {
        VIC.spr_color[0] = 7;  // P1
        VIC.spr_color[2] = 10; // P3
        VIC.spr_color[6] = 2;  // P2
        VIC.spr_color[7] = 14; // P4
    }
	*/
//    VIC.spr_color[3] = 1; // Bola branca
//	 VIC.spr_color[3] = 8; // Bola branca
//}

/*
    if (field_type == 1) { 
        // CAMPO MOLHADO (WET)
        VIC.spr_color[1] = 5; // Goleiros Norte/Sul (Raster)
    //    VIC.spr_color[4] = 5; // Goleiro Oeste
    //    VIC.spr_color[5] = 5; // Goleiro Leste
    } else {
        // CAMPO GRAMA (0) OU NEVE (2) -> TODOS ROXOS
        VIC.spr_color[1] = 4; // <--- ESTA LINHA CORRIGE O BUG DOS GOLEIROS N/S
    //    VIC.spr_color[4] = 4; 
    //    VIC.spr_color[5] = 4;
    }
*/
// --- CORREÇÃO DO P2 / GOLEIRO FANTASMA ---
//    if (player_type[2] == 2 || (game_mode == 5 && num_players < 2)) {
//        VIC.spr_ena &= ~0x40;  // Desliga o bit do Slot 6
//        spr_active_p2 = 0;
//        SPR_ZAG[6] = 0;        // Zera o ponteiro do sprite para eliminar qualquer dado visual
//        VIC.spr_pos[6].x = 0; 
//        VIC.spr_pos[6].y = 0; 
//        p2x = 0; 
//        p2y = 0;               // Evita colisões invisíveis
 //   } else {
 //       VIC.spr_ena |= 0x40;   
 //       spr_active_p2 = 1;
 //       SPR_ZAG[6] = 194;      // Restaura o ponteiro original para P2 de linha
 //   }
	
	asm("sei"); 
    *((unsigned int*)0x0314) = (unsigned int)interrupt_raster_handler;
    VIC.imr = 0x01;      
    VIC.ctrl1 &= 0x7F;   
    VIC.rasterline = 30; 
    asm("cli");



for (i = 0; i < 35; i++) {
    screen_backup[i] = SCREEN[12 * 40 + 5 + i];
    color_backup[i] = ((unsigned char*)0xD800)[12 * 40 + 5 + i];
}

// --- ANÚNCIO DA PARTIDA NO CAMPO ---
//if (game_mode == 8 ) {
   // --- ANÚNCIO DA PARTIDA NO CAMPO ---
//if (game_mode == 8 ) {
    len_h = strlen(team_names[home_team]);
    len_a = strlen(team_names[away_team]);
    
    // Total: Nome Home + Espaço (1) + "VS" (2) + Espaço (1) + Nome Away = len_h + len_a + 4
    total_len = len_h + len_a + 4; 
    start_x = (40 - total_len) / 2;

    // --- 1. BACKUP DINÂMICO ---
    for (i = 0; i < total_len; i++) {
        screen_backup[i] = SCREEN[12 * 40 + start_x + i];
        color_backup[i] = ((unsigned char*)0xD800)[12 * 40 + start_x + i];
    }

    // 2. Escreve os textos colados corretamente:
    // - Home termina em start_x + len_h
    // - Espaço em branco na posição start_x + len_h
    // - "VS" nas posições start_x + len_h + 1 e + 2
    // - Espaço em branco na posição start_x + len_h + 3
    // - Away começa em start_x + len_h + 4
    put_text(start_x, 12, (const unsigned char*)team_names[home_team]);
    put_text(start_x + len_h + 1, 12, (const unsigned char*)"VS");
    put_text(start_x + len_h + 4, 12, (const unsigned char*)team_names[away_team]);

    // 3. Aplica as cores na extensão correta
    for (i = 0; i < total_len; i++) {
        unsigned int pos = 12 * 40 + start_x + i;
        
        if (i < len_h) {
            ((unsigned char*)0xD800)[pos] = p1_color;
        } 
        // Pinta o bloco central (espaço + "VS" + espaço = 4 caracteres: do len_h até len_h + 3)
        else if (i >= len_h && i < len_h + 4) { 
            ((unsigned char*)0xD800)[pos] = 1; // Cor do "VS" e seus respiros
        } else {
            ((unsigned char*)0xD800)[pos] = p3_color;
        }
    }
						 
													
													

    // 4. Aguarda leitura
    for (i = 0; i < 120; i++) {
        wait_vblank();
        update_sfx();
    }

    // 5. LIMPEZA DINÂMICA PERFEITA
    // Agora o total_len cobre exatamente até o último caractere alterado!
						  
						 
		 
							  
    for (i = 0; i < total_len; i++) {
        SCREEN[12 * 40 + start_x + i] = screen_backup[i];
        ((unsigned char*)0xD800)[12 * 40 + start_x + i] = color_backup[i];
 
    }
//}
// --- DESENHO ESTÁTICO DA UI (FORA DO LOOP) ---
//if (game_mode == 8) {
    
		put_text(5, 0, (const unsigned char*)"P3:");
    	put_text(28, 0, (const unsigned char*)"P4:");
        put_text(5, 24, (const unsigned char*)"P1:");
		put_text(28, 24, (const unsigned char*)"P2:");
		put_num(2, 24, gols_time_a); // Soma P1+P2
        ((unsigned char*)0xD800)[24 * 40 + 2] = p1_color;
		((unsigned char*)0xD800)[24 * 40 + 3] = p1_color;

        put_num(2, 0, gols_time_b);  // Soma P3+P4
        ((unsigned char*)0xD800)[0 * 40 + 2] = p3_color;
		((unsigned char*)0xD800)[0 * 40 + 3] = p3_color;

 
//} 

// Desenha o "T:" do tempo uma única vez
if (gold_goal_active == 0) put_text(35, 0, (const unsigned char*)"T:");


//update_sprites();

//	if (num_players >= 2 && player_type[2] != 2) {
    VIC.spr_pos[6].x = p2x & 0xFF; VIC.spr_pos[6].y = p2y;
//} else {
    VIC.spr_pos[6].y = 0; // Esconde
//}
 

// BLINDAGEM FINAL DO MULTICOLOR ANTES DE ENTRAR NO LOOP
//    if (game_mode == 8 || game_mode == 7) {
//        *((unsigned char*)0xd01c) = 0xF7; // Garante Slots 1 e 4 em Multicolor
//    }



// 1. Aponta o vetor de scanner de teclado para um "RTS" ($FF1B na ROM do Kernal)
// Isso impede o Kernal de perder ciclos de CPU tentando ler o Joystick 1 como se fosse teclado.
//*((unsigned int*)0x028F) = 0xFF1B; 

// 2. Desativa a repetição de teclas do Kernal por segurança (255 = nenhuma tecla repete)
//*((unsigned char*)0x028A) = 0xFF; 

// 3. Zera o buffer de teclado para não sobrar nenhum "lixo" na memória
//*((unsigned char*)0x00C6) = 0;


// Loop principal

  while(1) {
	
		
        while(VIC.rasterline != 250); // Sincroniza com o frame
// Teste
//		VIC.bordercolor = 1;
		sys_tick++;
		

        // TESTE FORÇADO NO LOOP
 //   *((unsigned char*)0xd01c) = 0xF7;
 
 
//VIC.bordercolor = 0;
		read_joysticks();
//VIC.bordercolor = 1;		

		update_sfx();
//        VIC.bordercolor = 0;
  //      if (game_mode == 7 || game_mode == 8) {
  //       move_defender_south_ia();
  //      move_defender_north_ia();
		
 //  }
//	VIC.bordercolor = 1;
	// Boost Timer sem loop 'for' (Otimização extra)
    if (boost_timer[1] > 0) boost_timer[1]--;
    if (boost_timer[2] > 0) boost_timer[2]--;
    if (boost_timer[3] > 0) boost_timer[3]--;
    if (boost_timer[4] > 0) boost_timer[4]--;

        old_score = score;
		ia_frame_toggle++; // Incrementa a cada frame (0-255)


   

// --- 4. MOVIMENTAÇÃO JOGADORES (MODO 5) ---

 //VIC.bordercolor = 0;
 
#define MOVE_HUMAN(joy, px, py, p_id) \
    if (ball_possession != p_id) { \
        spd = (boost_timer[p_id] > 0) ? 3 : 2; \
        if (joy & 1) py -= spd; \
        if (joy & 2) py += spd; \
        if (joy & 4) px -= spd; \
        if (joy & 8) px += spd; \
    }
	
// 1. P1 é sempre humano (Joy2)
//MOVE_HUMAN(joy2, px, py, 1);

// 2. P2 (Só se move se NÃO for goleiro)
//if (player_type[2] == 2) {
//    p2x = 0; p2y = 0; // Zera as coordenadas de linha para evitar colisões fantasmas
//} else {
   
   if (player_type[2] == 1) {
        MOVE_HUMAN(joy1, p2x, p2y, 2);
    } else if (player_type[2] == 0) {
        // Roda IA do P2 apenas em frames pares
        move_player_ia_modo5(2);
    }

// 2. P1 (Joy2 ou IA)
if (player_type[1] == 1) {
    MOVE_HUMAN(joy2, px, py, 1);
} else if (player_type[1] == 0) {
    // Roda IA do P2 apenas em frames pares
    move_player_ia_modo5(1);
}





// 3. P3 (Em modos de torneio roda sempre. No Modo 5, só se tiver 3+ jogadores)
//if (game_mode == 5 && num_players < 3) {
//    p3x = 0; p3y = 0; // Zera colisão física
//} else {
    
	if (player_type[3] == 1) {
        MOVE_HUMAN(joy3, p3x, p3y, 3);
    } else if (player_type[3] == 0) {
        move_player_ia_modo5(3);
    }
//}

// 4. P4 (Em modos de torneio roda sempre. No Modo 5, só se tiver 4 jogadores)
//if (game_mode == 5 && num_players < 4) {
 //   p4x = 0; p4y = 0; // Zera colisão física
//} else {
    if (player_type[4] == 1) {
        MOVE_HUMAN(joy4, p4x, p4y, 4);
    } else if (player_type[4] == 0) {
        move_player_ia_modo5(4);
    }
//}

move_goleiros_ia();
//VIC.bordercolor = 1;	

//VIC.bordercolor = 0;	
	// 2. AGORA CHAMAMOS A SEPARAÇÃO (Insira aqui!)
    // Isso garante que se o P2 e P4 tentaram ir para o mesmo pixel,
    // eles sejam afastados antes da bola e do desenho na tela.
   if (use_defender==1) {
   move_defender_ia(0); // Zagueiro Norte
        move_defender_ia(1); // Zagueiro Sul
   }

	separate_all_players();
	

    // Mantém todos nos limites (quem não existe fica parado em 0,0 fora da tela)
	
	KEEP_IN_BOUNDS(px, py);
	KEEP_IN_BOUNDS(p2x, p2y);
	KEEP_IN_BOUNDS(p3x, p3y);
	KEEP_IN_BOUNDS(p4x, p4y);

	
	
	// --- CHAMADA DA BOLA PARA O MODO 5 ---
    if (ball_possession != 0) last_touch = ball_possession;

 

 update_players_anim(); 
 // VIC.bordercolor = 0;
	   handle_ball_logic_v2(); 
 //       VIC.bordercolor = 1;
	bounce_ball_at_walls();
    
          
    // P1 e Bola sempre visíveis
    VIC.spr_pos[0].x = (unsigned char)px; VIC.spr_pos[0].y = py;
    VIC.spr_pos[3].x = (unsigned char)bx; VIC.spr_pos[3].y = by;
    VIC.spr_pos[6].x = (unsigned char)p2x; VIC.spr_pos[6].y = p2y;
    VIC.spr_pos[2].x = (unsigned char)p3x; VIC.spr_pos[2].y = p3y;
	VIC.spr_pos[7].x = (unsigned char)p4x; VIC.spr_pos[7].y = p4y;

     if (use_defender == 1) {
    VIC.spr_pos[1].x = (unsigned char)dx_b; VIC.spr_pos[1].y = dy_b;
    VIC.spr_pos[4].x = (unsigned char)dx;   VIC.spr_pos[4].y = dy;
} else {
    VIC.spr_pos[1].x = 0; VIC.spr_pos[1].y = 0;
    VIC.spr_pos[4].x = 0; VIC.spr_pos[4].y = 0;
}
	
    
    // --- ATUALIZAÇÃO DO SPR_HI_X (Otimizado via bitwise) ---
    spr_hix_mask = 0;

    if (px  & 0x100) spr_hix_mask |= 0x01; 
    if (bx  & 0x100) spr_hix_mask |= 0x08; 
    if (p2x & 0x100) spr_hix_mask |= 0x40; 
    if (p3x & 0x100) spr_hix_mask |= 0x04; 
    if (p4x & 0x100) spr_hix_mask |= 0x80; 
/*
    if (game_mode == 5) {
        if (gx[2] & 0x100) spr_hix_mask |= 0x10;
        if (gx[3] & 0x100) spr_hix_mask |= 0x20;
    } else {
        if (dx_b & 0x100)  spr_hix_mask |= 0x02; 
        if (dx   & 0x100)  spr_hix_mask |= 0x10; 
    }
*/
 if (use_defender == 1) {
    if (dx_b & 0x100)  spr_hix_mask |= 0x02; 
        if (dx   & 0x100)  spr_hix_mask |= 0x10; 
  }

    VIC.spr_hi_x = spr_hix_mask;
	

 

 // Atualização de placar
    if (score > old_score) {
  //      if ( game_mode == 8) {
            
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
            
       //     if (is_overtime == 1) {
       //         break; 
       //     }
   //     } 
     
    }
// IA dos Goleiros Modo 5
//    move_goleiros_ia();
    
 //   VIC.bordercolor = 1;
	
//}	

// --- 7. UI, TEMPO E GATILHOS DE DIFICULDADE ---
        // --- 7. UI, TEMPO E GATILHOS DE DIFICULDADE ---
		if (sys_tick % 50 == 0) {
	
	//		if (  game_mode == 8) {
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
		// if (game_mode == 8 ) {
				// --- ADICIONE ISTO AQUI: Atualização dos Placares Somados ---
        gols_time_a = s1_final + s2_final;
        gols_time_b = s3_final + s4_final;

      put_num(2, 24, gols_time_a); // Soma P1+P2
        ((unsigned char*)0xD800)[24 * 40 + 2] = p1_color;
		((unsigned char*)0xD800)[24 * 40 + 3] = p1_color;

        put_num(2, 0, gols_time_b);  // Soma P3+P4
        ((unsigned char*)0xD800)[0 * 40 + 2] = p3_color;
		((unsigned char*)0xD800)[0 * 40 + 3] = p3_color;
		// }
		
 
			// Invertido: SÓ desenha o cronômetro se NÃO for modo GS (Modo 4 ou Modo 5 com Gold Goal)
	//		if (game_mode != 4 && gold_goal_active == 0 && golden_score_triggered == 0) {
			//	put_text(35, 0, (const unsigned char*)"T:"); 
				put_num(37, 0, (timer / 50));
//			}
//			else if (golden_score_triggered == 1) {
 //       put_text(37, 0, (const unsigned char*)"GS"); 
  //  }

		
		}

       if ( (game_mode == 8 && gold_goal_active == 1)) {
		    unsigned char win_condition_met = 0;
			
			if (sys_tick % 50 == 0) {
			gols_time_a = s1_final + s2_final;
			gols_time_b = s3_final + s4_final;
			
			if (gols_time_a >= 5 || gols_time_b >= 5) {
					win_condition_met = 1;
					put_num(8, 24, s1_final);
					put_num(31, 24, s2_final);
					put_num(8, 0, s3_final);
					put_num(31, 0, s4_final);

				}
	/*		
			// Lógica para Modo 5 (Individual)
			if (game_mode == 5) {
				if (s1_final >= 5 || s2_final >= 5 || s3_final >= 5 || s4_final >= 5) {
					win_condition_met = 1;
					put_num(8, 24, s1_final);
					put_num(31, 24, s2_final);
					put_num(8, 0, s3_final);
					put_num(31, 0, s4_final);
				}
			}
			// Lógica para Modo 7 (Time)
			else if (game_mode == 7 || game_mode == 8) {
				if (gols_time_a >= 5 || gols_time_b >= 5) {
					win_condition_met = 1;
					put_num(8, 24, s1_final);
					put_num(31, 24, s2_final);
					put_num(8, 0, s3_final);
					put_num(31, 0, s4_final);

				}
			}
			*/
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
		// ... (resto do loop anterior igual) ...

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
                    gols_time_a = s1_final + s2_final;
                    gols_time_b = s3_final + s4_final;

                    if (current_half == 1) {
                        // -----------------------------------------------------------
                        // TRANSICÃO DO 1º PARA O 2º TEMPO (2nd Half)
                        // -----------------------------------------------------------
                        current_half = 2;
                        timer = 2250; // Reseta o cronômetro para o segundo tempo
                        
                        // 1. Apito de fim de primeiro tempo
                        play_sfx_whistle();
                        sorteia_personalidades();
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
                   //     if (game_mode == 8 ) {
                       //     VIC.spr_pos[1].x = dx_b & 0xFF;  VIC.spr_pos[1].y = dy_b; // Zagueiro Sul
                       //     VIC.spr_pos[4].x = dx & 0xFF;   VIC.spr_pos[4].y = dy;   // Zagueiro Norte
                            // Garante a cor correta dependendo de qual tempo está rodando
                            // Garante a cor correta dependendo de qual tempo está rodando
                            if (current_half == 2) {
                                VIC.spr_color[1] = p3_color; // 2º Tempo: Zagueiro Sul é do Time A
                                VIC.spr_color[4] = p1_color; // 2º Tempo: Zagueiro Norte é do Time B
                            } else {
                                VIC.spr_color[1] = p3_color; // 1º Tempo: Zagueiro Sul é do Time B
                                VIC.spr_color[4] = p1_color; // 1º Tempo: Zagueiro Norte é do Time A
                            }
                   //     } // <--- AQUI FECHA O 'if (game_mode == 8 || ...)'
                      
//					  else if (game_mode == 5) {
    // --- ESCONDE OS ZAGUEIROS NO MODO 5 (Força posição 0,0) ---
//    VIC.spr_pos[1].x = 0;  VIC.spr_pos[1].y = 0; // Slot 1 em 0,0
//    VIC.spr_pos[4].x = 0;  VIC.spr_pos[4].y = 0; // Slot 4 em 0,0
//}
                            if (use_defender==0) {
    // --- ESCONDE OS ZAGUEIROS NO MODO 5 (Força posição 0,0) ---
    VIC.spr_pos[1].x = 0;  VIC.spr_pos[1].y = 0; // Slot 1 em 0,0
    VIC.spr_pos[4].x = 0;  VIC.spr_pos[4].y = 0; // Slot 4 em 0,0
}

                        play_sfx_whistle(); // Apito de início do 2º tempo
                        
                        // Continua o loop principal (não dá break!)
                    } // <--- AQUI CHAVE ADICIONADA CORRETAMENTE PARA FECHAR O 'if (current_half == 1)'
					
                    else {
						/*
                        // Se já estava no segundo tempo (current_half == 2)
                        if ((game_mode == 7 ) && gols_time_a == gols_time_b) {
                            // --- ATIVAÇÃO DO GOLDEN SCORE ---
                            golden_score_triggered = 1;
                       //     is_overtime = 1; 
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
                       // else {
                            // Fim de jogo definitivo
                            break; 
                       // }
                    }
					*/
                     break;					
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
	
	 
// --- PARTE 1: ANÚNCIO DO VENCEDOR ---
//if (game_mode == 8) {
    gols_time_a = s1_final + s2_final;
    gols_time_b = s3_final + s4_final;
    
    // Limpa a linha antes de escrever
    put_text(8, 12, (const unsigned char*)"                         ");

   if (gols_time_a != gols_time_b) {
        register unsigned char len;
        register unsigned char x_pos;
        const char* winner_name;

        if (gols_time_a > gols_time_b) {
            winner_name = (const char*)team_names[home_team];

        } else {
            winner_name = (const char*)team_names[away_team];

        }

        // 1. Descobre o tamanho real do nome (para em \0 ou no limite de 13)
        len = 0;
        while (winner_name[len] != 0 && len < 13) {
            len++;
        }
        // Se os nomes tiverem espaço em branco no final (padding), descarta os espaços
        while (len > 0 && winner_name[len - 1] == ' ') {
            len--;
        }

        // 2. Limpa a área da mensagem anterior (evita lixo visual de nomes mais longos)
        put_text(4, 12, (const unsigned char*)"                                 "); 

        // 3. Centraliza a mensagem completa (Nome + " WINS!") na tela de 40 colunas
        // (40 - (len + 6)) >> 1 faz uma divisão por 2 ultra-rápida via shift de bits
        x_pos = (40 - (len + 6)) >> 1;

        // 4. Exibe o nome e cola o " WINS!" logo na sequência exata
        put_text(x_pos, 12, (const unsigned char*)winner_name);
        put_text(x_pos + len, 12, (const unsigned char*)" WINS!");
    } 
    else {
        put_text(15, 12, (const unsigned char*)" DRAW GAME ");
 
    }
    
//} 

// --- PARTE 2: FINALIZAÇÃO (Pausa, limpeza e áudio) ---
// Roda incondicionalmente no final da partida, independentemente do modo

// Mata o áudio antes de sair
silence_sid(); 
setup_sid();

// Pequeno delay para o jogador ler quem venceu antes de seguir
for(i = 0; i < 100; i++) wait_vblank(); 

// Limpa os textos da tela central
put_text(5, 12, (const unsigned char*)"             ");
put_text(13, 12, (const unsigned char*)"                ");

// Delay estendido antes de trocar de tela
for(i = 0; i < 250; i++) {
    while(VIC.rasterline != 250); 
}

}
	


void main(void) {
    *((unsigned char*)0x01) = 0x36;
    *((unsigned char*)0xD015) = 0x00; // Desliga Sprites
    
    setup_hardware();   // Prepara os assets (tabelas, charset, etc.)
    init_random();
    
    while(1) {
        // 1. Chama o menu principal. 
        // Se o usuário escolher a opção de ir para o BASIC (menu_sel == 1),
        // a função chama_menu_asm("m1") fará o RTS/JMP para o BASIC direto de lá.
        show_menu(); 
        
        // 2. Se o fluxo retornou para cá, significa que o usuário escolheu jogar (game_mode == 8)
        if (game_mode == 8) {
            league_manager_loop(); 
            
            // 3. Ao sair do campeonato (quando escolhe "NO" no PLAY AGAIN),
            // limpamos as flags de estado para a próxima reinicialização limpa.
            game_mode = 0;
            current_fixture_idx = 0;
       //     tournament_phase = 0;
        }
        
        // O loop while(1) garante que o fluxo volte direto para o show_menu()
    }
}

