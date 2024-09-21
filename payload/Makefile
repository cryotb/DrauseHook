# Set the default target
all: build

# Custom LLVM compiler and its flags
LLVM_CXX := $(LLVM_PATH)clang++
LLVM_FLAGS := -m64 -nostdlib -fno-exceptions \
	-std=c++17 -Wno-macro-redefined -O1 -fno-builtin -Wno-microsoft-cast -ffreestanding \
	-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -fuse-ld=lld-link \
	-Wl,-entry:_kb -Wl,-base:0x1400000 -Wl,-force:multiple \
	-mllvm -aesSeed=${GLCI_AES_SEED} -mllvm -csobf -fno-ident -I"C:/src/obfuscator-drause/Headers/" \
	-fdata-sections -ffunction-sections -Wno-unused-command-line-argument \
	-D_WATERMARKED_USER=${GLCI_WATERMARKED_USER} -D_COMPILED_AES_SEED_VALUE=${GLCI_AES_SEED}

LLVM_ASMFLAGS := -m64 -nostdlib \
	-O0 -Wno-microsoft-cast -fuse-ld=lld-link

LLVM_LD_FLAGS := -fno-unwind-tables \
	-shared -Wl,-entry:_kb -Wl,-base:0x1400000 -Wl,-force:multiple

# Source and object files
SRCDIR := r5/src
SRCFEATURESDIR := r5/src/features
OBJDIR := obj

SRC := $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/*/*.cpp) $(wildcard $(SRCDIR)/*.asm)
OBJ := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(filter %.cpp,$(SRC))) \
       $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%.o, $(filter %.asm,$(SRC)))

EXECUTABLE := r5.dll

# Build the project using the LLVM compiler
build: $(OBJ)
	@echo "Linking..."
	$(LLVM_CXX) $(LLVM_FLAGS) $(OBJ) -o $(EXECUTABLE) $(LLVM_LD_FLAGS)

# Compile individual source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(LLVM_CXX) $(LLVM_FLAGS) -c $< -o $@
# Compile assembly
$(OBJDIR)/%.o: $(SRCDIR)/%.asm | $(OBJDIR)
	$(LLVM_CXX) $(LLVM_ASMFLAGS) -c $< -o $@ 

$(OBJDIR):
	@if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"
	@if not exist "$(OBJDIR)\features" mkdir "$(OBJDIR)\features"

.PHONY: clean
clean:
	@echo "Cleaning up..."
	rm -f $(OBJ) $(EXECUTABLE)

# CAN SAFELY IGNORE ARG UNUSED WARNINGS AS LONG AS
# THEY HAPPEN DURING LINKING PROCESS LIKE THIS:
# clang++ -m64 -nostdlib -fno-exceptions -std=c++17 -Wno-macro-redefined -O1 -fno-builtin -Wno-microsoft-cast -ffreestanding -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -fuse-ld=lld-link -mllvm -aesSeed=2717D2F4FD2CA7D1A7F5251A5EE4DB80 -mllvm -csobf -fno-ident -I"C:/src/obfuscator11/Headers/"  obj/core.o  obj/defs.o  obj/entcache.o  obj/entity.o  obj/hooks.o  obj/input.o  obj/logger.o  obj/main.o  obj/math.o  obj/menu.o  obj/netvars.o  obj/r5ac.o  obj/renderer.o  obj/respawn.o  obj/spectator_list.o  obj/stl.o  obj/tools.o  obj/vft.o  obj/features/esp.o  obj/features/movement.o  obj/hook-tramps.o  obj/stackcheck.o -o r5.dll -shared -Wl,-entry:_kb -Wl,-base:0x1400000 -Wl,-force:multiple -fno-unwind-tables -fdata-sections -ffunction-sections
#clang++: warning: argument unused during compilation: '-mllvm -aesSeed=2717D2F4FD2CA7D1A7F5251A5EE4DB80' [-Wunused-command-line-argument]
#clang++: warning: argument unused during compilation: '-mllvm -csobf' [-Wunused-command-line-argument]
#clang++: warning: argument unused during compilation: '-fno-ident' [-Wunused-command-line-argument]
#lld-link: warning: duplicate symbol: Xstr_decrypt
# THEY ARE ACTUALLY USED DURING COMPILATION, BUT NOT DURING LINKAGE. I CBA TO MAINTAIN 2 ARG LISTS SO KEEP IT LIKE THIS.

