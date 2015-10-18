;**********************************************************************c
;* Author: Marcin Dec
;* Date: 11.08.2015
;* FileName:        Pid_s.s
;* Dependencies:    
;* Processor:       dsPIC30F6013A
;* Compiler:        MPLAB® XC16 C Compiler
;*
;************************************************************************/
;struct Pid
;{
;   // external
;	0 int kp;
;	2 int ki;
;	4 int kd;
;	6 int offset;
;	8 int min;
;	10 int max;
;   12 int inverse;
;   // internal
;	14 long long int prevI;
;   22 int prevIn;
;}

;int PidExec(int in, struct Pid* pid)
;{
;	int out;
;
;	i += in * (pid->ki);
;	p = in * (pid->kp);
;	d = (in - prevIn) * (pid->kd);
;	prevIn = in;
;	out = p + i + d;
;	out >>= 16;
;	return out + pid->offset;
;}
 .text
 .global _PidExec
_PidExec:
    ;;;;;;;;;;; siple or inverse control ;;;;;;;;;;;;;;;;;;;;;;;;
    ;mov [w2+12], w2         ; fetch previous i value to w2
    ;lac w2, #7, a           ; load previous i value to accumulator
    ;mov w2, w2
    mov [w2+12], w3          ; fetch inverse value to w3
    cp0 w3                   ; compare inverse value with 0
    bra gt, INVERSE          ; if inverse value >0 skip simple error computing
    sub w1, w0, w0           ; compute direct error (setp - in)
    bra DIRECT
INVERSE:
    sub w0, w1, w0           ; compute inverse error ( in - setp)
DIRECT:
    ;;;;;;;;;;; Integral ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    add w2, #14, w1          ; to w2 load structure pointer inreased with 12 (prevI)
    mov #0x22, w3            ; load ACCA's address to w3
    mov [w1++], [w3++]       ; load ACCA with previous i value
    mov [w1++], [w3++]       ;
    mov.b [w1], [w3]         ;
	mov w0, w4              ; load current in to w4
	mov [w2+2], w5          ; fetch ki to w5
	mac w4*w5, a            ; multiply current in with ki and accumulate
    ;sac a, #-7, w3          ; preserve current i value
    clr b                    ; preserve current i value in ACCB
    add b                    ;
    ;;;;;;;;;;; Proportional ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [w2], w5            ; fetch kp to w5
    mac w4*w5, a            ; multiply current in with kp and accumulate
    ;;;;;;;;;;; Derivative ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [w2+22], w1         ; fetch prevIn value to w2
    mov [w2+4], w5          ; fetch kd to w5
    sub w4, w1, w4          ; subtract prevIn from current in and result load to w4
    mac w4*w5, a            ; multiply difference (in-prevIn) with kd and accumulate
    ;;;;;;;;;;; Output limit ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    sac a, #-7, w0          ; load PID result to w0
    mov [w2+6], w3          ; fetch offset value
    add w0, w3, w0          ; add offset to output
    mov [w2+10], w4         ; fetch max value
    sub w4, w0, w1          ; subtract max - out to make comparison
    bra gt, OUT_LE_MAX      ; if out <= max jump to OUT_LT_MAX
    mov w4, w0              ; alter w0 with max
    bra OUT_EQ_MAX
OUT_LE_MAX:
    mov [w2+8], w4          ; fetch min value
    sub w4, w0, w1          ; subtract max - out to make comparison
    bra lt, OUT_IN_RANGE    ; if out >= min jump to OUT_IN_RANGE
    mov w4, w0              ; alter w0 with min
    bra OUT_EQ_MIN
OUT_IN_RANGE:
    ;mov w3, [w2+12]         ; store current i value
    ;mov w2, w2
    add w2, #14, w1         ; to w2 load structure pointer inreased with 12 (prevI)
    mov #0x28, w3           ; load ACCB's address to w3
    mov [w3++], [w1++]      ; store current i value (ACCB)
    mov [w3++], [w1++]      ;
    mov.b [w3++], [w1++]    ;
OUT_EQ_MAX:
OUT_EQ_MIN:
;mov w1, w0		    ; to bypas Pid algorithm uncoment this and comment all the rest
  return
 .end
