       .section TicksSec,bss
       .global _Tick
       .global _Tick_100, _Tick_200, _Tick_400, _Tick_800
       .global _Tick_1600, _Tick_3200, _Tick_6400, _Tick_12800
       .global _Tick_25600, _Tick_51200, _Tick_102400, _Tick_204800
       .global _Tick_409600, _Tick_819200, _Tick_1638400, _Tick_3276800
 _Tick: .space 2    
 _Tick_100: .space 1
 _Tick_200: .space 1
 _Tick_400: .space 1
 _Tick_800: .space 1
 _Tick_1600: .space 1
 _Tick_3200: .space 1
 _Tick_6400: .space 1
 _Tick_12800: .space 1
 _Tick_25600: .space 1
 _Tick_51200: .space 1
 _Tick_102400: .space 1
 _Tick_204800: .space 1
 _Tick_409600: .space 1
 _Tick_819200: .space 1
 _Tick_1638400: .space 1
 _Tick_3276800: .space 1
       .text
       .global _Ticks
 _Ticks:     
	inc _Tick
	btss _Tick, #0
	btg _Tick_100, #0
	btss _Tick, #1
	btg _Tick_200, #0
	btss _Tick, #2
	btg _Tick_400, #0
	btss _Tick, #3
	btg _Tick_800, #0
	return
 .end

