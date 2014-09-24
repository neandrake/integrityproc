@echo off
@del integrityproc.jar>NUL 2>&1
@mkdir build_java>NUL 2>&1

@echo on
javac -d build_java src_java\net\neandrake\windows\IntegrityProcessLauncher.java
jar cvf integrityproc.jar build_java
@echo off
