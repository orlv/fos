%.o: %.S
	$(ECHO) Compiling $<
	$(AS) $(ASFLAGS) $< -o $@ -c

%.o: %.c
	$(ECHO) Compiling $<
	$(CC) $(CFLAGS) $< -o $@ -c

%.o: %.cpp
	$(ECHO) Compiling $<
	$(CXX) $(CXXFLAGS) $< -o $@ -c
