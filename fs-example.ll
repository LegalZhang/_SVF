; ModuleID = 'fs-example.opt'
source_filename = "fs-example.cpp"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: mustprogress noinline norecurse optnone ssp uwtable(sync)
define noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %x = alloca ptr, align 8
  %p = alloca ptr, align 8
  %q = alloca ptr, align 8
  %y = alloca ptr, align 8
  %z = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  store i32 10, ptr %a, align 4
  store i32 20, ptr %b, align 4
  %call = call ptr @malloc(i64 noundef 8) #2
  store ptr %call, ptr %x, align 8
  store ptr %a, ptr %p, align 8
  %0 = load ptr, ptr %p, align 8
  %1 = load ptr, ptr %x, align 8
  store ptr %0, ptr %1, align 8
  %2 = load ptr, ptr %x, align 8
  %3 = load ptr, ptr %2, align 8
  store ptr %3, ptr %y, align 8
  store i32 20, ptr %a, align 4
  store i32 30, ptr %b, align 4
  store ptr %a, ptr %q, align 8
  store ptr %b, ptr %q, align 8
  %4 = load ptr, ptr %q, align 8
  %5 = load ptr, ptr %x, align 8
  store ptr %4, ptr %5, align 8
  %6 = load ptr, ptr %x, align 8
  %7 = load ptr, ptr %6, align 8
  store ptr %7, ptr %z, align 8
  ret i32 0
}

; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #1

attributes #0 = { mustprogress noinline norecurse optnone ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }
attributes #1 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a,+zcm,+zcz" }
attributes #2 = { allocsize(0) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 1}
!4 = !{!"Homebrew clang version 19.1.3"}
