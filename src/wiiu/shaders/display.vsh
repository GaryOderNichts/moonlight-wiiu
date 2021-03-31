; $MODE = "UniformRegister"

; $NUM_SPI_VS_OUT_ID = 1
; texCoords
; $SPI_VS_OUT_ID[0].SEMANTIC_0 = 0

; C0
; $UNIFORM_VARS[0].name = "u_screenSize"
; $UNIFORM_VARS[0].type = "vec2"
; $UNIFORM_VARS[0].count = 1
; $UNIFORM_VARS[0].block = -1
; $UNIFORM_VARS[0].offset = 0

; R1
; $ATTRIB_VARS[0].name = "in_pos"
; $ATTRIB_VARS[0].type = "vec2"
; $ATTRIB_VARS[0].location = 0
; R2
; $ATTRIB_VARS[1].name = "in_texCoord"
; $ATTRIB_VARS[1].type = "vec2"
; $ATTRIB_VARS[1].location = 1

00 CALL_FS NO_BARRIER
01 ALU: ADDR(32) CNT(5)
    0  x: MUL*2  R1.x,   R1.x, C0.x
       y: MUL*2  R1.y,   R1.y, C0.y
    1  x: ADD    R1.x,   R1.x, -1.0f
       y: ADD    R1.y,  -R1.y, 1.0f
02 EXP_DONE: POS0, R1.xy01
03 EXP_DONE: PARAM0, R2.xy00 NO_BARRIER
END_OF_PROGRAM
