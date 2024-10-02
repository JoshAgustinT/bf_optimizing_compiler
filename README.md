# Bf Optimizing Compiler Report
by Joshua Tlatelpa-Agustin 
10-01-24
######  Github link of project:
https://github.com/JoshAgustinT/bf_optimizing_compiler
*bfcheck*: 0 failures, 1753 successes
---


#####   Definitions
 Simple loop 
1. Contains no loops or I/O
2. Has 0 net pointer movement
3. Changes p[0] by a total of +1 or -1
4. Contains no optimized simple loops
5. Contains no optimized seek loops

Seek loop
1. Loops that contain only < and > commands
2. Net change inside the loop is a power of 2 and their negative counterpart
3. Ie only net changes of {-32,-16, -8, -4, -2, 2, 4, 8, 16, 32}

---
##### Assembly Examples at bottom of page.

---
#####  Improvements Possible

Simple loops
> My current definition of a simple loop is that it doesn't contain any simplified loops within. However, simplified simple loops should be able to work. Dr. Regerh mentioned saving the contents of the cell at the beginning of a simple loop at a specified offset and keeping track of it. This way we could essentially merge simple loops that contain simplified simple loops inside with some algebra. Currently, my implementation does something like simple_expr = loop_size*(0:1,1:-2), where the first element is the offset from the original loop cell and the second element is what they do to the cell in each iteration. If we save the loop_size and find the offset between the two loop control cells it should allow us to merge the simple expr's and reach a fixed point. The implementation should be relatively straightforward since we're guaranteed to have 0 net pointer movement and only +-1 to our loop cell.

Seek loops
> My optimization currently supports seek loops with offsets of {-32, -16, -8, -4, -2, 2, 4, 8, 16, 32}. Since my machine only handles vectors up to 32 bytes, and machines with larger capacities are relatively rare, using vectors for larger offsets isn’t practical. With larger sizes, we would end up checking only one byte in the vector, which means we could simply use regular registers instead. Previously, I had a non-vector optimization that calculated the offset of a seek loop and added it directly to our tape address. This approach could replace seek loops outside our defined set. It would also allow us to extend our definition of a seek loop to include any loop that uses only the > and < operators. Implementing this change would be straightforward. It would function as a default method, and we'd use vector instructions only for the specific offsets of {-32, -16, -8, -4, -2, 2, 4, 8, 16, 32}.

Extra
> There is also still plenty of redundancy that can be removed, such as not reloading the tape location on every instruction. This alone nearly tripled the speed on some tests while I was testing it, but it was unstable. Need to switch around some major things for this to work, but the speed is in exchange for simplicity in this case.
---
#####  Benchmarks on Intel Ultra 9 185H, (14th gen mobile chip)
Running all test programs in *benches* folder.

* with no loop optimizations
```
real    0m16.385s
user    0m15.677s
sys     0m0.305s
```
* optimizing only simple loops
```
real    0m9.698s
user    0m9.104s
sys     0m0.224s
```
* optimizing only memory scans
```
real    0m15.398s
user    0m14.778s
sys     0m0.275s
```
* optimizing both simple loops and memory scans
```
real    0m9.560s
user    0m9.045s
sys     0m0.140s
```


--
My favorite test: hanoi.b

* No optimizations
```
real    0m3.178s
user    0m3.158s
sys     0m0.020s
```
* All optimizations
```
real    0m0.733s
user    0m0.725s
sys     0m0.000s
```

---

#####  To output x86 assembly:
* Build Compiler
    > g++ bf.cpp
* First argument will be treated as source bf program and output x86 assembly
    > ./a.out file.b
* Link our new 'bf.s' assembly file
    > gcc bf.s
* This is our executable file, simply run!
    > ./a.out 
---
#####  Using optimizations
* Simple Loop Elimination
    >  add a "-O" AFTER first argument to eliminate simple loops.
* Seek loop vector optimization
    >  add a "-v" AFTER first argument to optimize seek loops with vector instructions.
      alternatively, add "-O1" AFTER first argument to do both optimizations.
