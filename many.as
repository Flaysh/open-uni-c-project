; file ps.as
    .entr LIST
    .extern W
MAIN: add r3, LIST
LOOP: prn 48
lea W
inc r6
mov r3, K
su r1
bne 0
cmp K, #-6
bne %END
dec W
    .entry MAIN
jmp LOOP
add L3, 3
END: stop
STR: .string "abcd"
LIST data 6, -9
    .data -100
K .data 31
    .extern L3
