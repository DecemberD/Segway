/**********************************************************************c
* Author: Marcin Dec
* Date: 21.02.2015
* FileName:        sin.s
* Dependencies:    
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/


               	.section .data
                .global _variables2

yyyy:
.hword 0x0000
.hword 0x0000
.hword 0x0000
.hword 0x0000


;..................................................................................
            

                .section *, code  ;<-Syntax supported in MPLAB C30
                                                ;v1.30 and later


	.global	_SIN	; export
_SIN:

;............................................................................

	; Save working registers.
	
	push	w1
	push	w2
	;............................................................................
	;bset CORCON, #0
	mov		#yyyy, w2	; przenies adres yyyy do w2
	add		w1,w2,w2	; zwieksz adres o index przeslany jako argument
	mov		[w2],w1		; przenies @(yyyy+index) do w1
	add 	w0,w1,w0	; dodaj okres czesciowy przes³any w argumencie do w1 i umiesc w w0 (w wyniku otrzymujemy faze chwilowa)
	mov		w0, [w2]	; przenies faze chwilowa do @(yyyy+index)
	mov		w0, w6
	mov 	w0, w4		; uzyj fazy chwilowej (próbka sinusa) do wyliczenia kolejnej próbki. w4 jako x
	mov 	w0, w5		; w5 jako x (x zawiera sie od 0x8000 do 0x7fff)
	btsc	w5, #15		; sprawdz czy x ujemny, jezeli nie to przeskocz nastepna instrukcje
	neg		w5, w5		; skoro ujemny to zmien znak abs(x) -> w5
;	clr		a
	mpy.n 	w4*w5,a		; -(x*abs(x)) -> a
	;sftac	a,#-14
	mov		#0x7fff,w5	; 0x7fff -> w5
	mac		w4*w5,a		; -(x*abs(x)) + 0x7fff*x -> a
	;sftac	a,#16
	sac		a, w0		; a -> w0
;;;;;;;;;;;;;;;;;;;;;;;double prcision;;;;;;;;;;;;;;
	mov		w0,w4
	mov		w4,w5
	btsc	w5, #15
	neg		w5, w5
	mpy		w4*w5,a
	sac		a, w4
	mov		#0x1CCD,w5	;0x1CCC
	mpy		w4*w5,a
	mov		w0,w4
	mov		#0x6333,w5
	mac		w4*w5,a
	sac		a, w0					




	;............................................................................

	; Restore working registers.

	pop w2
	pop w1
	
;............................................................................

	return	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;............................................................................

	.end
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
OEF
