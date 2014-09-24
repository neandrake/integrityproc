@echo off
@mkdir build_java>NUL 2>&1

@echo on
javac -d build_java src_java\net\neandrake\windows\IntegrityProcessLauncher.java
javah -jni -classpath build_java\ -d src_cpp\ net.neandrake.windows.IntegrityProcessLauncher
@echo off