* Not using any flags will use default naive implementation.

---
#####  Bf programs
* Various bf programs found in benches folder
---
#####   Makefile commands
* make test - runs all programs in benches folder except input.b
* also a few other make targets not meant for use, simply debugging artifacts
---
#####  Caveats
* tape limit is 1,048,576 bytes and we start at the middle
---
##### Bf language spec:
* https://github.com/sunjay/brainfuck/blob/master/brainfuck.md
----
#####   Assembly Examples
    Simple loops
Naive simple loop executing 10 times.
```
; Add up our cells so their contents can help produce 
; Hello World!
start_loop_0:
movq    -8(%rbp), %rax
movb    (%rax), %cl
cmpb    $0, %cl
je      end_loop_0
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
movb    (%rax), %cl
addb    $1, %cl
movb    %cl, (%rax)
movq    -8(%rbp), %rax
subq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
subq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
subq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
subq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
movb    (%rax), %cl
subb    $1, %cl
movb    %cl, (%rax)
end_loop_0:
```
Optimized, identifies the cells changed during loop and sets them directly instead of looping 10 times. 
```
movq    -8(%rbp), %rax
movq    (%rax), %rcx
movq   %rax, %r12
addq   $1, %r12
movq    $7, %r15
imul   %rcx, %r15
addb    %r15b , (%r12)
movq   %rax, %r12
addq   $2, %r12
movq    $10, %r15
imul   %rcx, %r15
addb    %r15b , (%r12)
movq   %rax, %r12
addq   $3, %r12
movq    $3, %r15
imul   %rcx, %r15
addb    %r15b , (%r12)
movq   %rax, %r12
addq   $4, %r12
movq    $1, %r15
imul   %rcx, %r15
addb    %r15b , (%r12)
movb    $0, (%rax)
```
---
> Seek Loops

Naive Power of 4 Seek. ie [>>>>]
```
start_loop_0:
movq    -8(%rbp), %rax ;get current cell address
movb    (%rax), %cl
cmpb    $0, %cl ;compare 0
je      end_loop_0 ;if equal end loop
movq    -8(%rbp), %rax ;load cell address
addq    $1, %rax ;go to right cell
;repeat x4
movq    %rax, -8(%rbp) 
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
movq    -8(%rbp), %rax
addq    $1, %rax
movq    %rax, -8(%rbp)
end_loop_0:
movq    -8(%rbp), %rax
movb    (%rax), %cl
cmpb    $0, %cl ;now that we're 4 cells to the right, check if cell is 0
jne      start_loop_0 ;if not start over
```
Optimized, essentially we do what the previous code did, but 8 of the naive loops at a time.
```
; note 32 bytes = 256 bits
movq    -8(%rbp), %r8 ;get current cell address
movb    (%r8), %cl
cmpb    $0, %cl ;check if cell is 0, if so end loop
je      end_seek_loop_0
addq $1, %r8
subq    $32, %r8 ;go back 32 bytes
start_seek_loop_0: 
addq    $32, %r8 ;go 32 bytes ahead, on first iteration we'll be at our original cell
vmovdqa .four_offset_mask(%rip), %ymm0 ;256 bit mask to discard non offset values
vpor    (%r8), %ymm0, %ymm0 ;or 32 bytes from our tape with ymm0
vpxor   %xmm1, %xmm1, %xmm1 ;zero out xmm1, 256 bit zero vector
vpcmpeqb        %ymm1, %ymm0, %ymm0 ;find matches and set them as 1
vpmovmskb       %ymm0, %eax ;puts bitmask of result in eax
testl   %eax, %eax ;check if empty
je      start_seek_loop_0 ;if empty, jump and check the next 32 bytes
bsfl    %eax, %eax ; Bit Scan Forward to find first set bit
addq %rax, %r8 ;add result to our cell address
movq    %r8, -8(%rbp) ;save
end_seek_loop_0:
```

---

