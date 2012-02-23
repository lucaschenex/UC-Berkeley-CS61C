SOURCES := sim.c processor.c load_program.c disassemble.c memory.c
HEADERS := elf.h load_program.h processor.h disassemble.h memory.h

ASM_TESTS := simple insts rt3 rt13 rt25
C_TESTS := ackermann rt30

all: mips-sim $(addprefix mipscode/, $(C_TESTS) $(ASM_TESTS))

.PHONY: disasmtest runtest

mipscode/%.o: mipscode/%.s
	mips-gcc -c -o $@ $<

mipscode/%.o: mipscode/%.c
	mips-gcc -O2 -o $@ -c $<

$(addprefix mipscode/, $(C_TESTS)): %: %.c mipscode/mips.ld mipscode/crt.o
	mips-gcc -march=mips1 -O2 -o $(patsubst %.c, %.o, $<) -c $<
	mips-ld -o $@ mipscode/crt.o $(patsubst %.c, %.o, $<) -T mipscode/mips.ld

$(addprefix mipscode/, $(ASM_TESTS)): %: %.s mipscode/mips.ld
	mips-gcc -o $(patsubst %.s, %.o, $<) -c $<
	mips-ld -o $@ $(patsubst %.s, %.o, $<) -T mipscode/mips.ld

mips-sim: $(SOURCES) $(HEADERS)
	gcc -g -Wall -Werror -Wfatal-errors -O2 -o $@ $(SOURCES)

disasmtest: mips-sim mipscode/insts
	./mips-sim -d mipscode/insts > insts.dump
	@diff insts.dump mipscode/insts.dump && echo "DISASSEMBLY TEST PASSED!" || echo "DISASSEMBLY TEST FAILED!"

runtest: mips-sim mipscode/insts
	./mips-sim -r mipscode/insts > insts.trace
	@diff insts.trace mipscode/insts.trace && echo "RUN TEST PASSED!" || echo "RUN TEST FAILED!"

check-part1:
	./check-submission.sh

check-part2:
	./check-submission.sh part2

clean:
	rm -f mips-sim $(addprefix mipscode/, $(C_TESTS) $(ASM_TESTS)) $(addsuffix .o, $(addprefix mipscode/, $(C_TESTS) $(ASM_TESTS))) *.dump *.trace
