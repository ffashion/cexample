# att
[extended asm](http://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm)
[att asm](https://flint.cs.yale.edu/cs421/papers/x86-asm/asm.html )
1. All register names of the IA-32 architecture must be prefixed by a '%' sign
2. All literal values must be prefixed by a '$' sign. 
3. gcc inline asm 中使用%number 按顺序引用OutputOperands，InputOperands中的变量
