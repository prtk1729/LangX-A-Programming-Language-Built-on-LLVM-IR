; ModuleID = 'AdaLLVM'
source_filename = "AdaLLVM"

@VERSION = global i32 42, align 4
@0 = private unnamed_addr constant [16 x i8] c"Version: %d    \00", align 1
@1 = private unnamed_addr constant [16 x i8] c"Version: %d    \00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 12, i32* %x, align 4
  %x1 = alloca i32, align 4
  store i32 42, i32* %x1, align 4
  %x2 = load i32, i32* %x1, align 4
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i32 0, i32 0), i32 %x2)
  store i32 100, i32* %x, align 4
  %x3 = load i32, i32* %x, align 4
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @1, i32 0, i32 0), i32 %x3)
  ret i32 0
}
