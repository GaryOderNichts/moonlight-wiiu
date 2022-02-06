; $MODE = "UniformRegister"

; $NUM_SPI_VS_OUT_ID = 1
; texCoords
; $SPI_VS_OUT_ID[0].SEMANTIC_0 = 0

; R1
; $ATTRIB_VARS[0].name = "in_pos"
; $ATTRIB_VARS[0].type = "vec2"
; $ATTRIB_VARS[0].location = 0
; R2
; $ATTRIB_VARS[1].name = "in_texCoord"
; $ATTRIB_VARS[1].type = "vec2"
; $ATTRIB_VARS[1].location = 1

00 CALL_FS NO_BARRIER
01 EXP_DONE: POS0, R1.xy01
02 EXP_DONE: PARAM0, R2.xy00 NO_BARRIER
END_OF_PROGRAM
