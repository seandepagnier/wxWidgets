#
# Rule for Qt's moc precompiler:
#
$(srcdir)/src/qt/%.moc.cpp: $(srcdir)/include/wx/qt/%.h
	/home/reingart/qt/5.2.1/gcc_64/bin/moc $< -o $@
    
