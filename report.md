### What Was Implemented
* The first thing I did was exchange the representation of our instruction list when optimizing is enabled. Instead of a list<char>, it is now a list<string> to be able to easily support our new instructions. This involved some adjustment of the syntax of my previous profiler/compiler functionality.

* Next I implemented an algorithm to transform these simple/seek loops into a syntactical representation that I'll call an expr_simple and expr_seek instructions.

* Once that was done I replaced the previous start of the loop with our new expr instruction, and replace the rest of the loop with '#', simply a non-instruction that will be ignored during assembly generation.

* I then re-run the code that identifies simple/seek loops and redo this process until there aren't any left. 

* Finally all that was left to do was add cases for the expr instructions in my assembly generation loop and implement them.

### Adjustment of Definition of Simple Loops
Since we now have a new instruction expr_simple, which doesn't affect the tape position it is safe to include these as acceptable inside a simple loop. However, these innermost expr_simple would require adjustment of indices of the expr_simple. ie  p[1] would need to be changed to p[0] * p[(1+offset)], where the offset would be the new tape postion offset since being inside the outermost loop. To further clarify p[0] is meant to be the tape position at the beginning of a loop, which of course would need to be adjusted since we'd have a new p[0]. 

### Removing Simple Loops Assembly Examples

### Optimizing Seeking Loops with Vector Instructions Assembly Examples

### Cases Not Covered