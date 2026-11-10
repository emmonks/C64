.export _chama_menu_asm

.code

_chama_menu_asm:
    sei             ; Para tudo

    ; 1. APAGA A TELA IMEDIATAMENTE (Mata listras/goleiro)
    lda #$00
    sta $d011       
    sta $d015       

    ; 2. SEQUESTRA O VETOR DE RESET ($02a2 ou $fffc)
    ; Vamos colocar nosso código de cores na Página 2, que sobrevive ao reset inicial
    ldx #$00
@copy_reset:
    lda custom_reset,x
    sta $02a2,x     ; Copia para uma área segura da RAM
    inx
    cpx #$30
    bne @copy_reset

    ; 3. AJUSTA O PONTEIRO DE BOOT (Magic Link)
    ; Isso força o C64 a desviar para $02a2 assim que "acordar"
    lda #$a5
    sta $0283       ; Magic Flag
    lda #$a2
    sta $0284       ; LSB de $02a2
    lda #$02
    sta $0285       ; MSB de $02a2

    ; 4. EXECUTA O RESET
    jmp ($fffc)

; --- ESTE CÓDIGO RODA "DENTRO" DO BOOT ---
custom_reset:
    ; A. Força cores preto e branco
    lda #$00
    sta $d020       ; Borda
    sta $d021       ; Fundo
    lda #$01
    sta $0286       ; Letra Branca
    
    ; B. Injeta o comando no buffer
    ldx #$00
@loop:
    lda cmd_text,x
    sta $0277,x
    inx
    cpx #$13
    bne @loop
    stx $c6         ; Ativa o buffer

    ; C. Devolve para o BASIC já limpo
    jmp $e394       ; Salto oficial para o "READY" do BASIC

cmd_text:
    .byte $4c,$4f,$41,$44,$22,$4d,$31,$22,$2c,$38,$0d
    .byte $52,$55,$4e,$0d