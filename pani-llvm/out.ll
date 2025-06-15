; ModuleID = 'AdaLLVM'
source_filename = "AdaLLVM"

@0 = private unnamed_addr constant [10 x i8] c"Value: %d\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @0, i32 0, i32 0), i32 42)
  ret i32 0
}
